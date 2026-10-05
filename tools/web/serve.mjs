// serve.mjs — serve a web build's directory the way a static host does.
//
//     node tools/web/serve.mjs build/corpus [--port=8000]
//
// A page `./dado build --target=web` writes is a set of files — the page,
// `loader.js`, the module, its manifest — that a browser fetches over HTTP:
// opened from `file://` a module script does not load. This is the smallest
// host that serves them, with nothing to install: node's own `http`.
//
// **`_headers`.** A threaded build writes a `_headers` file beside its page
// (the convention Netlify and Cloudflare Pages read): blocks of a path
// pattern, `*` matching anything, followed by indented `Name: value` lines.
// This server applies them to every response whose path matches, so a
// threaded page served from here is cross-origin isolated exactly as it would
// be on such a host. `--no-headers` ignores the file, to see the page a host
// that does not read it serves.
//
// **`--csp=<policy>`** adds a `Content-Security-Policy` header to every
// response — how the page's proof shows that a page with no inline script
// runs under a policy without `'unsafe-inline'`.
//
// **A directory with no `index.html`** answers with its pages instead of a
// 404: one page is redirected to, several are listed. A web build writes
// `<package>.html`, so the URL printed at start opens the page as it stands.
//
// Importable too: `serve(dir, { port, csp, headers })` answers
// `{ server, url }` once listening; `port: 0` takes a free one.

import http from "node:http";
import fs from "node:fs";
import path from "node:path";

const TYPES = {
    ".html": "text/html; charset=utf-8",
    ".js": "text/javascript; charset=utf-8",
    ".mjs": "text/javascript; charset=utf-8",
    ".json": "application/json",
    ".wasm": "application/wasm",
    ".css": "text/css; charset=utf-8",
    ".build": "text/plain; charset=utf-8",
    ".txt": "text/plain; charset=utf-8",
    ".svg": "image/svg+xml",
    ".png": "image/png",
};

/** `_headers`' blocks: `[{ pattern: RegExp, headers: [[name, value]] }]`. */
export function parseHeaders(text) {
    const rules = [];
    for (const line of text.split("\n")) {
        if (line.trim() === "" || line.trim().startsWith("#")) {
            continue;
        }
        if (!/^\s/.test(line)) {
            const escaped = line.trim().replace(/[.+?^${}()|[\]\\]/g, "\\$&").replace(/\*/g, ".*");
            rules.push({ pattern: new RegExp(`^${escaped}$`), headers: [] });
            continue;
        }
        const colon = line.indexOf(":");
        if (colon > 0 && rules.length > 0) {
            rules[rules.length - 1].headers.push([line.slice(0, colon).trim(), line.slice(colon + 1).trim()]);
        }
    }
    return rules;
}

/** The `.html` files directly in `dir`, sorted. */
function pagesIn(dir) {
    try {
        return fs.readdirSync(dir).filter((f) => f.endsWith(".html") && !f.startsWith(".")).sort();
    } catch {
        return [];
    }
}

function escapeHtml(s) {
    return s.replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[c]);
}

/** Serve `dir`. */
export function serve(dir, { port = 8000, host = "127.0.0.1", csp = null, headers = true } = {}) {
    const root = path.resolve(dir);
    let rules = [];
    const headersFile = path.join(root, "_headers");
    if (headers && fs.existsSync(headersFile)) {
        rules = parseHeaders(fs.readFileSync(headersFile, "utf8"));
    }
    const server = http.createServer((request, response) => {
        const url = new URL(request.url, "http://localhost");
        let pathname = decodeURIComponent(url.pathname);
        const asked = pathname;
        if (pathname.endsWith("/")) {
            pathname += "index.html";
        }
        const file = path.join(root, pathname);
        if (!file.startsWith(root + path.sep) || pathname.split("/").includes("_headers")) {
            response.writeHead(404).end();
            return;
        }
        if (asked.endsWith("/") && !fs.existsSync(file)) {
            const pages = pagesIn(path.dirname(file));
            if (pages.length === 1) {
                response.writeHead(302, { Location: asked + encodeURIComponent(pages[0]), "Cache-Control": "no-store" });
                response.end();
                return;
            }
            if (pages.length > 1) {
                const items = pages.map((p) => `<li><a href="${escapeHtml(encodeURIComponent(p))}">${escapeHtml(p)}</a></li>`);
                const body = `<!doctype html><meta charset="utf-8"><title>pages</title><ul>${items.join("")}</ul>\n`;
                response.writeHead(200, { "Content-Type": "text/html; charset=utf-8", "Cache-Control": "no-store" });
                response.end(request.method === "HEAD" ? undefined : body);
                return;
            }
        }
        for (const rule of rules) {
            if (rule.pattern.test(pathname)) {
                for (const [name, value] of rule.headers) {
                    response.setHeader(name, value);
                }
            }
        }
        if (csp !== null) {
            response.setHeader("Content-Security-Policy", csp);
        }
        response.setHeader("Cache-Control", "no-store");
        fs.readFile(file, (error, body) => {
            if (error) {
                response.writeHead(404, { "Content-Type": "text/plain" });
                response.end(request.method === "HEAD" ? undefined : `no ${pathname}\n`);
                return;
            }
            response.writeHead(200, {
                "Content-Type": TYPES[path.extname(file)] ?? "application/octet-stream",
                "Content-Length": body.length,
            });
            response.end(request.method === "HEAD" ? undefined : body);
        });
    });
    return new Promise((resolve, reject) => {
        server.once("error", reject);
        server.listen(port, host, () => {
            resolve({ server, url: `http://${host}:${server.address().port}/` });
        });
    });
}

async function main(argv) {
    let dir = null;
    const options = {};
    for (const arg of argv) {
        if (arg.startsWith("--port=")) {
            options.port = Number(arg.slice("--port=".length));
        } else if (arg.startsWith("--csp=")) {
            options.csp = arg.slice("--csp=".length);
        } else if (arg === "--no-headers") {
            options.headers = false;
        } else if (dir === null && !arg.startsWith("-")) {
            dir = arg;
        } else {
            throw new Error("usage: node serve.mjs <dir> [--port=N] [--csp=<policy>] [--no-headers]");
        }
    }
    const { url } = await serve(dir ?? ".", options);
    process.stdout.write(`serving ${path.resolve(dir ?? ".")} at ${url}\n`);
    for (const page of pagesIn(path.resolve(dir ?? "."))) {
        process.stdout.write(`  ${url}${encodeURIComponent(page)}\n`);
    }
}

if (process.argv[1] && import.meta.url === (await import("node:url")).pathToFileURL(fs.realpathSync(process.argv[1])).href) {
    main(process.argv.slice(2)).catch((e) => {
        process.stderr.write(`serve.mjs: ${e.message}\n`);
        process.exitCode = 2;
    });
}
