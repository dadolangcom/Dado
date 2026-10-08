<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:json

a JSON writer (RFC 8259): values appended to text it owns.

    json.Writer w = json.writer(indent: 2)
    w.object()
    w.key("name")
    w.text("oak")
    w.key("sizes")
    w.array()
    w.int(600)
    w.float(18.5)
    w.end_array()
    w.end_object()
    try ref []char8 text = w.finish() else code:
        fmt.println(json.message(code))
        return
    defer #delete(text)

`text` is then:

    {
      "name": "oak",
      "sizes": [
        600,
        18.5
      ]
    }

The text grows on the allocator that was `#default` when `writer` was
called, so `json.writer() using a` puts it in `a`. Every call costs time in
proportion to what it writes.

Strings escape `"`, `\`, the control characters, DEL, U+0080 to U+009F,
U+2028 and U+2029, and pass other valid UTF-8 through. A byte that is not
valid UTF-8 becomes U+FFFD. Floats are the shortest text that reads back as
the same value. Keys are written in the order given, and nothing checks that
a key is not repeated.

A call that cannot write what it was asked fails the writer with a code:
the code is kept, later calls write nothing, and `finish` fails with it.
`message(code)` answers a sentence for each code.

It has no `foreign` block and no package state. It builds for every target,
`#NONE` and the web included, and every function raises into a script.

## Declarations

41 declarations, 25 public.

* `const i32 EXPECTED_KEY = 1` — A failure code: a value written inside an object with no key before it.
* `const i32 UNEXPECTED_KEY = 2` — A failure code: a key outside an object, or a second key before the first…
* `const i32 UNBALANCED = 3` — A failure code: an end that closes nothing or the wrong kind of container,…
* `const i32 NOT_FINITE = 4` — A failure code: `float` or `float32` was given a NaN or an infinity, which…
* `const i32 TOO_DEEP = 5` — A failure code: objects and arrays nested deeper than `MAX_DEPTH`.
* `const i32 NO_MEMORY = 6` — A failure code: the text could not grow.
* `const i32 MAX_DEPTH = 128` — How many objects and arrays may be open at once.
* `string8 message(i32 code)` — A sentence describing `code`, one of this package's failure codes; any…
* `type Writer` — One JSON text being written: make one with `writer`, write exactly one…
* `void object(^self)` — Opens an object: `{`. Inside it, write `key` before each value, then…
* `void end_object(^self)` — Closes the innermost open object: `}`. Fails with `UNBALANCED` when it…
* `void array(^self)` — Opens an array: `[`. Values written next are its elements, up to…
* `void end_array(^self)` — Closes the innermost open array: `]`. Fails with `UNBALANCED` when it is…
* `void key(^self, string8 name)` — Writes `name` as the next member's name, a JSON string escaped the way…
* `void text(^self, string8 s)` — Writes `s` as a JSON string. `"`, `\`, the control characters, DEL,…
* `void int(^self, i64 v)` — Writes `v` in decimal: `-42`. Fails with `EXPECTED_KEY` in an object with…
* `void uint(^self, u64 v)` — Writes `v` in decimal: `18446744073709551615`. Fails with `EXPECTED_KEY`
* `void float(^self, f64 v)` — Writes the shortest decimal that reads back as `v`: `0.1`, `1.0`,…
* `void float32(^self, f32 v)` — Writes the shortest decimal that reads back as `v` in single…
* `void boolean(^self, bool v)` — Writes `true` or `false`. Fails with `EXPECTED_KEY` in an object with no…
* `void null(^self)` — Writes `null`. Fails with `EXPECTED_KEY` in an object with no key…
* `i32 code(self)` — The code the writer failed with, or 0 while it has not failed.
* `!ref([]char8) finish(^self)` — Hands over the text, which the caller then owns and `#delete`s. Fails…
* `void destroy(^self)` — Frees the text without finishing. Does nothing after `finish`.
* `Writer writer(i32 indent = 0, i64 room = 64)` — An empty writer. With `indent` 0 or less the text is compact; above 0, each…
