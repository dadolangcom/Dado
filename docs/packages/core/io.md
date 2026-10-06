<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:io

core:io — files and the standard streams, through C's `<stdio.h>`.

`FILE` is the case a **bodiless foreign type** exists for: the standard
says a program may hold a `FILE *` and says nothing a program may do with
what it points at. Dado declares the name and nothing else, and that is
enough — `<stdio.h>` defines the type, so C spells it and C sizes it, and
this package simply never has a reason to hold one by value.

That is not a limitation being worked around. It is the accurate statement,
and it is checked: the emitted C includes the real `<stdio.h>`, so every
signature below is compared against the truth.

**Dado writes `\n`, on every platform.** A line ends in one byte, and no
function here turns it into `\r\n`: files are opened in binary mode, and on
Windows the three standard streams are switched to binary mode the first
time this package hands one out, so a file or a pipe receives exactly the
bytes written. A reader that wants `\r\n` asks for it by writing it.

**On Windows, text is UTF-8 in Dado and UTF-16 at the system call.** A path
is converted with `MultiByteToWideChar(CP_UTF8, …)` and opened, deleted and
moved through the `W` functions, so a name outside the ANSI code page works.
A sink over a console converts what it is handed and writes it with
`WriteConsoleW`, so UTF-8 output reads as itself on screen whatever the
console's code page is; a sink over a file or a pipe writes the bytes
unchanged. `write`, `write_line`, `write_byte` and `write_bytes` are the C
library's own and write bytes, which a console shows in its code page —
reach for a sink to put text on a Windows console.

## Declarations

93 declarations, 37 public.

* `Sink stdout_sink()` — The page's standard output, as a sink.
* `Sink stderr_sink()` — The page's standard error, as a sink.
* `type FILE`
* `^FILE stdin()`
* `^FILE stdout()`
* `^FILE stderr()`
* `^FILE stdin()` — POSIX has no text mode: a stream is bytes already.
* `^FILE stdout()`
* `^FILE stderr()`
* `^FILE open(cstring path, cstring mode)`
* `^FILE open(cstring path, cstring mode)`
* `^FILE open_read(cstring path)`
* `^FILE open_write(cstring path)`
* `^FILE open_append(cstring path)`
* `bool opened(^FILE stream)`
* `i32 close(^FILE stream)`
* `i32 write(^FILE stream, cstring text)` — Write bytes up to their `\0`. `fputs` finds the end by walking to that byte…
* `i32 write_line(^FILE stream, cstring text)`
* `i32 write_byte(^FILE stream, char8 byte)` — `fputc` takes an `int` and always has: C's character functions are written…
* `i32 flush(^FILE stream)`
* `i32 read_byte(^FILE stream)` — One byte, or `end_of_file()` when there are none left.
* `i32 end_of_file()`
* `i64 read_bytes(^FILE stream, []char8 into)` — Read up to `#len(into)` bytes into the caller's slice, and answer how many…
* `i64 write_bytes(^FILE stream, []char8 from)` — Write a slice's bytes, and answer how many were written. Short means the…
* `(i64 count, bool ok) read_line_into(^FILE stream, []char8 into)` — The next line, into the caller's own bytes. **This is the one to reach for.**
* `(cstring text, bool ok) read_line(^FILE stream)` — The next line as a `cstring`, without its newline, and whether there was one.
* `i32 line_capacity()` — The longest line `read_line` returns whole. Longer ones are truncated to it.
* `bool at_end(^FILE stream)`
* `bool failed(^FILE stream)`
* `void clear_errors(^FILE stream)` — Clear the end-of-file and error flags.
* `bool delete(cstring path)`
* `bool delete(cstring path)`
* `bool move(cstring from, cstring to)`
* `bool move(cstring from, cstring to)`
* `Sink sink(^FILE stream)` — A sink that writes to `stream`. The stream is borrowed and must outlive…
* `Sink stdout_sink()` — The process's standard output, as a sink.
* `Sink stderr_sink()` — The process's standard error, as a sink — a diagnostic that survives…
