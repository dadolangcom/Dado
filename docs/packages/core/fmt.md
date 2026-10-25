<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 6c56ce20508a (dirty) -->
# core:fmt

core:fmt — where rendered bytes go, and what shape they arrive in.

    import "core:fmt"

    [64]char8 store
    fmt.Cursor c = fmt.over(store[:])
    Sink out = fmt.to_bytes(&c)

    _ = fmt.put_string("n = ", out)
    _ = fmt.put_signed(42, out)
    string8 line = fmt.written(c)      // the bytes, as a view

**The compiler owns what a value looks like; this package owns where it goes
and how it is shaped.** `#render(v, sink)` renders one value in the one form
its type has — no width, no precision, no base, no alignment, because a
renderer driven by a type cannot take an opinion from a call. Every one of
those opinions lives here, and each is an ordinary function a program can
read.

## The six that write a message

    void print(..#any args)
    void println(..#any args)
    void eprint(..#any args)
    void eprintln(..#any args)
    void write(Sink out, ..#any args)
    void writeln(Sink out, ..#any args)

These are the compiler's retired `print` and `println`, and they are this
package's because the destination is a library's business and the *rendering*
is not: `#render(v, sink)` is the compiler's, per-type, and these six are a
`..#any` pack unrolled over it. `print` and `println` reach the terminal
through `core:io`'s `stdout_sink()`, `eprint` and `eprintln` through its
`stderr_sink()`; `write` and `writeln` take the sink the caller names. None
of them fails, so no call site writes `try`.

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

## Tier N W H, and what a freestanding target gives up

**Tier N W H.** No `foreign` block, so this file emits no `#include` of its
own, and every line of it builds hosted (Linux, macOS, Windows), on the web
and freestanding, with two differences on `#NONE`, both the target's:

* `print`, `println`, `eprint` and `eprintln` do not exist there: a
  freestanding target has no standard output or error to reach, and a call
  is refused (*package `fmt` declares no function `println`*) rather than
  silently doing nothing. `write` and `writeln` stay, because the caller
  names the sink.
* `#render` of a float is refused there (ERR1004: its conversion is the C
  library's), so `put`, `write` and `writeln` given a float, or a tuple
  holding one, are refused at that call. This package's own float putters
  and renderers are written in Dado and work everywhere: on `#NONE` a
  float is written with `put_float`, `put_float_shortest`, `put_float_exp`
  or `put_float_pad`.

That is the reason the stream sinks are **not here**: `stdout` is
`<stdio.h>`, and `core:io` owns it. `core:io` is imported for the four
printing functions at the foot of this file; it costs an importer nothing
it does not ask for, because the emitter follows **liveness** and not
imports: they are templates over a pack, so a program that calls none of
them instantiates none and emits no `<stdio.h>`.

The same rule keeps `core:math` out: `<math.h>` and `-lm` would ride along
with it on a hosted target, so the float path below does its own
arithmetic, and the NaN and infinity tests it needs are private here.

## What raises into a script

Every function here that neither takes nor makes a `Sink`, each promised by
the `#assert_raises` on the line above it: the `render_*` family (text into
the caller's `[]char8`, the script-safe face of each putter), `right`,
`left`, `zeros`, and the `Cursor` verbs `over`, `written` and `rewind`. Everything that takes or makes a `Sink` is not
promised, for one reason: a `Sink` is C's `(procedure, rawptr data)` pair,
and a script has no addresses. A script that wants text calls a renderer;
one that wants output calls its own `print`.

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

105 declarations, 46 public.

* `i64 put_bytes([]char8 bytes, Sink out)` — Hand `bytes` to `out`, and answer how many it took.
* `i64 put_string(string8 s, Sink out)` — The bytes of a `string8`, written whole — embedded `\0` included, because a…
* `i64 put_cstring(cstring c, Sink out)` — The bytes at `c`, up to its `\0`. One walk to find the length C threw away —…
* `i64 put_char(char8 b, Sink out)` — One byte.
* `i64 put_repeat(char8 b, i64 n, Sink out)` — `b`, `n` times. `n <= 0` writes nothing. This is what every alignment in the…
* `enum i32 Align` — Which side the padding goes on. A plain `i32`-backed enum, so it orders and…
* `type Pad` — How a field is padded: how wide it is at least, in display columns, which…
* `const Pad PLAIN = (0, Align.Left, ' ')` — No padding. The value an argument takes when a caller does not care.
* `Pad right(i32 width)` — Right-aligned in `width` columns, space-filled — a number in a column.
* `Pad left(i32 width)` — Left-aligned in `width` columns, space-filled — a name in a column.
* `Pad zeros(i32 width)` — Right-aligned in `width` columns, zero-filled — a fixed-width number.
* `i64 put_base(u64 value, u32 base, bool upper, Sink out)` — An unsigned value in `base`, with the digits above 9 written in the case…
* `i64 put_signed($T value, Sink out) where T is int && T in (i8, i16, i32, i64)` — A signed integer of any width, in decimal.
* `i64 put_unsigned($T value, Sink out) where T is int && T in (u8, u16, u32, u64)` — An unsigned integer of any width, in decimal.
* `i32 render_signed($T value, []char8 into) where T is int && T in (i8, i16, i32, i64)` — `value` in decimal, as `put_signed` writes it, at the start of `into`.
* `i32 render_unsigned($T value, []char8 into) where T is int && T in (u8, u16, u32, u64)` — `value` in decimal, as `put_unsigned` writes it, at the start of `into`.
* `i64 put_hex($T value, i32 digits, bool upper, bool prefix, Sink out) where T is int && T in (u8, u16, u32, u64)` — Hexadecimal, with an optional `0x` and an optional minimum digit count that…
* `i64 put_binary($T value, i32 digits, bool prefix, Sink out) where T is int && T in (u8, u16, u32, u64)` — Binary, with an optional `0b` and the same floor rule as `put_hex`.
* `i64 put_string_pad(string8 s, Pad p, Sink out)` — A `string8` in a field, measured in display columns (`chars.display_width`):
* `i64 put_signed_pad($T value, Pad p, Sink out) where T is int && T in (i8, i16, i32, i64)` — A signed decimal in a field.
* `i64 put_unsigned_pad($T value, Pad p, Sink out) where T is int && T in (u8, u16, u32, u64)` — An unsigned decimal in a field.
* `i64 put_bool(bool v, Sink out)` — `true` or `false`, which is what the compiler's own renderer writes.
* `i64 put_bool_as(bool v, string8 when_true, string8 when_false, Sink out)` — `v` as one of two words the caller chose — `yes`/`no`, `on`/`off`, `1`/`0`.
* `i64 put_quoted(string8 s, Sink out)` — `s` between double quotes, with the eight escapes the language's own literals…
* `i64 put_float($T value, i32 places, Sink out) where T is float` — `value` with exactly `places` places after the point, rounded half away from…
* `i64 put_float_pad($T value, i32 places, Pad p, Sink out) where T is float` — `value` as `put_float` writes it, in a field: the padding goes around the…
* `i32 render_float($T value, i32 places, []char8 into) where T is float` — `value` as `put_float` writes it, at the start of `into`. Answers how many…
* `i64 put_float_exp($T value, i32 places, Sink out) where T is float` — `value` in exponent notation, `d.ddde+NN`: one digit before the point,…
* `i32 render_float_exp($T value, i32 places, []char8 into) where T is float` — `value` as `put_float_exp` writes it, at the start of `into`. Answers how…
* `i64 put_float_shortest($T value, Sink out) where T is float` — `put_float` writes the places it is asked for; this writes **the fewest…
* `i32 render_float_shortest($T value, []char8 into) where T is float` — `value` as `put_float_shortest` writes it, at the start of `into`. Answers…
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
* `void eprint(..#any args)` — `args`, on standard error: a diagnostic, kept apart from a program's…
* `void eprintln(..#any args)` — `args`, on standard error, and then a newline.
