<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:fmt

core:fmt — where rendered bytes go, and what shape they arrive in.

    import "core:fmt"

    [64]char8 store
    fmt.Cursor c = fmt.over(store[:])
    Sink out = fmt.to_bytes(&c)

    _ = fmt.put_string("n = ", out)
    _ = fmt.put_signed(42, out)
    string line = fmt.written(c)       // the bytes, as a string

**The compiler owns what a value looks like; this package owns where it goes
and how it is shaped.** `#render(v, sink)` renders one value in the one form
its type has — no width, no precision, no base, no alignment, because a
renderer driven by a type cannot take an opinion from a call. Every one of
those opinions lives here, and each is an ordinary function a program can
read.

## The four that write a message

    public void print(..#any args)
    public void println(..#any args)
    public void write(Sink out, ..#any args)
    public void writeln(Sink out, ..#any args)

These are the compiler's retired `print` and `println`, and they are this
package's because the destination is a library's business and the *rendering*
is not: `#render(v, sink)` is the compiler's, per-type, and these four are a
`..#any` pack unrolled over it. They exist only inside a `when #OS != #NONE:`
— on a freestanding target the names do not exist and a call is refused
rather than silently doing nothing. `print` and `println` reach the terminal
through `core:io`'s `stdout_sink()`; `write` and `writeln` take the sink the
caller names. None of them fails, so no call site writes `try`.

They are the one place the sink is written **first**, and the rule below says
why that is the grammar's doing rather than a choice.

## The sink is last, and it is not optional

Every function below takes its `Sink` as the final parameter, because that is
where the language already puts the argument that says *where this goes*:
`#format(v…, alloc)` writes the allocator last and `#render(v…, sink)` writes
the sink last, and both for the same reason — a value has a rendering and a
destination does not, so the position is unambiguous by type and a reader
never counts arguments to find it.

A function that renders and hides its destination is the shape this package
exists to not have.

## Everything answers `i64`, and it is the count the sink took

Not `void` and not `bool`. A sink that took *some* of the bytes is a real
state — `to_bytes` over a full buffer is exactly that — so the answer is a
count, and a caller that cares compares it against what it asked for. It is
`#render`'s own answer and the two compose: a putter's result is the number a
`#render` into the same sink would have added.

A nil `proc` takes nothing and answers 0, so an unset `Sink` is inert rather
than a fault.

## This package includes nothing, and imports one thing

No `foreign` block — so this file emits no `#include` of its own, and a
program on a target whose OS is `#NONE` may use every line of it that exists
there. That is not an accident of what happened to be needed: it is the
reason the stream sinks are **not here**. `stdout` is `<stdio.h>`, a
`foreign` block's `#include` reaches every importer of the package holding
it, and a `fmt` that dragged `<stdio.h>` behind it would be a formatting
library no freestanding program could call. `core:io` owns `^FILE` already
and is where a sink over one belongs.

The one `import` is `core:io`, and it is what the four printing functions at
the foot of this file reach standard output through. It costs an importer
nothing it does not ask for, because the emitter follows **liveness** and not
imports: those four are templates over a pack, so a program that calls none
of them instantiates none of them and emits no `<stdio.h>`. The argument for
the rule above is therefore unchanged — what it forbids is a header this
package *cannot* avoid emitting, and an import is not one.

The same rule keeps `core:math` out: `<math.h>` and `-lm` would ride along
with it — unconditionally, because the float path below is concrete and calls
it — so that path does its own arithmetic.

## What an importer pays, measured

**Everything concrete in a package is emitted whether it is called or not**,
and the emitter's one exemption is an uncalled *template*. That is the
budget this package is written against, and the sharpened form of the rule is
worth stating because it is the half that is not obvious:

> A template is free only when nothing **concrete in its own package** calls
> it. A concrete function is always emitted, so every template it reaches is
> instantiated along with it, into every importer.

Measured on this package, with a program calling three of its functions:
954 lines of emitted C and 35 definitions before that rule was applied, and
**805 lines and 29 definitions after** — the difference being `put_quoted`
reaching the public `put_hex` template and the float path reaching the public
`put_unsigned` and `put_signed_pad` ones. Every internal call now goes to a
private primitive (`render_base`, `digit`, `write`), so the public templates
are instantiated by callers and by nothing else.

The float path is 129 of the remaining 568 lines of function bodies — 22% —
and it is **not** split into a package of its own. The split was priced: it
would have to make `render_base` and `digit` public to serve it, growing the
public surface in order to shrink the emitted C, and the `strings/parse`
precedent it would be citing is about a **capability** difference (a header
decides which targets may import the package) rather than a size one. The
numbers are here rather than the split, so a program that finds 129 lines
intolerable has them to argue with.

## What is deliberately absent

**There is no format string.** A literal is never a format string anywhere in
this language — the format is written by the compiler from each argument's
type — and a library that introduced one would be re-opening that decision
with a second, weaker parser. What replaces it is a `Pad` value and a putter
per shape: the opinion is a *value* you can name, hoist and reuse, and it is
checked by the type system rather than by a runtime scan of a string.

## Declarations

90 declarations, 39 public.

* `i64 put_bytes([]char8 bytes, Sink out)` — Hand `bytes` to `out`, and answer how many it took.
* `i64 put_string(string8 s, Sink out)` — The bytes of a `string`, written whole — embedded `\0` included, because a…
* `i64 put_cstring(cstring c, Sink out)` — The bytes at `c`, up to its `\0`. One walk to find the length C threw away —…
* `i64 put_char(char8 b, Sink out)` — One byte.
* `i64 put_repeat(char8 b, i32 n, Sink out)` — `b`, `n` times. `n <= 0` writes nothing. This is what every alignment in the…
* `enum i32 Align` — Which side the padding goes on. A plain `i32`-backed enum, so it orders and…
* `type Pad` — How a field is padded: how wide it is at least, which side the slack goes on,…
* `const Pad PLAIN = (0, Align.Left, ' ')` — No padding. The value an argument takes when a caller does not care.
* `Pad right(i32 width)` — Right-aligned in `width` columns, space-filled — a number in a column.
* `Pad left(i32 width)` — Left-aligned in `width` columns, space-filled — a name in a column.
* `Pad zeros(i32 width)` — Right-aligned in `width` columns, zero-filled — a fixed-width number.
* `i64 put_base(u64 value, u32 base, bool upper, Sink out)` — An unsigned value in `base`, with the digits above 9 written in the case…
* `i64 put_signed($T value, Sink out) where T is int && T in (i8, i16, i32, i64)` — A signed integer of any width, in decimal.
* `i64 put_unsigned($T value, Sink out) where T is int && T in (u8, u16, u32, u64)` — An unsigned integer of any width, in decimal.
* `i64 put_hex($T value, i32 digits, bool upper, bool prefix, Sink out) where T is int && T in (u8, u16, u32, u64)` — Hexadecimal, with an optional `0x` and an optional minimum digit count that…
* `i64 put_binary($T value, i32 digits, bool prefix, Sink out) where T is int && T in (u8, u16, u32, u64)` — Binary, with an optional `0b` and the same floor rule as `put_hex`.
* `i64 put_string_pad(string8 s, Pad p, Sink out)` — A `string` in a field.
* `i64 put_signed_pad($T value, Pad p, Sink out) where T is int && T in (i8, i16, i32, i64)` — A signed decimal in a field.
* `i64 put_unsigned_pad($T value, Pad p, Sink out) where T is int && T in (u8, u16, u32, u64)` — An unsigned decimal in a field.
* `i64 put_bool(bool v, Sink out)` — `true` or `false`, which is what the compiler's own renderer writes.
* `i64 put_bool_as(bool v, string8 when_true, string8 when_false, Sink out)` — `v` as one of two words the caller chose — `yes`/`no`, `on`/`off`, `1`/`0`.
* `i64 put_quoted(string8 s, Sink out)` — `s` between double quotes, with the eight escapes the language's own literals…
* `bool is_nan($T x) where T is float` — Whether `x` is a NaN, asked without `<math.h>`: a NaN is the one value that is…
* `bool is_infinite($T x) where T is float` — Whether `x` is an infinity, asked without `<math.h>`.
* `i64 put_float($T value, i32 decimals, Sink out) where T is float` — `value` with exactly `decimals` places after the point, rounded half away from…
* `i64 put_float_shortest($T value, Sink out) where T is float` — ── the shortest float that reads back ──────────────────────────────────────…
* `type Cursor` — Where a `to_bytes` sink is up to. `into` is the caller's storage, `used` is…
* `Cursor over([]char8 into)` — A cursor over storage the caller owns.
* `Sink to_bytes(^Cursor c)` — A sink that fills a cursor and drops what will not fit.
* `string8 written(Cursor c)` — What has landed in the cursor so far, as text. Free — a window over the…
* `void rewind(^Cursor c)` — Forget the bytes without forgetting the storage. The overflow flag goes with…
* `Sink to_run(^ref([]char8) dst)` — A sink that appends to a run the caller owns, growing it as needed.
* `Sink counting(^i64 n)` — A sink that writes nothing and adds every byte it is offered to `n`.
* `Sink discard()` — A sink that takes everything and keeps none of it. Useful where a `Sink` is…
* `i64 put($T v, Sink out)` — `v`, in the form its type has, into `out`.
* `void write(Sink out, ..#any args)` — `args`, rendered one after another into `out`.
* `void writeln(Sink out, ..#any args)` — `args`, and then a newline.
* `void print(..#any args)` — `args`, on standard output.
* `void println(..#any args)` — `args`, on standard output, and then a newline.
