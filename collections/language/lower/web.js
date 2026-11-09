// dados/web.js: what a script's module reaches its host through on the web,
// where a Dado program runs in a wasm module beside the page's JavaScript
// engine. Natively each VM is an engine of its own, with `print` and
// `println` its globals and every Dado function a script calls an export of a
// native module made for that engine; on the web there is one engine, the
// page's, and so one copy of each module however many VMs the program makes.
// So a module calls its host through here, and the host (the loader's
// `dados.mjs`, bound with `__dados_bind` before any module runs) routes each
// call to the VM that is running: `print` to that VM's sink, a Dado function
// to the thunk the program registered for that VM's context.

let host = null;

// The loader's host: `print(text)`, `println(text)` and
// `seam(module, name, args)`.
export function __dados_bind(h) {
    host = h;
}

export function print(s) {
    host.print(s);
}

export function println(s) {
    host.println(s);
}

// The Dado function `name` of the native module `module` (`dado:<package>`),
// as a function: each call goes to the running VM's thunk. The one strategy
// every crossing from a script into Dado takes on the web until the compiler
// writes a crossing inline at its site.
export function __dados_seam(module, name) {
    return (...args) => host.seam(module, name, args);
}

// ── inline crossings ─────────────────────────────────────────────────────
//
// What a site the compiler wrote inline calls. A site calls the Dado function
// (or its C shim) straight from the slot its module cached at start-up
// (`__dados_link`, called from `__dados_attach`), with each argument converted
// in the calling expression by the small helpers below, which V8 inlines;
// none of them is a wrapper of a function. A string argument is encoded into
// the module's scratch; a string answer is copied out of the module's memory.
// The views over that memory are taken once and re-taken only when it grew
// (a grow detaches every view: `fresh`).

// Whether a run has lent the script a Dado structure: while it is not zero, a
// site settles the lend before Dado runs (the lend lock), as a thunk does.
export let __dados_held = 0;

// The counters of the VM running: crossings into Dado, and string bytes
// encoded in and decoded out, since its frame began (`#script_pump`).
const NOBODY = { calls: 0, bytes_in: 0, bytes_out: 0, runs: 0 };
export let __dados_C = NOBODY;
// `__dados_C` while no VM runs: a site that can reach `#send` compares it.
export const __dados_none = NOBODY;

// A dev build's per-site counts, by site number: crossings, narrowing wraps
// that changed the value, and calls that left the fast path — a string encoded
// into a block of the heap or with a lone surrogate, a character that raised.
export let __dados_S = new Float64Array(0);
export let __dados_W = new Float64Array(0);
export let __dados_F = new Float64Array(0);

// Each pre-encoded literal's address in the data segment.
export const __dados_L = [];

// The stash: a site's arguments, evaluated once, and each string's or
// character's address (or code) and length once checked, by the argument's
// index. **Sized to every site**: a wasm function takes at most 1000
// parameters (the JS API's limit), and each argument is at least one.
export const __dados_a = [];
export const __dados_p = new Int32Array(1000);
export const __dados_n = new Int32Array(1000);

let memory = null;
let shared = false;
let U8 = new Uint8Array(0);
let I32 = new Int32Array(0);
let F64 = new Float64Array(0);
let SUB = new Uint8Array(0);
let G = 0;
let GF = 0;
let SCR = 0;
let CAP = 0;
let FLAGS = 0;
let cursor = 0;
let drop = null;
// A block of the module's heap standing in for the scratch while a call's
// strings do not fit it (`dados_room`), and its size; 0 for none. Given back
// (`dados_unroom`, `release`) when the call fails — a check refusing an
// argument, a failable's failure — or by the next crossing's stash.
let ROOM = 0;
let ROOM_CAP = 0;
let room = null;
let unroom = null;
// The stashed strings encoded so far, by index, to be moved with the scratch.
const ENCODED = new Int32Array(1000);
let encoded = 0;
const encoder = new TextEncoder();
const decoder = new TextDecoder("utf-8", { fatal: true });
const wellFormed = typeof "".isWellFormed === "function";
const MAX_SAFE = 9007199254740991;

function views() {
    const b = memory.buffer;
    U8 = new Uint8Array(b);
    I32 = new Int32Array(b);
    F64 = new Float64Array(b);
    SUB = U8.subarray(SCR, SCR + CAP);
}

// The views, re-taken when they no longer reach `end`: detached by a grow
// (length 0), or a shared memory that grew past them.
function fresh(end) {
    if (U8.length < end) views();
}

// The module's header (`dados_glue`), the functions in its slots handed to
// each module's `__dados_link`; `modules` the registry's. A module whose
// program crosses nothing inline exports no header.
export function __dados_attach(exports, mem, table, modules, sites) {
    __dados_S = new Float64Array(sites);
    __dados_W = new Float64Array(sites);
    __dados_F = new Float64Array(sites);
    if (typeof exports.dados_glue !== "function") return;
    memory = mem;
    shared = typeof SharedArrayBuffer !== "undefined" && mem.buffer instanceof SharedArrayBuffer;
    views();
    const g = exports.dados_glue() >>> 0;
    G = g >> 2;
    GF = (g + 40) >> 3;
    const count = I32[G + 1];
    const bound = (I32[G + 2] >>> 0) >> 2;
    SCR = I32[G + 3] >>> 0;
    CAP = I32[G + 4];
    FLAGS = (I32[G + 5] >>> 0) >> 2;
    const literals = (I32[G + 6] >>> 0) >> 2;
    for (let i = 0; i < I32[G + 7]; i++) __dados_L.push(I32[literals + i] >>> 0);
    views();
    // Each slot looked up once (`table.get` per call costs 28 ns; a cached
    // function 2 ns).
    const fns = [];
    for (let i = 0; i < count; i++) fns.push(table.get(I32[bound + i] >>> 0));
    drop = typeof exports.dados_drop === "function" ? exports.dados_drop : null;
    room = typeof exports.dados_room === "function" ? exports.dados_room : null;
    unroom = typeof exports.dados_unroom === "function" ? exports.dados_unroom : null;
    for (const [, ns] of modules) {
        if (typeof ns.__dados_link === "function") ns.__dados_link(fns);
    }
}

// The host's: the counters of the VM now running (or of none).
export function __dados_vm(counters) {
    __dados_C = counters === null ? NOBODY : counters;
}

// The host's: a run's lend begun (1) or over (-1).
export function __dados_hold(delta) {
    __dados_held += delta;
}

// The lend lock, while a lend is outstanding.
export function __dados_settle() {
    host.settle();
}

// After a crossing whose callee can reach `#send`: the any-channel bit, and
// the safe point when it is set. With no VM running the safe point is asked
// whatever the bit — a send to a channel already waiting sets no bit — and
// says whether this call sent what no handler can be called for. Answers `v`.
export function __dados_sig(v) {
    release();
    if (FLAGS !== 0) {
        if (__dados_C === NOBODY) {
            host.channels();
            return v;
        }
        fresh(4 * FLAGS + 4);
        if ((I32[FLAGS] & 1) !== 0) host.channels();
    }
    return v;
}

// ── an `int` into a sized integer, wrapping as Dado's written conversion does

export function __dados_n32(v) {
    return typeof v === "number" ? v | 0 : Number(BigInt.asIntN(32, v));
}
export function __dados_i8(v) {
    return typeof v === "number" ? (v << 24) >> 24 : Number(BigInt.asIntN(8, v));
}
export function __dados_u8(v) {
    return typeof v === "number" ? v & 255 : Number(BigInt.asUintN(8, v));
}
export function __dados_i16(v) {
    return typeof v === "number" ? (v << 16) >> 16 : Number(BigInt.asIntN(16, v));
}
export function __dados_u16(v) {
    return typeof v === "number" ? v & 65535 : Number(BigInt.asUintN(16, v));
}
// A 64-bit integer's halves, low first.
export function __dados_lo(v) {
    return typeof v === "number" ? v | 0 : Number(BigInt.asIntN(32, v));
}
export function __dados_hi(v) {
    return typeof v === "number" ? Math.floor(v / 4294967296) | 0 : Number(BigInt.asIntN(32, v >> 32n));
}
// An `int` or a `float` into a float: an `int` past 2^53 rounds.
export function __dados_f(v) {
    return typeof v === "number" ? v : Number(v);
}

// The same, in a dev build, counting at site `k` each wrap that changed the
// value (DX-12).
function wrapped(v, n, k) {
    if (typeof v === "number" ? n !== v : BigInt(n) !== v) __dados_W[k]++;
    return n;
}
export function __dados_wi32(v, k) {
    return wrapped(v, __dados_n32(v), k);
}
export function __dados_wu32(v, k) {
    const n = __dados_n32(v);
    if (typeof v === "number" ? n >>> 0 !== v : BigInt(n >>> 0) !== v) __dados_W[k]++;
    return n;
}
export function __dados_wi8(v, k) {
    return wrapped(v, __dados_i8(v), k);
}
export function __dados_wu8(v, k) {
    return wrapped(v, __dados_u8(v), k);
}
export function __dados_wi16(v, k) {
    return wrapped(v, __dados_i16(v), k);
}
export function __dados_wu16(v, k) {
    return wrapped(v, __dados_u16(v), k);
}
export function __dados_wf32(v, k) {
    const x = __dados_f(v);
    const r = Math.fround(x);
    if (typeof v === "number" ? r !== x && x === x : BigInt(r) !== v) __dados_W[k]++;
    return x;
}
export function __dados_wf64(v, k) {
    const x = __dados_f(v);
    if (typeof v !== "number" && BigInt(x) !== v) __dados_W[k]++;
    return x;
}

// ── the stash

// A new crossing's scratch: empty, and the static one again, the block the
// last crossing's strings needed given back.
function begin() {
    cursor = 0;
    encoded = 0;
    release();
}

// The block a call's strings needed, given back: the call is over, or failed.
function release() {
    if (ROOM !== 0) {
        const p = ROOM;
        ROOM = 0;
        ROOM_CAP = 0;
        unroom(p);
    }
}

export function __dados_stash1(a) {
    begin();
    __dados_a[0] = a;
}
export function __dados_stash2(a, b) {
    begin();
    __dados_a[0] = a;
    __dados_a[1] = b;
}
export function __dados_stash3(a, b, c) {
    begin();
    __dados_a[0] = a;
    __dados_a[1] = b;
    __dados_a[2] = c;
}
export function __dados_stash4(a, b, c, d) {
    __dados_stash3(a, b, c);
    __dados_a[3] = d;
}
export function __dados_stash5(a, b, c, d, e) {
    __dados_stash4(a, b, c, d);
    __dados_a[4] = e;
}
export function __dados_stash6(a, b, c, d, e, f) {
    __dados_stash5(a, b, c, d, e);
    __dados_a[5] = f;
}
export function __dados_stash7(a, b, c, d, e, f, g) {
    __dados_stash6(a, b, c, d, e, f);
    __dados_a[6] = g;
}
export function __dados_stash8(a, b, c, d, e, f, g, h) {
    __dados_stash7(a, b, c, d, e, f, g);
    __dados_a[7] = h;
}
// Past eight arguments, any count (DI-0275).
export function __dados_stashn(...args) {
    begin();
    for (let i = 0; i < args.length; i++) __dados_a[i] = args[i];
}

// Stashed string `i` encoded as UTF-8 into the scratch: a short ASCII string
// (16 characters or fewer) by a loop, anything else by `encodeInto` into the
// scratch's cached view. A string that does not fit, or holds a lone
// surrogate, is encoded the slow way (`slow8`), never sent down the thunk;
// `k` is the site, in a dev build, to count it.
export function __dados_in8(i, k) {
    const s = __dados_a[i];
    let n = -1;
    if (ROOM === 0) {
        fresh(SCR + CAP);
        const at = SCR + cursor;
        if (s.length <= 16 && CAP - cursor >= 16) {
            let k = 0;
            for (; k < s.length; k++) {
                const c = s.charCodeAt(k);
                if (c >= 0x80) break;
                U8[at + k] = c;
            }
            if (k === s.length) n = k;
        }
        if (n < 0 && !(wellFormed && !s.isWellFormed())) {
            if (shared) {
                const b = encoder.encode(s);
                if (b.length <= CAP - cursor) {
                    U8.set(b, at);
                    n = b.length;
                }
            } else {
                const r = encoder.encodeInto(s, cursor === 0 ? SUB : SUB.subarray(cursor));
                if (r.read === s.length) n = r.written;
            }
        }
        if (n >= 0) {
            __dados_p[i] = at;
            __dados_n[i] = n;
            ENCODED[encoded++] = i;
            cursor += n;
            __dados_C.bytes_in += n;
            return;
        }
    }
    if (k !== undefined) __dados_F[k]++;
    slow8(i, s);
}

// Stashed string `i`, the slow way: its bytes — a lone surrogate as the
// engine encodes it, its own three bytes, where `TextEncoder` writes U+FFFD —
// after the scratch's; when they do not fit, the scratch's strings so far are
// moved into a block of the module's heap big enough for them and these, and
// the rest of the call's strings follow them there.
function slow8(i, s) {
    const b = wellFormed && s.isWellFormed() ? encoder.encode(s) : engineUtf8(s);
    let base = ROOM !== 0 ? ROOM : SCR;
    const cap = ROOM !== 0 ? ROOM_CAP : CAP;
    if (b.length > cap - cursor) {
        const want = cursor + b.length;
        const p = room === null || want > 0x7fffffff ? 0 : room(want) >>> 0;
        if (p === 0) {
            release();
            const e = new Error("out of memory");
            e.name = "InternalError";
            throw e;
        }
        fresh(p + want);
        U8.copyWithin(p, base, base + cursor);
        for (let j = 0; j < encoded; j++) __dados_p[ENCODED[j]] += p - base;
        if (ROOM !== 0) unroom(ROOM);
        ROOM = p;
        ROOM_CAP = want;
        base = p;
    }
    fresh(base + cursor + b.length);
    U8.set(b, base + cursor);
    __dados_p[i] = base + cursor;
    __dados_n[i] = b.length;
    ENCODED[encoded++] = i;
    cursor += b.length;
    __dados_C.bytes_in += b.length;
}

// `s` as UTF-8 with each lone surrogate its own three bytes, as the engine
// copies a string out: the well-formed runs between them by `TextEncoder`.
function engineUtf8(s) {
    const parts = [];
    let from = 0;
    let n = 0;
    const len = s.length;
    for (let i = 0; i < len; i++) {
        const c = s.charCodeAt(i);
        if (c < 0xd800 || c > 0xdfff) continue;
        if (c <= 0xdbff) {
            const d = i + 1 < len ? s.charCodeAt(i + 1) : 0;
            if (d >= 0xdc00 && d <= 0xdfff) {
                i++;
                continue;
            }
        }
        // A lone surrogate: a high with no low after it, or a low with no
        // high before it (a paired low was skipped with its high above).
        const run = encoder.encode(s.slice(from, i));
        parts.push(run, Uint8Array.of(0xe0 | (c >> 12), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f)));
        n += run.length + 3;
        from = i + 1;
    }
    if (from === 0) return encoder.encode(s);
    const last = encoder.encode(s.slice(from));
    parts.push(last);
    n += last.length;
    const out = new Uint8Array(n);
    let at = 0;
    for (const p of parts) {
        out.set(p, at);
        at += p.length;
    }
    return out;
}

// Stashed string `i` as a `char32`: exactly one code point. Any other string
// raises the `Error` the thunk raises, naming the function `fn` and its
// parameter `param`; `k` is the site, in a dev build, to count it.
export function __dados_ch32(i, fn, param, k) {
    const s = __dados_a[i];
    const c = s.codePointAt(0);
    if (c === undefined || s.length !== (c > 0xffff ? 2 : 1) || (c >= 0xd800 && c <= 0xdfff)) {
        if (k !== undefined) __dados_F[k]++;
        // Its bytes crossed in before the check refused them, as natively.
        __dados_C.bytes_in += engineUtf8(s).length;
        release();
        throw new Error("`" + fn + "`'s parameter `" + param + "` is a character, which takes a string of exactly one code point");
    }
    __dados_p[i] = c;
    __dados_C.bytes_in += c < 0x80 ? 1 : c < 0x800 ? 2 : c < 0x10000 ? 3 : 4;
}

// Stashed string `i` as a `char16`: exactly one UTF-16 code unit, a lone
// surrogate included; any other string raises as `__dados_ch32` does.
export function __dados_ch16(i, fn, param, k) {
    const s = __dados_a[i];
    if (s.length !== 1) {
        if (k !== undefined) __dados_F[k]++;
        release();
        throw new Error("`" + fn + "`'s parameter `" + param + "` is a `char16`, which takes a string of exactly one UTF-16 code unit, and this one has " + s.length);
    }
    __dados_p[i] = s.charCodeAt(0);
}

// ── interned values (the compiler's `o7` pass)
//
// A Dado enum crosses inline as its value, and a function whose every answer
// is a literal string answers the literal's index: each module holds the
// tables the compiler wrote (a `Map` from each member's name to its value, and
// one from each value to the name it crosses as; an array of the literals and
// one of their bytes), so no name and no literal is encoded or decoded at the
// crossing. The counters count what the
// thunk's reader and writer would have copied, so they read the same either
// way.

// What the thunk's readers call `v`, a value an `int` reader refused: a number
// it refuses is never a 32-bit integer, so it is the engine's float.
function kindOf(v) {
    if (typeof v === "number") return "a float";
    if (typeof v === "bigint") return "an int";
    if (typeof v === "boolean") return "a bool";
    if (typeof v === "string") return "a string";
    if (v === undefined) return "nothing";
    if (Array.isArray(v)) return "an array";
    if (v instanceof Map) return "a map";
    return "a value of another kind";
}

// Stashed argument `i` of a Dado enum `N` names (a member's name to its
// value), read as the thunk's reader reads it: a member's name (its bytes
// counted in, as the reader copies them), or an `int` — a whole number no
// larger than 2^53, or a BigInt to 64 bits. Anything else raises the
// `TypeError` the thunk raises, naming the function `fn` and its parameter
// `param`. The value is left in the stash, for the site's narrowing.
export function __dados_ein(i, N, fn, param) {
    const v = __dados_a[i];
    if (typeof v === "string") {
        const n = N.get(v);
        // A member's name is ASCII: its bytes are its length.
        if (n !== undefined) {
            __dados_C.bytes_in += v.length;
            __dados_a[i] = n;
            return;
        }
        __dados_C.bytes_in += engineUtf8(v).length;
    } else if (typeof v === "number") {
        if (Number.isInteger(v) && v >= -MAX_SAFE && v <= MAX_SAFE) return;
    } else if (typeof v === "bigint") {
        __dados_a[i] = BigInt.asIntN(64, v);
        return;
    }
    release();
    throw new TypeError("`" + fn + "`'s parameter `" + param + "` takes an int, and was given " + kindOf(v));
}

// An enum's value `r` as the script holds it: the name `T` gives it (its
// bytes counted out), or the number no member names.
export function __dados_eout(T, r) {
    const name = T.get(r);
    if (name === undefined) return r;
    __dados_C.bytes_out += name.length;
    return name;
}

// A literal answer's index `r` among the function's literals `I` (each of
// `B` bytes), or -1 for an answer that is none of them, which is in the
// answer words as an uninterned string is.
export function __dados_lit(r, I, B) {
    if (r < 0) return __dados_out8();
    __dados_C.bytes_out += B[r];
    return I[r];
}

// ── answers

// The text of `n` bytes at `p`, copied: a short ASCII run by a loop, anything
// else by `TextDecoder`; bytes that are not UTF-8 as the engine reads them.
function text(p, n) {
    fresh(p + n);
    __dados_C.bytes_out += n;
    if (n <= 16) {
        let s = "";
        let k = 0;
        for (; k < n; k++) {
            const c = U8[p + k];
            if (c >= 0x80) break;
            s += String.fromCharCode(c);
        }
        if (k === n) return s;
    }
    const bytes = shared ? U8.slice(p, p + n) : U8.subarray(p, p + n);
    try {
        return decoder.decode(bytes);
    } catch {
        return host.decode(bytes);
    }
}

// A 64-bit integer from its halves, in its canonical tier.
function joined(lo, hi) {
    if (hi === lo >> 31) return lo;
    const v = hi * 4294967296 + (lo >>> 0);
    if (v >= -MAX_SAFE && v <= MAX_SAFE) return v;
    return BigInt.asIntN(64, (BigInt(hi) << 32n) | BigInt(lo >>> 0));
}

// A character as a one-code-point string; no Unicode scalar value is U+FFFD.
export function __dados_c32(v) {
    v >>>= 0;
    if ((v >= 0xd800 && v <= 0xdfff) || v > 0x10ffff) v = 0xfffd;
    return String.fromCodePoint(v);
}
export function __dados_c16(v) {
    return String.fromCharCode(v & 0xffff);
}
export function __dados_j(lo) {
    fresh(4 * G + 40);
    return joined(lo, I32[G + 9]);
}
export function __dados_out8() {
    fresh(4 * G + 40);
    return text(I32[G + 8] >>> 0, I32[G + 9]);
}
// A new string: copied, then ended on the allocator it was made on.
export function __dados_own8() {
    const s = __dados_out8();
    drop();
    return s;
}

// A failable's code: nonzero is the `Error` the thunk throws, carrying it.
function failed(code, label) {
    release();
    throw host.fail(label, code);
}
function answer(code, label) {
    if (code !== 0) failed(code, label);
    fresh(4 * G + 48);
}
export function __dados_ok(code, label) {
    if (code !== 0) failed(code, label);
}
export function __dados_oki(code, label) {
    answer(code, label);
    return I32[G + 8];
}
export function __dados_oku(code, label) {
    answer(code, label);
    return I32[G + 8] >>> 0;
}
export function __dados_okb(code, label) {
    answer(code, label);
    return I32[G + 8] !== 0;
}
export function __dados_okf(code, label) {
    answer(code, label);
    return F64[GF];
}
export function __dados_okj(code, label) {
    answer(code, label);
    return joined(I32[G + 8], I32[G + 9]);
}
export function __dados_okc32(code, label) {
    answer(code, label);
    return __dados_c32(I32[G + 8]);
}
export function __dados_okc16(code, label) {
    answer(code, label);
    return __dados_c16(I32[G + 8]);
}
export function __dados_oks(code, label) {
    answer(code, label);
    return __dados_out8();
}
export function __dados_oko(code, label) {
    answer(code, label);
    return __dados_own8();
}
// A failable's enum answer, by its table `T` (a `u32` one's, `__dados_okeu`).
export function __dados_oke(code, label, T) {
    answer(code, label);
    return __dados_eout(T, I32[G + 8]);
}
export function __dados_okeu(code, label, T) {
    answer(code, label);
    return __dados_eout(T, I32[G + 8] >>> 0);
}
// A failable's interned literal answer: its index in the first answer word
// and -1 in the second, or a string that is none of them, as `__dados_oks`.
export function __dados_okl(code, label, I, B) {
    answer(code, label);
    if (I32[G + 9] >= 0) return __dados_out8();
    __dados_C.bytes_out += B[I32[G + 8]];
    return I[I32[G + 8]];
}

// A dev build's per-site counts, for the host's report.
export function __dados_counts() {
    return { S: __dados_S, W: __dados_W, F: __dados_F };
}
