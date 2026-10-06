<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:threads

core:threads — portable threads, locks, condition variables and atomics.

A thin, framework-agnostic wrapper over POSIX threads, and the foundation
core:jobs builds its worker pool on. Handles are opaque `rawptr`s; a thread
entry is an ordinary Dado function reference passed to C as a function
pointer.

**It is Dado over the C library, with no C of its own.** A `foreign` block
names the header's own types and functions and cc checks the restatement
against `<pthread.h>`; the atomics are the language's `#atomic_*` verbs.

`pthread_join` takes a `pthread_t` **by value**, and that is the call this
package carried a two-file C shim for. It does not need one: a bodiless
`foreign` type is complete in C, so `pthread_t` crosses by value and `cc`
supplies the size — which is the only place the size was ever a question,
since restating it as an integer is true of glibc and false of Darwin,
where it is a pointer.

This is a `core:` library, not `app:` — it owes tests, because it wraps
lower-level C and OS code: a concurrency engine is exactly what a test is
for.

## What a spawned thread shares, and what it does not

**Its own:** the ambient allocator — a new thread starts at the root, C's
heap, never at the `using` its spawner has open, and a `using` it opens is
undone on it alone — and `core:runtime`'s scratch, whose depth, checkpoints
and arena are per thread (its first temp scope configures it). Any
package-level variable declared `@thread_local` is per thread the same way,
starting at its initializer.

**Shared, and not synchronised by anything but you:** every other
package-level variable, everything a pointer reaches, and every map — two
threads writing one `{K: V}`, even on disjoint keys, corrupt its table. Guard
those with `mutex_new`/`lock`/`unlock`, or hand each thread its own. Build a
test that spawns with `./dado test --sanitize=thread` as well as the default.

## Declarations

72 declarations, 31 public.

* `type ThreadFn: rawptr(rawptr)` — A thread entry: takes the argument pointer it was spawned with, returns a…
* `rawptr spawn(ThreadFn fn, rawptr arg)`
* `bool join(rawptr t)`
* `i32 hardware_concurrency()`
* `rawptr mutex_new()`
* `void lock(rawptr m)`
* `void unlock(rawptr m)`
* `void mutex_free(rawptr m)`
* `rawptr cond_new()`
* `void cond_wait(rawptr c, rawptr m)`
* `bool cond_wait_for(rawptr c, rawptr m, i64 ms)` — At most `ms` milliseconds; true when woken. A single-threaded module…
* `void cond_signal(rawptr c)`
* `void cond_broadcast(rawptr c)`
* `void cond_free(rawptr c)`
* `rawptr spawn(ThreadFn fn, rawptr arg)` — Spawn a thread running `fn(arg)`. Returns an opaque handle, or nil on failure.
* `bool join(rawptr t)` — Join and free a thread handle. True on success; a nil handle is a failure.
* `i32 hardware_concurrency()` — The online processor count, at least 1.
* `rawptr mutex_new()`
* `void lock(rawptr m)`
* `void unlock(rawptr m)`
* `void mutex_free(rawptr m)`
* `rawptr cond_new()`
* `void cond_wait(rawptr c, rawptr m)`
* `bool cond_wait_for(rawptr c, rawptr m, i64 ms)` — `cond_wait` for at most `ms` milliseconds: true when woken, false when…
* `void cond_signal(rawptr c)`
* `void cond_broadcast(rawptr c)`
* `void cond_free(rawptr c)`
* `i32 atomic_load(^i32 p)`
* `void atomic_store(^i32 p, i32 v)`
* `i32 atomic_fetch_add(^i32 p, i32 d)`
* `bool atomic_cas(^i32 p, i32 expected, i32 desired)`

## channels/back-pressure

### Back-pressure on a full queue

A queue never drops and never blocks, so a full one hands the decision to
the producer, in the `else` of its `try #send`. There are three honest
answers: **drop and count** (input nobody will miss), **keep it and try
again later** (work that must arrive: hold it in the producer and retry on
the next tick, as below), and **retry now** (a worker whose consumer is
another thread that is draining — the fan-in above).

```dado
package pressure

#channel(16) sample(i32 seq, f64 value)

type Pending: (bool held, i32 seq, f64 value, i32 dropped)

// Called once per tick. The held sample goes first, so none overtakes
// another; while it cannot go, the new one is dropped and counted.
void tick(^Pending p, i32 seq, f64 value):
    if p.held:
        try #send(sample, p.seq, p.value) else:
            p.dropped += 1
            return
        p.held = false
    try #send(sample, seq, value) else:
        p.held = true
        p.seq = seq
        p.value = value
```

**Full includes what a walk has taken and not yet committed.** A `#drain`
commits when its loop ends, so while the walk runs the entries it has
yielded still count against the capacity. A consumer that sends into the
queue it is draining, from inside that walk, and retries until the `#send`
succeeds, spins forever: the room it waits for appears only when its own
loop ends. Send into another channel, or keep the message and send after the
loop.


## channels/fan-in

### Fan-in from workers

Every worker sends into one queue; one thread drains it. A worker that
finds the queue full retries — the consumer is behind, and it catches up
at its next walk — so nothing is lost and no worker blocks on a lock.

```dado
package fan_in

import "core:threads"

const i32 WORKERS = 4

#channel(64) done(i32 worker, i64 value)

i32 next_id = 0

rawptr work(rawptr arg):
    i32 me = threads.atomic_fetch_add(&next_id, 1)
    for job in 0..<100:
        i64 value = i64(me) * 1000 + i64(job)
        for:
            try #send(done, me, value) else:
                continue
            break
    return nil

i64 run():
    [4]rawptr pool
    for i in 0..<WORKERS:
        pool[i] = threads.spawn(work, nil)
    i64 total = 0
    i32 seen = 0
    for seen < WORKERS * 100:
        for worker, value in #drain(done):
            total += value
            seen += 1
    for i in 0..<WORKERS:
        _ = threads.join(pool[i])
    return total
```


## channels/hand-off

### Handing consumption to another thread: `#release`

A channel has one consumer at a time, and a debug build fixes it at the
first walk. A program that moves the consuming to another thread — a
loader that drains until the scene is up and then hands over to the frame
thread — says so with `#release(ch)` where the first thread is done:
the next thread to walk the channel becomes its consumer. It is a consumer
statement, written on its own line and never inside a walk of the same
channel. With assertions it clears the owner word after checking this
thread holds it; with `--no-assert` it compiles to nothing. Nothing makes
the two threads take turns: the program orders them, as with `threads.join`
below, and a walk that overlaps another thread's still traps.

```dado
package hand_off

import "core:threads"

#channel(16) scene(i32 part)

i32 loaded = 0

rawptr frames(rawptr arg):
    for part in #drain(scene):
        loaded += part
    return nil

i32 load():
    for part in #drain(scene):
        loaded += part
    #release(scene)
    rawptr t = threads.spawn(frames, nil)
    _ = threads.join(t)
    return loaded
```


## channels/jobs

### Results from `core:jobs`

A job sends what it computed instead of writing into an array the caller
polls: the result carries its own index, the caller drains after the
barrier, and a result that arrives is complete — the `#send` published it.
Size the queue for the chunks in flight, or let a job retry.

```dado
package job_results

import "core:jobs"

#channel(64) partial(i32 begin, i64 sum)

void sum_chunk(^jobs.JobArgs a):
    i64 s = 0
    for i in a.begin..<a.end:
        s += i64(i)
    for:
        try #send(partial, a.begin, s) else:
            continue
        break

i64 total(^jobs.Scheduler s, i32 n):
    jobs.parallel_for(s, n, 256, sum_chunk, nil)
    i64 t = 0
    for begin, sum in #drain(partial):
        t += sum
    return t
```

The drain after the fork-join is complete only when every chunk fits in the
queue at once: here 64 chunks of 256, so `n` up to 16384. Past that, drain
while the batch runs, or use a bigger queue.


## channels/latest

### Coalesced state: `latest`

For state where only the newest value matters — a window size, a setting, a
target position — `#channel(latest)` holds one value. A `#send` replaces
what nobody took and cannot fail; the consumer's walk yields the newest
value once, or nothing when there is nothing new.

```dado
package coalesce

#channel(latest) target(f32 x, f32 y)

void track(f32 x, f32 y):
    #send(target, x, y)

(f32, f32) follow(f32 at_x, f32 at_y):
    f32 x = at_x
    f32 y = at_y
    for tx, ty in #drain(target):
        x = tx
        y = ty
    return (x, y)
```


## channels/per-frame

### Per-frame input

An input backend sends events as they happen, on its own thread; the frame
drains them at one point, before it updates. A frame that must bound its
work takes at most a budget with `#drain(ch, n)` and leaves the rest
queued for the next frame, in order.

```dado
package per_frame

#channel(256) key(i32 code, bool down)

i32 dropped = 0

void on_key(i32 code, bool down):
    try #send(key, code, down) else:
        dropped += 1

i32 frame(^[512]bool held):
    i32 handled = 0
    for code, down in #drain(key, 64):
        held[code & 511] = down
        handled += 1
    return handled
```


## channels/request-response

### Request and response: two channels

A channel is one-way, so a question and its answer are two: the server
consumes requests, the client consumes responses, and each has exactly one
consumer. The request carries an id the response echoes, because answers
need not come back in the order questions went out.

```dado
package ask

#channel(32) request(i32 id, i32 x)
#channel(32) response(i32 id, i32 y)

// The server's thread: answer everything asked so far.
void serve():
    for id, x in #drain(request):
        for:
            try #send(response, id, x * x) else:
                continue
            break

// The client's thread: ask, and later collect what has been answered.
bool ask(i32 id, i32 x):
    try #send(request, id, x) else:
        return false
    return true

i32 collect(^[32]i32 answers):
    i32 n = 0
    for id, y in #drain(response):
        answers[id & 31] = y
        n += 1
    return n
```

The server's retry is safe because it drains one channel and sends into the
other. Retrying a `#send` into the queue a walk is draining, from inside
that walk, never succeeds: see back-pressure.

