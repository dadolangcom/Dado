<!-- dadoc 1.0.0-rc.4 -->
<!-- commit f5f6761926d6 -->
# core:strings/parse

core:strings/parse — text to numbers and booleans.

    import "core:strings/parse"

    try f64 x = parse.to_f64("6.02214076e23") else code:
        ...                                    // not a number
    try i64 n = parse.to_i64("-42") else code:
        ...                                    // MALFORMED or RANGE
    try i32 port = parse.to_i32(text) else code:
        fmt.println(parse.message(code))
    try u64 mask = parse.to_u64("ff", 16) else code:
        ...
    try bool on = parse.to_bool("yes") else code:
        ...

**Tier N W H. Script class S.** Pure Dado: no `foreign` block, no
allocation, no package state. Every line compiles at every target —
freestanding, the web, and hosted (Linux, macOS, Windows) — and answers the
same bits for the same text on each. Its one import is `core:chars`, for
the ASCII byte classes, which is as pure.

**What raises into a script**: every function, each promised by the
`#assert_raises` on the line above it.

**Failure is `Error`**, this package's codes, 2400 to 2499. `MALFORMED`:
the text is not exactly what the function reads — empty, a stray byte,
space around it. `RANGE`: it is a number, and the type cannot hold it.
`RADIX`: the radix asked for is not 0 or 2..36. Floats never fail `RANGE`:
a magnitude past the largest finite value rounds to an infinity, which is
the correct answer. `message` is a sentence for each.

## The integers are strict

`to_i64`, `to_u64` and `to_i32` read the whole text or fail:

    [+|-] digits                     (`to_u64`: `+` only)

with each digit `0`..`9`, then `a`..`z` or `A`..`Z` for 10..35, below the
radix. Radix 0 reads a prefix: `0x`/`0X` is 16, `0o`/`0O` 8, `0b`/`0B` 2,
and anything else 10; the prefix follows the sign (`-0x10` is -16). No
space, no `_` between digits, no prefix under an explicit radix. A value
past the type's range fails `RANGE`, never wraps.

## `to_bool` reads what a configuration file writes

`true`, `yes`, `on` and `1` are true; `false`, `no`, `off` and `0` are
false; the words in any case. Anything else, space included, is
`MALFORMED`.

## `to_f64` and `to_f32` are strict and correctly rounded

The text must be a number and nothing else: `to_f64("12abc")`,
`to_f64(" 1")` and `to_f64("")` fail, through `!`, with `MALFORMED`. A
number that parses is the **nearest** `f64` (or `f32`) to the decimal the
text names, ties to even — what C's `strtod` answers in the default
rounding mode, whatever the number of digits. The grammar:

    [+|-] digits [. [digits]] [(e|E) [+|-] digits]
    [+|-] . digits [(e|E) [+|-] digits]
    [+|-] inf | infinity | nan          (any case)

Not accepted, where `strtod` would: leading space, C's hex floats
(`0x1p-3`), and `nan(...)` with a payload. A magnitude past the largest
finite value is an infinity and one below half the smallest subnormal is a
zero of the text's sign — that is the correct rounding, so neither fails.
`-nan` is a NaN with its sign bit set, which is what `strtod` answers.

Reading a single-precision value with `to_f32` is not `f32(to_f64(s))`:
rounding to 53 bits and then to 24 rounds twice, and a decimal that lands
near an `f32` halfway point can come out one unit wrong that way. `to_f32`
rounds once.

## How

Three stages, the first that can answer does:

1. **The text is scanned once** (`scan`), keeping the first nineteen
   significant digits in a `u64` and the decimal exponent. A value whose
   exponent puts it certainly past the range is answered here.
2. **Eisel-Lemire** (`eisel_lemire`): those digits times a 128-bit
   approximation of the power of ten (`POW10`, `powers.dado`), which settles
   all but a vanishing fraction of inputs with two 64×64 multiplications.
   It declines, rather than guessing, when the product sits too close to a
   rounding boundary, when the answer is subnormal, and when more than
   nineteen digits were written and the answer depends on the rest.
3. **Exact arithmetic** (`exact`) for everything it declines: the decimal
   as a big integer, divided or multiplied by the power of five exactly,
   rounded once from the true quotient and remainder. The first 800
   significant digits are kept and the rest recorded only as "something
   non-zero followed"; that loses nothing, because every `f64` and every
   halfway point between two of them is written exactly in at most 767
   significant digits, so digits past the 800th can move the value off such
   a point but never across one.

Stage 2 is a port of the Go standard library's `eiselLemire64` and
`eiselLemire32` (`strconv/eisel_lemire.go`), which follow Wuffs' C
implementation of the algorithm Michael Eisel and Daniel Lemire published in
2020. That code is under the licence below, and the port keeps its
structure and its step names so the two can be read side by side. Stages 1
and 3 are this package's own.

    Copyright 2020 The Go Authors. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

       * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
       * Neither the name of Google LLC nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The table the algorithm reads is generated, not transcribed:
`scripts/float-tables.py` writes `powers.dado` from exact integer
arithmetic and checks it row for row against Go's.

## Declarations

41 declarations, 8 public.

* `distinct enum i32 Error` — Why a read failed. The numbers are this package's, 2400 to 2499, apart…
* `string8 message(Error why)` — A sentence for `why`, for a person reading a tool's output.
* `Error!f64 to_f64(string8 s)` — The `f64` nearest the decimal `s` names, ties to even; `MALFORMED` when `s`
* `Error!f32 to_f32(string8 s)` — The `f32` nearest the decimal `s` names, rounded once; `MALFORMED` when `s`
* `Error!i64 to_i64(string8 s, i32 radix = 10)` — The integer `s` spells in `radix` (0 reads a prefix; see the header), as…
* `Error!u64 to_u64(string8 s, i32 radix = 10)` — The integer `s` spells in `radix`, as a `u64`: `to_i64`'s grammar with no…
* `Error!i32 to_i32(string8 s, i32 radix = 10)` — The integer `s` spells in `radix`, as an `i32`: `to_i64`'s grammar and…
* `Error!bool to_bool(string8 s)` — The boolean `s` names: `true`, `yes`, `on`, `1` or `false`, `no`, `off`,…
