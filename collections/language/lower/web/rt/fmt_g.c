/* The `#WEB` runtime: the float render, `%g`-exact.
 *
 * `dado_rt__fmt_g(buf, v)` writes exactly what glibc's
 * `snprintf(buf, n, "%g", v)` writes for every `double` — six significant
 * digits, rounded half-to-even on the exact binary value; `%e` style when the
 * rounded decimal exponent is below -4 or at least 6, `%f` style otherwise;
 * trailing zeros and a bare point removed; an exponent of at least two digits;
 * `inf`, `-inf` and `-0` — and answers the number of bytes written before the
 * terminating NUL, as `snprintf` does.
 *
 * **One departure: every NaN is `nan`** (R11.11), where glibc writes `-nan`
 * for one whose sign bit is set — which `0.0 / 0.0` is on x86-64. A NaN's sign
 * is not a value a program can compute with, and the hosted render
 * (`Emitter::render_form`'s float arm) makes the same check, so a NaN prints
 * one way on every target.
 *
 * **Buffer bound: `DADO_RT__FMT_G_BOUND` (14) bytes, NUL included.** The
 * longest renders are thirteen bytes, `-1.23457e-308` and `-1.23457e+308`;
 * `%f` style is at most `-0.000123457` (twelve).
 *
 * **Method: exact multi-precision decimal conversion.** A finite nonzero `v`
 * is `m * 2^e` with `m` an integer below 2^53. For `e >= 0` the integer
 * `m * 2^e` is formed exactly in base 10^9 limbs; for `e < 0` the value is
 * `m * 5^-e / 10^-e`, so the integer `m * 5^-e` is formed exactly and the
 * decimal point placed `-e` digits from its right. Every digit of the binary
 * value is then known, so the sixth-digit rounding sees the true remainder —
 * below, above or exactly one half — with no floating-point arithmetic and no
 * dependence on the rounding mode. The largest expansion is `m * 5^1074`, 767
 * digits in 86 limbs.
 *
 * **Attribution.** Modeled on musl's `fmt_fp` (`src/stdio/vfprintf.c`, musl
 * 1.2.5): the base-10^9 limb expansion, the digit-position bookkeeping and the
 * `%g` style choice follow it. It differs in forming the fraction as
 * `m * 5^k` where musl divides by 2^9 repeatedly, in never truncating the
 * expansion, and in rounding half-to-even directly where musl probes the
 * current rounding mode. musl is
 *
 *   Copyright (c) 2005-2020 Rich Felker, et al.
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to permit
 *   persons to whom the Software is furnished to do so, subject to the
 *   following conditions: The above copyright notice and this permission
 *   notice shall be included in all copies or substantial portions of the
 *   Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 *   NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *   DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 *   USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Freestanding C11; the only headers are `<stddef.h>` and `<stdint.h>`.
 */

#include <stddef.h>
#include <stdint.h>

#define DADO_RT__FMT_G_BOUND 14

int dado_rt__fmt_g(char *buf, double v);

#define DADO_RT__LIMB 1000000000u
#define DADO_RT__LIMBS 90 /* 86 needed: m * 5^1074 has 767 digits */

static const uint32_t dado_rt__pow10[10] = {
    1u, 10u, 100u, 1000u, 10000u, 100000u, 1000000u, 10000000u, 100000000u, 1000000000u,
};

/* Multiply the little-endian limbs `l[0..*n)` by `f`, where `f * 10^9` plus a
 * carry below 2^32 fits 64 bits. */
static void dado_rt__limbs_mul(uint32_t *l, int *n, uint64_t f) {
    uint64_t carry = 0;
    for (int i = 0; i < *n; i++) {
        uint64_t x = (uint64_t)l[i] * f + carry;
        l[i] = (uint32_t)(x % DADO_RT__LIMB);
        carry = x / DADO_RT__LIMB;
    }
    while (carry != 0) {
        l[(*n)++] = (uint32_t)(carry % DADO_RT__LIMB);
        carry /= DADO_RT__LIMB;
    }
}

static int dado_rt__digits(uint32_t x) { /* x != 0 */
    int d = 1;
    while (d < 10 && x >= dado_rt__pow10[d]) {
        d++;
    }
    return d;
}

static int dado_rt__put(char *buf, int at, const char *s) {
    while (*s != '\0') {
        buf[at++] = *s++;
    }
    return at;
}

int dado_rt__fmt_g(char *buf, double v) {
    union {
        double f;
        uint64_t u;
    } bits = {v};
    int at = 0;
    if (v != v) {
        at = dado_rt__put(buf, at, "nan");
        buf[at] = '\0';
        return at;
    }
    if (bits.u >> 63) {
        buf[at++] = '-';
    }
    unsigned biased = (unsigned)(bits.u >> 52) & 0x7ffu;
    uint64_t m = bits.u & ((UINT64_C(1) << 52) - 1u);
    if (biased == 0x7ffu) {
        at = dado_rt__put(buf, at, "inf");
        buf[at] = '\0';
        return at;
    }
    if (biased == 0 && m == 0) {
        buf[at++] = '0';
        buf[at] = '\0';
        return at;
    }
    int e2;
    if (biased == 0) {
        e2 = -1074;
    } else {
        m |= UINT64_C(1) << 52;
        e2 = (int)biased - 1075;
    }
    while ((m & 1u) == 0 && e2 < 0) { /* fewer factors of 5 to form */
        m >>= 1;
        e2++;
    }

    /* The exact integer D, with v = D * 10^e10. */
    uint32_t l[DADO_RT__LIMBS];
    int n = 0;
    l[n++] = (uint32_t)(m % DADO_RT__LIMB);
    if (m >= DADO_RT__LIMB) {
        l[n++] = (uint32_t)(m / DADO_RT__LIMB % DADO_RT__LIMB);
        if (m >= (uint64_t)DADO_RT__LIMB * DADO_RT__LIMB) {
            l[n++] = (uint32_t)(m / DADO_RT__LIMB / DADO_RT__LIMB);
        }
    }
    int e10 = 0;
    if (e2 > 0) {
        for (int e = e2; e > 0; e -= 29) {
            dado_rt__limbs_mul(l, &n, UINT64_C(1) << (e < 29 ? e : 29));
        }
    } else if (e2 < 0) {
        e10 = e2;
        int k = -e2;
        for (; k >= 13; k -= 13) {
            dado_rt__limbs_mul(l, &n, UINT64_C(1220703125)); /* 5^13 */
        }
        uint64_t f = 1;
        for (; k > 0; k--) {
            f *= 5u;
        }
        if (f != 1) {
            dado_rt__limbs_mul(l, &n, f);
        }
    }

    /* The leading seven digits of D, and whether anything below them is
     * nonzero. D's top limb is nonzero; the top two limbs hold at least ten
     * digits, so they alone decide the seven. */
    int top = dado_rt__digits(l[n - 1]);
    int nd = 9 * (n - 1) + top;
    uint64_t hi = l[n - 1];
    int hi_digits = top;
    int below = n - 1; /* limbs below the ones folded into `hi` */
    if (n >= 2) {
        hi = hi * DADO_RT__LIMB + l[n - 2];
        hi_digits += 9;
        below = n - 2;
    }
    uint64_t seven;
    int sticky = 0;
    if (hi_digits >= 7) {
        uint64_t div = 1;
        for (int i = 7; i < hi_digits; i++) {
            div *= 10u;
        }
        seven = hi / div;
        sticky = hi % div != 0;
    } else {
        seven = hi * dado_rt__pow10[7 - hi_digits];
    }
    for (int i = 0; i < below && !sticky; i++) {
        sticky = l[i] != 0;
    }

    /* Round to six, half to even on the exact value. */
    uint32_t q = (uint32_t)(seven / 10u);
    uint32_t r = (uint32_t)(seven % 10u);
    int x = nd - 1 + e10; /* the decimal exponent of the leading digit */
    if (r > 5u || (r == 5u && (sticky || (q & 1u)))) {
        q++;
        if (q == 1000000u) {
            q = 100000u;
            x++;
        }
    }
    char d[6];
    for (int i = 5; i >= 0; i--) {
        d[i] = (char)('0' + q % 10u);
        q /= 10u;
    }
    int last = 5; /* the last significant digit once trailing zeros go */
    while (last > 0 && d[last] == '0') {
        last--;
    }

    if (x < -4 || x >= 6) {
        buf[at++] = d[0];
        if (last > 0) {
            buf[at++] = '.';
            for (int i = 1; i <= last; i++) {
                buf[at++] = d[i];
            }
        }
        buf[at++] = 'e';
        buf[at++] = x < 0 ? '-' : '+';
        unsigned ex = (unsigned)(x < 0 ? -x : x);
        if (ex >= 100u) {
            buf[at++] = (char)('0' + ex / 100u);
        }
        buf[at++] = (char)('0' + ex / 10u % 10u);
        buf[at++] = (char)('0' + ex % 10u);
    } else if (x >= 0) {
        for (int i = 0; i <= x; i++) {
            buf[at++] = d[i];
        }
        if (last > x) {
            buf[at++] = '.';
            for (int i = x + 1; i <= last; i++) {
                buf[at++] = d[i];
            }
        }
    } else {
        buf[at++] = '0';
        buf[at++] = '.';
        for (int i = -1; i > x; i--) {
            buf[at++] = '0';
        }
        for (int i = 0; i <= last; i++) {
            buf[at++] = d[i];
        }
    }
    buf[at] = '\0';
    return at;
}

#undef DADO_RT__LIMB
#undef DADO_RT__LIMBS
