<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:time

core:time — measuring how long something took, and waiting.

    import "core:time"

    time.Instant started = time.instant()
    build()
    time.Duration took = time.elapsed(started)
    println("took ", time.as_milliseconds(took), "ms")

    time.sleep(time.milliseconds(250))

Two clocks, and the difference between them is the whole package. The
monotonic clock (`instant`, `now`) never goes backwards and has no meaningful
origin, so **only differences mean anything** — it is what you measure with.
The wall clock (`unix`, `unix_nanos`) can be set, stepped by NTP, and
printed — it is what you stamp with. Using the second to measure is the bug
this split exists to make hard to write.

**Two surfaces, one clock each.** `Duration` and `Instant` are integers of
nanoseconds and are the surface to reach for: an interval adds, subtracts and
compares exactly, at any uptime, with no rounding. `now`, `since`, `ms_since`
and `nap` are the older `f64`-seconds surface, kept because other packages
call them, and they read the same clock.

**What is not here yet**, and where it will go: calendar dates, formatting
and parsing a timestamp, time zones (UTC and the local offset, with no
time-zone database) and deadlines. Each is a function over `unix_nanos` or
over an `Instant` and a `Duration`, so it adds to this surface and changes
nothing above.

**One arm per platform**, each written against that platform's own clock
API: POSIX's `clock_gettime` and `nanosleep`, Win32's
`QueryPerformanceCounter`, `GetSystemTimePreciseAsFileTime` and `Sleep`,
and the web runtime's host calls. Each arm supplies the same three private
primitives — `mono_ns`, `wall_ns` and `wait_ns` — and everything public is
written once, below the arms, over those three.

**No `.c` beside this file.** There used to be one — `sysclock.c`, with
`sysclock.h` over it — and its stated reason was that `clock_gettime`,
`nanosleep` and `CLOCK_MONOTONIC` are POSIX rather than C99, so under a
strict `-std=` glibc declares none of them without a feature-test macro, and
a feature-test macro had to live in a `.c`. The first half is still true and
measured: with no macro, `-std=c11` gets `implicit declaration of function
'clock_gettime'` and `'CLOCK_MONOTONIC' undeclared`. The second half is not:
a `define` clause on a `foreign` block is emitted above *every* `#include` in
the generated translation unit, which is exactly where a feature-test macro
has to be, because `<features.h>` fixes the whole feature set the first time
any header pulls it in. So the macro is written here and the sidecar is gone.

## Declarations

56 declarations, 22 public.

* `distinct type Duration: i64` — An interval, in nanoseconds. `distinct`, so a `Duration` is not an `i64` a…
* `Duration nanoseconds(i64 n)`
* `Duration microseconds(i64 n)`
* `Duration milliseconds(i64 n)`
* `Duration seconds(i64 n)`
* `i64 as_nanoseconds(Duration d)` — The interval as a count of each unit, truncated toward zero.
* `i64 as_microseconds(Duration d)`
* `i64 as_milliseconds(Duration d)`
* `i64 as_seconds(Duration d)`
* `f64 as_seconds_f64(Duration d)` — The interval in seconds, with its fraction, for printing.
* `distinct type Instant: i64` — A reading of the monotonic clock, in nanoseconds from an origin nobody…
* `Instant instant()` — The monotonic clock, now. Never earlier than a reading taken before it.
* `Duration between(Instant earlier, Instant later)` — From `earlier` to `later`. Negative when the two were passed the wrong way…
* `Duration elapsed(Instant started)` — How long since `started`.
* `Instant add(Instant start, Duration d)` — `start` moved by `d` — a deadline, when `d` is a timeout.
* `void sleep(Duration d)` — Wait at least `d`. **Never returns early**: each platform's one wait may…
* `i64 unix_nanos()` — Nanoseconds since the Unix epoch, 1970-01-01 UTC. Wall time — settable, and…
* `i64 unix()` — Whole seconds since the Unix epoch.
* `f64 now()` — Seconds on the monotonic clock. The origin is unspecified on purpose:
* `void nap(i32 milliseconds)` — Wait `milliseconds`, once. **May return early on POSIX**, because a signal…
* `f64 since(f64 started)` — Seconds since `started`, which is the only thing `now` is for.
* `i64 ms_since(f64 started)` — Whole milliseconds since `started`, for a program counting frames rather…
