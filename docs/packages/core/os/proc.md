<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:os/proc

core:os/proc — running another program, and reading what it says.

    import "core:os/proc"
    import "core:io"

    ^io.FILE p = proc.read("git rev-parse --short HEAD")
    (cstring line, bool ok) = io.read_line(p)
    (bool exited, i32 status) = proc.close(p)

The one thing this package is careful about: **`close` is where the exit
status is**, not `read`. A pipe that opened tells you the shell started, and
nothing else — a command that does not exist still opens, prints to stderr,
and closes with 127. Reading to the end and then closing is the whole
protocol, and it is the same one C has.

`command` is handed to `/bin/sh`, so it is a shell line and not an argv:
pipes, redirection and globs all work, and so does everything an attacker
would like to put in a filename.

**`exec.dado` beside this file is the argv form, and it is the one to reach
for unless the shell is the point.** This paragraph used to end *"there is no
`exec` form yet, because that needs an array of strings crossing into C and
`foreign` has no spelling for one"*, which was measured false: `rawptr`
crosses, at a stated cost written on the `execvp` declaration there.

What these three functions cost, measured on one value — `a b ; touch FILE ;`
— handed to `read` the way a caller assembling a command would: the space
split one argument into two, the `;` **ran `touch`**, and its output arrived
in the caller's own pipe. The same value with a `'` in it instead killed the
shell line with *Unterminated quoted string* and came back as `(true, 2)`,
which no caller can tell from a program that ran and exited 2. Nothing in
`collections/` escapes a shell argument and there is no helper here that
would; build the command yourself and know what is in it.

**`command` is a `cstring` everywhere below.** It exists only to reach
`popen` and `system`, both of which read to a `\0`, so the parameter says so
and the caller says how it got one: a literal — which is what a shell line
usually is — already is one, and a command a program assembled goes through
`strings.clone_to_cstring` and a written `cstring(…)`, which is where the
copy that assembling it needs becomes visible.

## Declarations

273 declarations, 55 public.

* `const i32 DRAINED = 0` — What `drain` answers when it did not read anything, told apart. A caller that…
* `const i32 WAITED = 0 - 1`
* `const i32 BROKE = 0 - 2`
* `i32 drain(^io.FILE stream, []char8 into, i32 timeout_ms = 0)`
* `i32 drain(^io.FILE stream, []char8 into, i32 timeout_ms = 0)` — Read whatever `stream` has written, waiting at most `timeout_ms`.
* `const i32 ERRORS_INHERIT = 0` — What `duplex` does with the child's standard error.
* `const i32 ERRORS_PIPE = 1`
* `const i32 ERRORS_MERGE = 2`
* `const i32 ERRORS_DISCARD = 3`
* `type Duplex` — A running child and the three pipes to it. Fields are readable; change them…
* `bool started(Duplex d)` — Whether `duplex` started a child. A `Duplex` that did not holds no…
* `bool ignoring_sigpipe()` — Whether the first `duplex` in this process set `SIGPIPE` to ignored — false…
* `(i64 sent, i32 stopped) send_all(^Duplex d, []char8 bytes, i32 timeout_ms = 0)` — `send` until all of `bytes` is gone, or nothing moved for `timeout_ms`.
* `(bool exited, i32 status) stop(^Duplex d, i32 grace_ms)` — The whole ending in one call: close every pipe, give the child `grace_ms` to…
* `Duplex duplex([]cstring argv, i32 errors = ERRORS_INHERIT)`
* `i32 send(^Duplex d, []char8 bytes, i32 timeout_ms = 0)`
* `i32 receive(^Duplex d, []char8 into, i32 timeout_ms = 0)`
* `i32 receive_errors(^Duplex d, []char8 into, i32 timeout_ms = 0)`
* `bool close_input(^Duplex d)`
* `bool terminate(^Duplex d)`
* `bool kill(^Duplex d)`
* `(bool done, bool exited, i32 status) finished(^Duplex d)`
* `(bool done, bool exited, i32 status) finish(^Duplex d, i32 timeout_ms)`
* `void release(^Duplex d)`
* `Duplex duplex([]cstring argv, i32 errors = ERRORS_INHERIT)` — Start `argv` with its stdin and stdout on pipes to this process, and its…
* `i32 send(^Duplex d, []char8 bytes, i32 timeout_ms = 0)` — Write as much of `bytes` as the pipe will take, waiting at most `timeout_ms`
* `i32 receive(^Duplex d, []char8 into, i32 timeout_ms = 0)` — Whatever the child has written to stdout, waiting at most `timeout_ms`:
* `i32 receive_errors(^Duplex d, []char8 into, i32 timeout_ms = 0)` — `receive` for the stderr pipe, which exists only under `ERRORS_PIPE`;…
* `bool close_input(^Duplex d)` — Close the child's stdin, so its next read sees end-of-file. True if this…
* `bool terminate(^Duplex d)` — Ask the child to stop (`SIGTERM`). False for a child already reaped — its…
* `bool kill(^Duplex d)` — Stop the child whether it agrees or not (`SIGKILL`). Same refusals.
* `(bool done, bool exited, i32 status) finished(^Duplex d)` — Whether the child has ended, without waiting. `done` false is "still…
* `(bool done, bool exited, i32 status) finish(^Duplex d, i32 timeout_ms)` — `finished`, waiting up to `timeout_ms` for it (negative waits indefinitely).
* `void release(^Duplex d)` — Close every descriptor the parent still holds, without waiting. The child…
* `const i32 ARG_CAP = 255` — **The argv this package will assemble, at most.** C wants a null after the…
* `type Child` — A running program and the pipe its standard output arrives on.
* `bool spawned(Child c)` — Whether a `spawn` started anything.
* `(bool exited, i32 status) exec([]cstring argv)`
* `Child spawn([]cstring argv)`
* `(bool exited, i32 status) wait(Child c)`
* `(bool exited, i32 status) exec([]cstring argv)` — Run `argv` to completion with the caller's own streams, so its output goes…
* `Child spawn([]cstring argv)` — Start `argv` and read its standard output. The `read` twin.
* `(bool exited, i32 status) wait(Child c)` — Close the pipe and wait for the program. The `close` twin, and the same…
* `bool opened(^io.FILE stream)`
* `bool available()` — Whether there is a shell to run any of this with — C99's `system(NULL)`.
* `bool ok(bool exited, i32 status)` — Whether a `(exited, status)` pair means "it worked": ended by returning, and…
* `^io.FILE read(cstring command)` — Start `command` and read its standard output; nil when the pipe or the…
* `^io.FILE write(cstring command)` — Start `command` and write to its standard input.
* `(bool exited, i32 status) close(^io.FILE stream)` — Close the pipe and wait for the command. `(false, -1)` for a stream…
* `(bool exited, i32 status) run(cstring command)` — Run `command` to completion with the caller's own streams.
* `^io.FILE read(cstring command)` — Start `command` and read its standard output. `nil` when the pipe or the…
* `^io.FILE write(cstring command)` — Start `command` and write to its standard input.
* `(bool exited, i32 status) close(^io.FILE stream)` — Wait for the command and report how it ended.
* `(bool exited, i32 status) run(cstring command)` — Run `command` to completion with the caller's own streams — so its output…
* `ref []char8 command_line_utf8([]cstring argv)` — The UTF-8 command line `argv` quotes to: the program, then each…
