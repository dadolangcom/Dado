<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:math

std/math — the parts of C's `<math.h>` a program reaches for.

Every declaration is a **restatement**: Dado says what it believes the header
contains, the emitted C includes the real one, and the C compiler checks the
belief. Nothing here parses a header, and nothing here is a
reimplementation — the arithmetic is your platform's libm, which is a great
deal better tested than anything this file could contain.

    import "core:math"
    math.root(2.0)

**Linking.** `<math.h>` needs `-lm` on Linux and nothing on macOS, so the
clause-only `foreign` line below the declarations carries `link "-lm"`.

**On the web** there is no libm to link and no `<math.h>` to include:
`math.h` is one of the headers the `#WEB` runtime supplies itself. The
compiler emits the runtime's own math library into the program — musl's,
transliterated, so every browser computes the same bits — declaring each
function with C's prototype, and the restatements below are checked against
those exactly as they are against the header's on a hosted build. So the
`-lm` is the one thing here that is not the web's, and it is in a `when`.

This paragraph used to argue the other way — that linking is a toolchain
concern and not a language one, that the source cannot know which
library provides a header, and that a `link` file beside this one was
therefore the honest place for it. It was settled the
other way, and the reason is worth keeping: the objection was never to the
*value* but to the *variation*. A flag that differs by platform has to be
conditional, and a file beside the source could express that only by
inventing a selector format (`#LINUX:`) matched by string against `#OS`'s
member names — a second, weaker compile-time branch beside the one the
language already has. `-lm` here is unconditional; where a flag is not,
`when #OS == …` around a clause-only `foreign` line is what says so, and it
folds and diagnoses like every other `when`.

## Declarations

48 declarations, 28 public.

* `const f64 PI = 3.14159265358979323846`
* `const f64 TAU = 6.28318530717958647692`
* `f64 radians(f64 degrees)` — Degrees to radians and back — the conversion every input handler and every…
* `f64 degrees(f64 radians)`
* `f64 root(f64 x)`
* `f64 raised(f64 base, f64 exponent)`
* `f64 length(f64 x, f64 y)` — The length of a vector, computed the way libm computes it — `hypot` avoids…
* `f64 fused_multiply_add(f64 x, f64 y, f64 z)` — `x * y + z`, rounded **once**.
* `f64 magnitude(f64 x)`
* `f64 remainder(f64 numerator, f64 denominator)`
* `f64 down(f64 x)`
* `f64 up(f64 x)`
* `f64 toward_zero(f64 x)`
* `f64 nearest(f64 x)`
* `f64 e_to(f64 x)`
* `f64 ln(f64 x)`
* `f64 log_2(f64 x)`
* `f64 log_10(f64 x)`
* `f64 sine(f64 x)`
* `f64 cosine(f64 x)`
* `f64 tangent(f64 x)`
* `f64 angle(f64 y, f64 x)` — The angle of a vector, quadrant included — which is what `atan2` is for and…
* `bool is_nan(f64 x)`
* `bool is_infinite(f64 x)`
* `$T min(T a, T b) where T is scalar`
* `$T max(T a, T b) where T is scalar`
* `$T clamp(T x, T low, T high) where T is scalar`
* `$B power(B base, $E exponent) where B is float && E is int` — `base` raised to a whole-number power, by squaring.
