// sites.mjs — run a web build of a program that hosts DadoScript scripts, as
// `load.mjs` does, and then report every script's call of a Dado function:
// where it is, what it calls, how it crosses (`inline` or `thunk`) and why,
// and — in a dev build — how often it crossed, how many narrowing wraps there
// changed the value, and how many of its calls fell back to the thunk.
//
//     node tools/web/sites.mjs build/game.wasm
//
// The program's own output is untouched; the report goes to standard error
// after it, one line a site, in the order `dadoc --emit=strategies` lists them:
//
//     # sites
//     Probe.dados:16:38  main.w8  inline  calls 1  wraps 0  fallbacks 0  (the function itself)
//
// A release build (`./dado build --release`) has no per-site counters, and
// its lines say `calls -`. What the report reads is the scripts' own
// `dados/web.js` — the same module the program ran — through its registry:
// `__dados_counts()` and `sites`.

import { readFileSync, existsSync, realpathSync } from "node:fs";
import { pathToFileURL } from "node:url";
import { run, LOADER_STATUS } from "./load.mjs";
import { registryPath } from "./dados.mjs";

async function main(args) {
    const wasm = args[0];
    if (wasm === undefined) {
        process.stderr.write("usage: node tools/web/sites.mjs <module.wasm>\n");
        return LOADER_STATUS;
    }
    const manifest = readFileSync(`${wasm}.build`, "utf8");
    const path = registryPath(wasm);
    if (!existsSync(path)) {
        process.stderr.write(`sites.mjs: ${wasm} carries no scripts (no ${path})\n`);
        return LOADER_STATUS;
    }
    const registry = await import(pathToFileURL(realpathSync(path)).href);
    const seed = process.env.DADO_HASH_SEED;
    const status = await run(readFileSync(wasm), {
        manifest,
        hashSeed: seed === undefined || seed === "" ? null : BigInt(seed),
        registry,
    });
    const counts = registry.__dados_counts();
    // A release build's sites raise no per-site counter.
    const dev = counts.S.some((n) => n > 0);
    let report = "# sites\n";
    registry.sites.forEach(([at, callee, strategy, reason], k) => {
        if (strategy === "unreached") return;
        const n = (a) => (dev ? String(a[k]) : "-");
        report += `${at}  ${callee}  ${strategy}  calls ${n(counts.S)}  wraps ${n(counts.W)}  fallbacks ${n(counts.F)}  (${reason})\n`;
    });
    process.stderr.write(report);
    return status;
}

try {
    process.exitCode = await main(process.argv.slice(2));
} catch (e) {
    process.stderr.write(`sites.mjs: ${e?.stack ?? e}\n`);
    process.exitCode = LOADER_STATUS;
}
