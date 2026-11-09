//@ web/threads 10
//@ ### `--web-threads`: a threaded web module
//@
//@ `./dado build --target=web --web-threads` (and the driver's run and test
//@ commands with the same two flags) builds a module whose threads are real. The C is compiled
//@ with wasm's atomics and bulk memory (`-matomics -mbulk-memory`), and the
//@ module imports a **shared** memory of at most 1 GiB, which the loader
//@ creates and hands to every worker it starts. `threads.spawn` becomes a
//@ host call carrying a function-table slot and a pointer; the loader answers
//@ it with a worker — node's worker threads, a `Worker` in a page —
//@ that instantiates the same module on the same memory, points its stack
//@ pointer at a 1 MiB stack the spawner allocated, and runs the thread there.
//@ Each worker gets its own thread-local block before anything runs, so the
//@ ambient allocator and every `@thread_local` variable are per thread, as
//@ they are natively. `#heap` and the console each take a lock.
//@
//@ **`threads.join` waits**, with `memory.atomic.wait32`. Node's main thread may do
//@ that; **a browser's main thread may not** — the engine traps a wait there —
//@ so a join on it is refused: it answers `false` and says why on standard
//@ error. Join from a worker instead, or have the thread set a flag the frame
//@ loop reads. A thread that traps ends the program with status 134, as an
//@ abort on a native thread ends the process.
//@
//@ **A page serving a threaded module must be cross-origin isolated**, or it
//@ has no shared memory at all. The document is served with both headers:
//@
//@     Cross-Origin-Opener-Policy: same-origin
//@     Cross-Origin-Embedder-Policy: require-corp
//@
//@ Without `--web-threads` a web build is single-threaded, its atomics are
//@ plain loads and stores, and a `threads.spawn` it reaches is refused at
//@ compile time, naming the flag (`ERR1033`). The flag with any target but
//@ the web is refused too: a native build has its threads already.
