<!-- dadoc 1.0.0-rc.4 -->
<!-- commit 703d96d30a51 (dirty) -->
# core:log

core:log — levels, a fixed-shape log record, sinks, and a default logger.

    import "core:log"

    log.info("net", "listening on 8080")          // to standard error
    log.Record r = log.record(log.Level.Warn, "net", "retrying connect")
    log.stderr_sink().take(&r)

**Tier W H; N in part. Script class S in part.** On Linux, macOS, the BSDs,
Windows and the web the whole package is here. Built freestanding (`#OS ==
#NONE`) there is no stream to write a line to, so the sinks over streams
(`stdout_sink`, `stderr_sink`, `StreamSink`) and the default logger's
functions, which write to standard error unless told otherwise, are not
declared there, and a call names the missing function. The record, its
formatting, `Sink`, `Logger`, `Filter`, `Fanout` and `discard` stay: a
freestanding program logs through a sink it writes itself.

This is the mechanism — mechanism down, opinion up: it knows nothing about
workers, frames, or a barrier — that scheduling is `app:log`'s job. What
lives here is the shape every sink agrees on, the sinks every program gets
free, the two that compose others (a `Filter` keeps records at or above a
level, a `Fanout` hands each record to several sinks), and a default logger
for a program that wants none of that and calls `log.info`.

**The default logger** writes to standard error, keeping `Info` and above,
until `set_default` says otherwise. `trace`, `debug`, `info`, `warn` and
`error` make a record and hand it to it. It is one package variable: set it
once, before any thread logs, and read it from any.

**No allocation, ever.** `Tag` and `Msg` are fixed inline buffers, the same
idiom `app:ui`'s `Span` uses for names and captions: a `Record` can sit in a
caller's array — `app:log`'s per-worker ring, in particular — with no heap
involved and nothing to `delete`. The cost is the cap:

* **A message longer than `MSG_CAP` (160 bytes) is cut, and says so**: it
  keeps as many whole characters as fit before a closing `...`, so a reader
  of the log sees that something is missing, and a multi-byte character is
  never split. A program with more to say logs it in more than one record.
* **A tag longer than `TAG_CAP` (24 bytes) is cut to its first 24 bytes**,
  silently: a tag names a subsystem and is short by design.

**One write per record.** The stream sinks format a record into one line,
`LEVEL [tag] message\n` (`put_line`), and write it with one call, so lines
from two threads do not interleave mid-line.

**No timestamp, by default.** `core:time` is a separate dependency this
package does not need for its own sake; a sink that wants one reads the
clock itself when a record reaches it. Keeping `Record` fixed-shape and
dependency-thin is worth more than saving every sink the two extra lines.

**What raises into a script.** The record and its text — `level_label`,
`tag_of`, `tag_str`, `msg_of`, `msg_str`, `record` and `put_line` — and
`enabled` and `discard`, each promised by the `#assert_raises` above it.
Nothing that hands a record to a sink is (`write`, the default logger's
functions, `take` through a `Sink`): a call through a `Sink` may reach any
implementer in the program, the stream sinks among them, and those reach
`core:io`'s `fwrite`.

## Declarations

49 declarations, 38 public.

* `enum i32 Level: (Trace, Debug, Info, Warn, Error)` — Ascending severity — a plain (non-`distinct`) `i32`-backed enum, so…
* `string8 level_label(Level lvl)` — `lvl`'s fixed display name: `TRACE`, `DEBUG`, `INFO`, `WARN` or `ERROR`.
* `const i32 TAG_CAP = 24` — The longest tag a record keeps: a subsystem or scope name, "net",…
* `const i32 MSG_CAP = 160` — The longest message a record keeps: one line's worth. A longer one is cut…
* `type Tag: [TAG_CAP]char8` — A tag's bytes, of which a record's `tag_len` are used.
* `type Msg: [MSG_CAP]char8` — A message's bytes, of which a record's `msg_len` are used.
* `(Tag t, i32 n) tag_of(string8 s)` — `s` as a `Tag` and how many of its bytes are used: all of `s`, or its…
* `string8 tag_str(^Tag t, i32 n)` — Read the first `n` bytes of a `Tag` back as a string (a window, no copy;…
* `(Msg m, i32 n) msg_of(string8 s)` — `s` as a `Msg` and how many of its bytes are used: all of `s` when it…
* `string8 msg_str(^Msg m, i32 n)` — Read the first `n` bytes of a `Msg` back as a string (a window, no copy).
* `type Record` — One logged fact. Fixed shape, no allocation — see the package doc comment.
* `Record record(Level lvl, string8 tag, string8 msg)` — A record of `msg` at `lvl` under `tag`, each cut to its buffer as…
* `const i32 LINE_MAX = 5 + 1 + TAG_CAP + 3 + MSG_CAP + 1` — The most bytes `put_line` writes: the longest level name, the tag in…
* `i64 put_line(^Record r, []char8 into)` — `r` as the line the stream sinks write — `LEVEL [tag] message\n`, or…
* `trait Sink` — Where records go. `take` is handed each record by address and may write it,…
* `void take(^self, ^Record r)`
* `Sink discard()` — A sink that drops every record: for a `Logger` that should say nothing, or…
* `type Filter using Sink` — A minimum level in front of a sink: it hands on the records at `min` or…
* `void take(^self, ^Record r)` — Hand `r` to `next` when it is at `min` or above.
* `const i32 FANOUT_CAP = 8` — The most sinks a `Fanout` holds.
* `type Fanout using Sink` — Several sinks behind one: it hands each record to every sink added, in the…
* `bool add(^self, Sink s)` — Add `s`, answering whether there was room for it. Hand it a `Sink`
* `void take(^self, ^Record r)` — Hand `r` to every sink added.
* `type Logger: (Sink sink, Level min)` — A sink and the least severe level it is handed: what the default logger…
* `bool enabled(Logger l, Level lvl)` — Whether `l` would hand on a record at `lvl`.
* `void write(Logger l, Level lvl, string8 tag, string8 msg)` — A record of `msg` at `lvl` under `tag`, to `l`'s sink when `lvl` is at…
* `type StreamSink using Sink` — A line per record at `min` or above on `stream`, which the program…
* `void take(^self, ^Record r)` — Write `r` as one line on `stream`, when it is at `min` or above.
* `void take(^self, ^Record r)`
* `Sink stdout_sink()` — A line per record on `io.stdout()`. One package variable serves every…
* `Sink stderr_sink()` — A line per record on `io.stderr()`, where a diagnostic survives…
* `Logger default_logger()` — The default logger: standard error at `Info` and above, until…
* `void set_default(Logger l)` — Replace the default logger. Call it before any thread logs. Its sink…
* `void trace(string8 tag, string8 msg)` — `msg` under `tag`, at `Trace`, to the default logger.
* `void debug(string8 tag, string8 msg)` — `msg` under `tag`, at `Debug`, to the default logger.
* `void info(string8 tag, string8 msg)` — `msg` under `tag`, at `Info`, to the default logger.
* `void warn(string8 tag, string8 msg)` — `msg` under `tag`, at `Warn`, to the default logger.
* `void error(string8 tag, string8 msg)` — `msg` under `tag`, at `Error`, to the default logger.
