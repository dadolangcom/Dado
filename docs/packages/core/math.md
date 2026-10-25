<!-- dadoc 1.0.0-rc.8 -->
<!-- commit f4253b706f10 (dirty) -->
# core:math

core:math — the scalar mathematics a program reaches for: roots and powers,
exponentials and logarithms, trigonometry, rounding, classification, and
the comparisons C leaves out. Every function works on `f64`, except the
generic ones at the bottom of the file, which take any width.

    import "core:math"
    math.root(2.0)

**Tier N W H.** Every function builds and runs freestanding (`#NONE`), on
the web (`#WEB`) and hosted (Linux, macOS, Windows and the BSDs). Where an
answer comes from depends on the function and the target, and falls into
three kinds:

* **Exact, and Dado everywhere.** Rounding (`down`, `up`, `toward_zero`,
  `nearest`, `nearest_even`), `abs`, `with_sign_of`, `remainder`,
  the classification questions, `split_exponent`, `times_power_of_two`,
  the comparisons, `power`, `sign`, `lerp`, `smoothstep`, `gcd`, `lcm` and
  the conversions. Each has one right answer to the last bit, so writing it
  in Dado (`exact.dado` and this file) costs nothing in accuracy, and it
  is what lets `#NONE` and a script have them.
* **The C library's where there is one.** `root`, `raised`, `length`,
  `fused_multiply_add`, `e_to`, `ln`, `log_2`, `log_10`, `sine`,
  `cosine`, `tangent`, `arccosine` and `angle` are restatements of
  `<math.h>` when hosted and on `#WEB`, and on `#NONE`, which has no C
  library, the Dado transliterations of musl and msun in `soft.dado`.
  One exception: `fused_multiply_add` is `soft.dado`'s on every target
  but `#WEB`, because the `fma` a program built by the pinned zig links
  answers NaN where it should not (its comment says where).
* **The C library's when hosted, Dado's elsewhere.** `arcsine`,
  `arctangent`, the hyperbolic functions, `cube_root`, `two_to`,
  `ln_one_plus` and `e_to_minus_one` are `<math.h>`'s when hosted; the
  `#WEB` runtime's math library does not carry them, so on the web, as on
  `#NONE`, they are `soft.dado`'s.

**The square root is the C library's where there is one**, though
`soft_sqrt` gives the same bits: IEEE 754 defines the square root to be
correctly rounded, C's is, and so is the one written here, which the tests
check against it. C's is one instruction; the Dado one is a 53-step loop.
So `root` (and `linalg.length` with it) takes C's on every target that
has it and the Dado one only on `#NONE`, and the answer does not change
with the choice.

**Accuracy.** The C library's functions are as accurate as its authors
made them (glibc's, macOS's and musl's are within one unit in the last
place, most of them correctly rounded in practice). The Dado ones in
`soft.dado` are within one unit in the last place of the true answer, as
the musl and msun originals they transliterate are, and the tests hold
them to that against the hosted C library. The exact functions are exact.

**No `f32` variants.** Every function takes and answers `f64`. An `f32`
widens to `f64` exactly, and narrowing the answer back rounds once; for
`root` that double rounding is still correctly rounded (53 bits is more
than twice 24 plus two), and for the rest it adds at most half a unit of
the `f32`. A second set of names would double the surface for no answer a
caller cannot already get with `f32(math.sine(f64(x)))`. Nor are there
generic front doors: a `where T is float` door would refuse an `i32`
argument that widens to `f64` today. The generic
functions at the bottom (`min`, `max`, `clamp`, `power`, `sign`, `lerp`,
`smoothstep`) do take `f32`, since they need nothing from a library.

**Linking.** `<math.h>` needs `-lm` on Linux and the BSDs and accepts it on
macOS and on Windows (MinGW's `libm` is an empty archive there, the
functions living in the C runtime), so the clause-only `foreign` line
below the declarations carries `link "-lm"` for every hosted target.

**On the web** there is no libm to link and no `<math.h>` to include:
`math.h` is one of the headers the `#WEB` runtime supplies itself. The
compiler emits the runtime's own math library into the program — musl's,
transliterated, so every browser computes the same bits — declaring each
function with C's prototype, and the restatements below are checked against
those exactly as they are against the header's on a hosted build. That
library carries only what this package asked of it when it was written, so
the second, hosted-only block holds what it lacks, and the `-lm` is in the
same `when`.

This package once argued that linking is a toolchain concern and not a
language one, that the source cannot know which library provides a
header, and that a `link` file beside this one was therefore the honest
place for it. It was settled the other way, and the reason is worth
keeping: the objection was never to the *value* but to the *variation*. A
flag that differs by platform has to be conditional, and a file beside the
source could express that only by inventing a selector format (`#LINUX:`)
matched by string against `#OS`'s member names — a second, weaker
compile-time branch beside the one the language already has. Where a flag
varies, `when #OS == …` around a clause-only `foreign` line is what says
so, and it folds and diagnoses like every other `when`.

**What raises into a script.** Every function of the first kind, each
promised by the `#assert_raises` above it and checked at each
instantiation a program makes: they hold nothing foreign and nothing of
C's address model, so a script calling one runs it as script. The Dado
libm in `soft.dado` is promised too, each function on its own line, which
is what keeps `#NONE`'s answers pure. So is `root`: its C is `<math.h>`'s
`sqrt`, which IEEE 754 requires to be correctly rounded, so the checker
accepts that call as raising. The rest of the second and third kinds are
not promised, because on a hosted target they call into C, and each says
so on its own line: a transcendental is the platform's until a script's
own is pinned to the same bits.

**Thread-safety.** No function here has state; any thread may call any of
them. **Failure:** none. A domain error answers a NaN (`root(-1.0)`,
`ln(-1.0)`, `arcsine(2.0)`) and an overflow an infinity, as C's do.

## Declarations

248 declarations, 59 public.

* `f64 abs(f64 x)` — `|x|`: `x` with its sign bit cleared, so `abs(-0.0)` is `0.0` and a…
* `f64 with_sign_of(f64 x, f64 sign)` — `x` with the sign of `sign`: its magnitude, negated when `sign`'s sign bit…
* `f64 remainder(f64 numerator, f64 denominator)` — The remainder of `numerator / denominator` truncated toward zero: the…
* `f64 down(f64 x)` — The largest whole number not above `x`. C's `floor`.
* `f64 up(f64 x)` — The smallest whole number not below `x`. C's `ceil`.
* `f64 toward_zero(f64 x)` — `x` with its fraction dropped: the whole number nearest `x` in the…
* `f64 nearest(f64 x_in)` — The nearest whole number, halves rounded away from zero: `nearest(2.5)`
* `f64 nearest_even(f64 x)` — The nearest whole number, halves rounded to the even neighbour:
* `bool is_nan(f64 x)` — Whether `x` is a NaN, of either sign and any payload. A NaN is the one…
* `bool is_infinite(f64 x)` — Whether `x` is positive or negative infinity.
* `bool is_finite(f64 x)` — Whether `x` is neither infinite nor a NaN: a zero, a subnormal or a…
* `type Exponent` — An `f64` as a fraction and a power of two.
* `Exponent split_exponent(f64 x)` — `x` as `fraction * 2^exponent` with the fraction's magnitude in [0.5, 1):
* `f64 times_power_of_two(f64 x, i32 n)` — `x * 2^n`, computed by adjusting the exponent and rounded once if the answer…
* `const f64 PI = 3.14159265358979323846` — π, the half turn in radians.
* `const f64 TAU = 6.28318530717958647692` — 2π, the whole turn in radians.
* `const f64 E = 2.718281828459045` — e, the base of the natural logarithm.
* `const f64 SQRT2 = 1.4142135623730951` — The square root of two.
* `const f64 LN2 = 0.6931471805599453` — The natural logarithm of two.
* `const f64 LN10 = 2.302585092994046` — The natural logarithm of ten.
* `const f64 EPSILON = 2.220446049250313e-16` — The gap between 1.0 and the next `f64` above it, 2^-52: C's…
* `const f64 MAX_FINITE = 1.7976931348623157e+308` — The largest finite `f64`: C's `DBL_MAX`.
* `const f64 MIN_NORMAL = 2.2250738585072014e-308` — The smallest positive normal `f64`, 2^-1022: C's `DBL_MIN`.
* `const f64 MIN_SUBNORMAL = 5e-324` — The smallest positive `f64` of all, the subnormal 2^-1074.
* `f64 infinity()` — Positive infinity. A function, where the other values here are…
* `f64 nan()` — A quiet NaN, a function for the same reason as `infinity`. Compare with…
* `f64 radians(f64 degrees)` — Degrees to radians — the conversion every input handler and every rotated…
* `f64 degrees(f64 radians)` — Radians to degrees, the inverse of `radians`.
* `f64 root(f64 x)` — The square root, correctly rounded: NaN for a negative `x`, and `-0.0`
* `f64 cube_root(f64 x)` — The real cube root, defined for every `x`: `cube_root(-8.0)` is `-2.0`.
* `f64 raised(f64 base, f64 exponent)` — `base` raised to a real `exponent`. C's `pow`, with its special cases: any…
* `f64 length(f64 x, f64 y)` — The length of a vector, computed the way libm computes it — `hypot` avoids…
* `f64 fused_multiply_add(f64 x, f64 y, f64 z)` — `x * y + z`, rounded **once**.
* `f64 e_to(f64 x)` — e^x. C's `exp`.
* `f64 two_to(f64 x)` — 2^x. C's `exp2`; exact for a whole-number `x` in range.
* `f64 e_to_minus_one(f64 x)` — e^x - 1, accurate for an `x` near zero, where `e_to(x) - 1.0` loses every…
* `f64 ln(f64 x)` — The natural logarithm: NaN below zero, minus infinity at zero. C's `log`.
* `f64 ln_one_plus(f64 x)` — ln(1 + x), accurate for an `x` near zero, where `1.0 + x` would round `x`
* `f64 log_2(f64 x)` — The base-2 logarithm; exact at powers of two. C's `log2`.
* `f64 log_10(f64 x)` — The base-10 logarithm. C's `log10`.
* `f64 sine(f64 x)` — The sine. C's `sin`.
* `f64 cosine(f64 x)` — The cosine. C's `cos`.
* `f64 tangent(f64 x)` — The tangent. C's `tan`.
* `f64 arcsine(f64 x)` — The inverse sine, in [-π/2, π/2]; NaN outside [-1, 1]. C's `asin`.
* `f64 arccosine(f64 x)` — The inverse cosine, in [0, π]; NaN outside [-1, 1]. C's `acos`.
* `f64 arctangent(f64 x)` — The inverse tangent, in [-π/2, π/2]. C's `atan`. For the angle of a…
* `f64 angle(f64 y, f64 x)` — The angle of a vector, quadrant included — which is what `atan2` is for and…
* `f64 hyperbolic_sine(f64 x)` — The hyperbolic sine. C's `sinh`.
* `f64 hyperbolic_cosine(f64 x)` — The hyperbolic cosine. C's `cosh`.
* `f64 hyperbolic_tangent(f64 x)` — The hyperbolic tangent, in [-1, 1]. C's `tanh`.
* `$T min(T a, T b) where T is scalar` — The smaller of `a` and `b`; `a` on a tie.
* `$T max(T a, T b) where T is scalar` — The larger of `a` and `b`; `a` on a tie.
* `$T clamp(T x, T low, T high) where T is scalar` — `x` held to [`low`, `high`].
* `$T sign(T x) where T is scalar` — The sign of `x`: 1 above zero, -1 below, 0 otherwise — a zero of either…
* `$B power(B base, $P exponent) where B is float && P is int` — `base` raised to a whole-number power, by squaring.
* `$T lerp(T a, T b, T t) where T is float` — The point `t` of the way from `a` to `b`: `a` at 0, `b` at 1, and beyond…
* `$T smoothstep(T edge0, T edge1, T x) where T is float` — 0 at or below `edge0`, 1 at or above `edge1`, and the cubic 3u^2 - 2u^3…
* `$T gcd(T a, T b) where T is int` — The greatest common divisor of `a` and `b`, never negative:
* `$T lcm(T a, T b) where T is int` — The least common multiple of `a` and `b`, never negative: `lcm(4, 6)` is…
