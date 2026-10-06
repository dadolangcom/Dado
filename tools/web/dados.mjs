// dados.mjs — the host side of DadoScript on the web: what a `#WEB` module that runs
// scripts imports from the wasm module `dados`, answered over the JavaScript
// engine this file runs on.
//
// A Dado program that hosts DadoScript scripts calls QuickJS-ng's C API natively.
// On the web the compiler's runtime implements that API in C, in the
// `quickjs.c` it emits for the web, as calls into this file, and the scripts
// themselves are the modules the build wrote beside the module, which
// `registry.js` lists. `load.mjs` imports this file and adds its table to its
// own; nothing here is node-only, so a page can use it as it is.
//
// **Values.** A string, BigInt, symbol or object C holds is a slot in
// `Session.values`, with a count of C's references that `dados.dup` and
// `dados.free` keep. A slot whose count reaches zero is released and its index
// reused. Everything else lives in the `JSValue` itself. A `JSValue` is 16
// bytes: the payload at +0 (an `int32` or a `float64`, or a slot index) and
// the tag, an `int64`, at +8.
//
// **Contexts.** Natively each VM is an engine with its own globals. Here
// there is one engine, so a context is a record keyed by its C address: its
// global object (where `print` and `println` are put), its pending exception,
// and the modules linked for it. Every call that may run script code makes
// its context the current one for the call, and that is the VM a script's
// `print` and its calls of Dado functions go to (`dados/web.js`).
//
// **Modules.** `registry.js` lists every module the program carries, its
// namespace, and the names of what it imports. A module is linked for a
// context as the engine would link it: what it imports first, each name
// through the program's normaliser and loader (`dados_normalize`,
// `dados_load`), and a C module's init function run (`dados_init`). Its
// namespace is the one the page loaded; module code runs once, however many
// VMs link it.

// ── STOP (a16-stop): the stop's check and the sessions of workers ───────────
import { Stop, UNCATCHABLE } from "./stop.mjs";
// ── end STOP ────────────────────────────────────────────────────────────────

/** A program ending with a status (`dado.exit`), unwinding to the loader. No
 * script's `catch` keeps it (`UNCATCHABLE`, `dados/stop.js`). */
export class DadoExit {
    constructor(status) {
        this.status = status;
    }

    get [UNCATCHABLE]() {
        return true;
    }
}

const TAG_BIG_INT = -9;
const TAG_SYMBOL = -8;
const TAG_STRING = -7;
const TAG_STRING_ROPE = -6;
const TAG_MODULE = -3;
const TAG_OBJECT = -1;
const TAG_INT = 0;
const TAG_BOOL = 1;
const TAG_NULL = 2;
const TAG_UNDEFINED = 3;
const TAG_UNINITIALIZED = 4;
const TAG_EXCEPTION = 6;
const TAG_FLOAT64 = 8;

/** No exception pending. */
const NONE = Symbol("no exception");

/** What `JS_LoadModule` and `JS_EvalFunction` answer: a promise already
 * settled, whose state C reads without waiting for the page's job queue. */
class Settled {
    constructor(state, value) {
        this.state = state;
        this.value = value;
    }
}

const FULFILLED = 1;
const REJECTED = 2;

/** How many frames an error the engine makes keeps: QuickJS-ng's default
 * `Error.stackTraceLimit`. */
const FRAMES = 10;

/** A frame of this host's own, which the engine natively has no frame for:
 * this file, the wasm module, `dados/web.js` — and the stop's poll
 * (STOP: `stop.mjs`, `dados/stop.js`), where the ask's `Error` is made. */
function hostFrame(line) {
    return line.includes("dados.mjs:") || line.includes("wasm://") || line.includes("wasm-function[") ||
        line.includes("/dados/web.js:") || line.includes("stop.mjs:") || line.includes("/dados/stop.js:");
}

/** An error the engine raises on a C function's behalf, its stack read from
 * the script's frame outward as QuickJS-ng's is, past this host's own. */
function engineError(make) {
    const limit = Error.stackTraceLimit;
    Error.stackTraceLimit = 200;
    let e;
    try {
        e = make();
    } finally {
        Error.stackTraceLimit = limit;
    }
    if (typeof e.stack === "string") {
        const lines = e.stack.split("\n");
        const head = [];
        let at = 0;
        while (at < lines.length && !/^\s*at /.test(lines[at])) head.push(lines[at++]);
        const frames = lines.slice(at).filter((l) => !hostFrame(l)).slice(0, FRAMES);
        Object.defineProperty(e, "stack", {
            value: head.concat(frames).join("\n"),
            writable: true,
            configurable: true,
        });
    }
    return e;
}

/** What must not be turned into a script's exception: the program ending,
 * and a trap in the module. */
function passes(e) {
    return e instanceof DadoExit || e instanceof WebAssembly.RuntimeError;
}

/** The text of `bytes`, UTF-8 read as QuickJS-ng reads it: an encoded
 * surrogate is that code unit, and a byte that starts no sequence is U+FFFD. */
export function decodeUtf8(bytes) {
    let out = "";
    let units = [];
    const flush = () => {
        out += String.fromCharCode.apply(null, units);
        units = [];
    };
    for (let i = 0; i < bytes.length; ) {
        const b = bytes[i];
        let c = -1;
        let n = 1;
        if (b < 0x80) {
            c = b;
        } else if (b >= 0xc2 && b <= 0xdf && i + 1 < bytes.length && (bytes[i + 1] & 0xc0) === 0x80) {
            c = ((b & 0x1f) << 6) | (bytes[i + 1] & 0x3f);
            n = 2;
        } else if (
            b >= 0xe0 && b <= 0xef && i + 2 < bytes.length &&
            (bytes[i + 1] & 0xc0) === 0x80 && (bytes[i + 2] & 0xc0) === 0x80
        ) {
            c = ((b & 0x0f) << 12) | ((bytes[i + 1] & 0x3f) << 6) | (bytes[i + 2] & 0x3f);
            n = 3;
            if (c < 0x800) c = -1;
        } else if (
            b >= 0xf0 && b <= 0xf4 && i + 3 < bytes.length &&
            (bytes[i + 1] & 0xc0) === 0x80 && (bytes[i + 2] & 0xc0) === 0x80 && (bytes[i + 3] & 0xc0) === 0x80
        ) {
            c = ((b & 0x07) << 18) | ((bytes[i + 1] & 0x3f) << 12) | ((bytes[i + 2] & 0x3f) << 6) | (bytes[i + 3] & 0x3f);
            n = 4;
            if (c < 0x10000 || c > 0x10ffff) c = -1;
        }
        if (c < 0) {
            units.push(0xfffd);
            i += 1;
        } else {
            if (c >= 0x10000) {
                c -= 0x10000;
                units.push(0xd800 + (c >> 10), 0xdc00 + (c & 0x3ff));
            } else {
                units.push(c);
            }
            i += n;
        }
        if (units.length >= 4096) flush();
    }
    flush();
    return out;
}

/** `s` as UTF-8, a lone surrogate as its own three bytes; with `cesu8`, a
 * pair as two such. */
export function encodeUtf8(s, cesu8) {
    const out = [];
    for (let i = 0; i < s.length; i++) {
        let c = s.charCodeAt(i);
        if (!cesu8 && c >= 0xd800 && c <= 0xdbff && i + 1 < s.length) {
            const d = s.charCodeAt(i + 1);
            if (d >= 0xdc00 && d <= 0xdfff) {
                c = 0x10000 + ((c - 0xd800) << 10) + (d - 0xdc00);
                i++;
                out.push(0xf0 | (c >> 18), 0x80 | ((c >> 12) & 0x3f), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f));
                continue;
            }
        }
        if (c < 0x80) {
            out.push(c);
        } else if (c < 0x800) {
            out.push(0xc0 | (c >> 6), 0x80 | (c & 0x3f));
        } else {
            out.push(0xe0 | (c >> 12), 0x80 | ((c >> 6) & 0x3f), 0x80 | (c & 0x3f));
        }
    }
    return Uint8Array.from(out);
}

/**
 * One run's DadoScript host: the value table, the contexts, the modules. Made by
 * the loader before the module is instantiated, from the program's
 * `registry.js` namespace, and attached to the instance once it exists.
 */
export class Session {
    /**
     * `options.base` (STOP): what this session's slot ids start from — 0 on
     * the main thread, a multiple of 2^24 of its own on each worker, so a
     * release one thread makes of another's value (a VM stopped and ended
     * from another thread) names no slot of its own; `options.worker`:
     * whether a script here loads its VM's engine word at back-edges.
     */
    constructor(registry, options = {}) {
        this.base = options.base ?? 0; // STOP
        this.values = [undefined];
        this.counts = [0];
        this.released = [];
        this.interned = new Map();
        this.contexts = new Map();
        this.defs = new Map();
        this.modules = new Map();
        for (const [name, ns, imports] of registry.modules) {
            this.modules.set(name, { ns, imports });
        }
        this.registry = registry;
        this.running = 0;
        this.instance = null;
        this.memory = null;
        this.stop = new Stop(this, registry, options.worker === true); // STOP
        registry.__dados_bind({
            print: (s) => this.print("print", s),
            println: (s) => this.print("println", s),
            seam: (module, name, args) => this.seam(module, name, args),
            // ── inline crossings (a16-inline) ──
            settle: () => this.settle(),
            channels: () => this.channels(),
            fail: (label, code) => this.fail(label, code),
            decode: (bytes) => decodeUtf8(bytes),
        });
    }

    // STOP: the running context; a script on a worker reads its VM's word.
    get current() {
        return this.running;
    }

    set current(c) {
        this.running = c;
        this.stop?.enter(c);
    }

    attach(instance, memory) {
        this.instance = instance;
        this.memory = memory;
        // ── inline crossings (a16-inline): the slots cached, the views taken ──
        if (typeof this.registry.__dados_attach === "function") {
            this.registry.__dados_attach(
                instance.exports,
                memory(),
                instance.exports.__indirect_function_table,
                this.registry.modules,
                this.registry.sites === undefined ? 0 : this.registry.sites.length,
            );
        }
    }

    /** The context whose VM is running: what a script's `print` and its
     * calls of Dado reach, and whose counters an inline site raises. */
    get current() {
        return this.running;
    }

    set current(c) {
        this.running = c;
        if (typeof this.registry.__dados_vm === "function") {
            const cx = c === 0 ? undefined : this.contexts.get(c);
            this.registry.__dados_vm(cx === undefined ? null : cx.counters);
        }
    }

    // ── inline crossings (a16-inline) ────────────────────────────────────

    /** The lend lock before an inline crossing, while a run has lent the
     * script a Dado structure: as a thunk's entry, every lend fingerprinted. */
    settle() {
        this.exports.dados_settle(this.current);
    }

    /** The safe point after an inline crossing whose callee can reach
     * `#send`, the any-channel bit set: channels delivered to the running VM's
     * handlers; a stop a handler met is thrown into the script. */
    channels() {
        const c = this.current;
        if (this.exports.dados_channels(c) !== 0) throw this.take(c);
    }

    /** A failable Dado function's failure at an inline site: the `Error` its
     * thunk throws — the message naming the function and the code, `code`
     * the code. */
    fail(label, code) {
        const e = engineError(() => new Error(`\`${label}\` failed with code ${code}`));
        e.code = code;
        return e;
    }

    // ── Dado's direct calls of a script's package function (a17-export) ──

    /** The import `dados_fn.<name>` the module declares with `signature`: the
     * registry's function for it, made over this session, or `null` when the
     * registry has none or has it with another signature. */
    functionImport(name, signature) {
        const row = this.registry.functions?.find((r) => r[0] === name);
        if (row === undefined || row[1] !== signature) return null;
        return row[2](this);
    }

    /** A direct call's `string` argument: the `n` bytes at `p`, read as
     * UTF-8. */
    fnIn(p, n) {
        return decodeUtf8(this.bytes(p, n));
    }

    /** A direct call's `string` answer: its UTF-8 held until `dados.fn_take`
     * writes it, and its byte count. */
    fnOut(text) {
        this.fnText = encodeUtf8(String(text), false);
        return this.fnText.length;
    }

    /** A direct call that threw: the failure as a DadoScript `Error` — its text
     * and its stack on standard error, as a member call's — and its code.
     * The program's end and a stop pass through. */
    fnFailed(e) {
        if (passes(e)) throw e;
        const f = typeof globalThis.__dados_failure === "function" ? globalThis.__dados_failure(e) : e;
        let text = `${String(f)}\n`;
        if (f !== null && typeof f === "object" && typeof f.stack === "string") text += f.stack;
        if (!text.endsWith("\n")) text += "\n";
        this.out?.(2, encodeUtf8(text, false));
        const code = f !== null && typeof f === "object" && Number.isInteger(f.code) ? f.code | 0 : 0;
        return code === 0 ? 70 : code;
    }

    /** A dev build's per-site report: each script's call of Dado, how it
     * crosses and why, and how often it crossed, wrapped a narrowed value and
     * fell back to the thunk. */
    sites() {
        const rows = this.registry.sites === undefined ? [] : this.registry.sites;
        const counts = this.registry.__dados_counts === undefined ? null : this.registry.__dados_counts();
        return rows.map(([at, callee, strategy, reason], k) => ({
            at,
            callee,
            strategy,
            reason,
            calls: counts === null ? 0 : counts.S[k],
            wraps: counts === null ? 0 : counts.W[k],
            fallbacks: counts === null ? 0 : counts.F[k],
        }));
    }

    /** A crossing out of Dado into the script of context `c`, counted. */
    ran(c) {
        const cx = this.contexts.get(c);
        if (cx !== undefined) cx.counters.runs++;
    }

    /** The counters of the VM of context `c` since its frame began. */
    counters(c) {
        const cx = this.contexts.get(c);
        return cx === undefined ? null : cx.counters;
    }

    get exports() {
        return this.instance.exports;
    }

    view() {
        return new DataView(this.memory().buffer);
    }

    bytes(ptr, len) {
        return new Uint8Array(this.memory().buffer, ptr >>> 0, len >>> 0);
    }

    /** The NUL-terminated C string at `ptr`. */
    cstr(ptr) {
        const mem = new Uint8Array(this.memory().buffer);
        let end = ptr >>> 0;
        while (mem[end] !== 0) end++;
        return decodeUtf8(mem.subarray(ptr >>> 0, end));
    }

    // ── the value table ──────────────────────────────────────────────────

    /** A reference to `v`: an object or a symbol C already holds is its
     * slot again, so a slot's index is the object's identity, as an address
     * is natively; a string or a BigInt is a slot of its own. */
    slot(v) {
        const kind = typeof v;
        const identity = kind === "object" || kind === "function" || kind === "symbol";
        if (identity) {
            const known = this.interned.get(v);
            if (known !== undefined) {
                this.counts[known]++;
                return known + this.base;
            }
        }
        const id = this.released.length > 0 ? this.released.pop() : this.values.length;
        this.values[id] = v;
        this.counts[id] = 1;
        if (identity) this.interned.set(v, id);
        return id + this.base;
    }

    dup(slot) {
        const id = slot - this.base;
        if (id > 0 && this.counts[id] > 0) this.counts[id]++;
    }

    release(slot) {
        const id = slot - this.base;
        if (id > 0 && this.counts[id] > 0 && --this.counts[id] === 0) {
            const v = this.values[id];
            if (this.interned.get(v) === id) this.interned.delete(v);
            this.values[id] = undefined;
            this.released.push(id);
        }
    }

    /** How many slots C holds a reference to: what a test counts. */
    live() {
        return this.values.length - 1 - this.released.length;
    }

    /** The value of the `JSValue` at `p`, borrowed. */
    read(p) {
        const dv = this.view();
        const tag = dv.getInt32(p + 8, true);
        switch (tag) {
            case TAG_INT:
                return dv.getInt32(p, true);
            case TAG_FLOAT64:
                return dv.getFloat64(p, true);
            case TAG_BOOL:
                return dv.getInt32(p, true) !== 0;
            case TAG_NULL:
                return null;
            case TAG_UNDEFINED:
            case TAG_UNINITIALIZED:
                return undefined;
            case TAG_STRING:
            case TAG_STRING_ROPE:
            case TAG_OBJECT:
            case TAG_BIG_INT:
            case TAG_SYMBOL:
                return this.values[dv.getInt32(p, true) - this.base];
            case TAG_MODULE:
                return this.defs.get(dv.getInt32(p, true) >>> 0);
            default:
                return undefined;
        }
    }

    /** Write `tag` and a 32-bit payload. */
    put(p, tag, payload) {
        const dv = this.view();
        dv.setInt32(p, payload, true);
        dv.setInt32(p + 4, 0, true);
        dv.setInt32(p + 8, tag, true);
        dv.setInt32(p + 12, tag < 0 ? -1 : 0, true);
    }

    /** Write `v` as a `JSValue` at `p`: a new reference C owns. */
    write(p, v) {
        switch (typeof v) {
            case "number":
                if ((v | 0) === v && !Object.is(v, -0)) {
                    this.put(p, TAG_INT, v);
                } else {
                    const dv = this.view();
                    dv.setFloat64(p, v, true);
                    dv.setInt32(p + 8, TAG_FLOAT64, true);
                    dv.setInt32(p + 12, 0, true);
                }
                return;
            case "boolean":
                this.put(p, TAG_BOOL, v ? 1 : 0);
                return;
            case "undefined":
                this.put(p, TAG_UNDEFINED, 0);
                return;
            case "string":
                this.put(p, TAG_STRING, this.slot(v));
                return;
            case "bigint":
                this.put(p, TAG_BIG_INT, this.slot(v));
                return;
            case "symbol":
                this.put(p, TAG_SYMBOL, this.slot(v));
                return;
            default:
                if (v === null) {
                    this.put(p, TAG_NULL, 0);
                } else {
                    this.put(p, TAG_OBJECT, this.slot(v));
                }
        }
    }

    /** The slot `write` made at `p`, if it made one: what to release. */
    madeSlot(p) {
        const dv = this.view();
        const tag = dv.getInt32(p + 8, true);
        if (tag === TAG_STRING || tag === TAG_OBJECT || tag === TAG_BIG_INT || tag === TAG_SYMBOL) {
            return dv.getInt32(p, true);
        }
        return 0;
    }

    // ── contexts and exceptions ──────────────────────────────────────────

    context(c) {
        const cx = this.contexts.get(c);
        if (cx === undefined) {
            throw new Error(`dados: no context at ${c}`);
        }
        return cx;
    }

    raise(c, e) {
        this.context(c).exception = e;
    }

    /** Write the exception marker at `out`, with `e` pending on `c`. */
    failed(c, out, e) {
        if (passes(e)) throw e;
        this.raise(c, e);
        this.put(out, TAG_EXCEPTION, 0);
    }

    /** Run `f` with `c` the current context. */
    within(c, f) {
        const saved = this.current;
        this.current = c;
        try {
            return f();
        } finally {
            this.current = saved;
        }
    }

    /** `f()`'s answer written at `out`, or its throw pending on `c`. */
    answer(c, out, f) {
        let v;
        try {
            v = this.within(c, f);
        } catch (e) {
            this.failed(c, out, e);
            return;
        }
        this.write(out, v);
    }

    /** `f()`'s status — 1 — or -1 with its throw pending on `c`. */
    status(c, f) {
        try {
            this.within(c, f);
            return 1;
        } catch (e) {
            if (passes(e)) throw e;
            this.raise(c, e);
            return -1;
        }
    }

    // ── C functions ──────────────────────────────────────────────────────

    /** A JavaScript function calling the C function in table slot `fn`. */
    cfunction(c, fn, name, length) {
        const session = this;
        const f = function (...args) {
            return session.callC(c, fn, this, args, length);
        };
        Object.defineProperty(f, "name", { value: name });
        Object.defineProperty(f, "length", { value: length });
        return f;
    }

    /** A call of the C function in slot `fn`: its frame laid out in memory C
     * allocated — `this`, the arguments padded with `undefined` to the
     * function's length, the answer — and the call made through the exported
     * trampoline. */
    callC(c, fn, thisValue, args, length) {
        const n = Math.max(args.length, length);
        const frame = this.exports.dados_alloc(c, 16 * (n + 2)) >>> 0;
        if (frame === 0) {
            const e = new Error("out of memory");
            e.name = "InternalError";
            throw e;
        }
        const made = [];
        const lay = (at, v) => {
            this.write(at, v);
            const id = this.madeSlot(at);
            if (id !== 0) made.push(id);
        };
        lay(frame, thisValue);
        for (let i = 0; i < n; i++) {
            lay(frame + 16 * (i + 1), i < args.length ? args[i] : undefined);
        }
        const out = frame + 16 * (n + 1);
        const saved = this.current;
        this.current = c;
        try {
            this.exports.dados_call(fn, c, frame, args.length, frame + 16, out);
        } finally {
            this.current = saved;
            for (const id of made) this.release(id);
        }
        const tag = this.view().getInt32(out + 8, true);
        const value = this.read(out);
        const owned = this.madeSlot(out);
        this.exports.dados_free(c, frame);
        if (tag === TAG_EXCEPTION) {
            throw this.take(c);
        }
        if (owned !== 0) this.release(owned);
        return value;
    }

    /** The exception pending on `c`, taken. */
    take(c) {
        const cx = this.context(c);
        const e = cx.exception;
        cx.exception = NONE;
        return e === NONE ? undefined : e;
    }

    /** A C string of `s` in memory C allocated for `c`; the caller frees it. */
    alloc(c, s) {
        const b = encodeUtf8(s, false);
        const p = this.exports.dados_alloc(c, b.length + 1) >>> 0;
        if (p === 0) throw new Error("dados: out of memory");
        const mem = new Uint8Array(this.memory().buffer);
        mem.set(b, p);
        mem[p + b.length] = 0;
        return p;
    }

    // ── modules ──────────────────────────────────────────────────────────

    /** Link the module `name` for `c`, as the engine would: through the
     * program's normaliser and loader, everything it imports first, a C
     * module's init run. Answers its definition, or null with an exception
     * pending on `c`. */
    link(c, name, base) {
        const cx = this.context(c);
        const known = cx.linked.get(name);
        if (known !== undefined) return known;
        const bp = this.alloc(c, base);
        const np = this.alloc(c, name);
        const normal = this.exports.dados_normalize(c, bp, np) >>> 0;
        this.exports.dados_free(c, bp);
        this.exports.dados_free(c, np);
        if (normal === 0) return null;
        const resolved = this.cstr(normal);
        this.exports.dados_free(c, normal);
        const already = cx.linked.get(resolved);
        if (already !== undefined) return already;
        const rp = this.alloc(c, resolved);
        const defp = this.exports.dados_load(c, rp) >>> 0;
        this.exports.dados_free(c, rp);
        if (defp === 0) return null;
        const def = this.defs.get(defp);
        cx.linked.set(resolved, def);
        cx.linked.set(name, def);
        if (def.isC) {
            if (this.exports.dados_init(c, defp) < 0) {
                if (cx.exception === NONE) this.raise(c, new Error(`init of '${resolved}' failed`));
                return null;
            }
        } else {
            for (const imported of this.modules.get(resolved).imports) {
                if (this.link(c, imported, resolved) === null) return null;
            }
            // The runtime installs `__dados_failure` as it is evaluated: from
            // here a host reading it on this VM's global finds it.
            if (resolved === "dados:runtime") {
                cx.global.__dados_failure = globalThis.__dados_failure;
            }
        }
        return def;
    }

    namespace(def) {
        if (def.isC) {
            const ns = Object.create(null);
            for (const [k, v] of def.exports) ns[k] = v;
            return ns;
        }
        return def.ns;
    }

    // ── what dados/web.js calls ───────────────────────────────────────────

    print(which, text) {
        const c = this.current;
        const f = c !== 0 ? this.contexts.get(c)?.global[which] : undefined;
        if (typeof f !== "function") {
            throw new Error(`a script called \`${which}\` with no VM running it`);
        }
        return f(text);
    }

    seam(module, name, args) {
        const c = this.current;
        if (c === 0 || !this.contexts.has(c)) {
            throw new Error(`the Dado function \`${name}\` of \`${module}\` was called with no VM running`);
        }
        const def = this.within(c, () => this.link(c, module, ""));
        if (def === null) throw this.take(c);
        const f = def.exports.get(name);
        if (typeof f !== "function") {
            throw new ReferenceError(`'${module}' exports no '${name}'`);
        }
        return f(...args);
    }
}

// ── runs of scalars ─────────────────────────────────────────────────────────
//
// What `dados.read_run` and `dados.array_from` copy: a run of scalars between a
// script's Array and C storage, `kind` naming the storage as the generated
// glue does (`dado_rt__web_run_kind`): 0 i8, 1 u8, 2 i16, 3 u16, 4 i32, 5 u32,
// 6 i64, 7 u64, 8 f32, 9 f64, 10 bool. In, a typed view's `set` (one bulk
// copy) once every element is one the glue's reader takes without converting
// or raising — an integer Number within 2^53, any Number, a boolean — and
// otherwise nothing, so the glue reads element by element as before. Out, a
// sized Array filled by a loop, an int in its canonical tier.

const RUN_VIEWS = [Int8Array, Uint8Array, Int16Array, Uint16Array, Int32Array, Uint32Array,
    null, null, Float32Array, Float64Array, Uint8Array];

function writeRun(buffer, kind, dst, a, n) {
    const View = RUN_VIEWS[kind];
    if (kind === 10) {
        for (let i = 0; i < n; i++) if (typeof a[i] !== "boolean") return false;
        new View(buffer, dst, n).set(a);
        return true;
    }
    if (kind === 8 || kind === 9) {
        for (let i = 0; i < n; i++) if (typeof a[i] !== "number") return false;
        new View(buffer, dst, n).set(a);
        return true;
    }
    for (let i = 0; i < n; i++) {
        const x = a[i];
        if (typeof x !== "number" || !Number.isSafeInteger(x)) return false;
    }
    if (View !== null && View !== undefined) {
        // A typed array's store keeps the low bits, as C's narrowing cast does.
        new View(buffer, dst, n).set(a);
        return true;
    }
    if (kind !== 6 && kind !== 7) return false;
    const w = new Int32Array(buffer, dst, 2 * n);
    for (let i = 0; i < n; i++) {
        const x = a[i];
        const hi = Math.floor(x / 4294967296);
        w[2 * i] = x - hi * 4294967296;
        w[2 * i + 1] = hi;
    }
    return true;
}

function readRun(buffer, kind, src, n) {
    const out = new Array(n);
    if (kind === 10) {
        const b = new Uint8Array(buffer, src, n);
        for (let i = 0; i < n; i++) out[i] = b[i] !== 0;
        return out;
    }
    if (kind === 6 || kind === 7) {
        const w = new Int32Array(buffer, src, 2 * n);
        for (let i = 0; i < n; i++) {
            const lo = w[2 * i] >>> 0, hi = w[2 * i + 1];
            const v = hi * 4294967296 + lo;
            out[i] = hi >= -2097152 && hi < 2097152 && v !== -9007199254740992
                ? v
                : BigInt.asIntN(64, (BigInt(hi) << 32n) | BigInt(lo));
        }
        return out;
    }
    const t = new RUN_VIEWS[kind](buffer, src, n);
    for (let i = 0; i < n; i++) out[i] = t[i];
    return out;
}

/** The import every `dados` host call is: `make(session)` answers it. A
 * worker of a `--web-threads` module has a session of its own (STOP: the
 * loader's `workerSession`, from the registry's URL); one the loader could
 * not give a session — it was handed the registry as an object, which a
 * worker cannot receive — has none, and there each call traps, saying so,
 * rather than leaving its joiner waiting. */
function entry(signature, make) {
    return {
        signature,
        make: (ctx) => {
            if (ctx.dados !== undefined) return make(ctx.dados);
            return () => {
                ctx.out(2, encodeUtf8(
                    "dados: a script was reached from a worker thread that has no session of its own: give the loader the registry's URL (`scripts`), which a worker imports, not its namespace (`registry`)\n",
                    false,
                ));
                throw new WebAssembly.RuntimeError("dados: a script reached from a worker thread");
            };
        },
    };
}

/**
 * **What this file supplies**: the `dados` host calls, in `load.mjs`'s `HOST`
 * shape, each a function of the loader's context, whose `dados` is the run's
 * `Session`; and the runtime's `dado.exit`.
 */
export const DADOS_HOST = {
    "dado.exit": {
        signature: "(i32)->()",
        make: () => (status) => {
            throw new DadoExit(status);
        },
    },
    "dados.ctx_new": entry("(i32)->()", (s) => (c) => {
        s.contexts.set(c, {
            global: Object.create(null),
            exception: NONE,
            linked: new Map(),
            counters: { calls: 0, bytes_in: 0, bytes_out: 0, runs: 0 },
        });
    }),
    "dados.counters": entry("(i32,i32)->()", (s) => (c, out) => {
        const n = s.counters(c) ?? { calls: 0, bytes_in: 0, bytes_out: 0 };
        const dv = s.view();
        dv.setBigInt64(out, BigInt(n.calls), true);
        dv.setBigInt64(out + 8, BigInt(n.bytes_in), true);
        dv.setBigInt64(out + 16, BigInt(n.bytes_out), true);
    }),
    "dados.frame": entry("(i32)->()", (s) => (c) => {
        const n = s.counters(c);
        if (n !== null) {
            n.calls = 0;
            n.bytes_in = 0;
            n.bytes_out = 0;
            n.runs = 0;
        }
    }),
    // A direct call's `string` answer: the bytes the import encoded, written
    // at `into`.
    "dados.fn_take": entry("(i32)->()", (s) => (into) => {
        const b = s.fnText ?? new Uint8Array(0);
        new Uint8Array(s.memory().buffer, into >>> 0, b.length).set(b);
        s.fnText = null;
    }),
    "dados.hold": entry("(i32)->()", (s) => (delta) => {
        if (typeof s.registry.__dados_hold === "function") s.registry.__dados_hold(delta);
    }),
    "dados.ctx_free": entry("(i32)->()", (s) => (c) => {
        s.contexts.delete(c);
        for (const [p, def] of s.defs) {
            if (def.ctx === c) s.defs.delete(p);
        }
    }),
    "dados.dup": entry("(i32)->()", (s) => (id) => s.dup(id)),
    "dados.free": entry("(i32)->()", (s) => (id) => s.release(id)),
    "dados.string": entry("(i32,i32,i32,i32)->()", (s) => (c, p, len, out) => {
        s.write(out, decodeUtf8(s.bytes(p, len)));
    }),
    "dados.string16": entry("(i32,i32,i32,i32)->()", (s) => (c, p, len, out) => {
        const dv = s.view();
        let text = "";
        for (let i = 0; i < len; i++) text += String.fromCharCode(dv.getUint16(p + 2 * i, true));
        s.write(out, text);
    }),
    "dados.bigint": entry("(i32,i64,i32)->()", (s) => (c, v, out) => s.write(out, BigInt.asIntN(64, v))),
    "dados.array": entry("(i32,i32)->()", (s) => (c, out) => s.write(out, [])),
    "dados.to_bigint": entry("(i32,i32,i32)->(i32)", (s) => (c, vp, where) => {
        const v = s.read(vp);
        let r;
        if (typeof v === "bigint") {
            r = BigInt.asIntN(64, v);
        } else if (typeof v === "number" && Number.isInteger(v)) {
            r = BigInt.asIntN(64, BigInt(v));
        } else {
            s.raise(c, new TypeError("cannot convert to bigint"));
            return -1;
        }
        s.view().setBigInt64(where, r, true);
        return 0;
    }),
    "dados.to_cstring": entry("(i32,i32,i32,i32)->(i32)", (s) => (c, vp, lenp, cesu8) => {
        const v = s.read(vp);
        let text;
        try {
            text = s.within(c, () => (typeof v === "symbol" ? v.toString() : String(v)));
        } catch (e) {
            if (passes(e)) throw e;
            s.raise(c, e);
            return 0;
        }
        const b = encodeUtf8(text, cesu8 !== 0);
        const p = s.exports.dados_alloc(c, b.length + 1) >>> 0;
        if (p === 0) {
            const e = new Error("out of memory");
            e.name = "InternalError";
            s.raise(c, e);
            return 0;
        }
        const mem = new Uint8Array(s.memory().buffer);
        mem.set(b, p);
        mem[p + b.length] = 0;
        s.view().setInt32(lenp, b.length, true);
        return p;
    }),
    "dados.to_utf16": entry("(i32,i32,i32)->(i32)", (s) => (c, vp, lenp) => {
        const v = s.read(vp);
        let text;
        try {
            text = s.within(c, () => String(v));
        } catch (e) {
            if (passes(e)) throw e;
            s.raise(c, e);
            return 0;
        }
        const p = s.exports.dados_alloc(c, 2 * text.length + 2) >>> 0;
        if (p === 0) {
            s.raise(c, new Error("out of memory"));
            return 0;
        }
        const dv = s.view();
        for (let i = 0; i < text.length; i++) dv.setUint16(p + 2 * i, text.charCodeAt(i), true);
        dv.setUint16(p + 2 * text.length, 0, true);
        dv.setInt32(lenp, text.length, true);
        return p;
    }),
    "dados.is": entry("(i32,i32)->(i32)", (s) => (vp, what) => {
        const v = s.read(vp);
        switch (what) {
            case 0:
                return typeof v === "function" ? 1 : 0;
            case 1:
                return Array.isArray(v) ? 1 : 0;
            case 2:
                return v instanceof Map ? 1 : 0;
            default:
                return v instanceof Error ? 1 : 0;
        }
    }),
    "dados.length": entry("(i32,i32,i32)->(i32)", (s) => (c, vp, where) => {
        let n = 0;
        const r = s.status(c, () => {
            const len = Number(s.read(vp).length);
            n = Number.isNaN(len) || len <= 0 ? 0 : Math.min(Math.floor(len), Number.MAX_SAFE_INTEGER);
        });
        if (r < 0) return -1;
        s.view().setBigInt64(where, BigInt(n), true);
        return 0;
    }),
    "dados.global": entry("(i32,i32)->()", (s) => (c, out) => s.write(out, s.context(c).global)),
    "dados.get": entry("(i32,i32,i32,i32)->()", (s) => (c, op, np, out) => {
        const o = s.read(op);
        const name = s.cstr(np);
        s.answer(c, out, () => o[name]);
    }),
    "dados.set": entry("(i32,i32,i32,i32)->(i32)", (s) => (c, op, np, vp) => {
        const o = s.read(op);
        const name = s.cstr(np);
        const v = s.read(vp);
        return s.status(c, () => {
            o[name] = v;
        });
    }),
    "dados.define": entry("(i32,i32,i32,i32,i32)->(i32)", (s) => (c, op, np, vp, flags) => {
        const o = s.read(op);
        const name = s.cstr(np);
        const v = s.read(vp);
        return s.status(c, () => {
            Object.defineProperty(o, name, {
                value: v,
                configurable: (flags & 1) !== 0,
                writable: (flags & 2) !== 0,
                enumerable: (flags & 4) !== 0,
            });
        });
    }),
    "dados.get_index": entry("(i32,i32,i64,i32)->()", (s) => (c, op, idx, out) => {
        const o = s.read(op);
        s.answer(c, out, () => o[Number(idx)]);
    }),
    "dados.set_index": entry("(i32,i32,i64,i32)->(i32)", (s) => (c, op, idx, vp) => {
        const o = s.read(op);
        const v = s.read(vp);
        return s.status(c, () => {
            o[Number(idx)] = v;
        });
    }),
    "dados.read_run": entry("(i32,i32,i32,i32,i64)->(i32)", (s) => (c, vp, kind, dst, n) => {
        const a = s.read(vp);
        const len = Number(n);
        if (!Array.isArray(a) || a.length !== len) return 0;
        return writeRun(s.memory().buffer, kind, dst >>> 0, a, len) ? 1 : 0;
    }),
    "dados.array_from": entry("(i32,i32,i32,i64,i32)->()", (s) => (c, kind, src, n, out) => {
        s.write(out, readRun(s.memory().buffer, kind, src >>> 0, Number(n)));
    }),
    "dados.call": entry("(i32,i32,i32,i32,i32,i32)->()", (s) => (c, fp, tp, argc, argv, out) => {
        s.ran(c);
        const f = s.read(fp);
        const t = s.read(tp);
        const args = [];
        for (let i = 0; i < argc; i++) args.push(s.read(argv + 16 * i));
        s.answer(c, out, () => Reflect.apply(f, t, args));
    }),
    "dados.construct": entry("(i32,i32,i32,i32,i32)->()", (s) => (c, fp, argc, argv, out) => {
        s.ran(c);
        const f = s.read(fp);
        const args = [];
        for (let i = 0; i < argc; i++) args.push(s.read(argv + 16 * i));
        s.answer(c, out, () => Reflect.construct(f, args));
    }),
    "dados.cfunction": entry("(i32,i32,i32,i32,i32,i32)->()", (s) => (c, fn, np, length, magic, out) => {
        s.write(out, s.cfunction(c, fn, s.cstr(np), length));
    }),
    "dados.exception": entry("(i32,i32)->()", (s) => (c, out) => {
        const cx = s.context(c);
        if (cx.exception === NONE) {
            s.put(out, TAG_UNINITIALIZED, 0);
            return;
        }
        const e = cx.exception;
        cx.exception = NONE;
        s.write(out, e);
    }),
    "dados.throw": entry("(i32,i32)->()", (s) => (c, vp) => s.raise(c, s.read(vp))),
    "dados.error": entry("(i32,i32,i32,i32)->()", (s) => (c, kind, mp, len) => {
        const message = decodeUtf8(s.bytes(mp, len));
        const e = engineError(() => {
            if (kind === 1) return new TypeError(message);
            if (kind === 2) return new ReferenceError(message);
            const plain = new Error(message);
            if (kind === 3) plain.name = "InternalError";
            return plain;
        });
        s.raise(c, e);
    }),
    "dados.module_new": entry("(i32,i32,i32,i32)->(i32)", (s) => (c, defp, np, isC) => {
        const name = s.cstr(np);
        const known = s.modules.get(name);
        if (isC === 0 && known === undefined) {
            s.raise(c, new ReferenceError(`could not load module '${name}': the build wrote no file for it`));
            return 0;
        }
        s.defs.set(defp >>> 0, {
            ctx: c,
            name,
            isC: isC !== 0,
            exports: new Map(),
            declared: [],
            ns: known === undefined ? null : known.ns,
        });
        return 1;
    }),
    "dados.module_export": entry("(i32,i32,i32)->(i32)", (s) => (c, defp, np) => {
        s.defs.get(defp >>> 0).declared.push(s.cstr(np));
        return 0;
    }),
    "dados.module_set": entry("(i32,i32,i32,i32)->(i32)", (s) => (c, defp, np, vp) => {
        const def = s.defs.get(defp >>> 0);
        const name = s.cstr(np);
        if (!def.declared.includes(name)) {
            s.raise(c, new ReferenceError(`'${def.name}' declares no export '${name}'`));
            return -1;
        }
        def.exports.set(name, s.read(vp));
        return 0;
    }),
    "dados.eval_module": entry("(i32,i32,i32)->()", (s) => (c, defp, out) => {
        const def = s.defs.get(defp >>> 0);
        const linked = s.within(c, () => s.link(c, def.name, ""));
        if (linked === null) {
            s.write(out, new Settled(REJECTED, s.take(c)));
            return;
        }
        s.write(out, new Settled(FULFILLED, undefined));
    }),
    "dados.load_module": entry("(i32,i32,i32,i32)->()", (s) => (c, bp, np, out) => {
        const base = bp === 0 ? "" : s.cstr(bp);
        const name = s.cstr(np);
        const def = s.within(c, () => s.link(c, name, base));
        if (def === null) {
            s.put(out, TAG_EXCEPTION, 0);
            return;
        }
        s.write(out, new Settled(FULFILLED, s.namespace(def)));
    }),
    "dados.namespace": entry("(i32,i32,i32)->()", (s) => (c, defp, out) => {
        s.write(out, s.namespace(s.defs.get(defp >>> 0)));
    }),
    "dados.promise_state": entry("(i32,i32)->(i32)", (s) => (c, vp) => {
        const v = s.read(vp);
        if (v instanceof Settled) return v.state;
        if (v instanceof Promise) return 0;
        return -1;
    }),
    "dados.promise_result": entry("(i32,i32,i32)->()", (s) => (c, vp, out) => {
        const v = s.read(vp);
        s.write(out, v instanceof Settled ? v.value : undefined);
    }),
};

/** Whether a manifest's module runs scripts: it imports from `dados`. */
export function runsScripts(manifest) {
    return manifest.split("\n").some((line) => line.startsWith("wasm-import dados "));
}

/** Where `./dado build --target=web` writes a module's scripts: beside it, in
 * `<module without .wasm>.scripts/`, whose `registry.js` lists them. */
export function registryPath(wasmPath) {
    return `${wasmPath.replace(/\.wasm$/, "")}.scripts/registry.js`;
}

/** The registry of a program that carries no script. */
export const NO_SCRIPTS = { modules: [], sites: [], __dados_bind() {} };
