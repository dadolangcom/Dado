<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 6c56ce20508a (dirty) -->
# core:json

JSON (RFC 8259): a writer, a pull reader, and a tree.

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

and it reads back a value at a time, allocating nothing:

    json.Reader r = json.reader_text(text)
    ref []char8 scratch
    defer #delete(scratch)
    try r.object()
    for r.more():
        try string8 name = r.key(&scratch)
        if name == "sizes":
            try r.array()
            for r.more():
                try f64 size = r.float()
    try r.finish()

or whole, as a tree:

    try json.Document doc = json.parse_text(text)
    defer doc.destroy()
    try json.Value sizes = doc.root().get("sizes")
    try json.Value first = sizes.at(0)
    try i64 n = first.int()                     // 600

**The writer.** The text grows on the allocator that was `#default` when
`writer` was called, so `json.writer() using a` puts it in `a`. Every call
costs time in proportion to what it writes. Strings escape `"`, `\`, the
control characters, DEL, U+0080 to U+009F, U+2028 and U+2029, and pass
other valid UTF-8 through. A byte that is not valid UTF-8 becomes U+FFFD.
Floats are the shortest text that reads back as the same value. Keys are
written in the order given, and nothing checks that a key is not repeated.
A call that cannot write what it was asked fails the writer with a code:
the code is kept, later calls write nothing, and `finish` fails with it.

**The reader** (`reader` over bytes, `reader_text` over text, one reader
either way: `reader_text` views the text as bytes) is strict RFC 8259: no
comments, trailing commas, leading zeros, `NaN`, single quotes or
byte-order mark, and strings of well-formed UTF-8 whose `\u` escapes pair
their surrogates. A string or number read with no escape is a view into the
input; otherwise a read decodes into a scratch run the caller gives it.
Integers read exactly as `i64` or `u64` and fail with `OVERFLOW` when they
do not fit; any number reads as the nearest `f64`, correctly rounded by
core:strings/parse. Objects and arrays nest at most `MAX_DEPTH` deep, and
walking or skipping them costs no stack. Malformed input fails the reader
for good with a code and the byte offset it was found at.

**The tree** (`parse`, `parse_text`, `Reader.document`) holds a value in
three runs on `#default`: a node per value, every container's children,
and every string's text. `Value` is a handle into it whose lookups by
index cost the same at any size; `get` by key is linear in the members.
`Writer.value` writes a tree back.

**Bytes or text.** JSON is UTF-8 text (RFC 8259, section 8.1), so each way
in has a text form beside its bytes form: `reader_text`, `well_formed_text`
and `parse_text` take the `string8` a program usually holds, and view it as
the bytes `reader`, `well_formed` and `parse` read, copying nothing.

**Error codes: `Error`, 1900 to 1999**, a range no other package's codes
share; `message(why)` answers a sentence for each.

**Tier N W H. Script class S.** No `foreign` block and no package state:
it builds for every target, `#NONE` and `#WEB` included, and any thread
may use it, one thread to a writer, reader or document.

**What raises into a script.** Everything: every function and every
method of `Writer`, `Reader`, `Document` and `Value`, each promised by the
`#assert_raises` on the line above it.

## Declarations

124 declarations, 66 public.

* `distinct enum i32 Error` — Why a write or a read failed. The numbers are this package's, 1900 to…
* `const i32 MAX_DEPTH = 128` — How many objects and arrays may be open at once, in a writer and in a…
* `string8 message(Error why)` — A sentence describing `why`; any value that is not a member answers "not a…
* `type Writer` — One JSON text being written: make one with `writer`, write exactly one…
* `void Writer.object(^self)` — Opens an object: `{`. Inside it, write `key` before each value, then…
* `void Writer.end_object(^self)` — Closes the innermost open object: `}`. Fails with `UNBALANCED` when it…
* `void Writer.array(^self)` — Opens an array: `[`. Values written next are its elements, up to…
* `void Writer.end_array(^self)` — Closes the innermost open array: `]`. Fails with `UNBALANCED` when it is…
* `void Writer.key(^self, string8 name)` — Writes `name` as the next member's name, a JSON string escaped the way…
* `void Writer.text(^self, string8 s)` — Writes `s` as a JSON string. `"`, `\`, the control characters, DEL,…
* `void Writer.int(^self, i64 v)` — Writes `v` in decimal: `-42`. Fails with `EXPECTED_KEY` in an object with…
* `void Writer.uint(^self, u64 v)` — Writes `v` in decimal: `18446744073709551615`. Fails with `EXPECTED_KEY`
* `void Writer.float(^self, f64 v)` — Writes the shortest decimal that reads back as `v`: `0.1`, `1.0`,…
* `void Writer.float32(^self, f32 v)` — Writes the shortest decimal that reads back as `v` in single…
* `void Writer.boolean(^self, bool v)` — Writes `true` or `false`. Fails with `EXPECTED_KEY` in an object with no…
* `void Writer.null(^self)` — Writes `null`. Fails with `EXPECTED_KEY` in an object with no key…
* `void Writer.value(^self, Value v)` — Writes `v` and everything inside it, as `parse` read it: each kind by…
* `Error Writer.code(self)` — The code the writer failed with, or `Error(0)`, which is no member,…
* `Error!ref([]char8) Writer.finish(^self)` — Hands over the text, which the caller then owns and `#delete`s. Fails…
* `void Writer.destroy(^self)` — Frees the text without finishing. Does nothing after `finish`.
* `Writer writer(i32 indent = 0, i64 room = 64)` — An empty writer. With `indent` 0 or less the text is compact; above 0, each…
* `enum i32 Kind: (Null, Bool, Number, String, Array, Object)` — What a value is, as `Reader.kind` and `Value.kind` answer it.
* `type Reader` — A cursor over one JSON text. Make one with `reader` or `reader_text`, read…
* `Error!Kind Reader.kind(^self)` — What the next value is, without reading it. Fails with `OUT_OF_TURN`
* `Error!void Reader.object(^self)` — Opens the next value, an object; `more` then answers true once per…
* `Error!void Reader.array(^self)` — Opens the next value, an array; `more` then answers true once per…
* `bool Reader.more(^self)` — Whether the innermost open object or array has another member or…
* `Error!string8 Reader.key(^self, ^ref []char8 scratch)` — The next member's key, after which its value is due. The key is a view…
* `Error!string8 Reader.text(^self, ^ref []char8 scratch)` — The next value, a string, its escapes decoded. It is a view as `key`'s…
* `Error!ref([]char8) Reader.text_copy(^self)` — The next value, a string, its escapes decoded into a new run on…
* `Error!i64 Reader.int(^self)` — The next value, a number written as an integer, `-9223372036854775808`
* `Error!u64 Reader.uint(^self)` — The next value, a number written as an integer from 0 to…
* `Error!f64 Reader.float(^self)` — The next value, any number, as the nearest `f64`, ties to even. A…
* `Error!string8 Reader.number_text(^self)` — The next value, a number, as the text it is written in: a view into…
* `Error!bool Reader.boolean(^self)` — The next value, `true` or `false`. Fails with `WRONG_TYPE` for any…
* `Error!void Reader.null(^self)` — Reads the next value, `null`. Fails with `WRONG_TYPE` for any other…
* `Error!void Reader.skip(^self)` — Steps over the next value, whatever it holds, checking all of it. Fails…
* `Error!Document Reader.document(^self)` — The next value as a tree (see `Document`), made on `#default`; the…
* `Error!void Reader.finish(^self)` — Succeeds when the input is one whole value and whitespace. Reads…
* `bool Reader.done(self)` — Whether the top-level value has been read to its end.
* `Error Reader.code(self)` — The code the reader failed with, or `Error(0)`, which is no member,…
* `i64 Reader.offset(self)` — The byte offset in the input where the reader failed: of the byte that…
* `Reader reader([]u8 data, i32 max_depth = MAX_DEPTH)` — A reader at the start of `data`, which it reads in place; a string or a…
* `Reader reader_text(string8 text, i32 max_depth = MAX_DEPTH)` — `reader` over text: the same reader, over `text`'s bytes, nothing copied.
* `Error!void well_formed([]u8 data)` — Succeeds when `data` is exactly one JSON value and whitespace, by RFC 8259…
* `Error!void well_formed_text(string8 text)` — `well_formed` over text, nothing copied.
* `enum i32 Form: (Float, Int, Uint, Big)` — How a number is held, in a tree or as a `Reader` read it. `Int` is an…
* `type Document` — A JSON value and everything inside it, as `parse` and `Reader.document`
* `Value Document.root(^self)` — The top-level value. Valid while the document is not moved or…
* `void Document.destroy(^self)` — Frees the runs, and leaves an empty document.
* `type Value` — One value in a `Document`: the document and the node's index. A plain…
* `Kind Value.kind(self)` — What the value is.
* `Form Value.form(self)` — How a `Number` is held (see `Form`); `Float` for any other kind.
* `bool Value.is_null(self)` — Whether it is `null`.
* `Error!bool Value.boolean(self)` — A `Bool`'s value. Fails with `WRONG_TYPE` for any other kind.
* `Error!i64 Value.int(self)` — A `Number` that is an integer from -2^63 to 2^63 - 1. Fails with…
* `Error!u64 Value.uint(self)` — A `Number` that is an integer from 0 to 2^64 - 1. Fails with…
* `Error!f64 Value.float(self)` — A `Number` as the nearest `f64`; past the largest finite one, an…
* `Error!string8 Value.text(self)` — A `String`'s text, escapes decoded, as a view into the document. Fails…
* `i64 Value.len(self)` — How many elements an `Array` has or members an `Object` has; 0 for…
* `Error!Value Value.at(self, i64 i)` — An `Array`'s element `i`, or the value of an `Object`'s member `i`,…
* `Error!string8 Value.key(self, i64 i)` — The key of an `Object`'s member `i`, as a view into the document.
* `Error!Value Value.get(self, string8 name)` — The value of an `Object`'s member named `name`, the last one when the…
* `bool Value.has(self, string8 name)` — Whether an `Object` has a member named `name`; false for any other…
* `Error!Document parse([]u8 data)` — The one JSON value `data` holds, as a tree on `#default` that the caller…
* `Error!Document parse_text(string8 text)` — `parse` over text, nothing copied.
