// stop.mjs — the host's half of the stop's check on the web: what a script's
// `__dados_poll` (`dados/stop.js`, which the compiler writes beside the
// scripts) reaches, and the running VM's engine word kept current for a script
// on a worker.
//
// **The poll.** Natively QuickJS-ng calls the interrupt hook `core:script`
// installs at its back-edges. On the web a script's loop calls
// `__dados_poll`, and this asks the same hook, in the module, for the context
// that is running (`dados_interrupt`): 0 goes on; the fork's
// `JS_INTERRUPT_RAISE` (2) throws what the hook threw — the ask, a catchable
// `Error` whose `code` is `script.STOPPED`; anything else is the force, an
// error every lowered `catch` passes on (marked `UNCATCHABLE`), which reaches
// the host as the run's exception and which `core:script` reads as the stop.
// The hook may also wait, there, until a pause ends (`core:threads`' wait is
// `memory.atomic.wait32`, which a worker may execute).
//
// **The word.** On a worker of a `--web-threads` module a script can load the
// running VM's engine word from the shared memory at every back-edge, and
// ask the hook only when it is not 0 — kept, and switched off
// (`WORD_AT_EVERY_BACK_EDGE`): a worker keeps the budget, as the main thread
// does. The word's address is the web
// runtime's (`dados_word`, which `core:script`'s `JS_SetInterruptWord` set),
// read once per context; the session tells `dados/stop.js` which is running as
// runs enter and leave (`enter`). On the main thread the word is 0 and a
// budget of back-edges asks the hook instead.

/** What a lowered `catch` never keeps: the force, the program ending. */
export const UNCATCHABLE = Symbol.for("dados.uncatchable");

/** Whether a script on a worker loads its VM's word at every back-edge — else
 * it keeps the main thread's budget, whose poll asks the hook every `TICKS`
 * back-edges, as QuickJS-ng's own polls do.
 * **Off: a worker keeps the budget.** The load costs a back-edge about 7.2 ns
 * under node 22 and about 8.6–9.1 ns in headless Chromium 141 (the emitted
 * line with the word set, 10.6 ns), against the budget's 1.2–1.4 ns on both
 * and the ruled 1.2–1.6 ns (`tools/web/bench/stop_check.mjs`; in a browser,
 * the boundary baseline's `stop-check.html`) — so the browser confirmed
 * node, and a stop or a pause reaches a script on a worker within `TICKS`
 * back-edges, as on the main thread: the one place to change if that is
 * relitigated. */
const WORD_AT_EVERY_BACK_EDGE = false;

/** The fork's answer that raises what the hook threw. */
const RAISE = 2;

/** The force: QuickJS-ng's uncatchable error is an `InternalError`,
 * "interrupted". A new one each time, so a stack says where it struck. */
function forced() {
    const e = new Error("interrupted");
    e.name = "InternalError";
    Object.defineProperty(e, UNCATCHABLE, { value: true });
    return e;
}

/**
 * One session's stop: bound to the scripts' `dados/stop.js` (the registry's
 * `__dados_stop`), and told of each context a run makes current. `onWorker`:
 * whether this thread loads the word (a worker of a threaded module) rather
 * than keeping a budget.
 */
export class Stop {
    constructor(session, registry, onWorker) {
        this.session = session;
        this.module = registry.__dados_stop ?? null;
        this.onWorker = onWorker;
        this.mem = null;
        this.module?.bind({ poll: () => this.poll() });
    }

    /** `c` is now the running context (0: none). */
    enter(c) {
        if (!this.onWorker || !WORD_AT_EVERY_BACK_EDGE || this.module === null) return;
        if (c === 0) {
            this.module.track(0, null);
            return;
        }
        const cx = this.session.contexts.get(c);
        if (cx === undefined) {
            this.module.track(0, null);
            return;
        }
        if (cx.word === undefined) {
            cx.word = (this.session.exports.dados_word(c) >>> 0) >>> 2;
        }
        const buffer = this.session.memory().buffer;
        if (this.mem === null || this.mem.buffer.byteLength !== buffer.byteLength) {
            this.mem = new Int32Array(buffer);
        }
        this.module.track(cx.word, this.mem);
    }

    /** A script's back-edge asked: the hook's answer for the running
     * context, thrown when it says stop. */
    poll() {
        const s = this.session;
        const c = s.current;
        if (c === 0 || !s.contexts.has(c)) return;
        const r = s.exports.dados_interrupt(c);
        if (r === 0) return;
        if (r === RAISE) throw s.take(c);
        throw forced();
    }
}
