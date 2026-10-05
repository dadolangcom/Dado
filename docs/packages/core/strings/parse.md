<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:strings/parse

core:strings/parse — text to numbers.

    import "core:strings/parse"

    try f64 x = parse.to_f64("6.02214076e23") else code:
        ...                                    // not a number
    i32 n = parse.to_i32("42")

**This package includes nothing.** It had a `private foreign "stdlib.h"`
block, for `atoi` and `atof`, and that block made it hosted-only: a `foreign`
block's `#include` reaches every importer, `<stdlib.h>` is not one of the
C11 §4 freestanding headers, and `#NONE` and `#WEB` have no C library to
supply `strtod` anyway. Both parsers are Dado now, so every line here compiles at
every target, and the import line no longer decides one.

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

## `to_i32` is C's `atoi`, still forgiving

It keeps the contract it had when it was `atoi` through `foreign`: leading
white space skipped, a sign, the digits up to the first non-digit, and 0 for
text with no number at the front — no failure, ever. Only its implementation
moved, so that the header went with it. It still takes a `cstring`.

## Declarations

39 declarations, 4 public.

* `const i32 MALFORMED = 1` — The code `to_f64` and `to_f32` fail with: the text is not a number, or is a…
* `!f64 to_f64(string8 s)` — The `f64` nearest the decimal `s` names, ties to even; `MALFORMED` when `s`
* `!f32 to_f32(string8 s)` — The `f32` nearest the decimal `s` names, rounded once; `MALFORMED` when `s`
* `i32 to_i32(cstring c)` — `atoi`'s answer: white space skipped, an optional sign, then decimal digits…
