// load.mjs — run a `#WEB` module built by `./dado build --target=web`.
//
// Scaffolding: the loader DadoScript's generated glue replaces. It exists so a web
// build can be run and tested today, and it does the least that takes: read
// the module, check that every import the compiler declared is one this file
// supplies, instantiate, call `main`, flush the runtime's console, and turn
// what happened into an exit status the way a native run does.
//
// It is JavaScript because it runs in the one host Dado cannot run in: the
// page (and node, standing in for one). Everything else in the toolchain is
// Dado; this is the other side of the module's imports.
//
// Usable two ways, from one file:
//
//     node tools/web/load.mjs build/corpus/tape.wasm        # exit status 105
//
//     import { run } from "./load.mjs";                      # a page, or node
//     const status = await run(bytes, { manifest: text });
//
// **The manifest.** `--emit=build` prints one `wasm-import <module> <name>
// <signature>` line per function the module takes from its host and one
// `wasm-export <name> <signature>` per name it gives back. `./dado build
// --target=web` writes that output beside the module as `<module>.build`, and
// this reads it from there unless told otherwise. Every `wasm-import` line
// must name an entry of `HOST` below with the same signature, and every import
// the module itself declares must be on a line — checked before anything
// runs, so a missing import is a sentence naming it rather than a
// `LinkError` or a call into `undefined`.
//
// **The status.** A return from `main` is its value, reduced to the eight bits
// a process status holds, as a native run's is. A trap — a failed `#assert`,
// a `#panic`, an `abort`, an out-of-bounds access — is 134, the status a shell
// reports for SIGABRT, so `@test 134` means the same thing on both. A loader
// failure (a missing import, a bad manifest) is 2, before the module runs.
//
// **Threads** (a module built with `./dado --target=web --web-threads`). Such
// a module imports its memory (`wasm-import env memory memory`), and this
// creates it shared, at the limits the module declares. `dado.spawn(slot,
// arg)` starts a worker — node's `worker_threads`, a page's `Worker`, each
// running this same file — that instantiates the same module on the same
// memory, points its `__stack_pointer` at the first word of the record `arg`
// names (the stack the runtime allocated), and calls the function-table
// entry `slot` with `arg`. A worker that traps writes 2 into the record's
// second word and wakes it, so a `join` does not wait on it forever; under
// node it then ends the process with SIGABRT, status 134, which is what an
// `abort` on a native thread does. As natively, the program ends when `main`
// returns, whatever its threads are doing. A worker writes its output with
// `fs.writeSync`, which does not pass through the main thread — which may be
// blocked in a `join`.
//
// **A page** serving a threaded module must be cross-origin isolated, or it
// has no `SharedArrayBuffer` and so no shared memory: the document is served
// with `Cross-Origin-Opener-Policy: same-origin` and
// `Cross-Origin-Embedder-Policy: require-corp`. A page's main thread may not
// wait (`memory.atomic.wait32` traps there), so `dado.can_wait` answers 0 on it
// and the runtime refuses a `join` there with that reason; run `main` in a
// worker to join.

// **The host services** (`dado.arg_get`, `dado.env_*`, `dado.random_bytes`,
// `dado.is_terminal`, `dado.host_name`, `dado.fs_*`): what the runtime's
// `rt/web/host.c` wraps, the `dado/web.h` a `core:` package restates. Real
// where the host offers it, else `UNSUPPORTED` (-1): under node all of them;
// in a page the arguments and the environment come from the page's address
// (`?dado-arg=…` repeated, `?dado-env=NAME=value` repeated), random bytes are
// real, `is_terminal` is 0, and the host's name, the environment's writes and
// the files are unsupported. A failure is one of the small set of codes
// `SYS` lists, which `host.c` documents. A module whose `main` takes
// `(argc, argv)` exports `dado_argv`, which builds the argv in its own memory
// from `dado.arg_get`; this calls it and hands `main` the arguments —
// `node load.mjs m.wasm -- a b` (argv[0] is the module's path).

// ── DadoScript (a16-web) ──────────────────────────────────────────────────────────
// A module that runs DadoScript scripts imports the `dados` host calls and
// `dado.exit`, which `dados.mjs` supplies; its scripts are the modules the build
// wrote beside it, listed by their `registry.js`. This section and the four
// lines marked `DadoScript` below are all `load.mjs` knows of them.
import { NO_SCRIPTS, Session, DADOS_HOST, DadoExit, registryPath, runsScripts } from "./dados.mjs";
// ─────────────────────────────────────────────────────────────────────────────

/** The status a trap maps to: SIGABRT's shell status. */
export const TRAP_STATUS = 134;

/** The status a loader failure maps to, before the module runs. */
export const LOADER_STATUS = 2;

/** The runtime's console flush, which the module exports when it writes. */
export const FLUSH_EXPORT = "dado_flush";

/** The report of the run-time narrowing warning's counts, which a dev build
 * exports when the program narrows an `int` or a `float` at run time. */
export const NARROWINGS_EXPORT = "dado_narrowings";

/** A failure of the loader itself, reported before the module runs. */
export class LoaderError extends Error {}

// **What this loader supplies**, one entry per import: its module and name,
// the signature `--emit=build` spells for it, and a function of the running
// module's context that answers the import. Adding a host call is adding one
// entry here and nothing else.
//
// `ctx.memory()` is the module's exported memory (read after instantiation,
// so a grown memory is seen); `ctx.out(fd, bytes)` writes bytes to fd 1 or 2;
// `ctx.seed` is the pinned hash seed or `null`; `ctx.spawn(slot, arg)` starts
// a worker and answers 0, or nonzero when none could be started;
// `ctx.canWait` is whether this thread may block.
const HOST = {
    "dado.write": {
        signature: "(i32,i32,i32)->()",
        make: (ctx) => (fd, ptr, len) => {
            ctx.out(fd, new Uint8Array(ctx.memory().buffer, ptr >>> 0, len >>> 0));
        },
    },
    "dado.now_ns": {
        signature: "()->(i64)",
        make: () => () => BigInt(Math.round(performance.now() * 1e6)),
    },
    "dado.wall_ns": {
        signature: "()->(i64)",
        make: () => () => BigInt(Date.now()) * 1000000n,
    },
    "dado.seed": {
        signature: "()->(i64)",
        make: (ctx) => () => {
            if (ctx.seed !== null) {
                return BigInt.asIntN(64, BigInt(ctx.seed));
            }
            const words = new Uint32Array(2);
            crypto.getRandomValues(words);
            return BigInt.asIntN(64, (BigInt(words[0]) << 32n) | BigInt(words[1]));
        },
    },
    "dado.spawn": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => (slot, arg) => ctx.spawn(slot >>> 0, arg >>> 0),
    },
    "dado.can_wait": {
        signature: "()->(i32)",
        make: (ctx) => () => (ctx.canWait ? 1 : 0),
    },
    "dado.cores": {
        signature: "()->(i32)",
        make: () => () => hostCores,
    },
    // A dev build's run-time narrowing warning: one line, which the page's
    // console shows as a warning (node writes it to standard error).
    "dado.warn": {
        signature: "(i32,i32)->()",
        make: (ctx) => (ptr, len) => {
            console.warn(new TextDecoder().decode(new Uint8Array(ctx.memory().buffer, ptr >>> 0, len >>> 0).slice()));
        },
    },
};

/** **The counts of the run-time narrowing warning**, as the program ends: each
 * place the program's Dado narrowed with a loss more than once, and each one
 * its scripts did (dev builds both). */
function reportNarrowings(instance) {
    if (typeof instance.exports[NARROWINGS_EXPORT] === "function") {
        instance.exports[NARROWINGS_EXPORT]();
    }
    const scripts = globalThis.__dados_narrowing_reports;
    if (Array.isArray(scripts)) {
        for (const report of scripts) report();
    }
}

// ── the host services (`rt/web/host.c`) ────────────────────────────────────

/** The error codes of the host services: `rt/web/host.c` documents them. */
export const SYS = Object.freeze({
    UNSUPPORTED: -1,
    NOT_FOUND: -2,
    EXISTS: -3,
    DENIED: -4,
    NOT_DIR: -5,
    IS_DIR: -6,
    NOT_EMPTY: -7,
    BAD_FD: -8,
    INVALID: -9,
    NO_SPACE: -10,
    IO: -11,
    TOO_MANY: -12,
});

/** The code a node error is answered with. */
function sysCode(e) {
    switch (e?.code) {
        case "ENOENT":
            return SYS.NOT_FOUND;
        case "EEXIST":
            return SYS.EXISTS;
        case "EACCES":
        case "EPERM":
        case "EROFS":
            return SYS.DENIED;
        case "ENOTDIR":
            return SYS.NOT_DIR;
        case "EISDIR":
            return SYS.IS_DIR;
        case "ENOTEMPTY":
            return SYS.NOT_EMPTY;
        case "EBADF":
            return SYS.BAD_FD;
        case "EINVAL":
        case "ENAMETOOLONG":
        case "ELOOP":
        case "ESPIPE":
        case "ERR_INVALID_ARG_TYPE":
        case "ERR_INVALID_ARG_VALUE":
        case "ERR_OUT_OF_RANGE":
            return SYS.INVALID;
        case "ENOSPC":
        case "EDQUOT":
            return SYS.NO_SPACE;
        case "EMFILE":
        case "ENFILE":
            return SYS.TOO_MANY;
        case "ERR_WORKER_UNSUPPORTED_OPERATION":
            return SYS.UNSUPPORTED;
        default:
            return e instanceof RangeError || e instanceof TypeError ? SYS.INVALID : SYS.IO;
    }
}

/** A view of `len` bytes of the module's memory at `ptr`. */
function bytesAt(ctx, ptr, len) {
    return new Uint8Array(ctx.memory().buffer, ptr >>> 0, len >>> 0);
}

/** `len` bytes at `ptr`, copied out (a shared memory's view cannot be decoded). */
function copyAt(ctx, ptr, len) {
    return bytesAt(ctx, ptr, len).slice();
}

const utf8Out = new TextEncoder();
const utf8In = new TextDecoder();

/** Text at `(ptr, len)`, decoded as UTF-8. */
function textAt(ctx, ptr, len) {
    return utf8In.decode(copyAt(ctx, ptr, len));
}

/** A path at `(ptr, len)`: its bytes, as node takes a path that may not be UTF-8. */
function pathAt(ctx, ptr, len) {
    return Buffer.from(copyAt(ctx, ptr, len));
}

/** Write `bytes` (a Uint8Array, or text as UTF-8) into `(buf, cap)`: at most
 * `cap` bytes, answering the whole length. */
function give(ctx, bytes, buf, cap) {
    const b = typeof bytes === "string" ? utf8Out.encode(bytes) : bytes;
    const n = Math.min(b.length, cap >>> 0);
    if (n > 0) {
        bytesAt(ctx, buf, n).set(b.subarray(0, n));
    }
    return b.length;
}

/** Entries, each followed by a NUL byte, as one run of bytes. */
function nulList(entries) {
    const parts = entries.map((e) => (typeof e === "string" ? utf8Out.encode(e) : e));
    const out = new Uint8Array(parts.reduce((n, p) => n + p.length + 1, 0));
    let at = 0;
    for (const p of parts) {
        out.set(p, at);
        at += p.length + 1;
    }
    return out;
}

/** A host call answering a failure's code rather than throwing. */
const sys = (f) => (...args) => {
    try {
        return f(...args);
    } catch (e) {
        return sysCode(e);
    }
};

/** A node-only host call: unsupported anywhere else. */
const nodeOnly = (f) => (NODE ? sys(f) : () => SYS.UNSUPPORTED);

/** Whether `name` can name an environment variable. */
const envName = (name) => name !== "" && !name.includes("=") && !name.includes("\0");

/** Sleep `ms` on this thread (node may wait on every thread). */
function nap(ms) {
    Atomics.wait(new Int32Array(new SharedArrayBuffer(4)), 0, 0, ms);
}

/** Node's positional read or write, at `pos` (-1: the descriptor's own
 * position), falling back to its own position where it cannot seek, and
 * waiting out a descriptor that is not ready (a non-blocking standard input). */
function moveBytes(op, fd, view, pos) {
    let at = pos < 0n ? null : Number(pos);
    for (;;) {
        try {
            return op(fd, view, 0, view.length, at);
        } catch (e) {
            if (e.code === "ESPIPE" && at !== null) {
                at = null;
            } else if (e.code === "EAGAIN") {
                nap(1);
            } else {
                throw e;
            }
        }
    }
}

/** Write node's (bigint) stat into the 24-byte record at `out`. */
function writeStat(ctx, out, st) {
    const view = new DataView(ctx.memory().buffer, out >>> 0, 24);
    view.setBigInt64(0, BigInt(st.size), true);
    view.setBigInt64(8, BigInt(st.mtimeNs), true);
    const kind = st.isFile() ? 1 : st.isDirectory() ? 2 : st.isSymbolicLink() ? 3 : 0;
    view.setInt32(16, kind, true);
    view.setInt32(20, Number(BigInt(st.mode) & 0o7777n), true);
    return 0;
}

/** Node's open flags for the runtime's (`host.c`). */
function openFlags(flags) {
    const c = nodeFs.constants;
    const writes = (flags & 2) !== 0 || (flags & 4) !== 0;
    let f = writes ? ((flags & 1) !== 0 ? c.O_RDWR : c.O_WRONLY) : c.O_RDONLY;
    if (flags & 4) f |= c.O_APPEND;
    if (flags & 8) f |= c.O_CREAT;
    if (flags & 16) f |= c.O_TRUNC;
    if (flags & 32) f |= c.O_EXCL;
    return f;
}

Object.assign(HOST, {
    "dado.arg_count": {
        signature: "()->(i32)",
        make: (ctx) => () => ctx.args.length,
    },
    "dado.arg_get": {
        signature: "(i32,i32,i32)->(i32)",
        make: (ctx) => (index, buf, cap) =>
            index >= 0 && index < ctx.args.length ? give(ctx, ctx.args[index], buf, cap) : SYS.INVALID,
    },
    "dado.env_get": {
        signature: "(i32,i32,i32,i32)->(i32)",
        make: (ctx) =>
            sys((name, len, buf, cap) => {
                const key = textAt(ctx, name, len);
                const value = ctx.env === null ? process.env[key] : ctx.env.get(key);
                return value === undefined ? SYS.NOT_FOUND : give(ctx, value, buf, cap);
            }),
    },
    "dado.env_set": {
        signature: "(i32,i32,i32,i32)->(i32)",
        make: (ctx) =>
            sys((name, len, value, vlen) => {
                if (ctx.env !== null) return SYS.UNSUPPORTED;
                const key = textAt(ctx, name, len);
                const text = textAt(ctx, value, vlen);
                if (!envName(key) || text.includes("\0")) return SYS.INVALID;
                process.env[key] = text;
                return 0;
            }),
    },
    "dado.env_unset": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) =>
            sys((name, len) => {
                if (ctx.env !== null) return SYS.UNSUPPORTED;
                const key = textAt(ctx, name, len);
                if (!envName(key)) return SYS.INVALID;
                delete process.env[key];
                return 0;
            }),
    },
    "dado.env_list": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) =>
            sys((buf, cap) => {
                const pairs = ctx.env === null ? Object.entries(process.env) : [...ctx.env];
                return give(ctx, nulList(pairs.map(([k, v]) => `${k}=${v}`)), buf, cap);
            }),
    },
    "dado.random_bytes": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) =>
            sys((buf, len) => {
                // `getRandomValues` takes at most 65536 bytes, and no view of
                // a shared memory: fill a private block and copy it in.
                const block = new Uint8Array(Math.min(len >>> 0, 65536));
                for (let at = 0; at < len >>> 0; at += block.length) {
                    const n = Math.min(block.length, (len >>> 0) - at);
                    const part = block.subarray(0, n);
                    crypto.getRandomValues(part);
                    bytesAt(ctx, (buf >>> 0) + at, n).set(part);
                }
                return 0;
            }),
    },
    "dado.is_terminal": {
        signature: "(i32)->(i32)",
        make: () => (fd) => (NODE && fd >= 0 && nodeTty.isatty(fd) ? 1 : 0),
    },
    "dado.host_name": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((buf, cap) => give(ctx, nodeOs.hostname(), buf, cap)),
    },
    "dado.fs_open": {
        signature: "(i32,i32,i32,i32)->(i32)",
        make: (ctx) =>
            nodeOnly((path, len, flags, mode) =>
                nodeFs.openSync(pathAt(ctx, path, len), openFlags(flags), mode === 0 ? 0o666 : mode),
            ),
    },
    "dado.fs_read": {
        signature: "(i32,i32,i32,i64)->(i32)",
        make: (ctx) =>
            nodeOnly((fd, buf, len, pos) => moveBytes(nodeFs.readSync, fd, bytesAt(ctx, buf, len), pos)),
    },
    "dado.fs_write": {
        signature: "(i32,i32,i32,i64)->(i32)",
        make: (ctx) =>
            nodeOnly((fd, buf, len, pos) => {
                const view = bytesAt(ctx, buf, len);
                let done = 0;
                while (done < view.length) {
                    const at = pos < 0n ? -1n : pos + BigInt(done);
                    done += moveBytes(nodeFs.writeSync, fd, view.subarray(done), at);
                }
                return done;
            }),
    },
    "dado.fs_close": {
        signature: "(i32)->(i32)",
        make: () => nodeOnly((fd) => (nodeFs.closeSync(fd), 0)),
    },
    "dado.fs_sync": {
        signature: "(i32)->(i32)",
        make: () => nodeOnly((fd) => (nodeFs.fsyncSync(fd), 0)),
    },
    "dado.fs_truncate": {
        signature: "(i32,i64)->(i32)",
        make: () => nodeOnly((fd, size) => (nodeFs.ftruncateSync(fd, Number(size)), 0)),
    },
    "dado.fs_stat": {
        signature: "(i32,i32,i32,i32)->(i32)",
        make: (ctx) =>
            nodeOnly((path, len, follow, out) => {
                const p = pathAt(ctx, path, len);
                const st = follow ? nodeFs.statSync(p, { bigint: true }) : nodeFs.lstatSync(p, { bigint: true });
                return writeStat(ctx, out, st);
            }),
    },
    "dado.fs_fstat": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((fd, out) => writeStat(ctx, out, nodeFs.fstatSync(fd, { bigint: true }))),
    },
    "dado.fs_exists": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((path, len) => (nodeFs.existsSync(pathAt(ctx, path, len)) ? 1 : 0)),
    },
    "dado.fs_mkdir": {
        signature: "(i32,i32,i32)->(i32)",
        make: (ctx) =>
            nodeOnly((path, len, mode) => (nodeFs.mkdirSync(pathAt(ctx, path, len), { mode: mode === 0 ? 0o777 : mode }), 0)),
    },
    "dado.fs_rmdir": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((path, len) => (nodeFs.rmdirSync(pathAt(ctx, path, len)), 0)),
    },
    "dado.fs_unlink": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((path, len) => (nodeFs.unlinkSync(pathAt(ctx, path, len)), 0)),
    },
    "dado.fs_rename": {
        signature: "(i32,i32,i32,i32)->(i32)",
        make: (ctx) =>
            nodeOnly((from, flen, to, tlen) => (nodeFs.renameSync(pathAt(ctx, from, flen), pathAt(ctx, to, tlen)), 0)),
    },
    "dado.fs_list": {
        signature: "(i32,i32,i32,i32)->(i32)",
        make: (ctx) =>
            nodeOnly((path, len, buf, cap) =>
                give(ctx, nulList(nodeFs.readdirSync(pathAt(ctx, path, len), { encoding: "buffer" })), buf, cap),
            ),
    },
    "dado.fs_cwd": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((buf, cap) => give(ctx, process.cwd(), buf, cap)),
    },
    "dado.fs_chdir": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((path, len) => (process.chdir(textAt(ctx, path, len)), 0)),
    },
    "dado.fs_temp_dir": {
        signature: "(i32,i32)->(i32)",
        make: (ctx) => nodeOnly((buf, cap) => give(ctx, nodeOs.tmpdir(), buf, cap)),
    },
    "dado.fs_realpath": {
        signature: "(i32,i32,i32,i32)->(i32)",
        make: (ctx) =>
            nodeOnly((path, len, buf, cap) =>
                give(ctx, nodeFs.realpathSync(pathAt(ctx, path, len), { encoding: "buffer" }), buf, cap),
            ),
    },
});

/** The export that builds `main(argc, argv)`'s argv in the module's memory. */
export const ARGV_EXPORT = "dado_argv";

// DadoScript: the scripts' host calls join the table.
Object.assign(HOST, DADOS_HOST);

/** The import module of Dado's direct calls of a script's package function,
 * which the scripts' registry supplies. */
const FUNCTION_MODULE = "dados_fn";

/** The one import that is not a host call: a threaded module's memory. */
const MEMORY_IMPORT = "env.memory";

/** The host's processor count, at least 1. */
let hostCores = 1;
if (typeof navigator !== "undefined" && navigator.hardwareConcurrency) {
    hostCores = navigator.hardwareConcurrency;
} else if (typeof process !== "undefined" && process.versions?.node) {
    const os = await import("node:os");
    hostCores = Math.max(1, os.availableParallelism?.() ?? os.cpus().length);
}

/** Whether this code runs on node (else it is a page, or a page's worker). */
const NODE = typeof process !== "undefined" && process.versions?.node !== undefined;

/** Node's file system, operating system and terminal modules, where they exist. */
const nodeFs = NODE ? await import("node:fs") : null;
const nodeOs = NODE ? await import("node:os") : null;
const nodeTty = NODE ? await import("node:tty") : null;

/**
 * The limits the module declares for its imported memory, read from its
 * binary's import section — `{ initial, maximum, shared }` in pages — or
 * `null` when it imports none. Read from the bytes because the JS API does not
 * report an import's type.
 */
export function memoryImportLimits(bytes) {
    const b = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes);
    let at = 8;
    const leb = () => {
        let result = 0;
        let shift = 0;
        for (;;) {
            const byte = b[at++];
            result += (byte & 0x7f) * 2 ** shift;
            shift += 7;
            if ((byte & 0x80) === 0) {
                return result;
            }
        }
    };
    const name = () => {
        const n = leb();
        const text = new TextDecoder().decode(b.subarray(at, at + n));
        at += n;
        return text;
    };
    const limits = () => {
        const flags = b[at++];
        const initial = leb();
        const maximum = flags & 1 ? leb() : undefined;
        return { initial, maximum, shared: (flags & 2) !== 0 };
    };
    while (at < b.length) {
        const id = b[at++];
        const size = leb();
        const end = at + size;
        if (id !== 2) {
            at = end;
            continue;
        }
        const count = leb();
        for (let i = 0; i < count; i++) {
            const module = name();
            const field = name();
            const kind = b[at++];
            if (kind === 0) {
                leb();
            } else if (kind === 1) {
                at++;
                limits();
            } else if (kind === 2) {
                const l = limits();
                if (`${module}.${field}` === MEMORY_IMPORT) {
                    return l;
                }
            } else if (kind === 3) {
                at += 2;
            } else {
                throw new LoaderError(`the module's import section has an import of kind ${kind}`);
            }
        }
        return null;
    }
    return null;
}

/** The `wasm-import` lines of a manifest, as `{ key, signature }`. */
export function declaredImports(manifest) {
    const out = [];
    for (const line of manifest.split("\n")) {
        const parts = line.trim().split(" ");
        if (parts[0] !== "wasm-import") {
            continue;
        }
        if (parts.length !== 4) {
            throw new LoaderError(`malformed manifest line: ${line}`);
        }
        out.push({ key: `${parts[1]}.${parts[2]}`, signature: parts[3] });
    }
    return out;
}

/** The `wasm-export` lines of a manifest, as `{ name, signature }`. */
export function declaredExports(manifest) {
    const out = [];
    for (const line of manifest.split("\n")) {
        const parts = line.trim().split(" ");
        if (parts[0] !== "wasm-export") {
            continue;
        }
        if (parts.length !== 3) {
            throw new LoaderError(`malformed manifest line: ${line}`);
        }
        out.push({ name: parts[1], signature: parts[2] });
    }
    return out;
}

/** The kind `WebAssembly.Module.exports` reports for a manifest signature. */
function exportKind(signature) {
    return signature.startsWith("(") ? "function" : signature;
}

/**
 * Check the instance's exports against the manifest's `wasm-export` lines:
 * every name the compiler exported — the program's own `export`s among them —
 * is there, of its kind, and a function takes as many parameters as its
 * signature lists. JavaScript cannot read a wasm function's types, so the count
 * is what is checked. Throws `LoaderError` naming the
 * first that is not, before `main` runs.
 */
export function checkExports(instance, manifest) {
    for (const { name, signature } of declaredExports(manifest)) {
        const value = instance.exports[name];
        const kind = exportKind(signature);
        if (value === undefined) {
            throw new LoaderError(
                `the manifest lists the export ${name} ${signature}, and the module exports no ${name}`,
            );
        }
        if (kind === "function") {
            const params = signature.slice(1, signature.indexOf(")"));
            const count = params === "" ? 0 : params.split(",").length;
            if (typeof value !== "function" || value.length !== count) {
                throw new LoaderError(
                    `the manifest lists the export ${name} as ${signature}, and the module's ${name} ` +
                        (typeof value === "function" ? `takes ${value.length} parameters` : "is not a function"),
                );
            }
        }
    }
}

/**
 * Fill `dados:exports` (`dado-exports.js` beside the scripts):
 * the program's own `export`s — the manifest's `c-export` lines, each the
 * instance's function of that name — frozen, and the memory. The glue's
 * exports stay private. Nothing for a module that runs no scripts, which has
 * no such file.
 */
export function publishExports(dados, instance, manifest, memory) {
    const fill = dados?.registry?.__dado_exports_fill;
    if (typeof fill !== "function") {
        return;
    }
    const published = {};
    for (const line of manifest.split("\n")) {
        const parts = line.trim().split(" ");
        if (parts[0] === "c-export" && parts.length === 2) {
            published[parts[1]] = instance.exports[parts[1]];
        }
    }
    fill(Object.freeze(published), memory);
}

/**
 * Check the module's imports against the manifest and this loader, and build
 * the import object. Throws `LoaderError` naming the first import that is not
 * declared, not supplied, or supplied with another signature.
 */
export function imports(module, manifest, ctx) {
    const declared = new Map(declaredImports(manifest).map((d) => [d.key, d.signature]));
    for (const [key, signature] of declared) {
        if (key === MEMORY_IMPORT && signature === "memory") {
            continue;
        }
        if (key.startsWith(`${FUNCTION_MODULE}.`)) {
            continue;
        }
        const host = HOST[key];
        if (host === undefined) {
            throw new LoaderError(
                `the module imports ${key} ${signature}, and this loader supplies no ${key}`,
            );
        }
        if (host.signature !== signature) {
            throw new LoaderError(
                `the module imports ${key} as ${signature}, and this loader supplies it as ${host.signature}`,
            );
        }
    }
    const object = {};
    for (const imp of WebAssembly.Module.imports(module)) {
        const key = `${imp.module}.${imp.name}`;
        if (imp.kind === "memory" && key === MEMORY_IMPORT && declared.get(key) === "memory") {
            object[imp.module] ??= {};
            object[imp.module][imp.name] = ctx.memory();
            continue;
        }
        // Dado's direct calls of a script's package function (a17-export):
        // each the registry's function, written for this import.
        if (imp.module === FUNCTION_MODULE && imp.kind === "function" && declared.has(key)) {
            const f = ctx.dados?.functionImport(imp.name, declared.get(key)) ?? null;
            if (f === null) {
                throw new LoaderError(
                    `the module imports ${key} ${declared.get(key)}, and the scripts' registry has no such function`,
                );
            }
            object[imp.module] ??= {};
            object[imp.module][imp.name] = f;
            continue;
        }
        if (imp.kind !== "function" || !declared.has(key)) {
            throw new LoaderError(
                `the module imports ${key} (${imp.kind}), which its manifest does not declare — ` +
                    `the compiler's import table and the module disagree`,
            );
        }
        object[imp.module] ??= {};
        object[imp.module][imp.name] = HOST[key].make(ctx);
    }
    return object;
}

/**
 * Instantiate `bytes` and run its `main`. Answers the exit status.
 *
 * `options.manifest` is the `--emit=build` text; `options.hashSeed` pins the
 * hash seed (else host entropy); `options.out(fd, bytes)` receives output
 * (default: node's stdout/stderr, or the console on a page).
 * `options.args` is the program's arguments, argv[0] first (default
 * `["main"]`); `options.env` a `Map` the environment is read from, which
 * makes it read-only (default: node's `process.env`, or an empty one off node).
 * `options.override(object, registry)`, a bench's only (the boundary
 * baseline's bare import, `toys-lab/baseline/`), may replace an import in the
 * import object before the module is instantiated; `registry` is the
 * scripts' registry, or null.
 */
export async function run(bytes, options = {}) {
    const manifest = options.manifest ?? "";
    const decoder = new TextDecoder();
    const out = options.out ?? defaultOut(decoder);
    const limits = memoryImportLimits(bytes);
    let memory = null;
    if (limits !== null) {
        memory = sharedMemory(limits);
    }
    const module = await WebAssembly.compile(bytes);
    const seed = options.hashSeed ?? null;
    const args = (options.args ?? ["main"]).map(String);
    const env = options.env !== undefined ? new Map(options.env) : NODE ? null : new Map();
    const ctx = {
        memory: () => memory,
        out,
        seed,
        args,
        env,
        spawn: (slot, arg) =>
            spawnWorker({ module, memory, manifest, seed, slot, arg, args, env, dados: ctx.dadosThread }), // STOP
        canWait: canWaitHere(),
    };
    // DadoScript: a module that runs scripts gets a session over their registry —
    // `options.registry`, the namespace of its `registry.js`, or
    // `options.scripts`, the URL to import it from.
    if (runsScripts(manifest)) {
        ctx.dados = new Session(options.registry ?? (await importScripts(options.scripts)));
        ctx.dados.out = out;
        ctx.dadosThread = workerScripts(options.scripts); // STOP
    }
    const object = imports(module, manifest, ctx);
    options.override?.(object, ctx.dados?.registry ?? null);
    const instance = await WebAssembly.instantiate(module, object);
    memory = instance.exports.memory;
    checkExports(instance, manifest);
    publishExports(ctx.dados, instance, manifest, memory);
    ctx.dados?.attach(instance, ctx.memory); // DadoScript
    const main = instance.exports.main;
    if (typeof main !== "function") {
        throw new LoaderError("the module exports no `main`");
    }
    // The runtime's console buffers standard output; this hands over what is
    // held. The runtime flushes before its own traps, and this is also asked
    // after any other trap (an access out of bounds), which an instance that
    // trapped still answers. Absent when the program writes nothing.
    const flush = () => {
        reportNarrowings(instance);
        if (typeof instance.exports[FLUSH_EXPORT] === "function") {
            instance.exports[FLUSH_EXPORT]();
        }
    };
    // The stack's top before `main` runs: after a stack overflow the
    // pointer is left out of bounds, and the flush would trap too, so it is
    // put back here first (DI-0302). A module linked without the export
    // (an older build) keeps the old answer: what was buffered is lost.
    const stackPointer = instance.exports.__stack_pointer;
    const stackTop = stackPointer instanceof WebAssembly.Global ? stackPointer.value : null;
    let status;
    try {
        // `main(void)`, or `main(argc, argv)` with the host's arguments, the
        // argv built in the module's memory by its `dado_argv`.
        if (main.length === 0) {
            status = main();
        } else {
            const build = instance.exports[ARGV_EXPORT];
            const argv = typeof build === "function" ? build(args.length) : 0;
            status = argv === 0 ? main(0, 0) : main(args.length, argv);
        }
    } catch (e) {
        if (e instanceof WebAssembly.RuntimeError) {
            // A stack that overflowed leaves `__stack_pointer` out of bounds:
            // it is put back to the top, so the flush has a stack to run on.
            // Should the flush trap all the same, what it held is lost, and
            // the trap is still the program's end.
            if (stackTop !== null) {
                stackPointer.value = stackTop;
            }
            try {
                flush();
            } catch (again) {
                if (!(again instanceof WebAssembly.RuntimeError)) {
                    throw again;
                }
            }
            explainTrap(e, out);
            return TRAP_STATUS;
        }
        // DadoScript: the runtime ended the program with a status (`dado.exit`).
        if (e instanceof DadoExit) {
            flush();
            return e.status & 0xff;
        }
        throw e;
    }
    flush();
    return (status ?? 0) & 0xff;
}

/**
 * **A trap the runtime did not report, said on fd 2.** The runtime's own
 * (`#panic`, a failed `#assert`, a bound) write their report and then execute
 * `unreachable`, so that one is silent here; any other (an access out of
 * bounds, which is what a stack overflow is with the stack first, DI-0302)
 * would otherwise end the program with 134 and not a word.
 */
function explainTrap(e, out) {
    if (/unreachable/i.test(e.message)) {
        return;
    }
    const why = /out of bounds/i.test(e.message)
        ? ": most often the stack overflowing, a recursion too deep for it"
        : "";
    out(2, new TextEncoder().encode(`dado: the module trapped (${e.message})${why}\n`));
}

/** DadoScript: the scripts' `registry.js`, imported from `url`. */
async function importScripts(url) {
    if (url === undefined) {
        throw new LoaderError(
            "the module runs DadoScript scripts, and no registry of them was given: pass " +
                "`scripts` (the URL of the build's registry.js) or `registry` to run()",
        );
    }
    return import(url);
}

// ── STOP (a16-stop): a worker's own session ─────────────────────────────────
// A VM belongs to the thread that made it, so a VM made on a worker lives in
// that worker's JavaScript: each worker of a module that runs scripts imports
// the registry itself and keeps a session of its own, its slot ids in a range
// of its own (`Session`'s `base`). What a worker is handed: the registry's
// URL and the counter that numbers the sessions.

/** What a spawned worker needs for a session, or null (no URL to import). */
function workerScripts(url) {
    if (url === undefined || typeof SharedArrayBuffer === "undefined") return null;
    return { scripts: url, sessions: new Int32Array(new SharedArrayBuffer(4)) };
}

/** A worker's session: numbered 1–127 (the main thread's is 0), each 2^24
 * slot ids wide, so the ids of all of them fit an `int32`. */
async function workerSession(dados) {
    const n = (Atomics.add(dados.sessions, 0, 1) % 127) + 1;
    return new Session(await import(dados.scripts), { base: n * 2 ** 24, worker: true });
}
// ── end STOP ────────────────────────────────────────────────────────────────

/** A shared memory at the limits a threaded module declares for it. */
function sharedMemory(limits) {
    if (!limits.shared || limits.maximum === undefined) {
        throw new LoaderError(
            "the module imports a memory that is not shared with a maximum, which only a threaded build does and every threaded build declares",
        );
    }
    if (typeof SharedArrayBuffer === "undefined") {
        throw new LoaderError(
            "this module is threaded and this host has no SharedArrayBuffer: a page must be served " +
                "cross-origin isolated (Cross-Origin-Opener-Policy: same-origin, " +
                "Cross-Origin-Embedder-Policy: require-corp)",
        );
    }
    return new WebAssembly.Memory({
        initial: limits.initial,
        maximum: limits.maximum,
        shared: true,
    });
}

/** Whether this thread may execute `memory.atomic.wait32`: every thread but
 * a page's main thread. */
function canWaitHere() {
    if (NODE) {
        return true;
    }
    return typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope;
}

/** Start a worker running `thread`; 0 when started. */
function spawnWorker(thread) {
    if (thread.memory === null) {
        return 1;
    }
    try {
        if (NODE) {
            // Every thread reads and writes the one environment, as natively.
            const worker = new nodeWorkerThreads.Worker(new URL(import.meta.url), {
                workerData: { dadoThread: thread },
                env: nodeWorkerThreads.SHARE_ENV,
            });
            // The program ends when `main` returns, as a native one does; a
            // worker still running does not keep it alive.
            worker.unref();
        } else if (!pageSpawn(thread)) {
            const worker = new Worker(new URL(import.meta.url), { type: "module" });
            worker.postMessage({ dadoThread: thread });
        }
        return 0;
    } catch {
        return 1;
    }
}

/** Node's `worker_threads`, loaded once where it exists. */
const nodeWorkerThreads = NODE ? await import("node:worker_threads") : null;

/**
 * A worker's life: instantiate the module on the shared memory, set the stack
 * pointer from the record, call the slot. A trap marks the record trapped and
 * wakes its joiner; under node it then ends the process with SIGABRT, as an
 * `abort` on a native thread does.
 */
async function runThread(thread) {
    const { module, memory, manifest, seed, slot, arg, dados } = thread;
    const args = thread.args ?? [];
    const env = thread.env ?? null;
    let out;
    if (NODE) {
        const fs = await import("node:fs");
        out = (fd, bytes) => writeAll(fs, fd === 2 ? 2 : 1, bytes.slice());
    } else {
        out = defaultOut(new TextDecoder());
    }
    const ctx = {
        memory: () => memory,
        out,
        seed,
        args,
        env,
        spawn: (s, a) => spawnWorker({ module, memory, manifest, seed, slot: s, arg: a, args, env, dados }), // STOP
        canWait: true,
    };
    if (dados) ctx.dados = await workerSession(dados); // STOP
    if (ctx.dados) ctx.dados.out = out;
    const instance = await WebAssembly.instantiate(module, imports(module, manifest, ctx));
    publishExports(ctx.dados, instance, manifest, memory);
    ctx.dados?.attach(instance, ctx.memory); // STOP
    const words = new Int32Array(memory.buffer);
    instance.exports.__stack_pointer.value = words[arg >>> 2];
    try {
        instance.exports.__indirect_function_table.get(slot)(arg);
    } catch (e) {
        // STOP: the program's end (`dado.exit`) on a worker — a script failing
        // untried there. A worker cannot end the program with a status while
        // the main thread runs, so it ends as a trapped thread does, saying so.
        if (e instanceof DadoExit) {
            out(2, new TextEncoder().encode(
                `dados: the program ended with status ${e.status} on a worker thread, and on the web a worker cannot end the program with a status: it ends as a trapped thread does\n`,
            ));
        } else if (!(e instanceof WebAssembly.RuntimeError)) {
            throw e;
        }
        if (typeof instance.exports[FLUSH_EXPORT] === "function") {
            try {
                instance.exports[FLUSH_EXPORT]();
            } catch {
                // A console the trap left locked: what it held is lost.
            }
        }
        Atomics.store(words, (arg >>> 2) + 1, 2);
        Atomics.notify(words, (arg >>> 2) + 1);
        if (NODE) {
            process.kill(process.pid, "SIGABRT");
            return;
        }
        pageThreadTrapped();
        throw e;
    }
}

/** `fs.writeSync` until every byte is written: a pipe may take part of it. */
function writeAll(fs, fd, bytes) {
    let at = 0;
    while (at < bytes.length) {
        try {
            at += fs.writeSync(fd, bytes, at, bytes.length - at);
        } catch (e) {
            if (e.code !== "EAGAIN") {
                throw e;
            }
        }
    }
}

function defaultOut(decoder) {
    if (typeof process !== "undefined" && process.stdout) {
        return (fd, bytes) => (fd === 2 ? process.stderr : process.stdout).write(bytes.slice());
    }
    const toPage = pageOut();
    if (toPage !== null) {
        return toPage;
    }
    return (fd, bytes) => (fd === 2 ? console.error : console.log)(decoder.decode(bytes));
}

async function main(argv) {
    let wasm = null;
    let build = null;
    let rest = [];
    for (let i = 0; i < argv.length; i++) {
        if (argv[i] === "--") {
            rest = argv.slice(i + 1);
            break;
        } else if (argv[i] === "--build") {
            build = argv[++i];
        } else if (wasm === null) {
            wasm = argv[i];
        } else {
            throw new LoaderError(`usage: node load.mjs <module.wasm> [--build <manifest>] [-- <argument>...]`);
        }
    }
    if (wasm === null) {
        throw new LoaderError(`usage: node load.mjs <module.wasm> [--build <manifest>] [-- <argument>...]`);
    }
    const fs = await import("node:fs");
    build ??= `${wasm}.build`;
    let manifest;
    try {
        manifest = fs.readFileSync(build, "utf8");
    } catch {
        throw new LoaderError(
            `no manifest at ${build}: \`./dado build --target=web\` writes one beside the module, ` +
                `or pass --build <the output of dadoc --emit=build>`,
        );
    }
    const seed = process.env.DADO_HASH_SEED;
    // DadoScript: the scripts beside the module, when it runs any. A program that
    // reaches the engine and carries no script has no registry, and runs on
    // an empty one: a module it then asks for is refused by name.
    let scripts;
    let registry;
    if (runsScripts(manifest)) {
        const { pathToFileURL } = await import("node:url");
        const path = registryPath(wasm);
        if (fs.existsSync(path)) {
            scripts = pathToFileURL(fs.realpathSync(path)).href;
        } else {
            registry = NO_SCRIPTS;
        }
    }
    return run(fs.readFileSync(wasm), {
        manifest,
        hashSeed: seed === undefined || seed === "" ? null : BigInt(seed),
        scripts,
        registry,
        args: [wasm, ...rest],
    });
}

// A worker this file started, under node or in a page.
const threadData = nodeWorkerThreads?.isMainThread === false ? nodeWorkerThreads.workerData : null;
if (threadData?.dadoThread) {
    await runThread(threadData.dadoThread);
} else if (!NODE && typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope) {
    self.addEventListener("message", (event) => {
        if (event.data?.dadoThread) {
            runThread(event.data.dadoThread);
        }
    });
} else if (typeof process !== "undefined" && process.argv?.[1]) {
    // Run as a script — not when imported, by a page or by another module.
    const { pathToFileURL } = await import("node:url");
    const { realpathSync } = await import("node:fs");
    if (import.meta.url === pathToFileURL(realpathSync(process.argv[1])).href) {
        try {
            process.exitCode = await main(process.argv.slice(2));
        } catch (e) {
            process.stderr.write(`load.mjs: ${e instanceof LoaderError ? e.message : e.stack}\n`);
            process.exitCode = LOADER_STATUS;
        }
    }
}

// ============================================================================
// The page. `dadoc --page` writes this file beside the
// module as `loader.js` with a call to `runPage()` appended, and the page's
// `<script type="module" src="loader.js" data-dado-module="<module>.wasm"
// data-dado-manifest="<module>.wasm.build">` runs it (with
// `data-dado-scripts="<module>.scripts/registry.js"` for a program hosting
// DadoScript scripts, whose registry it imports by that URL). Everything the page
// needs is in this section; the rest of the file is touched in three places
// only, each a call into it: `spawnWorker` (a page worker asks the page to
// start a thread), `defaultOut` (a page worker writes to the page) and
// `runThread` (a page thread that traps says so).
//
// **Output.** Standard output and standard error go to the console, a line
// at a time, and into `<pre id="dado-output">` as they are written — standard
// error in a `<span class="dado-stderr">` — the element made at the end of the
// body when the page has none. **Status.** When the program ends, its status
// is written into `#dado-status` (made when absent), set as `data-dado-status`
// on `<html>` and as `window.dadoPage.status`, and announced with a `dado-exit`
// event; `window.dadoPage.stdout` and `.stderr` hold what was written. A
// `?dado-seed=N` in the page's address pins the hash seed. **Arguments and
// environment**: each `?dado-arg=…` in the address is an argument, after
// argv[0], the module's path; each `?dado-env=NAME=value` a variable of the
// program's environment, which it can read and not write.
//
// **A threaded page** must be cross-origin isolated; when it is not, the
// loader runs nothing and says which header is missing. It runs `main` in a
// worker, because a page's own thread may not wait and `threads.join` does;
// every thread is a worker the page starts (a worker that is blocked in a
// join could not start one), and each writes to the page through a
// `BroadcastChannel` named after the run. The program ends when `main`
// returns; a thread that traps ends it with 134, as natively.
// ============================================================================

/** A page worker's name starts with this; the rest names its run. */
const PAGE_WORKER = "dado-page:";

/** In a worker a page started, the channel to the page; else `null`. */
let pageChannelOnce;
function pageChannel() {
    if (pageChannelOnce === undefined) {
        pageChannelOnce = null;
        if (
            !NODE &&
            typeof WorkerGlobalScope !== "undefined" &&
            self instanceof WorkerGlobalScope &&
            typeof self.name === "string" &&
            self.name.startsWith(PAGE_WORKER)
        ) {
            pageChannelOnce = new BroadcastChannel(self.name);
        }
    }
    return pageChannelOnce;
}

/** A page worker's output, posted to the page; `null` anywhere else. */
function pageOut() {
    const channel = pageChannel();
    if (channel === null) {
        return null;
    }
    return (fd, bytes) => channel.postMessage({ dadoOut: { fd: fd === 2 ? 2 : 1, bytes: bytes.slice() } });
}

/** In a page worker, ask the page to start the thread: true when asked. */
function pageSpawn(thread) {
    if (pageChannel() === null) {
        return false;
    }
    self.postMessage({ dadoSpawn: thread });
    return true;
}

/** A page thread trapped: the page ends the program with 134. */
function pageThreadTrapped() {
    pageChannel()?.postMessage({ dadoTrap: true });
}

// A threaded page's `main`, in the worker the page started for it.
if (pageChannel() !== null) {
    self.addEventListener("message", async (event) => {
        const job = event.data?.dadoMain;
        if (!job) {
            return;
        }
        const channel = pageChannel();
        try {
            const status = await run(job.bytes, {
                manifest: job.manifest,
                hashSeed: job.hashSeed,
                args: job.args,
                env: job.env,
                out: pageOut(),
                ...pageScripts(job.manifest, job.scripts),
            });
            channel.postMessage({ dadoExit: status });
        } catch (e) {
            channel.postMessage({ dadoFail: e instanceof LoaderError ? e.message : String(e?.stack ?? e) });
        }
    });
}

/** DadoScript on the page: what `run` is given for the program's scripts — the
 * registry's URL, which the page's `<script data-dado-scripts>` names and
 * every worker imports by, or, for a module that reaches the engine and
 * carries no script, the empty registry (as `main` does under node). */
function pageScripts(manifest, scripts) {
    if (!runsScripts(manifest)) {
        return {};
    }
    return scripts === undefined ? { registry: NO_SCRIPTS } : { scripts };
}

/** The page's environment: each `?dado-env=NAME=value`, as `[name, value]`
 * pairs (a later one of a name wins). */
function pageEnv(query) {
    const env = new Map();
    for (const pair of query.getAll("dado-env")) {
        const at = pair.indexOf("=");
        if (at > 0) {
            env.set(pair.slice(0, at), pair.slice(at + 1));
        }
    }
    return [...env];
}

/** Whether a manifest is a threaded module's: it imports its memory. */
function threadedManifest(manifest) {
    return declaredImports(manifest).some((d) => d.key === MEMORY_IMPORT);
}

/** The page's output element and status line, made when the page has none. */
function pageView() {
    let output = document.getElementById("dado-output");
    if (output === null) {
        output = document.createElement("pre");
        output.id = "dado-output";
        document.body.append(output);
    }
    let status = document.getElementById("dado-status");
    if (status === null) {
        status = document.createElement("p");
        status.id = "dado-status";
        output.before(status);
    }
    return { output, status };
}

/** The headers whose absence leaves this page not cross-origin isolated. */
async function missingIsolationHeaders() {
    const both = [
        "Cross-Origin-Opener-Policy: same-origin",
        "Cross-Origin-Embedder-Policy: require-corp",
    ];
    try {
        const response = await fetch(location.href, { method: "HEAD", cache: "no-store" });
        const coop = (response.headers.get("cross-origin-opener-policy") ?? "").trim();
        const coep = (response.headers.get("cross-origin-embedder-policy") ?? "").trim();
        const missing = [];
        if (coop !== "same-origin") {
            missing.push(both[0]);
        }
        if (coep !== "require-corp" && coep !== "credentialless") {
            missing.push(both[1]);
        }
        return missing;
    } catch {
        return both;
    }
}

async function fetchOk(url, what) {
    let response;
    try {
        response = await fetch(url);
    } catch (e) {
        throw new LoaderError(`cannot fetch the ${what} at ${url}: ${e.message}`);
    }
    if (!response.ok) {
        throw new LoaderError(`cannot fetch the ${what} at ${url}: ${response.status} ${response.statusText}`);
    }
    return response;
}

/**
 * Run the page's program: fetch the module and its manifest named by the
 * page's `<script data-dado-module>`, run `main`, and report what it wrote
 * and its status on the page. Answers the status.
 */
export async function runPage() {
    const view = pageView();
    const page = { status: null, stdout: "", stderr: "" };
    globalThis.dadoPage = page;
    const decoders = [null, new TextDecoder(), new TextDecoder()];
    const pending = ["", "", ""];
    let done = false;
    const write = (fd, text) => {
        if (text === "") {
            return;
        }
        if (fd === 2) {
            page.stderr += text;
            const span = document.createElement("span");
            span.className = "dado-stderr";
            span.textContent = text;
            view.output.append(span);
        } else {
            page.stdout += text;
            view.output.append(document.createTextNode(text));
        }
        const lines = (pending[fd] + text).split("\n");
        pending[fd] = lines.pop();
        for (const line of lines) {
            (fd === 2 ? console.error : console.log)(line);
        }
    };
    // A partial line written after the program finished — by a Dado function
    // a page's callback calls once `main` returned — goes to the console when
    // the task that wrote it ends, so nothing waits for a newline that may
    // never come; it is on the page already.
    let tail = false;
    const lineEnd = () => {
        tail = false;
        for (const fd of [1, 2]) {
            if (pending[fd] !== "") {
                (fd === 2 ? console.error : console.log)(pending[fd]);
                pending[fd] = "";
            }
        }
    };
    // What is written after the program finished is shown too: `main`
    // returning ends the run, not the page, and a page's callbacks may still
    // call Dado functions that print.
    const out = (fd, bytes) => {
        const which = fd === 2 ? 2 : 1;
        write(which, decoders[which].decode(bytes, { stream: true }));
        if (done && !tail && pending[which] !== "") {
            tail = true;
            setTimeout(lineEnd, 0);
        }
    };
    let settle;
    const settled = new Promise((resolve) => {
        settle = resolve;
    });
    const workers = [];
    const finish = (status, why) => {
        if (done) {
            return;
        }
        for (const fd of [1, 2]) {
            write(fd, decoders[fd].decode());
            if (pending[fd] !== "") {
                (fd === 2 ? console.error : console.log)(pending[fd]);
                pending[fd] = "";
            }
        }
        done = true;
        for (const worker of workers) {
            worker.terminate();
        }
        let said;
        if (why !== undefined) {
            said = `the loader could not start the program (status ${status}): ${why}`;
            console.error(`loader.js: ${why}`);
        } else if (status === TRAP_STATUS) {
            said = `the program trapped (status ${status})`;
        } else {
            said = `the program exited with status ${status}`;
        }
        view.status.textContent = said;
        page.status = status;
        document.documentElement.dataset.dadoStatus = String(status);
        globalThis.dispatchEvent(new CustomEvent("dado-exit", { detail: status }));
        settle(status);
    };
    try {
        const script = document.querySelector("script[data-dado-module]");
        if (script === null) {
            throw new LoaderError("the page has no <script data-dado-module>, which names the module to run");
        }
        const here = import.meta.url;
        const moduleUrl = new URL(script.dataset.dadoModule, here);
        const manifestUrl = new URL(script.dataset.dadoManifest ?? `${script.dataset.dadoModule}.build`, here);
        const [manifest, bytes] = await Promise.all([
            fetchOk(manifestUrl, "build manifest").then((r) => r.text()),
            fetchOk(moduleUrl, "module").then((r) => r.arrayBuffer()),
        ]);
        const query = new URLSearchParams(location.search);
        const seed = query.get("dado-seed");
        const hashSeed = seed === null || seed === "" ? null : BigInt(seed);
        const args = [script.dataset.dadoModule, ...query.getAll("dado-arg")];
        const env = pageEnv(query);
        // The registry of the program's scripts, by URL: `dadoc --page` names
        // it on the tag when the build wrote scripts.
        const scripts =
            script.dataset.dadoScripts === undefined ? undefined : new URL(script.dataset.dadoScripts, here).href;
        if (!threadedManifest(manifest)) {
            finish(await run(bytes, { manifest, hashSeed, args, env, out, ...pageScripts(manifest, scripts) }));
            return settled;
        }
        if (!globalThis.crossOriginIsolated) {
            const missing = await missingIsolationHeaders();
            const served =
                missing.length === 0
                    ? "it was served with both headers and is still not isolated (is it in a frame?)"
                    : `it was served without ${missing.map((h) => `\`${h}\``).join(" and ")}`;
            finish(
                LOADER_STATUS,
                `this page runs a threaded module (built with --web-threads) and is not cross-origin isolated, ` +
                    `so it has no shared memory and the program cannot start: ${served}. Serve the page with ` +
                    "`Cross-Origin-Opener-Policy: same-origin` and `Cross-Origin-Embedder-Policy: require-corp`; " +
                    "the build wrote both in `_headers` beside the page, for a host that reads that file",
            );
            return settled;
        }
        const name = `${PAGE_WORKER}${Math.random().toString(36).slice(2)}`;
        const channel = new BroadcastChannel(name);
        channel.addEventListener("message", (event) => {
            const m = event.data;
            if (m?.dadoOut) {
                out(m.dadoOut.fd, m.dadoOut.bytes);
            } else if (m?.dadoExit !== undefined) {
                finish(m.dadoExit & 0xff);
            } else if (m?.dadoTrap) {
                finish(TRAP_STATUS);
            } else if (m?.dadoFail !== undefined) {
                finish(LOADER_STATUS, m.dadoFail);
            }
        });
        const start = (message, isMain) => {
            const worker = new Worker(new URL(import.meta.url), { type: "module", name });
            worker.addEventListener("message", (event) => {
                if (event.data?.dadoSpawn) {
                    start({ dadoThread: event.data.dadoSpawn }, false);
                }
            });
            worker.addEventListener("error", (event) => {
                event.preventDefault();
                if (isMain) {
                    finish(LOADER_STATUS, `the worker running main failed: ${event.message}`);
                } else {
                    finish(TRAP_STATUS);
                }
            });
            workers.push(worker);
            worker.postMessage(message, message.dadoMain ? [message.dadoMain.bytes] : []);
        };
        start({ dadoMain: { bytes, manifest, hashSeed, scripts, args, env } }, true);
        settled.then(() => channel.close());
    } catch (e) {
        finish(LOADER_STATUS, e instanceof LoaderError ? e.message : String(e?.stack ?? e));
    }
    return settled;
}
