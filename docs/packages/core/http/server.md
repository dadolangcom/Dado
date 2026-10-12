<!-- dadoc 1.0.0-rc.4 -->
<!-- commit 549ca2ce6a97 -->
# core:http/server

core:http/server — a small web server that reads like Python's.

    import "core:http/server"

    server.Response index(server.Request req):
        return server.html("<h1>hello</h1>")

    server.Response user(server.Request req):
        return server.text(#format("user ", req.param("id"), "\n"))

    i32 main():
        server.App app = server.app()
        app.get("/", index)
        app.get("/users/{id}", user)
        app.files("/static", "public")
        return app.run(port: 8000)

**The high level is a handler that takes a `Request` and returns a
`Response`.** A route pattern is a path whose `{name}` segments capture
(`req.param("name")`) and whose last segment may be `{name...}`, which
captures the rest of the path. The query and form fields decode on asking
(`req.query("q")`, `req.form("name")`), headers are asked by name in any case
(`req.header("content-type")`), and the response is made by one call —
`text`, `html`, `json`, `redirect`, `status` — and adjusted by chaining
(`.with_header(…)`, `.with_status(…)`). HEAD is answered for every GET
route; a path that matches a route of another method is answered 405 with
`Allow`; anything else is 404, or what `app.on_not_found` returns.

**The low level is a route whose handler writes the response itself**:
`app.raw("GET", "/events", stream)` hands it the request and a `^Writer`
that sets the status and the headers and then writes the body in as many
pieces as it likes, flushed to the connection as they come — no
`Content-Length`, the body ends when the connection closes. And the `App`'s
slots are its configuration, set before `run`: the address to bind, the most
bytes a request may take, how long to wait for them, whether to keep a
connection open between requests, whether to log.

**Tier [W] H. Script class S for the router and the wire, T for serving.**
On Linux, macOS, the BSDs and Windows the whole package runs, over
`core:http`'s socket arms. On the web it compiles, `run` and `listen` fail
as `core:http.listen` does there (`UNSUPPORTED`: a page has no listening
socket), and `App.handle` — bytes in, bytes out — still routes and answers,
with a `files` route answering 501 since a page has no directory to read.
A target with no operating system is refused, by the `#static_assert` below.

**What raises into a script.** The pieces that are bytes in and bytes out
are promised, each by the `#assert_raises` above it: `parse`, `decode`,
`content_type`, `respond`, `text`, `html`, `json`, `redirect`, `status`,
`not_found`, `bad_request`, `escape`, `serialize`, `serialize_with`,
`reason` and `app`, and the methods of `Request`, `Response` and the route
table through `raises_test.dado`. Not promised: `App.handle`, `run`,
`listen`, `step` and the `Writer`'s writing, because a `files` route reads
the disk (`core:os/fs`) and a writer on a connection sends through
`core:http`'s sockets — both the platform's C. A route's handler is the
caller's, and is promised or not by its own author.

**Handlers are plain functions**, with no state of their own: a handler
that needs some keeps it in its package. A route holding state of its own
(a context the app carries, or a trait) is a later addition.

**Memory is per request and is nobody's job.** Each request runs inside its
own `core:runtime` scratch frame, installed as `#default`, so everything a
handler allocates — a `#format`ted body, a decoded query value, a file it
read — is reclaimed when the response has been sent. A handler never frees
anything, and must not keep a view of anything past its return.

**One request at a time**, on the thread that calls `run` or `step`: the
accept is non-blocking and the read waits at most `read_timeout_ms`. A
request body may come with `Content-Length` or chunked. By default a
connection is answered and closed (`Connection: close`); set
`keep_alive_ms` and a client that asks to keep it gets its next request
read on the same connection, waiting that long for it and at most
`keep_alive_requests` times — which, one connection at a time, holds every
other client off for as long as it waits, and is why it is off unless asked.
There is no TLS. That is the server this is: a tool's dashboard, a dev
server for a web build, a webhook — not a front door on the internet, which
is also why it binds `127.0.0.1` unless `host` says otherwise.

## Declarations

87 declarations, 60 public.

* `type Header: (string8 name, string8 value)` — One `name: value` pair: a request header, a response header, or a route's…
* `const i32 MAX_HEADERS = 100` — The most headers a request may carry; one more is answered 431.
* `type Request` — One parsed request. Its strings view the bytes it was parsed from.
* `string8 header(self, string8 name, string8 fallback = "")` — The value of the header `name`, matched without regard to case, or…
* `bool has_header(self, string8 name)` — Whether the request carries the header `name`, in any case.
* `string8 param(self, string8 name, string8 fallback = "")` — What the route's `{name}` captured, or `fallback`. A capture is a view of…
* `string8 query(self, string8 name, string8 fallback = "")` — The query parameter `name`, percent-decoded (`+` read as a space), or…
* `bool has_query(self, string8 name)` — Whether the query names `name`, with or without a value.
* `string8 form(self, string8 name, string8 fallback = "")` — A field of an `application/x-www-form-urlencoded` body, decoded as…
* `bool is(self, string8 method)` — Whether the request's method is `method`, which is case-sensitive as…
* `distinct type Refused: (i32 status)` — The request cannot be served: `status` is what to answer instead — 400 (it…
* `union Parsed: (Request, http.Pending, Refused)` — What `parse` answers: the request, more bytes owed (`core:http`'s…
* `Parsed parse(string8 raw)` — Parse one request from `raw`. Answers the `Request`, or `http.Pending`
* `string8 decode(string8 s)` — Percent-decode `s`, reading `+` as a space. A string with nothing to decode…
* `type Response` — A status, the headers to send with it, and a body.
* `Response with_header(self, string8 name, string8 value)` — A copy with `name: value` added. Headers are sent in the order added,…
* `Response with_status(self, i32 status)` — A copy with the status changed.
* `string8 header(self, string8 name)` — The value of the header `name`, matched without regard to case, or "".
* `Response respond(string8 body, string8 content_type, i32 status = 200)` — A body of `content_type`, sent as given.
* `Response text(string8 body, i32 status = 200)` — A plain-text body, UTF-8.
* `Response html(string8 body, i32 status = 200)` — An HTML page, UTF-8.
* `Response json(string8 body, i32 status = 200)` — The body is sent as written: this server has no JSON encoder of its own.
* `Response redirect(string8 location, i32 status = 302)` — A redirect to `location`: 302 unless asked for another (301, 303, 307, 308).
* `Response status(i32 code)` — An empty response with only a status — 204, say — or one whose body is the…
* `Response not_found()` — 404, with its reason phrase as the body.
* `Response bad_request()` — 400, with its reason phrase as the body.
* `string8 escape(string8 s)` — `s` with `&`, `<`, `>`, `"` and `'` written as HTML's entities, for text from…
* `ref []char8 serialize(Response r, bool head_only = false)` — The bytes of `r` as an HTTP/1.1 response, on `#default`: the status line, the…
* `ref []char8 serialize_with(Response r, bool head_only, []Header extra, bool keep_alive = false)` — `serialize`, with `extra` headers before the response's own — the app's…
* `string8 reason(i32 code)` — The reason phrase HTTP gives `code`, or "" for one it does not name.
* `type Handler: Response(Request)` — A handler: the request in, the response out.
* `type RawHandler: void(Request, ^Writer)` — A raw handler: the request, and the writer it answers through.
* `distinct type Files: (string8 dir)` — A `files` route: the directory it serves.
* `union Action: (Handler, RawHandler, Files)` — What a route does with a request that matches it: hand it to a handler,…
* `type Route: (string8 method, string8 pattern, Action action)` — One entry of an app's route table, as `get`, `raw` or `files` adds it.
* `type App` — An app: its routes, and its configuration, set before `run` or `listen`.
* `void get(^self, string8 pattern, Handler h)` — A route for GET (and so HEAD) on `pattern`.
* `void post(^self, string8 pattern, Handler h)` — A route for POST on `pattern`.
* `void put(^self, string8 pattern, Handler h)` — A route for PUT on `pattern`.
* `void patch(^self, string8 pattern, Handler h)` — A route for PATCH on `pattern`.
* `void delete(^self, string8 pattern, Handler h)` — A route for DELETE on `pattern`.
* `void route(^self, string8 method, string8 pattern, Handler h)` — A route for `method` — any method's name, or "" for every method.
* `void raw(^self, string8 method, string8 pattern, RawHandler h)` — A route whose handler writes the response itself, through a `Writer`.
* `void files(^self, string8 prefix, string8 dir)` — Serve the files under `dir` at `prefix`: `/static/app.js` is…
* `void header(^self, string8 name, string8 value)` — Send `name: value` with every response this app makes — a policy…
* `void on_not_found(^self, Handler h)` — What answers a request no route matches, in place of the 404.
* `i32 run(^self, u16 port = 8000, i64 max_requests = 0, f64 seconds = 0.0)` — Serve on `port` until `max_requests` have been answered or `seconds`
* `http.Error!void listen(^self, u16 port)` — Start listening on `port` at `host`, for a caller that runs its own…
* `bool step(^self)` — Answer at most one waiting connection — every request it carries, when…
* `void close(^self)` — Stop listening. Routes and configuration are kept, so `listen` may be…
* `void destroy(^self)` — Free what the app holds. It is not usable after.
* `ref []char8 handle(^self, string8 raw)` — Answer one complete request given as bytes, without a socket — the whole…
* `App app()` — A new app with the default configuration: bound to `127.0.0.1`, 1 MiB…
* `string8 content_type(string8 path)` — The content type for `path`'s extension.
* `type Writer` — Where one response goes: a connection, or (for `App.handle`) a buffer.
* `void set_status(^self, i32 code)` — Set the status the head will carry: 200 unless set.
* `void header(^self, string8 name, string8 value)` — Add a header to the head. After the first `write` it is too late, and…
* `bool write(^self, string8 data)` — Send `data` as the next piece of the body, sending the head first if it…
* `void flush_head(^self)` — Send the head now, with no body yet.
* `bool ok(self)` — Whether the peer is still there to write to.
