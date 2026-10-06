<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:http

core:http — a minimal, non-blocking HTTP/1.1 server over TCP.

    import "core:http"

    http.Server s = http.listen(8080)
    if !http.valid(s):
        // port already bound
    defer http.stop(s)

    // once per frame / loop turn — never blocks:
    http.Accepted a = http.poll(s)
    if a.ok:
        if http.method() == http.Method.Get && strings.starts_with(http.path(), "/"):
            http.respond(a.conn, 200, "OK", "text/html", body, alloc)

**Why this package exists.** `core:net` is a `Reliable`/`Unreliable` wrapper
over ENet (UDP) — the shape a game session wants, and the wrong shape for a
browser or `curl`, which speak TCP and HTTP. This package is the other half:
a byte-stream socket a request/response protocol rides on. It is deliberately
small — enough to serve a status page from a tool's own event loop, not a
general web framework — and it owes tests, because it stands directly on the
platform's BSD sockets.

**This package has no C of its own.** `<sys/socket.h>`, `<netinet/in.h>`,
`<arpa/inet.h>`, `<sys/time.h>`, `<fcntl.h>` and `<unistd.h>` are bound
directly and the bodies below are the whole implementation, layouts
included. What that owns, and what the two platforms disagree about, is
written beside `SockAddrIn` and above `send_flags`.

**Non-blocking by construction, single-threaded by design.** The listen
socket is non-blocking, so `poll` accepts a waiting connection or answers
"nothing" immediately — it is meant to be called from one place, one turn of
a caller's loop, the same contract `app:input.poll` and `core:net.poll`
already have. There is no worker thread and no queue here: a caller that
wants concurrency puts it above this line, not inside it.

**One request buffer, reused.** Like `core:io.read_line`, the parsed method
and path live in package-level storage that the next `poll` overwrites — so
read `method()` / `path()` (or copy what you need out of them) before polling
again. That is the single-threaded contract made concrete, and it is why this
package holds no per-connection state beyond the descriptor `Accepted` hands
back.

## Declarations

75 declarations, 22 public.

* `distinct type Server: i32`
* `distinct type Conn: i32`
* `const i32 PATHCAP = 512`
* `const i32 REQCAP = 16384`
* `enum i32 Method: (Get, Post, Head, Options, Other)`
* `type Accepted: (bool ok, Conn conn)` — What `poll` hands back: whether a connection was accepted this turn, and the…
* `Server listen(u16 port)` — Start listening on `port` (all local interfaces). Check `valid` before use —…
* `bool valid(Server s)`
* `void stop(Server s)` — Stop the server. The listen socket is closed; connections already handed to a…
* `Accepted poll(Server s)` — Accept at most one waiting connection and read its request line, or answer…
* `string8 body()` — The body of the request `poll` last accepted — the bytes after the blank line…
* `Method method()` — The method of the request `poll` last accepted.
* `string8 path()` — The request-target (path) of the request `poll` last accepted, as a string…
* `const i32 RESPCAP = 32768` — The largest response this package assembles at once. A status page is a few…
* `void respond(Conn c, i32 status, string8 reason, string8 ctype, string8 body)` — Write a complete HTTP/1.1 response to `c` and close it. `status`/`reason` are…
* `void respond_cors(Conn c, i32 status, string8 reason, string8 ctype, string8 body)` — Like `respond`, but with a permissive CORS header set — for an endpoint a…
* `void close_conn(Conn c)` — Close a connection without answering it — for a caller that decides, after…
* `Conn accept(Server s)` — Accept one waiting connection without reading from it, or answer a `Conn` that…
* `bool open(Conn c)` — Whether `c` is a connection, rather than the nothing `accept` answers.
* `i64 receive(Conn c, []char8 into)` — Read what has arrived on `c` into `into`, waiting at most the connection's read…
* `i64 send(Conn c, []char8 from)` — Write all of `from` to `c`. Answers how many bytes were sent; fewer than…
* `void set_read_timeout(Conn c, i32 milliseconds)` — Set how long a `receive` on `c` waits for bytes before answering -1.
