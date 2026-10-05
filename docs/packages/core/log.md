<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:log

core:log — levels, a fixed-shape log record, and pluggable sinks.

    import "core:log"

    log.Record r = log.record(log.Level.Warn, "net", "retrying connect")
    log.emit(log.stdout_sink(), &r)

This is the mechanism — mechanism down, opinion up: it knows
nothing about workers, frames, or a barrier — that scheduling is
`app:log`'s job. What lives here is the shape every sink agrees
on and the one sink every program gets free.

**No allocation, ever.** `Tag` and `Msg` are fixed inline buffers, the
same idiom `app:ui`'s `Span` uses for names and captions: a `Record` can
sit in a caller's array — `app:log`'s per-worker ring, in particular —
with no heap involved and nothing to `delete`. The cost is the cap: a tag
or message longer than its buffer is silently truncated, same stance
`core:strings.substring` and friends take rather than failing a log call.

**No timestamp.** `core:time` is a separate dependency this package does
not need for its own sake; a sink that wants one reads the clock itself
when a record reaches it. Keeping `Record` fixed-shape and dependency-thin
is worth more than saving every sink the two extra lines.

## Declarations

19 declarations, 16 public.

* `enum i32 Level: (Trace, Debug, Info, Warn, Error)` — Ascending severity — a plain (non-`distinct`) `i32`-backed enum, so…
* `cstring level_name(Level lvl)` — `lvl`'s fixed display name. Returns `cstring` and not `string` — every…
* `const i32 TAG_CAP = 24`
* `const i32 MSG_CAP = 160`
* `type Tag: [TAG_CAP]char8`
* `type Msg: [MSG_CAP]char8`
* `(Tag t, i32 n) tag_of(string8 s)`
* `string8 tag_str(^Tag t, i32 n)` — Read the first `n` bytes of a `Tag` back as a string (a window, no copy;…
* `(Msg m, i32 n) msg_of(string8 s)`
* `string8 msg_str(^Msg m, i32 n)`
* `type Record` — One logged fact. Fixed shape, no allocation — see the package doc comment.
* `Record record(Level lvl, string8 tag, string8 msg)`
* `type SinkFn: void(^Record, rawptr)` — ── sinks ────────────────────────────────────────────────────────────────…
* `type Sink: (SinkFn fn, rawptr data)`
* `void emit(Sink sink, ^Record r)`
* `Sink stdout_sink()` — The one sink every program gets with no setup: a line per record on…
