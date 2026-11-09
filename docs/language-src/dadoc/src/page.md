//@ web/page 20
//@ ### The page: an HTML template with `dado:` markers
//@
//@ `./dado build --target=web` writes, beside `<package>.wasm`, a page that
//@ runs it in a browser: `<package>.html`, the `loader.js` it loads, and,
//@ for a `--web-threads` build, a headers file named _headers. The page comes from a
//@ template the author owns, given with `--html=<template.html>`; without
//@ one the toolchain's default is used, which holds the markers and a
//@ `<pre id="dado-output">` for the program's output.
//@
//@ A template is ordinary HTML. Where it writes `<!-- dado:NAME -->` — a
//@ comment, so the template opens and validates as a page of its own — the
//@ build writes the marker's output:
//@
//@ * `dado:importmap` — the `<script type="importmap">` mapping the
//@   JavaScript modules the build writes beside the module to their files,
//@   with each file's hash: a program that hosts DadoScript scripts has one,
//@   for the scripts the build writes beside the module.
//@   Nothing, when the build writes no such module. **Required.**
//@ * `dado:preload` — a `<link rel="modulepreload">` for each of those
//@   modules.
//@ * `dado:loader` — the `<script type="module" src="loader.js">` that
//@   fetches the module, instantiates it and runs its entry point. **Required.**
//@ * `dado:title` — a `<title>` naming the package. Write your own `<title>`
//@   instead to call the page something else.
//@ * `dado:base` — a `<base href>` from `--base=<prefix>` (default `./`), so
//@   the page works from a subpath or loads its files from another origin.
//@
//@ Every file the page references is a file of its own, never inlined, and
//@ each reference carries `integrity="sha384-…"`, so a page served under a
//@ Content-Security-Policy with no `'unsafe-inline'` runs: such a policy
//@ needs `'wasm-unsafe-eval'` in `script-src` for the module itself, and,
//@ when the import map is not empty, the hash the build prints for it.
//@
//@ A program that hosts DadoScript scripts runs on the page as it runs natively.
//@ The build writes its scripts beside the module, in `<package>.scripts/`,
//@ and names each file in the module's build manifest; the page preloads
//@ every one with its hash, the import map holds every hash, and the
//@ loader's tag names the scripts' `registry.js`, which the loader imports
//@ before the entry point runs — on a threaded page, in the worker that runs it,
//@ and again in each thread's. A script file changed after the build is
//@ refused by the browser, and the program does not start.
//@
//@ A release build (`--release`) writes the scripts as one file: every
//@ module the compiler emitted — each class's, and the runtime's — is
//@ wrapped in a function of `registry.js` itself, which also holds the
//@ registry's table, so the page preloads one file with one hash for them.
//@ The scripts' own JavaScript modules and `dado-exports.js` stay files of
//@ their own, each preloaded with its hash. Nothing in it is evaluated from
//@ a string, so the strict policy above still holds. A dev build keeps one
//@ file per module, for `#script_reload` and a readable stack. `dado build
//@ --release` warns when its page still loads more than ten files — the
//@ module and its manifest, `loader.js` and its modules, and the script
//@ files.
//@
//@ A template is refused at build time (`ERR1034`), naming the marker and
//@ the repair, when a required marker is missing, a marker is written
//@ twice, a `dado:` name is not one of the five, or a marker stands where
//@ its output cannot work: inside a `<script>`, `<style>`, `<template>`,
//@ `<textarea>` or `<title>` body; `dado:importmap` after a module script,
//@ after `dado:preload` or after `dado:loader`; `dado:base` after a marker
//@ that writes a URL.
//@
//@ On the page, standard output and standard error go to the console and
//@ into `<pre id="dado-output">` (made at the end of the body when the
//@ template has none), standard error in a `<span class="dado-stderr">`.
//@ When the program ends, its status — what the entry point returned, 134 for a trap, 2
//@ when the loader could not start it — is written into the element whose id is
//@ dado-status (made when absent), set as `data-dado-status` on the `<html>` element and
//@ on `window.dadoPage.status`, and announced with a `dado-exit` event. A
//@ `?dado-seed=N` in the page's address pins the hash seed, as
//@ `DADO_HASH_SEED` does for `./dado run`.
//@
//@ The module's stack comes first in its memory, below its data, so a
//@ recursion too deep for it runs out of bounds and traps — 134, with
//@ `dado: the module trapped` and the engine's reason on standard error,
//@ after what the program printed before it — instead of growing down into
//@ the program's globals. It is 8 MiB in every build, dev, test and
//@ `--release` alike, so a recursion that fits one fits the others. A
//@ `--web-threads` worker's stack is not guarded: it is 1 MiB from the
//@ heap, and an overflow there writes into the block below it.
//@
//@ A threaded page (`--web-threads`) must be cross-origin isolated, so the
//@ build writes _headers beside it — the static-host convention — with
//@ `Cross-Origin-Opener-Policy: same-origin` and
//@ `Cross-Origin-Embedder-Policy: require-corp`. A page served without
//@ them does not run the program: it says, on the page and on the console,
//@ which of the two headers is missing. A threaded page runs its entry point in a
//@ worker, where `threads.join` may wait, and each thread in a worker of
//@ its own.
//@
//@ `./dado run --target=web` still runs the module under node; the page is a
//@ build artefact, run by a browser.
