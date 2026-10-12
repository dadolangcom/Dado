<!-- dadoc 1.0.0-rc.4 -->
<!-- commit 703d96d30a51 (dirty) -->
# core:jobs

core:jobs — a general task system and the worker pool the runtime parallelises
itself with. A job is the C-classic pair — Dado has no closures — of a
function reference plus a data pointer. The surface is
stable across two engines — a **serial** one (zero worker threads: every job
runs inline on submit) and a **pooled** one (a central MPMC queue behind a
mutex+condvar, N worker threads, batch completion via atomic counters). A
caller writes the same code either way; threads are only how fast it goes.

**A job hands its result back through a channel.** It has no return value and
no handle to hang one on, so it `#send`s into a queue channel the caller
declared, and the caller drains it — once the batch is joined, or on the
next frame, whichever it wants:

    #channel(64) tile_done(i32 tile, i64 checksum)
    private i32 g_tiles_lost

    void render_tile(^jobs.JobArgs a):
        for t in a.begin..<a.end:
            try #send(tile_done, t, checksum_of(t)) else:
                _ = #atomic_fetch_add(&g_tiles_lost, 1, #RELAXED)

    void frame(^jobs.Scheduler s):
        jobs.parallel_for(s, 64, 4, render_tile, nil)
        for tile, checksum in #drain(tile_done):
            show(tile, checksum)

A value is copied into the channel's slot, so nothing the job wrote has to
outlive it; a `ref` moves to the thread that drains. **Size the channel for
the batch.** The thread that drains is usually the one waiting, and a wait
helps by running jobs on the waiting thread — so a job that retried on
`FULL` there would be waiting on itself. Count the refusal instead, as
above, or keep the value for a later batch. And **drain from one thread** —
the one that submits is the natural choice — for the channel's whole life.

So the scheduler's only answer about a job is *finished or not*: `wait` for a
handle, or `parallel_for`, which joins before it returns. There is no
polling of a result array through a handle.

Scheduler home is `core:jobs`, not `app:jobs`: no `app`
dependency, broadly reusable; `app:app` owns one instance and layers the frame
barrier on top. Engine v1 is the simplest correct one: a
central queue, tuned for "obviously correct under a sanitiser," not throughput.

**The concurrency contract.** The ambient allocator is per thread, so a job
allocates the way any Dado code does: a bare `#make`/`#format` in a job
reads *its worker thread's* `#default`, which is the root — C's
heap — unless the job opened a `using`, and a `using` a job opens is that
worker's alone and is undone when its suite ends. **A job runs with its
worker's ambient, not with its submitter's**: a `using` around `submit` or
`parallel_for` does not reach a worker, because it changed the ambient on
the submitting thread only. So the worker's private scratch arena is handed
over as `JobArgs.scratch`, and the idiom for temporaries is `using
ja.scratch:` inside the job (or an explicit `#make(T, n, ja.scratch)`) —
both allocate from an arena no other worker touches, and it is reset with
`reset_scratch`, so nothing a job means to keep goes there. `core:runtime`'s
scratch is per thread too, so a job may use `enter_temp`/`close_temp`; it
is its worker's, and the worker's first scope configures it. A
`parallel_for` chunk owns a disjoint index range, so nothing here is shared
across workers; **a map or any other structure a job shares with another
thread is not synchronised** and needs a `core:threads` mutex.

**Tier W H.** The pooled engine runs wherever `core:threads` does: Linux,
macOS, the BSDs, Windows, and a web module built with `--web-threads`. A
single-threaded web module has no thread to start, so there `start` runs
the serial engine whatever `nthreads` asks for, and answers true;
`Scheduler.nworkers` says which engine runs. A `#NONE` build is refused by
the static assertion below: the pool is `core:threads`', and a freestanding
target has no second thread to run it on.

**The scratch arenas grow on the allocator `start` was called under**
(`#default`: C's heap, unless a `using` around `start` says otherwise), and
keep it until `stop`. That is the one allocator this package captures, and
it is captured on purpose: the arenas outlive the `using` they were made in
and are reset and grown from every worker thread, whose own ambient is the
root.

**What raises into a script.** Nothing here. A job is a function reference
and a `rawptr` to its data, the scheduler's handles are `core:threads`'
thread and lock handles, and its counters are atomic accesses, so every
function reaches something a script cannot hold. A script that wants work done in parallel
asks Dado code that owns a scheduler.

This is a `core:` library and owes tests: it is a concurrency engine and
wraps core:threads.

## The atomics are the language's, and the ordering is written at each call

Every atomic access below is a `#atomic_*` intrinsic over an ordinary `i32`
the scheduler already owns, with its ordering written at the access. The
verb takes the ordering as a constant, so the C compiler emits exactly that
instruction; `core:threads`' `atomic_*` functions take an `Order` argument
and pick the verb with a `switch`, which costs nothing once inlined but
says less at the call than the verb does. Each ordering here is chosen:

  * **the batch counter's decrement is `#ACQ_REL`** — release, so that a
    thread which sees the count reach zero sees everything the job wrote
    before it; acquire, so that the decrementing thread sees the other
    chunks' decrements. This is the one pairing the whole barrier rests on.
  * **every read of a batch counter is `#ACQUIRE`**, which is the other half
    of it: `join_batch` is what observes zero.
  * **`running` is `#RELEASE` to store and `#ACQUIRE` to read**, the ordinary
    shutdown-flag pairing.
  * **`hnext` is `#RELAXED`**, and that is the one worth defending: it is a
    ticket dispenser and nothing is published through it. Every ticket is
    distinct because the increment is atomic, and the *slot* it names is then
    claimed by a compare-exchange that carries its own ordering. A stronger
    ordering here would buy nothing and cost a fence per submission.

Each `Scheduler` keeps its own workers' identities, so any number of them
may run at once, each with its own pool.

## Declarations

28 declarations, 14 public.

* `const i32 MAX_WORKERS = 64` — The most participants one scheduler has: the calling thread and up to 63…
* `const i32 QCAP = 8192` — The job ring's capacity. A submission into a full ring runs on the…
* `const i32 HCAP = 8192` — The batch-counter ring's capacity: how many batches can be unfinished at…
* `type JobArgs` — What a job sees. `scratch` is this worker's private arena — `using…
* `type JobFn: void(^JobArgs)` — A job: called with its `JobArgs`, once per `submit` and once per chunk of a…
* `distinct type Handle: i32` — A completion token. Serial handles are always done; a pooled handle names a…
* `type Scheduler` — The scheduler, owned by the caller: a package variable, since the inline…
* `bool start(^Scheduler s, i32 nthreads)` — Starts the scheduler with `nthreads` worker threads (0 is the serial…
* `void stop(^Scheduler s)` — Stops the pool: lets the workers drain the queue, wakes and joins them,…
* `void reset_scratch(^Scheduler s)` — Resets every participant's scratch arena, taking back everything jobs…
* `Handle submit(^Scheduler s, JobFn fn, rawptr data)` — Submits one job. The serial engine runs it now, on this thread; the pooled…
* `Handle submit_after(^Scheduler s, JobFn fn, rawptr data, Handle dep)` — Submits `fn` to run after `dep` has finished. It waits for `dep` on this…
* `void parallel_for(^Scheduler s, i32 n, i32 grain, JobFn fn, rawptr data)` — Runs `fn` over [0, n) in chunks of `grain` indices (1 when `grain` is…
* `void wait(^Scheduler s, Handle h)` — Waits for a handle's job to finish, running other queued jobs on this…
