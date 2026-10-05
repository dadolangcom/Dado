// browse.mjs — run a web build's page in a headless browser and say what it did.
//
//     node tools/web/browse.mjs build/corpus tape.html [--seed=1] [--csp=<policy>]
//                               [--no-headers] [--timeout=30000] [--lines]
//
// Serves the directory with `serve.mjs` (so a `_headers` file applies),
// opens the page in headless Chromium through Playwright, waits for the
// loader to set `data-dado-status` on `<html>`, and prints one JSON object:
// the status, what the program wrote to stdout and stderr (from
// `window.dadoPage`), the status line's text, the page's text as a reader
// sees it, whether the page was cross-origin isolated, the console, any
// uncaught error, every Content-Security-Policy violation, and every request
// that failed, and the `data-*` attributes of `<html>`. With `--lines`, one
// field per line instead — its name, a space, its value as JSON — and an
// array as one line per element, which a reader with no JSON parser splits.
// The compiler's end-to-end test of the page runs this.
//
// **Playwright is found, not installed**: `DADO_PLAYWRIGHT` (the module's
// directory), else wherever node resolves `playwright`, else `npm root -g`'s.
// Exit 3, saying so, where there is none or its browser does not start, so a
// test can skip naming the reason; 2 for a usage error; 0 otherwise, whatever
// the program's status (that is in the JSON).

import { createRequire } from "node:module";
import { execFileSync } from "node:child_process";
import path from "node:path";
import { serve } from "./serve.mjs";

function playwright() {
    const require = createRequire(import.meta.url);
    const tries = [];
    if (process.env.DADO_PLAYWRIGHT) {
        tries.push(process.env.DADO_PLAYWRIGHT);
    }
    tries.push("playwright");
    try {
        const root = execFileSync("npm", ["root", "-g"], { encoding: "utf8" }).trim();
        tries.push(path.join(root, "playwright"));
    } catch {
        // No npm: the other two are all there is.
    }
    for (const name of tries) {
        try {
            return require(name);
        } catch {
            // The next.
        }
    }
    return null;
}

async function main(argv) {
    const positional = [];
    const options = { seed: null, csp: null, headers: true, timeout: 30000, lines: false };
    for (const arg of argv) {
        if (arg.startsWith("--seed=")) {
            options.seed = arg.slice("--seed=".length);
        } else if (arg.startsWith("--csp=")) {
            options.csp = arg.slice("--csp=".length);
        } else if (arg === "--no-headers") {
            options.headers = false;
        } else if (arg === "--lines") {
            options.lines = true;
        } else if (arg.startsWith("--timeout=")) {
            options.timeout = Number(arg.slice("--timeout=".length));
        } else if (!arg.startsWith("-")) {
            positional.push(arg);
        } else {
            positional.length = 99;
        }
    }
    if (positional.length !== 2) {
        process.stderr.write(
            "usage: node browse.mjs <dir> <page.html> [--seed=N] [--csp=<policy>] [--no-headers] [--timeout=ms]\n",
        );
        return 2;
    }
    const pw = playwright();
    if (pw === null) {
        process.stderr.write("browse.mjs: no playwright module (set DADO_PLAYWRIGHT to its directory)\n");
        return 3;
    }
    let browser;
    try {
        browser = await pw.chromium.launch();
    } catch (e) {
        process.stderr.write(`browse.mjs: headless Chromium did not start: ${e.message.split("\n")[0]}\n`);
        return 3;
    }
    const { server, url } = await serve(positional[0], { port: 0, csp: options.csp, headers: options.headers });
    const report = { console: [], errors: [], violations: [], failed: [] };
    try {
        const page = await browser.newPage();
        page.on("console", (m) => report.console.push(`${m.type()}: ${m.text()}`));
        page.on("pageerror", (e) => report.errors.push(String(e)));
        page.on("requestfailed", (r) => report.failed.push(`${r.url()}: ${r.failure()?.errorText}`));
        page.on("response", (r) => {
            if (r.status() >= 400) {
                report.failed.push(`${r.url()}: ${r.status()}`);
            }
        });
        await page.addInitScript(() => {
            globalThis.__toyViolations = [];
            document.addEventListener("securitypolicyviolation", (e) => {
                globalThis.__toyViolations.push(`${e.violatedDirective} ${e.blockedURI}`);
            });
        });
        const query = options.seed === null ? "" : `?dado-seed=${encodeURIComponent(options.seed)}`;
        await page.goto(`${url}${positional[1]}${query}`);
        let timedOut = false;
        try {
            await page.waitForFunction(() => document.documentElement.dataset.dadoStatus !== undefined, null, {
                timeout: options.timeout,
            });
        } catch {
            timedOut = true;
        }
        Object.assign(
            report,
            await page.evaluate(() => ({
                status: globalThis.dadoPage?.status ?? null,
                attribute: document.documentElement.dataset.dadoStatus ?? null,
                stdout: globalThis.dadoPage?.stdout ?? null,
                stderr: globalThis.dadoPage?.stderr ?? null,
                statusText: document.getElementById("dado-status")?.textContent ?? null,
                output: document.getElementById("dado-output")?.textContent ?? null,
                isolated: globalThis.crossOriginIsolated === true,
                title: document.title,
                violations: globalThis.__toyViolations ?? [],
                data: Object.entries(document.documentElement.dataset).map(([k, v]) => `${k}=${v}`),
            })),
        );
        report.timedOut = timedOut;
    } finally {
        await browser.close();
        server.close();
    }
    if (options.lines) {
        let text = "";
        for (const [key, value] of Object.entries(report)) {
            for (const item of Array.isArray(value) ? value : [value]) {
                text += `${key} ${JSON.stringify(item)}\n`;
            }
        }
        process.stdout.write(text);
    } else {
        process.stdout.write(`${JSON.stringify(report)}\n`);
    }
    return 0;
}

process.exitCode = await main(process.argv.slice(2)).catch((e) => {
    process.stderr.write(`browse.mjs: ${e.stack}\n`);
    return 2;
});
