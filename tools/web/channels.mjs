// channels.mjs — a Dado program's channels, read and written from JavaScript.
//
// The reference the DadoScript glue starts from: the channel helper family the
// compiler emits into every program that holds a channel (its runtime's
// `channel.h`), transliterated helper by helper, over typed-array views of
// the module's memory at the offsets the program's channel directory records.
// **Nothing here calls into wasm after start-up**: the directory's address
// is asked once, and every send, walk, look and length after that is loads
// and stores on memory the module shares with its host. That is the property
// the layout exists for.
//
//     import { ChannelDirectory } from "./channels.mjs";
//     const dir = ChannelDirectory.read(instance.exports.memory,
//                                      instance.exports.dado_rt__channel_directory());
//     const q = dir.channel("main.to_js");
//     if (dir.channelAny()) for (const i of dir.look()) ...    // the watcher
//     dir.drain(q, (m) => ...);                               // #drain(q)
//     dir.send(dir.channel("main.from_js"), { id: 1, v: 2n }); // try #send(...)
//
// **Which accesses, and why.** Every access to a word another party writes —
// `tail`, `head`, `owner`, a slot's `seq`, a `latest`'s `word` and
// `front_state`, the flag word and the bitmap — is an `Atomics` operation.
// On a shared memory (threaded `#WEB`) they are what makes this correct:
// `Atomics.*` are sequentially consistent, stronger than every order the
// helpers ask for, so each helper's orders hold. On an unshared memory
// (single-threaded `#WEB`, the default) a plain typed-array access would be
// correct too — nothing runs concurrently, and the C side compiles every
// atomic to a plain access — but `Atomics` on an unshared buffer is legal and
// means the same thing, so this file has **one** path, the one that is right
// on both, and the unshared case is its degenerate one. The payload is plain
// data ordered by `seq`, as in C, and is read and written through a
// `DataView` (little-endian, unaligned-safe).
//
// **64-bit counters** are `BigUint64Array` elements and `BigInt` values: a
// claim counter never wraps, and JavaScript's `Number` would lose it past
// 2^53. The helpers' unsigned arithmetic is restated where it matters: C's
// `p - h >= cap` is true when `h > p` (the subtraction wraps), and here that
// is written out as `h > p || p - h >= cap`.
//
// **What is derived and what is hard-coded.** Every number *about a channel* —
// its instance's address, `sizeof`, the offsets of `tail`, `head`, `owner`
// and the slots, the slot stride, the capacity, each payload field's name,
// offset, size and kind — is read from the directory, where C wrote it with
// `offsetof`/`sizeof`. What the directory does **not** describe is its own
// three structs, whose pointer-width fields no `offsetof` reaches: this file
// hard-codes their wasm32 (ILP32) layout — `DIRECTORY_LAYOUT` below — and
// checks it against the table (the version, each entry's index, the names).
// It also hard-codes that a queue slot's message sits **8 bytes** into the
// slot, after its `seq`: true of every message whose alignment is at most 8,
// which is every message of scalars; checked against each field's extent.
//
// **A `ref` field** reads as the address it holds and cannot be written: a
// `ref` moves ownership of memory the program's allocator holds, and this side
// can neither give the program one nor free one it takes. Draining a channel
// whose message holds a `ref` from here leaks each one (the discard included);
// such a channel is the program's to consume.

/** The directory layout version this reader understands. */
export const DIRECTORY_VERSION = 1;

/** The directory's own structs on wasm32, where a pointer is 4 bytes and a
 * `uint64_t` is 8-aligned. Not in the directory: these are its shape. */
export const DIRECTORY_LAYOUT = Object.freeze({
    // struct dado_rt__channel_directory_t
    header: { version: 0, count: 4, flags: 8, bits: 12, names: 16, entries: 20 },
    // struct dado_rt__channel_entry, 96 bytes
    entry: {
        size: 96,
        index: 0,
        kind: 4,
        name: 8,
        fieldCount: 12,
        capacity: 16,
        instance: 24,
        fieldOffset: 32,
        sizeof: 40,
        tail: 48,
        head: 56,
        owner: 64,
        slots: 72,
        stride: 80,
        fields: 88,
    },
    // struct dado_rt__channel_field, 24 bytes
    field: { size: 24, name: 0, kind: 4, offset: 8, bytes: 16 },
    // A pointer, and a `const char *` in the name table.
    pointer: 4,
    // A queue slot is { _Atomic uint64_t seq; msg m; }: `m` after `seq`.
    message: 8,
});

/** A field's scalar kind, as the directory records it. */
export const FIELD_KIND = Object.freeze({
    0: "composite",
    1: "bool",
    2: "i8",
    3: "i16",
    4: "i32",
    5: "i64",
    6: "u8",
    7: "u16",
    8: "u32",
    9: "u64",
    10: "f32",
    11: "f64",
    12: "char8",
    13: "char16",
    14: "char32",
    15: "rawptr",
    16: "ref",
});

// How each kind of field is read from a `DataView` at `p` (little-endian);
// a composite field is its bytes, copied.
const READERS = {
    bool: (dv, p) => dv.getUint8(p) !== 0,
    i8: (dv, p) => dv.getInt8(p),
    i16: (dv, p) => dv.getInt16(p, true),
    i32: (dv, p) => dv.getInt32(p, true),
    i64: (dv, p) => dv.getBigInt64(p, true),
    u8: (dv, p) => dv.getUint8(p),
    u16: (dv, p) => dv.getUint16(p, true),
    u32: (dv, p) => dv.getUint32(p, true),
    u64: (dv, p) => dv.getBigUint64(p, true),
    f32: (dv, p) => dv.getFloat32(p, true),
    f64: (dv, p) => dv.getFloat64(p, true),
    char8: (dv, p) => dv.getUint8(p),
    char16: (dv, p) => dv.getUint16(p, true),
    char32: (dv, p) => dv.getUint32(p, true),
    rawptr: (dv, p) => dv.getUint32(p, true),
    ref: (dv, p) => dv.getUint32(p, true),
    composite: (dv, p, size) => new Uint8Array(dv.buffer.slice(p, p + size)),
};

// The helpers' constants.
const SIG_DIRTY = 1n;
const SIG_OPEN = 1n;
const NOT_DIRTY = 0xffff_ffff_ffff_fffen; // ~DADO_RT__CHAN_DIRTY as a uint64_t
const CHANL_MID = 3;
const CHANL_DIRTY = 16;
const CHANL_WRITING = 32;
const CHANNEL_ANY = 1;
const TWO_TO_53 = 1n << 53n;

/**
 * **The consumer id of a DadoScript engine** in a channel's owner word. A native
 * thread's id is the address of an 8-byte per-thread object, so a multiple of
 * 8, never below 1024 (natively the page at 0 is unmapped; on `#WEB` static
 * data and thread storage begin at the linker's global base, 1024); bit 0 of
 * the word is set while one of the consumer's queue walks is open. An engine
 * takes an id from the range no object can be at: `8 * (k + 1)` for engine
 * `k`, below 1024 — 127 engines. This file is engine 0.
 */
export const ENGINE_ID = 8n;

/** The first id that is a thread's rather than an engine's. */
export const FIRST_THREAD_ID = 1024n;

/** A walk the one-consumer rule refuses, or a directory this file cannot read. */
export class ChannelError extends Error {}

/** Whether the host stores typed-array elements little-endian, as wasm does. */
function hostIsLittleEndian() {
    return new Uint8Array(new Uint16Array([1]).buffer)[0] === 1;
}

function cString(bytes, at) {
    let end = at;
    while (bytes[end] !== 0) {
        end++;
    }
    return new TextDecoder().decode(bytes.subarray(at, end));
}

/** The program's channels, and the helpers over them. */
export class ChannelDirectory {
    /**
     * Read the directory at `address` in `memory` (a `WebAssembly.Memory`)
     * — the value the module's `dado_rt__channel_directory` export answers,
     * asked once. `engine` is this reader's consumer id.
     */
    static read(memory, address, engine = ENGINE_ID) {
        return new ChannelDirectory(memory, address >>> 0, engine);
    }

    constructor(memory, address, engine) {
        if (!hostIsLittleEndian()) {
            throw new ChannelError("this host stores typed arrays big-endian; wasm memory is little-endian");
        }
        if ((engine & 7n) !== 0n || engine === 0n || engine >= FIRST_THREAD_ID) {
            throw new ChannelError(`an engine id is a nonzero multiple of 8 below ${FIRST_THREAD_ID}, not ${engine}`);
        }
        this.memory = memory;
        this.engine = engine;
        this.#views();
        const L = DIRECTORY_LAYOUT;
        const dv = this.dv;
        this.version = dv.getUint32(address + L.header.version, true);
        if (this.version !== DIRECTORY_VERSION) {
            throw new ChannelError(
                `the channel directory is version ${this.version}; this reader reads version ${DIRECTORY_VERSION}`,
            );
        }
        this.count = dv.getUint32(address + L.header.count, true);
        this.flagsAt = dv.getUint32(address + L.header.flags, true);
        this.bitsAt = dv.getUint32(address + L.header.bits, true);
        const namesAt = dv.getUint32(address + L.header.names, true);
        const entriesAt = dv.getUint32(address + L.header.entries, true);
        if (this.flagsAt % 4 !== 0 || this.bitsAt % 8 !== 0) {
            throw new ChannelError("the flag word or the bitmap is misaligned: not a wasm32 directory");
        }
        const name = (i) => cString(this.bytes, dv.getUint32(namesAt + i * L.pointer, true));
        const u64 = (at) => Number(dv.getBigUint64(at, true));
        this.channels = [];
        this.byName = new Map();
        for (let i = 0; i < this.count; i++) {
            const e = entriesAt + i * L.entry.size;
            const index = dv.getUint32(e + L.entry.index, true);
            if (index !== i) {
                throw new ChannelError(`directory entry ${i} says it is ${index}: the entry layout is not wasm32's`);
            }
            const kind = dv.getUint32(e + L.entry.kind, true);
            const fieldCount = dv.getUint32(e + L.entry.fieldCount, true);
            const fieldsAt = dv.getUint32(e + L.entry.fields, true);
            const fields = [];
            for (let f = 0; f < fieldCount; f++) {
                const at = fieldsAt + f * L.field.size;
                const k = dv.getUint32(at + L.field.kind, true);
                const fkind = FIELD_KIND[k] ?? "composite";
                fields.push({
                    name: name(dv.getUint32(at + L.field.name, true)),
                    kind: fkind,
                    get: READERS[fkind],
                    offset: u64(at + L.field.offset),
                    size: u64(at + L.field.bytes),
                });
            }
            const s = {
                index,
                name: name(dv.getUint32(e + L.entry.name, true)),
                kind: kind === 0 ? "queue" : "latest",
                capacity: u64(e + L.entry.capacity),
                instance: dv.getUint32(e + L.entry.instance, true),
                fieldOffset: u64(e + L.entry.fieldOffset),
                size: u64(e + L.entry.sizeof),
                tail: u64(e + L.entry.tail), // a latest's `word`
                head: u64(e + L.entry.head), // a latest's `front_state`
                owner: u64(e + L.entry.owner),
                slots: u64(e + L.entry.slots), // a latest's `buf`
                stride: u64(e + L.entry.stride),
                fields,
                bitWord: index >> 6,
                bit: 1n << BigInt(index & 63),
            };
            const message = s.kind === "queue" ? L.message : 0;
            for (const f of fields) {
                if (message + f.offset + f.size > s.stride) {
                    throw new ChannelError(`${s.name}.${f.name} lies outside its slot: the message is not ${message} bytes in`);
                }
            }
            s.message = message;
            this.channels.push(s);
            this.byName.set(s.name, s);
        }
    }

    // The views, remade when the memory has grown (a grown memory detaches
    // the old buffer). Every helper below asks for them first.
    #views() {
        const buffer = this.memory.buffer;
        if (buffer === this.buffer) {
            return;
        }
        this.buffer = buffer;
        this.u32 = new Uint32Array(buffer);
        this.u64 = new BigUint64Array(buffer);
        this.dv = new DataView(buffer);
        this.bytes = new Uint8Array(buffer);
    }

    /** The channel called `name` (its qualified name, `package.channel`). */
    channel(name) {
        const s = this.byName.get(name);
        if (s === undefined) {
            throw new ChannelError(`the program has no live channel ${name}`);
        }
        return s;
    }

    /** A channel declared as a field: its entry, placed at `base` + its offset. */
    instance(s, base) {
        return { ...s, instance: base + s.fieldOffset };
    }

    // Word indices into the views.
    #w64(s, off) {
        const at = s.instance + off;
        return at / 8;
    }

    // ---- the program-wide words ------------------------------------------------------

    /** CHANNEL_ANY: some channel changed since the watcher last looked. One load. */
    channelAny() {
        this.#views();
        return (Atomics.load(this.u32, this.flagsAt / 4) & CHANNEL_ANY) !== 0;
    }

    /** Whether channel `s`'s bit is set in the bitmap. */
    bitSet(s) {
        this.#views();
        return (Atomics.load(this.u64, this.bitsAt / 8 + s.bitWord) & s.bit) !== 0n;
    }

    /**
     * The watcher's look: clear CHANNEL_ANY first, then exchange each bitmap
     * word with 0, and answer the channels whose bits it took. The caller walks
     * each of them before it looks again — clear, then walk.
     */
    look() {
        this.#views();
        Atomics.and(this.u32, this.flagsAt / 4, ~CHANNEL_ANY);
        const taken = [];
        const words = (this.count + 63) >> 6;
        for (let w = 0; w < words; w++) {
            const was = Atomics.exchange(this.u64, this.bitsAt / 8 + w, 0n);
            for (let b = 0; b < 64 && w * 64 + b < this.count; b++) {
                if (was & (1n << BigInt(b))) {
                    taken.push(this.channels[w * 64 + b]);
                }
            }
        }
        return taken;
    }

    // dado_rt__chan_raise
    #raise(s) {
        Atomics.or(this.u64, this.bitsAt / 8 + s.bitWord, s.bit);
        Atomics.or(this.u32, this.flagsAt / 4, CHANNEL_ANY);
    }

    // ---- the payload -----------------------------------------------------------------

    #read(s, at) {
        const dv = this.dv;
        const m = {};
        for (const f of s.fields) {
            m[f.name] = f.get(dv, at + f.offset, f.size);
        }
        return m;
    }

    #write(s, at, values) {
        const dv = this.dv;
        for (const f of s.fields) {
            const v = values[f.name];
            if (v === undefined) {
                throw new ChannelError(`${s.name}: no value for ${f.name}`);
            }
            const p = at + f.offset;
            switch (f.kind) {
                case "bool":
                    dv.setUint8(p, v ? 1 : 0);
                    break;
                case "i8":
                    dv.setInt8(p, v);
                    break;
                case "i16":
                    dv.setInt16(p, v, true);
                    break;
                case "i32":
                    dv.setInt32(p, v, true);
                    break;
                case "i64":
                    dv.setBigInt64(p, BigInt(v), true);
                    break;
                case "u8":
                case "char8":
                    dv.setUint8(p, v);
                    break;
                case "u16":
                case "char16":
                    dv.setUint16(p, v, true);
                    break;
                case "u32":
                case "char32":
                case "rawptr":
                    dv.setUint32(p, v, true);
                    break;
                case "u64":
                    dv.setBigUint64(p, BigInt(v), true);
                    break;
                case "f32":
                    dv.setFloat32(p, v, true);
                    break;
                case "f64":
                    dv.setFloat64(p, v, true);
                    break;
                case "ref":
                    // A `ref` moves ownership of memory the program's allocator
                    // holds; a host cannot hand the consumer one it did not get
                    // from that allocator.
                    throw new ChannelError(`${s.name}.${f.name} is a ref, which JavaScript cannot own`);
                default:
                    if (!(v instanceof Uint8Array) || v.length !== f.size) {
                        throw new ChannelError(`${s.name}.${f.name} is ${f.size} bytes of composite data`);
                    }
                    this.bytes.set(v, p);
            }
        }
    }

    // ---- the queue, producer side ------------------------------------------------------

    // dado_rt__chanq_claim: [pos, first], or null when FULL.
    #claim(s) {
        const u64 = this.u64;
        const tail = this.#w64(s, s.tail);
        const head = this.#w64(s, s.head);
        const cap = BigInt(s.capacity);
        let w = Atomics.load(u64, tail);
        let h = Atomics.load(u64, head);
        for (;;) {
            const p = w >> 1n;
            if (h > p || p - h >= cap) {
                // full as far as this producer has seen — or `h` is newer than
                // `w` (the consumer committed past a stale tail), not full.
                h = Atomics.load(u64, head);
                if (h > p) {
                    w = Atomics.load(u64, tail);
                    continue;
                }
                if (p - h >= cap) {
                    return null; // FULL: linearized at this load
                }
                continue;
            }
            const want = BigInt.asUintN(64, ((p + 1n) << 1n) | SIG_DIRTY);
            const seen = Atomics.compareExchange(u64, tail, w, want);
            if (seen === w) {
                return [p, (w & SIG_DIRTY) === 0n];
            }
            w = seen;
        }
    }

    /**
     * `try #send(s, …)`: `values` is an object keyed by the parameters' names.
     * Answers true, or false when the queue is FULL (nothing claimed; the
     * caller decides — drop, count, or retry later). Never blocks.
     */
    send(s, values) {
        this.#views();
        if (s.kind !== "queue") {
            return this.sendLatest(s, values);
        }
        const claimed = this.#claim(s);
        if (claimed === null) {
            return false;
        }
        const [pos, first] = claimed;
        const slot = s.instance + s.slots + Number(pos % BigInt(s.capacity)) * s.stride;
        this.#write(s, slot + s.message, values);
        // dado_rt__chanq_publish
        Atomics.store(this.u64, slot / 8, pos + 1n);
        if (first) {
            this.#raise(s);
        }
        return true;
    }

    // ---- the one-consumer guard ----------------------------------------------------------

    // dado_rt__chan_claim_owner
    #claimOwner(s, mark) {
        const owner = this.#w64(s, s.owner);
        const o = Atomics.compareExchange(this.u64, owner, 0n, this.engine | mark);
        if (o === 0n) {
            return 0n;
        }
        const first = o & ~SIG_OPEN;
        if (first !== this.engine) {
            throw new ChannelError(
                `channel walked by a second consumer: ${s.name}\n  its consumer is ${describe(first)}, and this is ${describe(this.engine)}`,
            );
        }
        return o;
    }

    // dado_rt__chan_open
    #open(s) {
        const o = this.#claimOwner(s, SIG_OPEN);
        if (o === 0n) {
            return;
        }
        if (o & SIG_OPEN) {
            throw new ChannelError(`channel walked inside a walk of itself: ${s.name}`);
        }
        Atomics.store(this.u64, this.#w64(s, s.owner), o | SIG_OPEN);
    }

    // dado_rt__chan_close
    #close(s) {
        Atomics.store(this.u64, this.#w64(s, s.owner), this.engine);
    }

    // ---- the queue, consumer side (one consumer) -------------------------------------

    // A walk: `#drain(s, limit)` when `consume`, `#peek(s)` when not. `visit`
    // is the loop body, called with each message; it may answer `false` to
    // `break`. An entry whose body began is consumed, whatever the body does —
    // including a throw, which commits before it propagates.
    #walk(s, consume, limit, visit) {
        this.#views();
        const u64 = this.u64;
        const cap = BigInt(s.capacity);
        const tail = this.#w64(s, s.tail);
        const head = this.#w64(s, s.head);
        const bits = this.bitsAt / 8 + s.bitWord;
        this.#open(s);
        // dado_rt__chanq_begin
        let lim;
        if ((Atomics.load(u64, bits) & s.bit) === 0n) {
            lim = Atomics.and(u64, tail, NOT_DIRTY) >> 1n;
        } else {
            lim = Atomics.load(u64, tail) >> 1n;
        }
        // dado_rt__chanq_head
        const h = Atomics.load(u64, head);
        let i = h;
        if (!consume && h !== lim) {
            this.#raise(s); // dado_rt__chanq_peeked
        }
        // The walk counts `k` from `h` as a Number — at most the capacity —
        // so a message costs no BigInt arithmetic. While every position is
        // below 2^53 a `seq` compares exactly as a Number; past it, as a
        // BigInt. `i = h + k` is the position the helpers call `i`.
        const count = Number(lim - h);
        const n = limit === undefined ? count : Math.max(0, Math.min(count, Number(limit)));
        const start = Number(h % cap);
        const exact = lim + cap < TWO_TO_53;
        const hn = Number(h);
        const base = s.instance + s.slots;
        let k = 0;
        try {
            while (k < n) {
                const slot = base + ((start + k) % s.capacity) * s.stride;
                // dado_rt__chanq_ready
                const seq = Atomics.load(u64, slot / 8);
                if (exact ? Number(seq) !== hn + k + 1 : seq !== h + BigInt(k + 1)) {
                    this.#raise(s);
                    break;
                }
                const m = this.#read(s, slot + s.message);
                k++;
                if (visit(m) === false) {
                    break;
                }
            }
        } finally {
            if (consume) {
                // dado_rt__chanq_commit
                i = h + BigInt(k);
                if (Atomics.load(u64, head) !== i) {
                    Atomics.store(u64, head, i);
                }
                if (i !== lim) {
                    this.#raise(s);
                }
            }
            this.#close(s);
        }
        const walked = k;
        return walked;
    }

    /** `for … in #drain(s)` / `#drain(s, limit)`: answers how many were walked. */
    drain(s, visit, limit) {
        if (s.kind !== "queue") {
            return this.#latestWalk(s, true, visit);
        }
        return this.#walk(s, true, limit, visit);
    }

    /** `for … in #peek(s)`: consumes nothing. */
    peek(s, visit) {
        if (s.kind !== "queue") {
            return this.#latestWalk(s, false, visit);
        }
        return this.#walk(s, false, undefined, visit);
    }

    /** `#drain(s)` / `#drain(s, n)` as a statement: discard, committed at once. */
    discard(s, limit) {
        return this.drain(s, () => true, limit);
    }

    /** `#len(s)`: exact from the consumer at quiescence, approximate otherwise. */
    len(s) {
        this.#views();
        if (s.kind !== "queue") {
            // dado_rt__chanl_len
            const w = Atomics.load(this.u32, (s.instance + s.tail) / 4);
            const fs = Atomics.load(this.u32, (s.instance + s.head) / 4);
            return (w & CHANL_DIRTY) | (fs & 2) ? 1 : 0;
        }
        // dado_rt__chanq_len
        const t = Atomics.load(this.u64, this.#w64(s, s.tail)) >> 1n;
        const h = Atomics.load(this.u64, this.#w64(s, s.head));
        if (h >= t) {
            return 0;
        }
        return t - h > BigInt(s.capacity) ? s.capacity : Number(t - h);
    }

    // ---- latest ------------------------------------------------------------------------

    /** `#send` into a `latest`: never fails; a write that finds another writer
     * mid-`#send` is superseded and returns at once (answering false). */
    sendLatest(s, values) {
        this.#views();
        const u32 = this.u32;
        const word = (s.instance + s.tail) / 4;
        // dado_rt__chanl_claim
        let w = Atomics.load(u32, word);
        for (;;) {
            if (w & CHANL_WRITING) {
                return false;
            }
            const seen = Atomics.compareExchange(u32, word, w, w | CHANL_WRITING);
            if (seen === w) {
                break;
            }
            w = seen;
        }
        const back = ((w >> 2) & 3) ^ 2;
        this.#write(s, s.instance + s.slots + back * s.stride, values);
        // dado_rt__chanl_publish
        w = Atomics.load(u32, word);
        for (;;) {
            const mid = w & CHANL_MID;
            const b = ((w >> 2) & 3) ^ 2;
            const n = b | (((mid ^ 2) & 3) << 2) | CHANL_DIRTY;
            const seen = Atomics.compareExchange(u32, word, w, n);
            if (seen === w) {
                break;
            }
            w = seen;
        }
        if (!(w & CHANL_DIRTY)) {
            this.#raise(s);
        }
        return true;
    }

    // dado_rt__chanl_take: the buffer to read, or -1.
    #take(s, consume) {
        const u32 = this.u32;
        const word = (s.instance + s.tail) / 4;
        const frontState = (s.instance + s.head) / 4;
        let w = Atomics.load(u32, word);
        let front;
        if (w & CHANL_DIRTY) {
            for (;;) {
                const mid = w & CHANL_MID;
                const back = ((w >> 2) & 3) ^ 2;
                const f = 3 - mid - back;
                const n = (w & ~(CHANL_MID | CHANL_DIRTY)) | f; // new mid = old front
                const seen = Atomics.compareExchange(u32, word, w, n);
                if (seen === w) {
                    break;
                }
                w = seen;
            }
            front = w & CHANL_MID; // the consumer's front is the old mid
            Atomics.store(u32, frontState, 3);
        } else {
            front = 3 - (w & CHANL_MID) - (((w >> 2) & 3) ^ 2);
        }
        const fs = Atomics.load(u32, frontState);
        if (!(fs & 2)) {
            return -1;
        }
        if (consume) {
            Atomics.store(u32, frontState, fs & ~2);
        }
        return front;
    }

    #latestWalk(s, consume, visit) {
        this.#views();
        this.#claimOwner(s, 0n); // dado_rt__chan_own
        const front = this.#take(s, consume);
        if (!consume && front >= 0) {
            this.#raise(s); // dado_rt__chanl_peeked
        }
        if (front < 0) {
            return 0;
        }
        visit(this.#read(s, s.instance + s.slots + front * s.stride));
        return 1;
    }
}

/** A consumer id as a report names it: a thread's address, or an engine. */
export function describe(id) {
    if (id < FIRST_THREAD_ID) {
        return `DadoScript engine ${id / 8n - 1n}`;
    }
    return `thread 0x${id.toString(16)}`;
}
