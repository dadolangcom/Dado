<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 2b5ebeeb9435 (dirty) -->
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

This is a `core:` library and owes tests: it is a concurrency engine and
wraps core:threads.

## The atomics are the language's, and the ordering is written at each call

Every atomic access below is a `#atomic_*` intrinsic over an ordinary `i32`
the scheduler already owns, and not a call to `core:threads`' wrappers. Those
are one verb each at `#SEQ_CST`, because a function cannot take an ordering
as a parameter: the ordering has to be a constant at the machine
instruction, which is exactly why `#atomic_*` takes a `#` member the checker
folds and refuses. A wrapper fixes one ordering for every caller, and this
package wants four.

So the orderings below are written per access rather than paid for uniformly,
and each is chosen rather than defaulted:

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

`core:threads` keeps its atomic wrappers and its tests: they are what a
program that holds a cell and not an ordering needs, and `jobs` not calling
them is not a reason to delete them.

## Declarations

29 declarations, 14 public.

* `const i32 MAX_WORKERS = 64`
* `const i32 QCAP = 8192`
* `const i32 HCAP = 8192`
* `type JobArgs` — What a job sees. `scratch` is this worker's private arena — `using…
* `type JobFn: void(^JobArgs)`
* `distinct type Handle: i32` — A completion token. Serial handles are always done; a pooled handle names a…
* `type Scheduler` — The scheduler. One instance, owned by the caller (a package global — the inline…
* `bool start(^Scheduler s, i32 nthreads)` — Start the scheduler with `nthreads` worker threads (0 → the serial engine, all…
* `void stop(^Scheduler s)` — Stop the pool: signal drain, wake all workers, join them, free primitives and…
* `void reset_scratch(^Scheduler s)` — Reset the per-participant scratch arenas — the app calls this once a frame.
* `Handle submit(^Scheduler s, JobFn fn, rawptr data)` — Submit one job. Serial: runs inline now. Pooled: enqueued; `wait` the handle.
* `Handle submit_after(^Scheduler s, JobFn fn, rawptr data, Handle dep)` — Submit `fn` to run after `dep` completes. v1: since callers wait on `dep`
* `void parallel_for(^Scheduler s, i32 n, i32 grain, JobFn fn, rawptr data)` — Fork-join over [0, n) in `grain`-sized chunks, then join before returning.
* `void wait(^Scheduler s, Handle h)` — Cooperatively wait for a handle: the waiting thread runs other ready jobs while…
