<!-- dadoc 1.0.0-rc.4 -->
<!-- commit 703d96d30a51 (dirty) -->
# core:time

core:time — measuring how long something took, and waiting.

    import "core:time"

    time.Instant started = time.instant()
    build()
    time.Duration took = time.elapsed(started)
    println("took ", time.as_milliseconds(took), "ms")

    time.sleep(time.milliseconds(250))

Two clocks, and the difference between them is the whole package. The
monotonic clock (`instant`) never goes backwards and has no meaningful
origin, so **only differences mean anything** — it is what you measure with.
The wall clock (`unix`, `unix_nanos`) can be set, stepped by NTP, and
printed — it is what you stamp with. Using the second to measure is the bug
this split exists to make hard to write.

**`Duration` and `Instant` are integers of nanoseconds**, so an interval
adds, subtracts and compares exactly, at any uptime, with no rounding. A
reading in seconds or milliseconds is a conversion at the end:
`as_seconds_f64(elapsed(started))`, `as_milliseconds(elapsed(started))`.

**Tier W H; N in part. Script class S in part.** Built for the web, Linux,
macOS, the BSDs and Windows, the whole package is here. Built freestanding
(`#OS == #NONE`) there is no clock to read and nothing to wait on, so
everything that reads one — `instant`, `elapsed`, `sleep`, `unix`,
`unix_nanos`, `stopwatch`, `deadline` and the methods of `Stopwatch` and
`Deadline` that read the clock — is not declared there, and a call names
the missing function. What is arithmetic stays: `Duration`, `Instant`,
`between`/`add`, `deadline_from`, the calendar and RFC 3339, which take
their time as an argument.

**The calendar is UTC and proleptic Gregorian**, with no time zones and no
leap seconds, as Unix time itself has none. `utc` turns nanoseconds since
the epoch into a `DateTime` and `from_utc` turns one back;
`days_from_civil` and `civil_from_days` are the day count underneath, valid
for any year an `i64` of days can hold. `format_rfc3339` and `put_rfc3339`
write the timestamp form of RFC 3339 in UTC (`2026-10-08T14:03:07.25Z`),
and `parse_rfc3339` reads one with any offset and answers it in UTC.

**Measuring and timing out.** A `Stopwatch` adds up the intervals it ran
for and can be stopped and started again; a `Deadline` is an `Instant` a
timeout away, which a loop asks whether it has passed and how long is left.
Each has `_at` forms that take the current `Instant` as an argument, so the
arithmetic can be tested, and run in a script, without a clock.

**Failure: `Error`, 2600 to 2699**, a range no other package's codes use.
`from_utc`, `put_rfc3339`, `format_rfc3339` and `parse_rfc3339` are
`Error!`, and `message(why)` answers a sentence for each code:

* `MALFORMED` — the text is not an RFC 3339 timestamp.
* `INVALID_DATE` — a field is out of its range: a 13th month, a 31st of
  April, a 61st second.
* `OUT_OF_RANGE` — the moment is outside what an `i64` of nanoseconds holds
  (1677-09-21 to 2262-04-11), or a year RFC 3339 cannot write (0000 to 9999).
* `NO_ROOM` — the buffer handed to `put_rfc3339` is too short; `RFC3339_MAX`
  bytes always suffice.
* `NO_MEMORY` — `format_rfc3339`'s allocation failed.

**What raises into a script.** Everything written in Dado alone is promised
by the `#assert_raises` on the line above it: the `Duration` constructors
and readers, `between` and `add`, the calendar, RFC 3339 and `message`, and
the `_at` methods of `Stopwatch` and `Deadline` (through `raises_test.dado`,
since a method cannot carry the promise). What reads a clock or waits is
not: each platform's clock is a foreign call (`clock_gettime`,
`QueryPerformanceCounter`, the web host's `performance.now()`), and a
script reads its own clock.

**Thread-safety.** No package state: every function is safe from any
thread. A `Stopwatch` is a plain value, one thread at a time per value.

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

105 declarations, 53 public.

* `distinct type Duration: i64` — An interval, in nanoseconds. `distinct`, so a `Duration` is not an `i64` a…
* `Duration nanoseconds(i64 n)` — `n` nanoseconds.
* `Duration microseconds(i64 n)` — `n` microseconds. Like the three below, `n` is multiplied into nanoseconds,…
* `Duration milliseconds(i64 n)` — `n` milliseconds.
* `Duration seconds(i64 n)` — `n` whole seconds.
* `Duration minutes(i64 n)` — `n` minutes.
* `Duration hours(i64 n)` — `n` hours.
* `i64 as_nanoseconds(Duration d)` — The interval as a count of nanoseconds: its own value.
* `i64 as_microseconds(Duration d)` — The interval as whole microseconds, truncated toward zero.
* `i64 as_milliseconds(Duration d)` — The interval as whole milliseconds, truncated toward zero.
* `i64 as_seconds(Duration d)` — The interval as whole seconds, truncated toward zero.
* `f64 as_seconds_f64(Duration d)` — The interval in seconds, with its fraction, for printing.
* `distinct type Instant: i64` — A reading of the monotonic clock, in nanoseconds from an origin nobody…
* `Duration between(Instant earlier, Instant later)` — From `earlier` to `later`. Negative when the two were passed the wrong way…
* `Instant add(Instant start, Duration d)` — `start` moved by `d` — a deadline, when `d` is a timeout. It does not…
* `Instant instant()` — The monotonic clock, now. Never earlier than a reading taken before it.
* `Duration elapsed(Instant started)` — How long since `started`.
* `void sleep(Duration d)` — Wait at least `d`. **Never returns early**: each platform's one wait may…
* `i64 unix_nanos()` — Nanoseconds since the Unix epoch, 1970-01-01 UTC. Wall time — settable, and…
* `i64 unix()` — Whole seconds since the Unix epoch.
* `type Stopwatch` — An accumulating timer over the monotonic clock: it adds up the intervals…
* `void start(^self)` — Start it, or keep it running if it is already.
* `void stop(^self)` — Stop it, adding the interval since it started to its total. A…
* `void reset(^self)` — Stopped, at zero.
* `void restart(^self)` — At zero, and running from now.
* `Duration elapsed(self)` — The total it has run for, including the interval in progress.
* `void start_at(^self, Instant now)` — `start`, at the reading `now`.
* `void stop_at(^self, Instant now)` — `stop`, at the reading `now`.
* `Duration elapsed_at(self, Instant now)` — `elapsed`, at the reading `now`.
* `Stopwatch stopwatch()` — A stopwatch at zero, running from now.
* `type Deadline` — A moment on the monotonic clock by which something has to be done: a…
* `bool passed(self)` — Whether the deadline has passed: true from the moment it falls due.
* `Duration left(self)` — What is left before it falls due, and zero once it has — never…
* `bool passed_at(self, Instant now)` — `passed`, at the reading `now`.
* `Duration left_at(self, Instant now)` — `left`, at the reading `now`.
* `Deadline deadline(Duration timeout)` — The deadline `timeout` from now.
* `Deadline deadline_from(Instant start, Duration timeout)` — The deadline `timeout` after `start`. A timeout too long for the clock…
* `type DateTime` — A moment as the calendar writes it, in UTC. `month` is 1 to 12, `day` 1 to…
* `enum i32 Weekday: (Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday)` — The days of the week, in the order `weekday` counts them, Sunday first, as…
* `bool is_leap_year(i64 year)` — Whether `year` has a 29th of February.
* `i32 days_in_month(i64 year, i32 month)` — How many days `month` (1 to 12) of `year` has; 0 for a month that is not one.
* `i64 days_from_civil(i64 year, i32 month, i32 day)` — Days from 1970-01-01 to the date `year`-`month`-`day`, negative before it.
* `(i64 year, i32 month, i32 day) civil_from_days(i64 days)` — The date `days` after 1970-01-01, or before it when negative.
* `Weekday weekday_of_days(i64 days)` — The day of the week of a count of days since 1970-01-01, which was a…
* `Weekday weekday(DateTime t)` — The day of the week `t` falls on.
* `DateTime utc(i64 nanos)` — The UTC calendar moment `nanos` nanoseconds after the Unix epoch — what…
* `Error!i64 from_utc(DateTime t)` — Nanoseconds since the Unix epoch at the UTC moment `t`: `utc` the other way.
* `const i64 RFC3339_MAX = 30` — The longest text `put_rfc3339` writes: `YYYY-MM-DDThh:mm:ss.nnnnnnnnnZ`.
* `Error!i64 put_rfc3339(i64 nanos, []char8 into)` — The moment `nanos` after the Unix epoch as an RFC 3339 timestamp in UTC,…
* `Error!ref []char8 format_rfc3339(i64 nanos)` — `put_rfc3339` into new text on the ambient allocator. Fails as it does, and…
* `Error!i64 parse_rfc3339(string8 text)` — An RFC 3339 timestamp as nanoseconds since the Unix epoch, in UTC:
* `distinct enum i32 Error` — Why a calendar or RFC 3339 function failed. The numbers are this package's,…
* `string8 message(Error why)` — A sentence for each of this package's codes, and one for any other value.
