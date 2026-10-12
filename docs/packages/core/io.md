<!-- dadoc 1.0.0-rc.4 -->
<!-- commit f94cdafd7289 -->
# core:io

core:io — files and the standard streams, through C's `<stdio.h>` where
there is one, and through the host where there is not.

    try ^io.FILE f = io.open_read("notes.txt") else why:
        println("notes.txt: ", os.message(why))
        return 1
    defer _ = io.close(f)
    for:
        try ref []char8 line = io.read_line(f) else why:
            break                               // os.Error.END, or a failure
        defer #delete(line)
        ...

**Tier W H; N in part. Script class H.** Every public name exists on Linux,
macOS, the BSDs, Windows and the web. What each target can do differs:

* **Hosted** (Linux, macOS, the BSDs, Windows): the C library's streams, the
  file system, and everything below.
* **The web** (`#WEB`): standard output and standard error write through the
  host's console, and standard input is empty. **A file cannot be opened
  there yet**: `open` and its kin fail `UNSUPPORTED`, and `seek` and `tell`
  answer failure. The arm is written over a handful of private `host_`
  functions so that host calls for a file system, when the runtime has them,
  replace those functions and nothing else.
* **Freestanding** (`#NONE`): no streams at all, since there is no C library
  and no host to write to, so `FILE` and every function over one are not
  declared there and a call names the missing function. What is arithmetic
  over memory stays: `Reader`, `Cursor`, `read_all` and `copy`, over
  whatever reader and sink the program builds itself. That is deliberate,
  and it is why this package refuses `#NONE` by absence rather than with a
  `#static_assert`: `core:fmt` imports it, and builds freestanding.

`FILE` is the case a **bodiless foreign type** exists for: the standard
says a program may hold a `FILE *` and says nothing a program may do with
what it points at. Dado declares the name and nothing else, and that is
enough — `<stdio.h>` defines the type, so C spells it and C sizes it, and
this package simply never has a reason to hold one by value. On the web,
where there is no `<stdio.h>`, `FILE` is a small type of this package's own
holding a descriptor and two flags; a program holds a `^FILE` the same way.

**Readers and sinks.** A `Sink` (the builtin `(proc, data)` pair `#render`
writes into) is the write side. A `Reader` is the read side, a trait:
`Cursor` reads bytes already in memory, `StreamReader` (from `reader` and
`stdin_reader`) reads a stream, and a program's own type takes it on with
`using`. `read_all` drains a reader into new text, and `copy` pumps a reader
into a sink.

**Failure is `os.Error`**, the one code space `core:os`, `core:os/fs` and
`core:os/proc` share (`os.Error.NOT_FOUND`, `os.Error.PERMISSION`, …), read
from `errno` or `GetLastError()` at the moment of failure, and
`os.message(why)` is the sentence. `open` and its kin, `read_line`,
`read_line_into`, `read_all` and a reader's `read` are failable; the rest
answer C's own shape — a short count, `end_of_file()`, `at_end` and
`failed` — because that is what the C library knows. To delete or move a
file, `core:os/fs` has `remove_file` and `rename`.

**What raises into a script.** `cursor`, a `Cursor`'s `read`, and
`read_all` over a cursor, promised through `raises_test.dado` (a method
and a binder carry no `#assert_raises` of their own). Nothing else here
can be: this package is the boundary with C's streams, so every other
function reaches a `foreign` call or holds a `^FILE` (an opaque C type) or
the `rawptr` of a `Sink`. A script has its own `print`, and reads files
through its host.

**Thread-safety.** The C library locks each stream for each call, so two
threads may write one stream and each call's bytes stay together. Nothing
here keeps state of its own beyond the Windows console's held bytes, which
belong to the standard output and error they stand for.

**Dado writes `\n`, on every platform.** A line ends in one byte, and no
function here turns it into `\r\n`: files are opened in binary mode, and on
Windows the three standard streams are switched to binary mode the first
time this package hands one out, so a file or a pipe receives exactly the
bytes written. A reader that wants `\r\n` asks for it by writing it.

**On Windows, text is UTF-8 in Dado and UTF-16 at the system call.** A path
is converted with `MultiByteToWideChar(CP_UTF8, …)` and opened through
`_wfopen`, so a name outside the ANSI code page works. A sink over a
console converts what it is handed and writes it with `WriteConsoleW`, so
UTF-8 output reads as itself on screen whatever the console's code page is;
a sink over a file or a pipe writes the bytes unchanged. `write`,
`write_line`, `write_byte` and `write_bytes` are the C library's own and
write bytes, which a console shows in its code page — reach for a sink to
put text on a Windows console.

## Declarations

137 declarations, 63 public.

* `trait Reader` — A source of bytes: the read side of a `Sink`. A type takes it on with…
* `type Cursor using Reader` — Bytes already in memory, read from the front: `at` is how many have been…
* `os.Error!i64 read(^self, []char8 into)` — The next bytes, up to `#len(into)`; zero at the end. Never fails.
* `Cursor cursor([]char8 bytes)` — A cursor at the start of `bytes`.
* `os.Error!ref []char8 read_all(^Reader r)` — Everything `r` has left, as new text on the ambient allocator, which the…
* `os.Error!i64 copy(^Reader from, Sink to)` — Every byte `from` has left, written to `to`, answering how many were…
* `enum i32 Whence: (Start, Current, End)` — Where `seek` counts an offset from: the start of the stream, the current…
* `type FILE: (i32 fd, bool eof, bool err)` — A stream: the descriptor it reads or writes (0, 1 and 2 are the…
* `^FILE stdin()` — The page's standard input, which reads as empty.
* `^FILE stdout()` — The page's standard output: the host's console, or node's stdout.
* `^FILE stderr()` — The page's standard error.
* `os.Error!^FILE open(string8 path, string8 mode)` — Open `path` with C's `mode`: on the web, which has no file system to…
* `i32 close(^FILE stream)` — Close a stream: 0, or `end_of_file()` for `nil`. Closing a standard…
* `i32 write(^FILE stream, string8 text)` — Write `text`: 0, or `end_of_file()` when the stream cannot take it.
* `i32 write_byte(^FILE stream, char8 byte)` — Write one byte, answering it, or `end_of_file()` when it was not taken.
* `i32 flush(^FILE stream)` — 0, and nothing more yet. The runtime holds standard output until its…
* `i32 read_byte(^FILE stream)` — One byte, or `end_of_file()` when there are none left — which on the web…
* `i32 end_of_file()` — The value `read_byte` answers at the end: -1, as C's `EOF` is on every…
* `i64 read_bytes(^FILE stream, []char8 into)` — Read up to `#len(into)` bytes, answering how many arrived; short means…
* `i64 write_bytes(^FILE stream, []char8 from)` — Write a slice's bytes, answering how many were written.
* `bool at_end(^FILE stream)` — Whether a read has reached the end of the stream.
* `bool failed(^FILE stream)` — Whether a read or a write on the stream has failed.
* `void clear_errors(^FILE stream)` — Clear the end-of-file and error flags.
* `bool seek(^FILE stream, i64 offset, Whence from)` — Move the stream's position: `false` on the web, where no stream can.
* `i64 tell(^FILE stream)` — The stream's position, or -1 — always, on the web.
* `Sink sink(^FILE stream)` — A sink that writes to `stream`, borrowed for as long as the sink is used.
* `Sink stdout_sink()` — The page's standard output, as a sink.
* `Sink stderr_sink()` — The page's standard error, as a sink.
* `type FILE` — A C stream: what `open` answers and every function here takes, held…
* `^FILE stdin()` — The process's standard input. In binary mode from the first call.
* `^FILE stdout()` — The process's standard output. In binary mode from the first call.
* `^FILE stderr()` — The process's standard error. In binary mode from the first call.
* `^FILE stdin()` — The process's standard input. POSIX has no text mode: a stream is…
* `^FILE stdout()` — The process's standard output.
* `^FILE stderr()` — The process's standard error.
* `os.Error!^FILE open(string8 path, string8 mode)`
* `os.Error!^FILE open(string8 path, string8 mode)`
* `i32 close(^FILE stream)` — Flush and close `stream`: 0, or `end_of_file()` when the flush or the…
* `i32 write(^FILE stream, string8 text)` — Write `text`: 0, or `end_of_file()` when the stream did not take all of…
* `i32 write_byte(^FILE stream, char8 byte)` — `fputc` takes an `int` and always has: C's character functions are written…
* `i32 flush(^FILE stream)` — Hand what the C library holds for `stream` to the system: 0, or…
* `i32 read_byte(^FILE stream)` — One byte, or `end_of_file()` when there are none left.
* `i32 end_of_file()` — The value `read_byte` answers at the end: C's `EOF`, a negative `int`
* `i64 read_bytes(^FILE stream, []char8 into)` — Read up to `#len(into)` bytes into the caller's slice, and answer how many…
* `i64 write_bytes(^FILE stream, []char8 from)` — Write a slice's bytes, and answer how many were written. Short means the…
* `bool at_end(^FILE stream)` — Whether a read has reached the end of `stream`: the end-of-file flag,…
* `bool failed(^FILE stream)` — Whether a read or a write on `stream` has failed: the error flag, which…
* `void clear_errors(^FILE stream)` — Clear the end-of-file and error flags.
* `Sink sink(^FILE stream)` — A sink that writes to `stream`. The stream is borrowed and must outlive…
* `Sink stdout_sink()` — The process's standard output, as a sink.
* `Sink stderr_sink()` — The process's standard error, as a sink — a diagnostic that survives…
* `bool seek(^FILE stream, i64 offset, Whence from)` — Move the stream's position to `offset` bytes from `from`, answering…
* `i64 tell(^FILE stream)` — The stream's position in bytes from its start, or -1 when it has none…
* `os.Error!^FILE open_read(string8 path)` — Open `path` for reading, as bytes: `open(path, "rb")`.
* `os.Error!^FILE open_write(string8 path)` — Open `path` for writing, created if absent and emptied if present:
* `os.Error!^FILE open_append(string8 path)` — Open `path` for writing at its end, created if absent: `open(path, "ab")`.
* `i32 write_line(^FILE stream, string8 text)` — Write `text`, then a `\n`: 0, or `end_of_file()` when either failed.
* `type StreamReader using Reader` — A stream, read as a `Reader`. The stream is borrowed and must outlive…
* `os.Error!i64 read(^self, []char8 into)` — What `read_bytes` reads; fails `IO` when the read failed with…
* `StreamReader reader(^FILE stream)` — A reader over `stream`, which is borrowed and must outlive the reader.
* `StreamReader stdin_reader()` — The process's standard input, as a reader (empty on the web).
* `os.Error!ref []char8 read_line(^FILE stream)` — The next line of `stream`, as new text on the ambient allocator, which…
* `os.Error!i64 read_line_into(^FILE stream, []char8 into)` — The next line of `stream`, into the caller's own `into`, answering how…
