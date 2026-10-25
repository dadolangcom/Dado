<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 56eb0a49a5fe (dirty) -->
# core:os/proc

core:os/proc — running another program, and reading what it says.

    import "core:os"
    import "core:os/proc"
    import "core:io"

    try ^io.FILE p = proc.read("git rev-parse --short HEAD") else why:
        println("cannot start git: ", os.message(why))
        return 1
    try ref []char8 line = io.read_line(p) else:
        ...
    try proc.Ended e = proc.close(p) else why:
        ...
    switch e:
        case proc.Exited x: println("exited ", x.code)
        case proc.Signaled s: println("killed by signal ", s.signal)

The one thing this package is careful about: **`close` is where the
ending is**, not `read`. A pipe that opened tells you the shell started,
and nothing else — a command that does not exist still opens, prints to
stderr, and closes `Exited(127)`. Reading to the end and then closing is
the whole protocol, and it is the same one C has.

**How a program ended is an `Ended`**: a union of `Exited` (it returned a
code) and `Signaled` (a signal ended it), so a `switch` over one names both
and needs no `else`. A wait that could not happen is a failure, not a third
kind of ending. `ok(e)` is "exited 0".

**Failure is `os.Error`** for what the system refused — a pipe, a fork, a
program that could not be started — and **`Pipe`**, this package's own,
for a read or a write that did not move bytes: `Pipe.WAITED` when nothing
was ready in time, `Pipe.BROKE` when the other end is gone. Zero bytes from
a read is the end of the output, which is no failure.

`command` is handed to `/bin/sh` (`cmd.exe` on Windows), so it is a shell
line and not an argv: pipes, redirection and globs all work, and so does
everything an attacker would like to put in a filename. Nothing in
`collections/` escapes a shell argument; build the command yourself and
know what is in it. **`exec.dado` beside this file is the argv form, and it
is the one to reach for unless the shell is the point**; `duplex.dado` is a
child written to and read from at once.

**On Windows** the surface is the same and the shell is `cmd.exe`: a
`command` is a `cmd.exe` line (the `Windows` section below says how it is
run), an argv is quoted into one command line by Windows' own rule, and a
child and everything it starts are one job. `windows.dado` says what differs
underneath, once.

**Tier H.** Linux, macOS, the BSDs and Windows. A page and a freestanding
build have no process to start, and the package refuses both at build time.

**What raises into a script.** `ok`, promised by the `#assert_raises` above
it: it looks at a value. Nothing else is, because everything else starts,
feeds, reads or ends a child through the system, and what reports on the
process (`available`, `ignoring_sigpipe`) answers about something only
those calls make.

**Thread-safety.** A `Duplex`, a `Child` or a stream belongs to one thread
at a time. Starting children from two threads at once is not supported: the
first `duplex` decides `SIGPIPE`'s disposition unlocked, and on Windows the
table of children started is not locked either.

## Declarations

289 declarations, 33 public.

* `Pipe!i64 drain(^io.FILE stream, []char8 into, i32 timeout_ms = 0)`
* `enum i32 Errors` — What `duplex` does with the child's standard error.
* `type Duplex` — A running child and the three pipes to it. Fields are readable; change them…
* `bool started(Duplex d)` — Whether this `Duplex` is a started child: false for one nobody started (a…
* `bool ignoring_sigpipe()` — Whether the first `duplex` in this process set `SIGPIPE` to ignored — false…
* `Pipe!i64 send(^Duplex d, []char8 bytes, i32 timeout_ms = 0)` — Write as much of `bytes` as the pipe will take, waiting at most…
* `Pipe!i64 receive(^Duplex d, []char8 into, i32 timeout_ms = 0)` — Whatever the child has written to stdout, waiting at most `timeout_ms`:
* `Pipe!i64 receive_errors(^Duplex d, []char8 into, i32 timeout_ms = 0)` — `receive` for the stderr pipe, which exists only under `Errors.Pipe`;…
* `Pipe!void send_all(^Duplex d, []char8 bytes, i32 timeout_ms = 0)` — `send` until all of `bytes` is gone, or nothing moved for `timeout_ms`.
* `Pipe!Ended finished(^Duplex d)` — How the child ended, without waiting: fails `Pipe.WAITED` while it is…
* `Pipe!Ended finish(^Duplex d, i32 timeout_ms)` — `finished`, waiting up to `timeout_ms` for it (negative waits…
* `Pipe!Ended stop(^Duplex d, i32 grace_ms)` — The whole ending in one call: close every pipe, give the child `grace_ms`
* `os.Error!Duplex duplex([]string8 argv, Errors errors = Errors.Inherit)`
* `bool close_input(^Duplex d)`
* `bool terminate(^Duplex d)`
* `bool kill(^Duplex d)`
* `void close_pipes(^Duplex d)`
* `const i32 ARG_CAP = 255` — **The arguments this package will pass, at most.** Over the cap is a…
* `type Child` — A running program and the pipe its standard output arrives on. `pid` is kept…
* `os.Error!Ended exec([]string8 argv)` — Run `argv` to completion with the caller's own streams. Fails `INVALID`
* `os.Error!Child spawn([]string8 argv)` — Start `argv` and read its standard output.
* `os.Error!Ended wait(Child c)` — Close the pipe and wait for the program.
* `distinct type Exited: (i32 code)` — It returned `code`: 0 to 255 on POSIX (127 when the program could not be…
* `distinct type Signaled: (i32 signal)` — A signal ended it: `signal` is its number (`SIGKILL` is 9). On Windows,…
* `union Ended: (Exited, Signaled)` — How a program ended. A wait that could not happen is a failure of the…
* `bool ok(Ended e)` — Whether `e` means "it worked": it returned, and returned zero.
* `distinct enum i32 Pipe` — Why `drain`, `send`, `send_all`, `receive` or `receive_errors` moved no…
* `string8 message(Pipe why)` — A sentence for `why`, for a person.
* `bool available()` — Whether there is a shell to run a command line with — C99's…
* `os.Error!^io.FILE read(string8 command)` — Start `command` and read its standard output. Fails with the system's…
* `os.Error!^io.FILE write(string8 command)` — Start `command` and write to its standard input.
* `os.Error!Ended close(^io.FILE stream)` — Close the pipe and wait for the command. Fails `INVALID` for a stream…
* `os.Error!Ended run(string8 command)` — Run `command` to completion with the caller's own streams.
