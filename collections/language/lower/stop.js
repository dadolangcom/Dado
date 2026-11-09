// dados/stop.js: the stop's check in a script's module on the web. Natively
// the engine polls its interrupt hook at back-edges; on the web the script's
// own code does, with what this module exports, at the head of every loop the
// compiler writes — in a module with one thread
//
//     if (--tick$ < 0) tick$ = __dados_poll();
//
// and in a `--web-threads` module
//
//     if (__dados_word !== 0 ? Atomics.load(__dados_mem, __dados_word) !== 0 : --tick$ < 0) tick$ = __dados_poll();
//
// On a worker of a `--web-threads` module `__dados_word` is the index of the
// running VM's engine word in the shared memory, loaded at every back-edge
// (a `SharedArrayBuffer` word: the JIT cannot hoist an `Atomics.load` out of
// a loop), and only a word that is not 0 asks the hook. On the main thread it
// is 0, and a budget of back-edges (`tick$`, the module's own) asks the hook
// once per `TICKS`. The host (the loader's `stop.mjs`) keeps both current as
// runs enter and leave, and `__dados_poll` asks it: the hook may wait (a
// pause), throw the catchable `Error` with `script.STOPPED` (the ask), or
// answer the force, which is thrown as an object every lowered `catch`
// passes on (`__dados_pass`): a forced script runs no more of its code, not
// even a `defer`.

let host = null;

/** Back-edges between two asks of the hook on the main thread. */
export const TICKS = 10000;

/** The running VM's engine word, as an index into `__dados_mem`; 0 where the
 * budget is used instead. */
export let __dados_word = 0;

/** The shared memory, as 32-bit words. */
export let __dados_mem = new Int32Array(0);

/** What every lowered `catch` passes on: the force, the program ending, a
 * trap in the module. Marked with this symbol, which is the host's too. */
const UNCATCHABLE = Symbol.for("dados.uncatchable");

/** The loader's host: `poll()`, which throws when the script must stop. */
export function bind(h) {
    host = h;
}

/** The running VM's word (an index), and the memory it is in, as a run enters
 * or leaves: 0 for the budget. */
export function track(word, mem) {
    __dados_word = word;
    if (mem !== null) __dados_mem = mem;
}

/** Ask the hook; answers the next budget. */
export function __dados_poll() {
    host.poll();
    return TICKS;
}

/** At the top of every lowered `catch`: what no script may catch is thrown on. */
export function __dados_pass(e) {
    if (typeof e === "object" && e !== null && (e[UNCATCHABLE] === true || e instanceof WebAssembly.RuntimeError)) {
        throw e;
    }
}
