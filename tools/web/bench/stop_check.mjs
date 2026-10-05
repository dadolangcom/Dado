// stop_check.mjs — what the stop's check costs a back-edge under node
// (dado_emit_js's `web_stop`): a hot loop with no check, and with each flavour
// of it, each flavour timed alone in a fresh worker (so one flavour's
// feedback never shapes another's code), with `dados/stop.js` (the emitter's
// `stop.js`) imported for the live bindings the emitted code reads. The
// "main" rows run with the word 0 (the main thread's budget path), the
// "worker" rows with a word in a `SharedArrayBuffer`. A worker's thread is a
// thread like the main one, so the cost is the same wherever it is timed.
// Run: `node tools/web/bench/stop_check.mjs [rounds]`.
import { Worker, isMainThread, workerData, parentPort } from "node:worker_threads";
import os from "node:os";
import * as stop from "../../../crates/dado_emit_js/src/stop.js";
import { __dados_word, __dados_mem, __dados_poll } from "../../../crates/dado_emit_js/src/stop.js";

const N = 5e7;
const FLAVOURS = [
    ["main", "none"],
    ["main", "plain"],
    ["main", "budget"],
    ["main", "budgetClock"],
    ["main", "emitted"],
    ["worker", "word"],
    ["worker", "emitted"],
];

if (isMainThread) {
    const rounds = Number(process.argv[2] ?? 3);
    const runs = {};
    for (let r = 0; r < rounds; r++) {
        for (const [where, flavour] of FLAVOURS) {
            const ns = await new Promise((resolve, reject) => {
                const w = new Worker(new URL(import.meta.url), {
                    workerData: { where, flavour, sab: new SharedArrayBuffer(1 << 16) },
                });
                w.on("message", resolve);
                w.on("error", reject);
            });
            (runs[`${where} ${flavour}`] ??= []).push(...ns);
        }
    }
    const median = (a) => a.sort((x, y) => x - y)[a.length >> 1];
    const none = median(runs["main none"]);
    console.log(`node ${process.version}, ${os.cpus().length} x ${os.cpus()[0].model}, load ${os.loadavg().map((l) => l.toFixed(2)).join(" ")}`);
    console.log(`median ns per iteration over ${rounds} rounds x 7 timings of ${N}; the check's cost over \`none\`:`);
    for (const k of Object.keys(runs)) {
        const m = median(runs[k]);
        console.log(`  ${k.padEnd(20)} ${m.toFixed(3)}  ${k === "main none" ? "" : "+" + (m - none).toFixed(3)}`);
    }
} else {
    stop.bind({ poll: () => {} });
    if (workerData.where === "worker") {
        stop.track(64, new Int32Array(workerData.sab));
    } else {
        stop.track(0, null);
    }
    let tick$ = 0;
    let flag = 0;
    const loops = {
        none: (n) => { let s = 0; for (let i = 0; i < n; i++) { s = (s + i) | 0; } return s; },
        plain: (n) => { let s = 0; for (let i = 0; i < n; i++) { if (flag !== 0) __dados_poll(); s = (s + i) | 0; } return s; },
        budget: (n) => { let s = 0; for (let i = 0; i < n; i++) { if (--tick$ < 0) tick$ = __dados_poll(); s = (s + i) | 0; } return s; },
        budgetClock: (n) => { let s = 0; for (let i = 0; i < n; i++) { if (--tick$ < 0) { performance.now(); tick$ = __dados_poll(); } s = (s + i) | 0; } return s; },
        word: (n) => { let s = 0; for (let i = 0; i < n; i++) { if (Atomics.load(__dados_mem, __dados_word) !== 0) __dados_poll(); s = (s + i) | 0; } return s; },
        emitted: (n) => { let s = 0; for (let i = 0; i < n; i++) { if (__dados_word !== 0 ? Atomics.load(__dados_mem, __dados_word) !== 0 : --tick$ < 0) tick$ = __dados_poll(); s = (s + i) | 0; } return s; },
    };
    const f = loops[workerData.flavour];
    f(1e6);
    f(1e6);
    const out = [];
    for (let r = 0; r < 7; r++) {
        const t = process.hrtime.bigint();
        f(N);
        out.push(Number(process.hrtime.bigint() - t) / N);
    }
    parentPort.postMessage(out);
}
