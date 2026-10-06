<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 2b5ebeeb9435 (dirty) -->
# app:events

app:events — retired onto channels. This file is the migration note, and the
test beside it is the pattern, running.

**There is no code here any more, on purpose.** `app:events` was a ring of
`(i32 id, rawptr data)` per job worker, drained by `poll` at the frame
barrier, dropping (and counting) what did not fit. That is a channel without
types: a bounded mailbox that many threads fill and one drains. The language
has the channel now, so each event kind a program had is one declaration of
its own, and the package that stood in for it has nothing left to say in
code.

## The migration, call by call

A program that wrote

    import ev "app:events"

    const i32 EV_HIT = 1
    private [MAX_HITS]i32 g_hits    // outlived the job, for the rawptr's sake

    void collide_job(^app.JobArgs a):
        g_hits[a.begin] = compute_damage(a)
        ev.notify(EV_HIT, rawptr(&g_hits[a.begin]), a.worker)

    !void process(f32 dt):
        [8]ev.Notification ns
        i32 n = ev.poll(ns[:])
        for i in 0..<n:
            if ns[i].id == EV_HIT:
                handle_hit(ns[i].data)

writes, with no import at all,

    #channel(256) on_hit(i32 target, i32 damage)
    private i32 g_lost_hits

    void collide_job(^app.JobArgs a):
        try #send(on_hit, a.begin, compute_damage(a)) else:
            _ = #atomic_fetch_add(&g_lost_hits, 1, #RELAXED)   // full: the sender sees it

    !void process(f32 dt):
        for target, damage in #drain(on_hit):
            handle_hit(target, damage)

and each piece of the old surface has one replacement:

  * **`const i32 EV_…` ids** → one `#channel` per kind. The `switch` on `id`
    is gone, and so is the chance of reading one kind's payload as another's.
  * **`rawptr data`** → typed parameters. Plain data is **copied into the
    slot**, so the package array that existed only to outlive the job is
    gone — the lifetime rule this package's header spent a paragraph on no
    longer exists. Data that must not be copied travels as a `ref T` or
    `ref []T`, whose ownership moves to whoever drains it (`#delete` it in
    the walk, or keep it).
  * **`notify(id, data, worker)`** → `try #send(ch, …) else …`. The send is
    lock-free from any thread, so there is no `worker` argument and no
    per-worker ring to pick.
  * **overflow dropping, and `dropped(worker)`** → **`FULL`, seen by the
    sender** at its `else`. It decides: count it, retry later, keep the
    value for the next frame, or `#delete` a `ref` it still owns. Size the
    channel for a frame's worth — a job that retries on `FULL` while the
    thread that drains is the one waiting for it (a `wait` helps by running
    jobs) spins for ever.
  * **`poll([]Notification out)`** → `for … in #drain(ch)`, or
    `#drain(ch, n)` for at most `n`. The walk commits when its loop exits.
  * **`pending()`** → `#len(ch)`.
  * **the merge order** (worker 0 first, then 1..N) → claim order, which is
    the order the sends happened in; one producer's messages stay in its
    order either way.

## Where the drain goes

**One thread drains a channel for its whole life.** Under `app:app` that is
the frame thread, and the place is `process` (or `ready`, after a `wait`),
the same place `poll` was. A debug build traps a walk from a second thread
by name; a hand-off of the draining role from one thread to another, even a
joined one, is the same trap, so pick the thread once.

## Declarations

0 declarations, 0 public.

