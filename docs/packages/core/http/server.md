<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
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
`Allow`; anything else is 404, or what `app.fallback` returns.

**The low level is a route whose handler writes the response itself**:
`app.raw("GET", "/events", stream)` hands it the request and a `^Writer`
that sets the status and the headers and then writes the body in as many
pieces as it likes, flushed to the connection as they come — no
`Content-Length`, the body ends when the connection closes. And the `App`'s
slots are its configuration, set before `run`: the most bytes a request may
take, how long to wait for them, whether to log.

**Memory is per request and is nobody's job.** Each request runs inside its
own `core:runtime` scratch frame, installed as `#default`, so everything a
handler allocates — a `#format`ted body, a decoded query value, a file it
read — is reclaimed when the response has been sent. A handler never frees
anything, and must not keep a view of anything past its return.

**One request at a time**, on the thread that calls `run` or `step`: the
accept is non-blocking and the read waits at most `read_timeout_ms`. A
connection is answered and closed (`Connection: close`); there is no
keep-alive, no TLS and no chunked request body. That is the server this is:
a tool's dashboard, a dev server for a web build, a webhook — not a front
door on the internet.

## Declarations

83 declarations, 65 public.

* `type Header: (string8 name, string8 value)` — One `name: value` pair: a request header, a response header, or a route's…
* `const i32 MAX_HEADERS = 100` — The most headers a request may carry; one more is answered 431.
* `type Request`
* `string8 header(self, string8 name, string8 fallback = "")` — The value of the header `name`, matched without regard to case, or…
* `bool has_header(self, string8 name)`
* `string8 param(self, string8 name, string8 fallback = "")` — What the route's `{name}` captured, or `fallback`. A capture is a view of…
* `string8 query(self, string8 name, string8 fallback = "")` — The query parameter `name`, percent-decoded (`+` read as a space), or…
* `bool has_query(self, string8 name)`
* `string8 form(self, string8 name, string8 fallback = "")` — A field of an `application/x-www-form-urlencoded` body, decoded as…
* `bool is(self, string8 method)` — Whether the request's method is `method`, which is case-sensitive as…
* `const i32 PARSE_DONE = 0` — How a parse ended: `Done`, `Partial` (the headers have not all arrived, or…
* `const i32 PARSE_PARTIAL = 1`
* `(Request req, i32 outcome) parse(string8 raw)` — Parse one request from `raw`. Answers the request and `PARSE_DONE`, or…
* `(i64 total, bool ok) expected_size(string8 raw)` — How many bytes the whole request in `raw` takes, once its head has arrived:
* `bool same_fold(string8 a, string8 b)` — ASCII case-insensitive equality: header names are case-insensitive.
* `string8 decode(string8 s)` — Percent-decode `s`, reading `+` as a space. A string with nothing to decode…
* `type Response`
* `Response with_header(self, string8 name, string8 value)` — A copy with `name: value` added. Headers are sent in the order added,…
* `Response with_status(self, i32 status)` — A copy with the status changed.
* `string8 header(self, string8 name)` — The value of the header `name`, matched without regard to case, or "".
* `Response respond(string8 body, string8 content_type, i32 status = 200)` — A body of `content_type`, sent as given.
* `Response text(string8 body, i32 status = 200)`
* `Response html(string8 body, i32 status = 200)`
* `Response json(string8 body, i32 status = 200)` — The body is sent as written: this server has no JSON encoder of its own.
* `Response redirect(string8 location, i32 status = 302)` — A redirect to `location`: 302 unless asked for another (301, 303, 307, 308).
* `Response status(i32 code)` — An empty response with only a status — 204, say — or one whose body is the…
* `Response not_found()`
* `Response bad_request()`
* `string8 escape(string8 s)` — `s` with `&`, `<`, `>`, `"` and `'` written as HTML's entities, for text from…
* `ref []char8 serialize(Response r, bool head_only = false)` — The bytes of `r` as an HTTP/1.1 response, on `#default`: the status line, the…
* `ref []char8 serialize_with(Response r, bool head_only, []Header extra)` — `serialize`, with `extra` headers after the response's own — the app's…
* `string8 reason(i32 code)` — The reason phrase HTTP gives `code`, or "" for one it does not name.
* `type Handler: Response(Request)` — A handler: the request in, the response out.
* `type RawHandler: void(Request, ^Writer)` — A raw handler: the request, and the writer it answers through.
* `type Route`
* `type App`
* `void get(^self, string8 pattern, Handler h)`
* `void post(^self, string8 pattern, Handler h)`
* `void put(^self, string8 pattern, Handler h)`
* `void patch(^self, string8 pattern, Handler h)`
* `void delete(^self, string8 pattern, Handler h)`
* `void route(^self, string8 method, string8 pattern, Handler h)` — A route for `method` — any method's name, or "" for every method.
* `void raw(^self, string8 method, string8 pattern, RawHandler h)` — A route whose handler writes the response itself, through a `Writer`.
* `void files(^self, string8 prefix, string8 dir)` — Serve the files under `dir` at `prefix`: `/static/app.js` is…
* `void header(^self, string8 name, string8 value)` — Send `name: value` with every response this app makes — a policy…
* `void on_not_found(^self, Handler h)` — What answers a request no route matches, in place of the 404.
* `i32 run(^self, u16 port = 8000, i64 max_requests = 0, f64 seconds = 0.0)` — Serve on `port` until `max_requests` have been answered or `seconds`
* `bool listen(^self, u16 port)` — Start listening on `port`, for a caller that runs its own loop with…
* `bool step(^self)` — Answer at most one waiting request, and say whether there was one. Never…
* `void close(^self)`
* `void destroy(^self)` — Free what the app holds. It is not usable after.
* `ref []char8 handle(^self, string8 raw)` — Answer one complete request given as bytes, without a socket — the whole…
* `App app()` — A new app with the default configuration: 1 MiB requests, a 5 s read.
* `(bool hit, ref []Header params) match(string8 pattern, string8 path, bool prefix = false)` — Whether `path` matches `pattern`, and what its `{name}`s captured. Segments…
* `string8 content_type(string8 path)` — The content type for `path`'s extension.
* `type Writer`
* `void set_status(^self, i32 code)`
* `void header(^self, string8 name, string8 value)` — Add a header to the head. After the first `write` it is too late, and…
* `bool write(^self, string8 data)` — Send `data` as the next piece of the body, sending the head first if it…
* `void flush_head(^self)` — Send the head now, with no body yet.
* `bool ok(self)` — Whether the peer is still there to write to.
* `void send_response(^self, Response r, bool head_only)` — A whole response at once, with its `Content-Length`.
* `void finish(^self)` — What a raw handler that wrote nothing still owes: its head.
* `Writer connection_writer(http.Conn c)`
* `Writer buffered_writer()`
