<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 6c56ce20508a (dirty) -->
# core:threads

core:threads: threads, locks, condition variables and atomics, on every
hosted target and on the web.

    import "core:threads"

    rawptr work(rawptr arg):
        return arg

    threads.Thread t = threads.spawn(work, nil)
    try rawptr answer = threads.join_result(t) else why:
        fmt.println("join: ", threads.message(why))

**Tier W H.** Linux, macOS, the BSDs and Windows run a thread on the
operating system's own: POSIX threads on the first three, and on Windows
`_beginthreadex`, slim reader/writer locks and condition variables. On the
web a thread is a worker on the module's shared memory, which a module has
only when it is built with `--web-threads`. A single-threaded web module
has no second thread: there `spawn` is refused at compile time (ERR1033),
`try_spawn` answers nil at run time, and every lock and wait works on the
one thread there is. A `#NONE` build is refused by the static assertion
below, because a freestanding target has no scheduler to run a second
thread on and its one thread needs no lock.

**What raises into a script.** `message` alone, promised by the
`#assert_raises` above it. Everything else is a thread primitive, and could
not: spawning, joining, locking and waiting call the operating system or the
web runtime through a `foreign` block; a handle is a pointer into C's heap;
and the atomics, `Once` and `current_id` are atomic accesses. A script
shares no memory with another thread, so it has no use for any of them.

**Handles are typed**: `Thread`, `Mutex`, `Cond` and `RwLock` are each a
`distinct` pointer into C's heap, so a condition variable handed where a
mutex is asked, or two arguments swapped, is a compile error. A nil handle
is the failed one (`spawn`, `mutex_new`, `cond_new` and `rwlock_new` answer
nil when they cannot make theirs), and every function that takes a handle
ignores a nil one, so a failed `mutex_new` makes `lock` do nothing rather
than crash. `Semaphore`, `WaitGroup`, `Barrier` and `Once` are values a
caller keeps in place (a package variable, or memory every thread can
reach) and passes by address; they are written here, in Dado, over the
mutex and condition variable, so they behave alike on every target, and
their constructors fail rather than answer one that cannot work.

**Failure: `Error`, 2500 to 2599**, a range no other package's codes use.
`join_result` fails `NO_THREAD` for a nil handle, `CANNOT_WAIT` on a
browser's main thread and `JOIN_FAILED` when the system refuses; the
constructors of `Semaphore`, `WaitGroup` and `Barrier` fail `NO_MEMORY`
when their mutex or condition variable cannot be made.

**An interval is a `time.Duration`**: `cond_wait_for`,
`Semaphore.acquire_for` and `WaitGroup.wait_for` take one, measured on a
monotonic clock.

## What a spawned thread shares, and what it does not

Its own: the ambient allocator (a new thread starts at the root, C's heap,
never at the `using` its spawner has open, and a `using` it opens is undone
on it alone) and `core:runtime`'s scratch, whose depth, checkpoints and
arena are per thread (its first temp scope configures it). Any
package-level variable declared `@thread_local` is per thread the same way,
starting at its initializer.

Shared, and synchronised only by the program: every other package-level
variable, everything a pointer reaches, and every map. Two threads writing
one `{K: V}`, even on disjoint keys, corrupt its table. Guard those with
`mutex_new`, `lock` and `unlock`, or hand each thread its own. Build a test
that spawns with `./dado test --sanitize=thread` as well as the default.

## Declarations

254 declarations, 73 public.

* `distinct type Thread: rawptr` — A running thread, from `spawn` or `try_spawn`, until `join` or…
* `distinct type Mutex: rawptr` — A mutex, from `mutex_new`, until `mutex_free`. Nil is the one that could not…
* `distinct type Cond: rawptr` — A condition variable, from `cond_new`, until `cond_free`. Nil is the one…
* `distinct type RwLock: rawptr` — A reader/writer lock, from `rwlock_new`, until `rwlock_free`. Nil is the one…
* `distinct enum i32 Error` — Why a join or a constructor failed. The numbers are this package's, 2500 to…
* `string8 message(Error why)` — A sentence for each of this package's codes, and one for any other value.
* `type ThreadFn: rawptr(rawptr)` — A thread's entry point: it takes the argument it was spawned with and…
* `type OnceFn: void(rawptr)` — What `Once.call` runs: the data pointer it was given.
* `Thread spawn(ThreadFn fn, rawptr arg)` — Starts a thread running `fn(arg)` and answers its handle, or nil when the…
* `Thread try_spawn(ThreadFn fn, rawptr arg)` — `spawn` for code that may be built single-threaded: a single-threaded web…
* `bool join(Thread t)` — Waits for the thread to finish and frees its handle. Answers true when the…
* `Error!rawptr join_result(Thread t)` — Waits for the thread to finish, frees its handle, and answers the pointer…
* `i32 hardware_concurrency()` — The number of processors this program may run threads on, at least 1:
* `void yield()` — Gives the rest of this thread's time slice to another ready thread, if…
* `u64 current_id()` — The calling thread's number: 1 for the first thread that asks, then 2,…
* `Mutex mutex_new()` — A new unlocked mutex, or nil when it cannot be allocated or initialised.
* `void lock(Mutex m)` — Blocks until this thread holds `m`.
* `bool try_lock(Mutex m)` — Takes `m` if no thread holds it, without waiting. True when this thread now…
* `void unlock(Mutex m)` — Releases `m`, which this thread holds.
* `void mutex_free(Mutex m)` — Frees `m`, which no thread may hold or wait on.
* `Cond cond_new()` — A new condition variable, or nil when it cannot be allocated or…
* `void cond_wait(Cond c, Mutex m)` — Waits for a signal on `c`, releasing `m` (which this thread holds) while it…
* `bool cond_wait_for(Cond c, Mutex m, time.Duration d)` — `cond_wait` for at most `d`: true when woken, false when the time ran out…
* `void cond_signal(Cond c)` — Wakes one thread waiting on `c`, if any.
* `void cond_broadcast(Cond c)` — Wakes every thread waiting on `c`.
* `void cond_free(Cond c)` — Frees `c`, which no thread may be waiting on.
* `RwLock rwlock_new()` — A new unlocked reader/writer lock, or nil when it cannot be allocated or…
* `void read_lock(RwLock l)` — Blocks until this thread may read under `l`.
* `bool try_read_lock(RwLock l)` — Takes `l` for reading if no writer holds it, without waiting. True when…
* `void read_unlock(RwLock l)` — Ends this thread's read of `l`.
* `void write_lock(RwLock l)` — Blocks until this thread is the one writer under `l`.
* `bool try_write_lock(RwLock l)` — Takes `l` for writing if nobody holds it, without waiting. True when this…
* `void write_unlock(RwLock l)` — Ends this thread's write under `l`.
* `void rwlock_free(RwLock l)` — Frees `l`, which no thread may hold.
* `enum i32 Order: (Relaxed, Acquire, Release, AcqRel, SeqCst)` — A memory ordering, weakest first. A load given `Release` or `AcqRel`
* `T atomic_load(^$T p, Order order = Order.SeqCst) where T in (i32, u32, i64, u64, rawptr)` — The value at `p`.
* `void atomic_store(^$T p, T v, Order order = Order.SeqCst) where T in (i32, u32, i64, u64, rawptr)` — Writes `v` at `p`.
* `T atomic_exchange(^$T p, T v, Order order = Order.SeqCst) where T in (i32, u32, i64, u64, rawptr)` — Writes `v` at `p` and answers the value that was there.
* `(T, bool) atomic_compare_exchange(^$T p, T expected, T desired, Order order = Order.SeqCst) where T in (i32, u32, i64, u64, rawptr)` — Writes `desired` at `p` if `expected` is there, and answers what was there…
* `bool atomic_cas(^$T p, T expected, T desired, Order order = Order.SeqCst) where T in (i32, u32, i64, u64, rawptr)` — `atomic_compare_exchange`, answering only whether it wrote.
* `T atomic_fetch_add(^$T p, T d, Order order = Order.SeqCst) where T in (i32, u32, i64, u64)` — Adds `d` at `p` and answers the value that was there before. It wraps on…
* `T atomic_fetch_sub(^$T p, T d, Order order = Order.SeqCst) where T in (i32, u32, i64, u64)` — Subtracts `d` at `p` and answers the value that was there before. It wraps…
* `T atomic_fetch_and(^$T p, T bits, Order order = Order.SeqCst) where T in (i32, u32, i64, u64)` — Ands `bits` into the value at `p` and answers the value that was there…
* `T atomic_fetch_or(^$T p, T bits, Order order = Order.SeqCst) where T in (i32, u32, i64, u64)` — Ors `bits` into the value at `p` and answers the value that was there…
* `T atomic_fetch_xor(^$T p, T bits, Order order = Order.SeqCst) where T in (i32, u32, i64, u64)` — Xors `bits` into the value at `p` and answers the value that was there…
* `void atomic_fence(Order order = Order.SeqCst)` — Orders the memory accesses before it against those after it, as `order`
* `type AtomicBool` — A `bool` read and written atomically. The verbs take no `bool` cell, so it…
* `bool AtomicBool.load(^self, Order order = Order.SeqCst)` — The value.
* `void AtomicBool.store(^self, bool v, Order order = Order.SeqCst)` — Sets the value to `v`.
* `bool AtomicBool.exchange(^self, bool v, Order order = Order.SeqCst)` — Sets the value to `v` and answers the one it replaced.
* `bool AtomicBool.compare_exchange(^self, bool expected, bool desired, Order order = Order.SeqCst)` — Sets the value to `desired` if it is `expected`, and answers whether it…
* `type Once` — Runs a function once, whichever thread asks first. Its zero value has not…
* `void Once.call(^self, OnceFn fn, rawptr data)` — Runs `fn(data)` if no call has run it yet, and returns once it has run.
* `bool Once.done(^self)` — Whether the function has run to its end.
* `type Semaphore` — A counting semaphore: `acquire` takes one unit, waiting while there are…
* `void Semaphore.acquire(^self)` — Takes a unit, waiting until one is free.
* `bool Semaphore.try_acquire(^self)` — Takes a unit if one is free, without waiting. True when it took one.
* `bool Semaphore.acquire_for(^self, time.Duration d)` — Takes a unit, waiting at most `d` for one. True when it took one.
* `void Semaphore.release(^self, i64 n = 1)` — Gives back `n` units (one by default), waking as many waiters. A count…
* `i64 Semaphore.available(^self)` — The units free now, which may change before the caller reads it.
* `void Semaphore.destroy(^self)` — Frees its mutex and condition variable. No thread may be waiting.
* `Error!Semaphore semaphore(i64 initial)` — A semaphore holding `initial` units (0 when `initial` is negative). Fails…
* `type WaitGroup` — Waits for a counted set of tasks: `add` counts them in, each task calls…
* `void WaitGroup.add(^self, i64 n)` — Adds `n` to the count. When the count reaches 0 or below it is 0, and…
* `void WaitGroup.done(^self)` — One task finished: `add(-1)`.
* `void WaitGroup.wait(^self)` — Waits until the count is 0.
* `bool WaitGroup.wait_for(^self, time.Duration d)` — Waits at most `d` for the count to reach 0. True when it did.
* `void WaitGroup.destroy(^self)` — Frees its mutex and condition variable. No thread may be waiting.
* `Error!WaitGroup wait_group()` — A wait group with a count of 0. Fails `NO_MEMORY` when its mutex or…
* `type Barrier` — Holds each of a fixed number of threads at `wait` until all of them have…
* `bool Barrier.wait(^self)` — Waits until `parties` threads have called `wait` in this round. Answers…
* `void Barrier.destroy(^self)` — Frees its mutex and condition variable. No thread may be waiting.
* `Error!Barrier barrier(i32 parties)` — A barrier for `parties` threads (1 when `parties` is below 1). Fails…

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
    [4]threads.Thread pool
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
    threads.Thread t = threads.spawn(frames, nil)
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

