<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 6c56ce20508a (dirty) -->
# core:http

core:http — HTTP/1.1 over TCP: connections for a server and a client, a
small blocking client, and the wire format both are written on.

    import "core:http"
    import "core:fmt"

    try http.Response r = http.get("example.com", 80, "/") else why:
        fmt.println("get: ", http.message(why))
        return 1
    defer r.destroy()
    fmt.println(r.status, " ", r.header("content-type"))

A server is `core:http/server`, the router built on this package: it
listens with `listen`, takes each connection with `accept`, reads it with
`receive` until `request_end` says the request is whole, and answers with
`send`. A client is `get`, `post` and `request`, or `dial` and
`read_response` for one that sends its own bytes.

**Tier [W] H. Script class S for the wire, T for the sockets.** Built for
Linux, macOS, the BSDs or Windows, the whole package is here, over
`core:net`'s sockets. Built for the web it compiles and every call that
needs a socket fails `Error.UNSUPPORTED` — a page has no listening socket
and no raw TCP, and node's sockets are asynchronous where these calls wait —
while the wire (`wire.dado`) works unchanged. A target with no operating
system has no sockets, and the `#static_assert` below refuses it by name.

**What raises into a script.** The wire is pure Dado and promised, each
function by the `#assert_raises` above it: `head_end`, `head_value`,
`request_end`, `response_end`, `status_code`, `chunked_end`, `unchunk_to`,
`unchunk`, `build_request` and `parse_response`, with `message` here and
each of `Response`'s methods. What touches a socket is not
and cannot be: it reaches `core:net`, which calls the platform's C and
holds the platform's descriptors. A script that wants HTTP builds and
parses it here and asks its host for the bytes.

**Listening binds the address it is given.** `listen("127.0.0.1", port)` is
the loopback interface only, which is what a tool's status page or a dev
bridge wants; `"0.0.0.0"` (or `"::"`) is every interface, and is a decision
worth writing down where it is made. The listening socket is nonblocking,
so `accept` answers `Error.WOULD_BLOCK` at once when nobody is waiting.

**The client waits.** `get`, `post` and `request` connect, send, and read
the whole response — `Content-Length`, chunked, or until the server closes
— each step bounded by `timeout_ms`, and answer a `Response` whose bytes
are on `#default` until `destroy`. One request per connection
(`Connection: close`), IPv4 or IPv6, no TLS: `https://` is not here.

**Errors.** Every failable function fails with a member of `Error`
(codes 1100 to 1199), and `message(why)` is a sentence for each. A
connection's failures are `core:net`'s, read into this package's members
at the moment they happen.

**Thread-safety.** Nothing here shares state: the client, the wire, and
`accept`/`receive`/`send` on distinct connections are safe from any
thread.

## Declarations

48 declarations, 36 public.

* `const i64 MAX_RESPONSE = 16777216` — The most bytes a response may take unless a call says otherwise: 16 MiB.
* `Error!Response get(string8 host, u16 port, string8 target, i32 timeout_ms = 5000)` — GET `target` (a path and query, `/` when empty) from `host:port`.
* `Error!Response post(string8 host, u16 port, string8 target, string8 content_type, string8 body, i32 timeout_ms = 5000)` — POST `body`, of `content_type`, to `target` on `host:port`.
* `Error!Response request(string8 method, string8 host, u16 port, string8 target, []Header headers, string8 body, i32 timeout_ms = 5000, i64 limit = MAX_RESPONSE)` — Send `method` for `target` to `host:port` with `headers` (after `Host`) and…
* `Error!Response read_response(Conn c, i32 timeout_ms = 5000, i64 limit = MAX_RESPONSE, bool head_only = false)` — Read one whole response from `c`, which stays open: at most `limit` bytes,…
* `distinct enum i32 Error` — Why a call failed.
* `string8 message(Error why)` — A sentence for `why`, for a person: what went wrong and, where there is…
* `distinct type Server: i64` — A listening socket. End it with `stop`.
* `distinct type Conn: i64` — One accepted, or dialed, connection. End it with `close_conn`.
* `Error!Server listen(string8 host, u16 port)` — Start listening on `host:port`, where `host` is a numeric address of this…
* `void stop(Server s)` — Stop the server: the listen socket is closed. Connections already accepted…
* `Error!Conn accept(Server s)` — Accept one waiting connection without reading from it. The connection…
* `Error!Conn dial(string8 host, u16 port, i32 timeout_ms = 5000)` — Connect to `host:port` — a name or a numeric address, IPv4 or IPv6 —…
* `Error!i64 receive(Conn c, []u8 into)` — Read what has arrived on `c` into `into`, waiting at most the connection's…
* `Error!void send(Conn c, []u8 from)` — Write all of `from` to `c`, as many writes as it takes. Fails…
* `Error!void set_timeout(Conn c, i32 milliseconds)` — Give every `receive` and every `send` on `c` at most `milliseconds` to…
* `void close_conn(Conn c)` — Close a connection.
* `type Header: (string8 name, string8 value)` — One `name: value` line of a head, both views of the bytes it was read from.
* `distinct type Whole: (i64 length)` — The message is whole, and is `length` bytes: its head, the blank line and…
* `distinct type Pending: (i64 at_least)` — More bytes are owed. `at_least` is how long the message will be once whole,…
* `distinct type ToClose: (i64 body_at)` — From `response_end`: the body runs until the server closes the connection,…
* `union Extent: (Whole, Pending, ToClose)` — How far a message at the front of some bytes reaches: whole, not yet, or…
* `Error!i64 head_end(string8 raw)` — Where the head of the whole message in `raw` ends: the offset of the…
* `string8 head_value(string8 head, string8 name, string8 fallback = "")` — The value of the header `name` in `head` (the bytes before the blank line),…
* `Error!Extent request_end(string8 raw)` — How far the request at the front of `raw` reaches: `Whole` with its…
* `Error!Extent response_end(string8 raw, bool head_only = false)` — How far the response at the front of `raw` reaches: `Whole`, `Pending`, or…
* `i32 status_code(string8 head)` — The three-digit status of a status line (`HTTP/1.1 200 OK`), or 0 when the…
* `Error!Extent chunked_end(string8 body)` — How far the chunked body at the front of `body` reaches: `Whole` with every…
* `Error!i64 unchunk_to([]char8 out, string8 body)` — Write the data of the chunked body `body` into `out`, without its framing,…
* `Error!ref []char8 unchunk(string8 body)` — The data of the chunked body `body`, on `#default`. Fails…
* `ref []char8 build_request(string8 method, string8 host, u16 port, string8 target, []Header headers, string8 body)` — The bytes of an HTTP/1.1 request, on `#default`: the request line, `Host`
* `type Response` — A response a server sent: its status, its headers and its body, every one a…
* `string8 Response.header(self, string8 name, string8 fallback = "")` — The value of the header `name`, matched without regard to case, or…
* `bool Response.has_header(self, string8 name)` — Whether the response carries the header `name`, in any case.
* `void Response.destroy(^self)` — Give back the bytes the response holds. Its strings are not readable…
* `Error!Response parse_response(ref []char8 data, bool head_only = false)` — Parse the whole response in `data`, which the answer takes ownership of…
