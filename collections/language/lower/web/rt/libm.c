/* The `#WEB` runtime: the math library.
 *
 * The `<math.h>` functions `core:math` reaches, as the runtime's own C, so a
 * `#WEB` program computes the same bits in every browser and under node:
 * ECMAScript leaves `Math.sin` and its kin *implementation-approximated*, and
 * this file is C11 with no host call, so the answer is the module's, not the
 * engine's. The same holds natively: the file is compiled with no fused
 * multiply-add contraction (`FP_CONTRACT OFF` below; wasm has no fused
 * instruction to contract to) and with musl's no-FMA path chosen everywhere.
 *
 * **What is here, and why.** `core:math` declares `sqrt pow fabs fmod floor
 * ceil trunc round hypot fma exp log log2 log10 sin cos tan atan2` and calls
 * the `isnan`/`isinf` macros; `core:linalg` declares `sqrt sin cos tan acos
 * atan2`; the programs that run on the web reach nothing else (only
 * `vectors`, which calls `sqrt`). So:
 *
 *   * `sqrt floor ceil trunc fabs copysign` are one wasm instruction each
 *     (`f64.sqrt` `f64.floor` `f64.ceil` `f64.trunc` `f64.abs`
 *     `f64.copysign`): each is defined as its `__builtin_`, which clang lowers
 *     to that instruction for wasm32. They are functions, not macros, so a
 *     program may take their address and an emitted `_Generic` check of a
 *     restated prototype sees C's own. They are defined only for `__wasm__`:
 *     elsewhere a builtin such as `__builtin_floor` may be a call of `floor`,
 *     which here would be a call of itself, and a native build (the tests)
 *     takes these six from its C library.
 *   * `isnan` and `isinf` are macros over `__builtin_isnan` and
 *     `__builtin_isinf_sign`, as `<math.h>` would give them.
 *   * the rest are transliterated from musl 1.2.5 `src/math` below: `exp log
 *     log2 pow` (Szabolcs Nagy's implementations for Arm, with their tables),
 *     `log10 sin cos tan acos atan2 hypot fma round fmod` (FreeBSD msun, via
 *     musl), and their helpers `scalbn atan __rem_pio2 __rem_pio2_large __sin
 *     __cos __tan __math_*`, which are `static` here and renamed into
 *     `dado_rt__…`. `round` is not an instruction: wasm's `f64.nearest` rounds
 *     half to even and C's `round` half away from zero.
 *
 * `tests/webrt/gen_libm.py` produced the part below the marker from the musl
 * tarball, and says exactly what it changed: includes, configuration
 * conditionals resolved (no fast FMA, `FLT_EVAL_METHOD == 0`, a double-only
 * `__rem_pio2_large` table), `double_t` spelled `double`, internal names
 * renamed, per-file statics prefixed, each file's macros `#undef`ed after it.
 * The algorithms and constants are musl's, untouched.
 *
 * **Licences.** musl is MIT:
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
 * The `exp log log2 pow` sources and tables are "Copyright (c) 2018, Arm
 * Limited. SPDX-License-Identifier: MIT". The msun-derived files carry Sun
 * Microsystems' notice, kept verbatim where each begins below: "Copyright (C)
 * 1993 by Sun Microsystems, Inc. All rights reserved. Developed at SunPro, a
 * Sun Microsystems, Inc. business. Permission to use, copy, modify, and
 * distribute this software is freely granted, provided that this notice is
 * preserved."
 *
 * Freestanding C11; the only headers are `<stdint.h>` and `<float.h>`.
 */

#include <float.h>
#include <stdint.h>

#if defined(__clang__)
#pragma STDC FP_CONTRACT OFF
#endif

_Static_assert(FLT_EVAL_METHOD == 0, "the math library assumes double arithmetic is double");

double sqrt(double x);
double floor(double x);
double ceil(double x);
double trunc(double x);
double fabs(double x);
double copysign(double x, double y);
double exp(double x);
double log(double x);
double log2(double x);
double log10(double x);
double pow(double x, double y);
double sin(double x);
double cos(double x);
double tan(double x);
double acos(double x);
double atan2(double y, double x);
double hypot(double x, double y);
double fma(double x, double y, double z);
double round(double x);
double fmod(double x, double y);

#if !defined(isnan)
#define isnan(x) __builtin_isnan(x)
#endif
#if !defined(isinf)
#define isinf(x) __builtin_isinf_sign(x)
#endif

/* ── the six that are one instruction on wasm32 ────────────────────────── */

#if defined(__wasm__)
double sqrt(double x) {
    return __builtin_sqrt(x);
}
double floor(double x) {
    return __builtin_floor(x);
}
double ceil(double x) {
    return __builtin_ceil(x);
}
double trunc(double x) {
    return __builtin_trunc(x);
}
double fabs(double x) {
    return __builtin_fabs(x);
}
double copysign(double x, double y) {
    return __builtin_copysign(x, y);
}
#endif

/* ── musl's `src/internal/libm.h`, the part the files below use ─────────── */

#define DADO_RT__WANT_ROUNDING 1
#define DADO_RT__IS_SIGNALING(x) 0
#define DADO_RT__LIKELY(x) __builtin_expect(!!(x), 1)
#define DADO_RT__UNLIKELY(x) __builtin_expect(x, 0)
#define dado_rt__asuint64(f) ((union {double _f; uint64_t _i;}){f})._i
#define dado_rt__asdouble(i) ((union {uint64_t _i; double _f;}){i})._f
#define DADO_RT__EXTRACT_WORDS(hi, lo, d)                                                       \
    do {                                                                                       \
        uint64_t dado_rt__u = dado_rt__asuint64(d);                                              \
        (hi) = dado_rt__u >> 32;                                                                \
        (lo) = (uint32_t)dado_rt__u;                                                            \
    } while (0)
#define DADO_RT__GET_HIGH_WORD(hi, d)                                                           \
    do {                                                                                       \
        (hi) = dado_rt__asuint64(d) >> 32;                                                      \
    } while (0)
#define DADO_RT__GET_LOW_WORD(lo, d)                                                            \
    do {                                                                                       \
        (lo) = (uint32_t)dado_rt__asuint64(d);                                                  \
    } while (0)
#define DADO_RT__INSERT_WORDS(d, hi, lo)                                                        \
    do {                                                                                       \
        (d) = dado_rt__asdouble(((uint64_t)(hi) << 32) | (uint32_t)(lo));                       \
    } while (0)
#define DADO_RT__SET_HIGH_WORD(d, hi) DADO_RT__INSERT_WORDS(d, hi, (uint32_t)dado_rt__asuint64(d))
#define DADO_RT__SET_LOW_WORD(d, lo) DADO_RT__INSERT_WORDS(d, dado_rt__asuint64(d) >> 32, lo)

static inline double dado_rt__eval_as_double(double x) {
    double y = x;
    return y;
}
static inline double dado_rt__fp_barrier(double x) {
    volatile double y = x;
    return y;
}
static inline void dado_rt__fp_force_eval(double x) {
    volatile double y;
    y = x;
    (void)y;
}
/* Evaluate an expression for its floating-point side effects only. */
#define DADO_RT__FORCE_EVAL(x)                                                                  \
    do {                                                                                       \
        if (sizeof(x) == sizeof(float)) {                                                      \
            volatile float dado_rt__y = (float)(x);                                             \
            (void)dado_rt__y;                                                                   \
        } else {                                                                               \
            volatile double dado_rt__y = (double)(x);                                           \
            (void)dado_rt__y;                                                                   \
        }                                                                                      \
    } while (0)

/* ── below: generated by tests/webrt/gen_libm.py from musl 1.2.5 ───────── */

/* ---- musl src/math/__math_xflow.c ---- */

static double dado_rt__math_xflow(uint32_t sign, double y)
{
	return dado_rt__eval_as_double(dado_rt__fp_barrier(sign ? -y : y) * y);
}


/* ---- musl src/math/__math_oflow.c ---- */

static double dado_rt__math_oflow(uint32_t sign)
{
	return dado_rt__math_xflow(sign, 0x1p769);
}


/* ---- musl src/math/__math_uflow.c ---- */

static double dado_rt__math_uflow(uint32_t sign)
{
	return dado_rt__math_xflow(sign, 0x1p-767);
}


/* ---- musl src/math/__math_divzero.c ---- */

static double dado_rt__math_divzero(uint32_t sign)
{
	return dado_rt__fp_barrier(sign ? -1.0 : 1.0) / 0.0;
}


/* ---- musl src/math/__math_invalid.c ---- */

static double dado_rt__math_invalid(double x)
{
	return (x - x) / (x - x);
}


/* ---- musl src/math/scalbn.c ---- */

static double dado_rt__scalbn(double x, int n)
{
	union {double f; uint64_t i;} u;
	double y = x;

	if (n > 1023) {
		y *= 0x1p1023;
		n -= 1023;
		if (n > 1023) {
			y *= 0x1p1023;
			n -= 1023;
			if (n > 1023)
				n = 1023;
		}
	} else if (n < -1022) {
		/* make sure final n < -53 to avoid double
		   rounding in the subnormal range */
		y *= 0x1p-1022 * 0x1p53;
		n += 1022 - 53;
		if (n < -1022) {
			y *= 0x1p-1022 * 0x1p53;
			n += 1022 - 53;
			if (n < -1022)
				n = -1022;
		}
	}
	u.i = (uint64_t)(0x3ff+n)<<52;
	x = y * u.f;
	return x;
}


/* ---- musl src/math/exp_data.h ---- */

/*
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define EXP_TABLE_BITS 7
#define EXP_POLY_ORDER 5
#define EXP_USE_TOINT_NARROW 0
#define EXP2_POLY_ORDER 5
struct dado_rt__exp_table {
	double invln2N;
	double shift;
	double negln2hiN;
	double negln2loN;
	double poly[4]; /* Last four coefficients.  */
	double exp2_shift;
	double exp2_poly[EXP2_POLY_ORDER];
	uint64_t tab[2*(1 << EXP_TABLE_BITS)];
};


/* ---- musl src/math/exp_data.c ---- */

/*
 * Shared data between exp, exp2 and pow.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define N (1 << EXP_TABLE_BITS)

static const struct dado_rt__exp_table dado_rt__exp_data = {
// N/ln2
.invln2N = 0x1.71547652b82fep0 * N,
// -ln2/N
.negln2hiN = -0x1.62e42fefa0000p-8,
.negln2loN = -0x1.cf79abc9e3b3ap-47,
// Used for rounding when !TOINT_INTRINSICS
.shift = 0x1.8p52,
// exp polynomial coefficients.
.poly = {
// abs error: 1.555*2^-66
// ulp error: 0.509 (0.511 without fma)
// if |x| < ln2/256+eps
// abs error if |x| < ln2/256+0x1p-15: 1.09*2^-65
// abs error if |x| < ln2/128: 1.7145*2^-56
0x1.ffffffffffdbdp-2,
0x1.555555555543cp-3,
0x1.55555cf172b91p-5,
0x1.1111167a4d017p-7,
},
.exp2_shift = 0x1.8p52 / N,
// exp2 polynomial coefficients.
.exp2_poly = {
// abs error: 1.2195*2^-65
// ulp error: 0.507 (0.511 without fma)
// if |x| < 1/256
// abs error if |x| < 1/128: 1.9941*2^-56
0x1.62e42fefa39efp-1,
0x1.ebfbdff82c424p-3,
0x1.c6b08d70cf4b5p-5,
0x1.3b2abd24650ccp-7,
0x1.5d7e09b4e3a84p-10,
},
// 2^(k/N) ~= H[k]*(1 + T[k]) for int k in [0,N)
// tab[2*k] = asuint64(T[k])
// tab[2*k+1] = asuint64(H[k]) - (k << 52)/N
.tab = {
0x0, 0x3ff0000000000000,
0x3c9b3b4f1a88bf6e, 0x3feff63da9fb3335,
0xbc7160139cd8dc5d, 0x3fefec9a3e778061,
0xbc905e7a108766d1, 0x3fefe315e86e7f85,
0x3c8cd2523567f613, 0x3fefd9b0d3158574,
0xbc8bce8023f98efa, 0x3fefd06b29ddf6de,
0x3c60f74e61e6c861, 0x3fefc74518759bc8,
0x3c90a3e45b33d399, 0x3fefbe3ecac6f383,
0x3c979aa65d837b6d, 0x3fefb5586cf9890f,
0x3c8eb51a92fdeffc, 0x3fefac922b7247f7,
0x3c3ebe3d702f9cd1, 0x3fefa3ec32d3d1a2,
0xbc6a033489906e0b, 0x3fef9b66affed31b,
0xbc9556522a2fbd0e, 0x3fef9301d0125b51,
0xbc5080ef8c4eea55, 0x3fef8abdc06c31cc,
0xbc91c923b9d5f416, 0x3fef829aaea92de0,
0x3c80d3e3e95c55af, 0x3fef7a98c8a58e51,
0xbc801b15eaa59348, 0x3fef72b83c7d517b,
0xbc8f1ff055de323d, 0x3fef6af9388c8dea,
0x3c8b898c3f1353bf, 0x3fef635beb6fcb75,
0xbc96d99c7611eb26, 0x3fef5be084045cd4,
0x3c9aecf73e3a2f60, 0x3fef54873168b9aa,
0xbc8fe782cb86389d, 0x3fef4d5022fcd91d,
0x3c8a6f4144a6c38d, 0x3fef463b88628cd6,
0x3c807a05b0e4047d, 0x3fef3f49917ddc96,
0x3c968efde3a8a894, 0x3fef387a6e756238,
0x3c875e18f274487d, 0x3fef31ce4fb2a63f,
0x3c80472b981fe7f2, 0x3fef2b4565e27cdd,
0xbc96b87b3f71085e, 0x3fef24dfe1f56381,
0x3c82f7e16d09ab31, 0x3fef1e9df51fdee1,
0xbc3d219b1a6fbffa, 0x3fef187fd0dad990,
0x3c8b3782720c0ab4, 0x3fef1285a6e4030b,
0x3c6e149289cecb8f, 0x3fef0cafa93e2f56,
0x3c834d754db0abb6, 0x3fef06fe0a31b715,
0x3c864201e2ac744c, 0x3fef0170fc4cd831,
0x3c8fdd395dd3f84a, 0x3feefc08b26416ff,
0xbc86a3803b8e5b04, 0x3feef6c55f929ff1,
0xbc924aedcc4b5068, 0x3feef1a7373aa9cb,
0xbc9907f81b512d8e, 0x3feeecae6d05d866,
0xbc71d1e83e9436d2, 0x3feee7db34e59ff7,
0xbc991919b3ce1b15, 0x3feee32dc313a8e5,
0x3c859f48a72a4c6d, 0x3feedea64c123422,
0xbc9312607a28698a, 0x3feeda4504ac801c,
0xbc58a78f4817895b, 0x3feed60a21f72e2a,
0xbc7c2c9b67499a1b, 0x3feed1f5d950a897,
0x3c4363ed60c2ac11, 0x3feece086061892d,
0x3c9666093b0664ef, 0x3feeca41ed1d0057,
0x3c6ecce1daa10379, 0x3feec6a2b5c13cd0,
0x3c93ff8e3f0f1230, 0x3feec32af0d7d3de,
0x3c7690cebb7aafb0, 0x3feebfdad5362a27,
0x3c931dbdeb54e077, 0x3feebcb299fddd0d,
0xbc8f94340071a38e, 0x3feeb9b2769d2ca7,
0xbc87deccdc93a349, 0x3feeb6daa2cf6642,
0xbc78dec6bd0f385f, 0x3feeb42b569d4f82,
0xbc861246ec7b5cf6, 0x3feeb1a4ca5d920f,
0x3c93350518fdd78e, 0x3feeaf4736b527da,
0x3c7b98b72f8a9b05, 0x3feead12d497c7fd,
0x3c9063e1e21c5409, 0x3feeab07dd485429,
0x3c34c7855019c6ea, 0x3feea9268a5946b7,
0x3c9432e62b64c035, 0x3feea76f15ad2148,
0xbc8ce44a6199769f, 0x3feea5e1b976dc09,
0xbc8c33c53bef4da8, 0x3feea47eb03a5585,
0xbc845378892be9ae, 0x3feea34634ccc320,
0xbc93cedd78565858, 0x3feea23882552225,
0x3c5710aa807e1964, 0x3feea155d44ca973,
0xbc93b3efbf5e2228, 0x3feea09e667f3bcd,
0xbc6a12ad8734b982, 0x3feea012750bdabf,
0xbc6367efb86da9ee, 0x3fee9fb23c651a2f,
0xbc80dc3d54e08851, 0x3fee9f7df9519484,
0xbc781f647e5a3ecf, 0x3fee9f75e8ec5f74,
0xbc86ee4ac08b7db0, 0x3fee9f9a48a58174,
0xbc8619321e55e68a, 0x3fee9feb564267c9,
0x3c909ccb5e09d4d3, 0x3feea0694fde5d3f,
0xbc7b32dcb94da51d, 0x3feea11473eb0187,
0x3c94ecfd5467c06b, 0x3feea1ed0130c132,
0x3c65ebe1abd66c55, 0x3feea2f336cf4e62,
0xbc88a1c52fb3cf42, 0x3feea427543e1a12,
0xbc9369b6f13b3734, 0x3feea589994cce13,
0xbc805e843a19ff1e, 0x3feea71a4623c7ad,
0xbc94d450d872576e, 0x3feea8d99b4492ed,
0x3c90ad675b0e8a00, 0x3feeaac7d98a6699,
0x3c8db72fc1f0eab4, 0x3feeace5422aa0db,
0xbc65b6609cc5e7ff, 0x3feeaf3216b5448c,
0x3c7bf68359f35f44, 0x3feeb1ae99157736,
0xbc93091fa71e3d83, 0x3feeb45b0b91ffc6,
0xbc5da9b88b6c1e29, 0x3feeb737b0cdc5e5,
0xbc6c23f97c90b959, 0x3feeba44cbc8520f,
0xbc92434322f4f9aa, 0x3feebd829fde4e50,
0xbc85ca6cd7668e4b, 0x3feec0f170ca07ba,
0x3c71affc2b91ce27, 0x3feec49182a3f090,
0x3c6dd235e10a73bb, 0x3feec86319e32323,
0xbc87c50422622263, 0x3feecc667b5de565,
0x3c8b1c86e3e231d5, 0x3feed09bec4a2d33,
0xbc91bbd1d3bcbb15, 0x3feed503b23e255d,
0x3c90cc319cee31d2, 0x3feed99e1330b358,
0x3c8469846e735ab3, 0x3feede6b5579fdbf,
0xbc82dfcd978e9db4, 0x3feee36bbfd3f37a,
0x3c8c1a7792cb3387, 0x3feee89f995ad3ad,
0xbc907b8f4ad1d9fa, 0x3feeee07298db666,
0xbc55c3d956dcaeba, 0x3feef3a2b84f15fb,
0xbc90a40e3da6f640, 0x3feef9728de5593a,
0xbc68d6f438ad9334, 0x3feeff76f2fb5e47,
0xbc91eee26b588a35, 0x3fef05b030a1064a,
0x3c74ffd70a5fddcd, 0x3fef0c1e904bc1d2,
0xbc91bdfbfa9298ac, 0x3fef12c25bd71e09,
0x3c736eae30af0cb3, 0x3fef199bdd85529c,
0x3c8ee3325c9ffd94, 0x3fef20ab5fffd07a,
0x3c84e08fd10959ac, 0x3fef27f12e57d14b,
0x3c63cdaf384e1a67, 0x3fef2f6d9406e7b5,
0x3c676b2c6c921968, 0x3fef3720dcef9069,
0xbc808a1883ccb5d2, 0x3fef3f0b555dc3fa,
0xbc8fad5d3ffffa6f, 0x3fef472d4a07897c,
0xbc900dae3875a949, 0x3fef4f87080d89f2,
0x3c74a385a63d07a7, 0x3fef5818dcfba487,
0xbc82919e2040220f, 0x3fef60e316c98398,
0x3c8e5a50d5c192ac, 0x3fef69e603db3285,
0x3c843a59ac016b4b, 0x3fef7321f301b460,
0xbc82d52107b43e1f, 0x3fef7c97337b9b5f,
0xbc892ab93b470dc9, 0x3fef864614f5a129,
0x3c74b604603a88d3, 0x3fef902ee78b3ff6,
0x3c83c5ec519d7271, 0x3fef9a51fbc74c83,
0xbc8ff7128fd391f0, 0x3fefa4afa2a490da,
0xbc8dae98e223747d, 0x3fefaf482d8e67f1,
0x3c8ec3bc41aa2008, 0x3fefba1bee615a27,
0x3c842b94c3a9eb32, 0x3fefc52b376bba97,
0x3c8a64a931d185ee, 0x3fefd0765b6e4540,
0xbc8e37bae43be3ed, 0x3fefdbfdad9cbe14,
0x3c77893b4d91cd9d, 0x3fefe7c1819e90d8,
0x3c5305c14160cc89, 0x3feff3c22b8f71f1,
},
};

#undef N

/* ---- musl src/math/exp.c ---- */

/*
 * Double-precision e^x function.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define N (1 << EXP_TABLE_BITS)
#define InvLn2N dado_rt__exp_data.invln2N
#define NegLn2hiN dado_rt__exp_data.negln2hiN
#define NegLn2loN dado_rt__exp_data.negln2loN
#define Shift dado_rt__exp_data.shift
#define T dado_rt__exp_data.tab
#define C2 dado_rt__exp_data.poly[5 - EXP_POLY_ORDER]
#define C3 dado_rt__exp_data.poly[6 - EXP_POLY_ORDER]
#define C4 dado_rt__exp_data.poly[7 - EXP_POLY_ORDER]
#define C5 dado_rt__exp_data.poly[8 - EXP_POLY_ORDER]

/* Handle cases that may overflow or underflow when computing the result that
   is scale*(1+TMP) without intermediate rounding.  The bit representation of
   scale is in SBITS, however it has a computed exponent that may have
   overflown into the sign bit so that needs to be adjusted before using it as
   a double.  (int32_t)KI is the k used in the argument reduction and exponent
   adjustment of scale, positive k here means the result may overflow and
   negative k means the result may underflow.  */
static inline double dado_rt__exp__specialcase(double tmp, uint64_t sbits, uint64_t ki)
{
	double scale, y;

	if ((ki & 0x80000000) == 0) {
		/* k > 0, the exponent of scale might have overflowed by <= 460.  */
		sbits -= 1009ull << 52;
		scale = dado_rt__asdouble(sbits);
		y = 0x1p1009 * (scale + scale * tmp);
		return dado_rt__eval_as_double(y);
	}
	/* k < 0, need special care in the subnormal range.  */
	sbits += 1022ull << 52;
	scale = dado_rt__asdouble(sbits);
	y = scale + scale * tmp;
	if (y < 1.0) {
		/* Round y to the right precision before scaling it into the subnormal
		 range to avoid double rounding that can cause 0.5+E/2 ulp error where
		 E is the worst-case ulp error outside the subnormal range.  So this
		 is only useful if the goal is better than 1 ulp worst-case error.  */
		double hi, lo;
		lo = scale - y + scale * tmp;
		hi = 1.0 + y;
		lo = 1.0 - hi + y + lo;
		y = dado_rt__eval_as_double(hi + lo) - 1.0;
		/* Avoid -0.0 with downward rounding.  */
		if (DADO_RT__WANT_ROUNDING && y == 0.0)
			y = 0.0;
		/* The underflow exception needs to be signaled explicitly.  */
		dado_rt__fp_force_eval(dado_rt__fp_barrier(0x1p-1022) * 0x1p-1022);
	}
	y = 0x1p-1022 * y;
	return dado_rt__eval_as_double(y);
}

/* Top 12 bits of a double (sign and exponent bits).  */
static inline uint32_t dado_rt__exp__top12(double x)
{
	return dado_rt__asuint64(x) >> 52;
}

double exp(double x)
{
	uint32_t abstop;
	uint64_t ki, idx, top, sbits;
	double kd, z, r, r2, scale, tail, tmp;

	abstop = dado_rt__exp__top12(x) & 0x7ff;
	if (DADO_RT__UNLIKELY(abstop - dado_rt__exp__top12(0x1p-54) >= dado_rt__exp__top12(512.0) - dado_rt__exp__top12(0x1p-54))) {
		if (abstop - dado_rt__exp__top12(0x1p-54) >= 0x80000000)
			/* Avoid spurious underflow for tiny x.  */
			/* Note: 0 is common input.  */
			return DADO_RT__WANT_ROUNDING ? 1.0 + x : 1.0;
		if (abstop >= dado_rt__exp__top12(1024.0)) {
			if (dado_rt__asuint64(x) == dado_rt__asuint64(-__builtin_inf()))
				return 0.0;
			if (abstop >= dado_rt__exp__top12(__builtin_inf()))
				return 1.0 + x;
			if (dado_rt__asuint64(x) >> 63)
				return dado_rt__math_uflow(0);
			else
				return dado_rt__math_oflow(0);
		}
		/* Large x is special cased below.  */
		abstop = 0;
	}

	/* exp(x) = 2^(k/N) * exp(r), with exp(r) in [2^(-1/2N),2^(1/2N)].  */
	/* x = ln2/N*k + r, with int k and r in [-ln2/2N, ln2/2N].  */
	z = InvLn2N * x;
	/* z - kd is in [-1, 1] in non-nearest rounding modes.  */
	kd = dado_rt__eval_as_double(z + Shift);
	ki = dado_rt__asuint64(kd);
	kd -= Shift;
	r = x + kd * NegLn2hiN + kd * NegLn2loN;
	/* 2^(k/N) ~= scale * (1 + tail).  */
	idx = 2 * (ki % N);
	top = ki << (52 - EXP_TABLE_BITS);
	tail = dado_rt__asdouble(T[idx]);
	/* This is only a valid scale when -1023*N < k < 1024*N.  */
	sbits = T[idx + 1] + top;
	/* exp(x) = 2^(k/N) * exp(r) ~= scale + scale * (tail + exp(r) - 1).  */
	/* Evaluation is optimized assuming superscalar pipelined execution.  */
	r2 = r * r;
	/* Without fma the worst case error is 0.25/N ulp larger.  */
	/* Worst case error is less than 0.5+1.11/N+(abs poly error * 2^53) ulp.  */
	tmp = tail + r + r2 * (C2 + r * C3) + r2 * r2 * (C4 + r * C5);
	if (DADO_RT__UNLIKELY(abstop == 0))
		return dado_rt__exp__specialcase(tmp, sbits, ki);
	scale = dado_rt__asdouble(sbits);
	/* Note: tmp == 0 or |tmp| > 2^-200 and scale > 2^-739, so there
	   is no spurious underflow here even without fma.  */
	return dado_rt__eval_as_double(scale + scale * tmp);
}

#undef N
#undef InvLn2N
#undef NegLn2hiN
#undef NegLn2loN
#undef Shift
#undef T
#undef C2
#undef C3
#undef C4
#undef C5

/* ---- musl src/math/log_data.h ---- */

/*
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define LOG_TABLE_BITS 7
#define LOG_POLY_ORDER 6
#define LOG_POLY1_ORDER 12
struct dado_rt__log_table {
	double ln2hi;
	double ln2lo;
	double poly[LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
	double poly1[LOG_POLY1_ORDER - 1];
	struct {
		double invc, logc;
	} tab[1 << LOG_TABLE_BITS];
	struct {
		double chi, clo;
	} tab2[1 << LOG_TABLE_BITS];
};


/* ---- musl src/math/log_data.c ---- */

/*
 * Data for log.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define N (1 << LOG_TABLE_BITS)

static const struct dado_rt__log_table dado_rt__log_data = {
.ln2hi = 0x1.62e42fefa3800p-1,
.ln2lo = 0x1.ef35793c76730p-45,
.poly1 = {
// relative error: 0x1.c04d76cp-63
// in -0x1p-4 0x1.09p-4 (|log(1+x)| > 0x1p-4 outside the interval)
-0x1p-1,
0x1.5555555555577p-2,
-0x1.ffffffffffdcbp-3,
0x1.999999995dd0cp-3,
-0x1.55555556745a7p-3,
0x1.24924a344de3p-3,
-0x1.fffffa4423d65p-4,
0x1.c7184282ad6cap-4,
-0x1.999eb43b068ffp-4,
0x1.78182f7afd085p-4,
-0x1.5521375d145cdp-4,
},
.poly = {
// relative error: 0x1.926199e8p-56
// abs error: 0x1.882ff33p-65
// in -0x1.fp-9 0x1.fp-9
-0x1.0000000000001p-1,
0x1.555555551305bp-2,
-0x1.fffffffeb459p-3,
0x1.999b324f10111p-3,
-0x1.55575e506c89fp-3,
},
/* Algorithm:

	x = 2^k z
	log(x) = k ln2 + log(c) + log(z/c)
	log(z/c) = poly(z/c - 1)

where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
into the ith one, then table entries are computed as

	tab[i].invc = 1/c
	tab[i].logc = (double)log(c)
	tab2[i].chi = (double)c
	tab2[i].clo = (double)(c - (double)c)

where c is near the center of the subinterval and is chosen by trying +-2^29
floating point invc candidates around 1/center and selecting one for which

	1) the rounding error in 0x1.8p9 + logc is 0,
	2) the rounding error in z - chi - clo is < 0x1p-66 and
	3) the rounding error in (double)log(c) is minimized (< 0x1p-66).

Note: 1) ensures that k*ln2hi + logc can be computed without rounding error,
2) ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close to
a single rounding error when there is no fast fma for z*invc - 1, 3) ensures
that logc + poly(z/c - 1) has small error, however near x == 1 when
|log(x)| < 0x1p-4, this is not enough so that is special cased.  */
.tab = {
{0x1.734f0c3e0de9fp+0, -0x1.7cc7f79e69000p-2},
{0x1.713786a2ce91fp+0, -0x1.76feec20d0000p-2},
{0x1.6f26008fab5a0p+0, -0x1.713e31351e000p-2},
{0x1.6d1a61f138c7dp+0, -0x1.6b85b38287800p-2},
{0x1.6b1490bc5b4d1p+0, -0x1.65d5590807800p-2},
{0x1.69147332f0cbap+0, -0x1.602d076180000p-2},
{0x1.6719f18224223p+0, -0x1.5a8ca86909000p-2},
{0x1.6524f99a51ed9p+0, -0x1.54f4356035000p-2},
{0x1.63356aa8f24c4p+0, -0x1.4f637c36b4000p-2},
{0x1.614b36b9ddc14p+0, -0x1.49da7fda85000p-2},
{0x1.5f66452c65c4cp+0, -0x1.445923989a800p-2},
{0x1.5d867b5912c4fp+0, -0x1.3edf439b0b800p-2},
{0x1.5babccb5b90dep+0, -0x1.396ce448f7000p-2},
{0x1.59d61f2d91a78p+0, -0x1.3401e17bda000p-2},
{0x1.5805612465687p+0, -0x1.2e9e2ef468000p-2},
{0x1.56397cee76bd3p+0, -0x1.2941b3830e000p-2},
{0x1.54725e2a77f93p+0, -0x1.23ec58cda8800p-2},
{0x1.52aff42064583p+0, -0x1.1e9e129279000p-2},
{0x1.50f22dbb2bddfp+0, -0x1.1956d2b48f800p-2},
{0x1.4f38f4734ded7p+0, -0x1.141679ab9f800p-2},
{0x1.4d843cfde2840p+0, -0x1.0edd094ef9800p-2},
{0x1.4bd3ec078a3c8p+0, -0x1.09aa518db1000p-2},
{0x1.4a27fc3e0258ap+0, -0x1.047e65263b800p-2},
{0x1.4880524d48434p+0, -0x1.feb224586f000p-3},
{0x1.46dce1b192d0bp+0, -0x1.f474a7517b000p-3},
{0x1.453d9d3391854p+0, -0x1.ea4443d103000p-3},
{0x1.43a2744b4845ap+0, -0x1.e020d44e9b000p-3},
{0x1.420b54115f8fbp+0, -0x1.d60a22977f000p-3},
{0x1.40782da3ef4b1p+0, -0x1.cc00104959000p-3},
{0x1.3ee8f5d57fe8fp+0, -0x1.c202956891000p-3},
{0x1.3d5d9a00b4ce9p+0, -0x1.b81178d811000p-3},
{0x1.3bd60c010c12bp+0, -0x1.ae2c9ccd3d000p-3},
{0x1.3a5242b75dab8p+0, -0x1.a45402e129000p-3},
{0x1.38d22cd9fd002p+0, -0x1.9a877681df000p-3},
{0x1.3755bc5847a1cp+0, -0x1.90c6d69483000p-3},
{0x1.35dce49ad36e2p+0, -0x1.87120a645c000p-3},
{0x1.34679984dd440p+0, -0x1.7d68fb4143000p-3},
{0x1.32f5cceffcb24p+0, -0x1.73cb83c627000p-3},
{0x1.3187775a10d49p+0, -0x1.6a39a9b376000p-3},
{0x1.301c8373e3990p+0, -0x1.60b3154b7a000p-3},
{0x1.2eb4ebb95f841p+0, -0x1.5737d76243000p-3},
{0x1.2d50a0219a9d1p+0, -0x1.4dc7b8fc23000p-3},
{0x1.2bef9a8b7fd2ap+0, -0x1.4462c51d20000p-3},
{0x1.2a91c7a0c1babp+0, -0x1.3b08abc830000p-3},
{0x1.293726014b530p+0, -0x1.31b996b490000p-3},
{0x1.27dfa5757a1f5p+0, -0x1.2875490a44000p-3},
{0x1.268b39b1d3bbfp+0, -0x1.1f3b9f879a000p-3},
{0x1.2539d838ff5bdp+0, -0x1.160c8252ca000p-3},
{0x1.23eb7aac9083bp+0, -0x1.0ce7f57f72000p-3},
{0x1.22a012ba940b6p+0, -0x1.03cdc49fea000p-3},
{0x1.2157996cc4132p+0, -0x1.f57bdbc4b8000p-4},
{0x1.201201dd2fc9bp+0, -0x1.e370896404000p-4},
{0x1.1ecf4494d480bp+0, -0x1.d17983ef94000p-4},
{0x1.1d8f5528f6569p+0, -0x1.bf9674ed8a000p-4},
{0x1.1c52311577e7cp+0, -0x1.adc79202f6000p-4},
{0x1.1b17c74cb26e9p+0, -0x1.9c0c3e7288000p-4},
{0x1.19e010c2c1ab6p+0, -0x1.8a646b372c000p-4},
{0x1.18ab07bb670bdp+0, -0x1.78d01b3ac0000p-4},
{0x1.1778a25efbcb6p+0, -0x1.674f145380000p-4},
{0x1.1648d354c31dap+0, -0x1.55e0e6d878000p-4},
{0x1.151b990275fddp+0, -0x1.4485cdea1e000p-4},
{0x1.13f0ea432d24cp+0, -0x1.333d94d6aa000p-4},
{0x1.12c8b7210f9dap+0, -0x1.22079f8c56000p-4},
{0x1.11a3028ecb531p+0, -0x1.10e4698622000p-4},
{0x1.107fbda8434afp+0, -0x1.ffa6c6ad20000p-5},
{0x1.0f5ee0f4e6bb3p+0, -0x1.dda8d4a774000p-5},
{0x1.0e4065d2a9fcep+0, -0x1.bbcece4850000p-5},
{0x1.0d244632ca521p+0, -0x1.9a1894012c000p-5},
{0x1.0c0a77ce2981ap+0, -0x1.788583302c000p-5},
{0x1.0af2f83c636d1p+0, -0x1.5715e67d68000p-5},
{0x1.09ddb98a01339p+0, -0x1.35c8a49658000p-5},
{0x1.08cabaf52e7dfp+0, -0x1.149e364154000p-5},
{0x1.07b9f2f4e28fbp+0, -0x1.e72c082eb8000p-6},
{0x1.06ab58c358f19p+0, -0x1.a55f152528000p-6},
{0x1.059eea5ecf92cp+0, -0x1.63d62cf818000p-6},
{0x1.04949cdd12c90p+0, -0x1.228fb8caa0000p-6},
{0x1.038c6c6f0ada9p+0, -0x1.c317b20f90000p-7},
{0x1.02865137932a9p+0, -0x1.419355daa0000p-7},
{0x1.0182427ea7348p+0, -0x1.81203c2ec0000p-8},
{0x1.008040614b195p+0, -0x1.0040979240000p-9},
{0x1.fe01ff726fa1ap-1, 0x1.feff384900000p-9},
{0x1.fa11cc261ea74p-1, 0x1.7dc41353d0000p-7},
{0x1.f6310b081992ep-1, 0x1.3cea3c4c28000p-6},
{0x1.f25f63ceeadcdp-1, 0x1.b9fc114890000p-6},
{0x1.ee9c8039113e7p-1, 0x1.1b0d8ce110000p-5},
{0x1.eae8078cbb1abp-1, 0x1.58a5bd001c000p-5},
{0x1.e741aa29d0c9bp-1, 0x1.95c8340d88000p-5},
{0x1.e3a91830a99b5p-1, 0x1.d276aef578000p-5},
{0x1.e01e009609a56p-1, 0x1.07598e598c000p-4},
{0x1.dca01e577bb98p-1, 0x1.253f5e30d2000p-4},
{0x1.d92f20b7c9103p-1, 0x1.42edd8b380000p-4},
{0x1.d5cac66fb5ccep-1, 0x1.606598757c000p-4},
{0x1.d272caa5ede9dp-1, 0x1.7da76356a0000p-4},
{0x1.cf26e3e6b2ccdp-1, 0x1.9ab434e1c6000p-4},
{0x1.cbe6da2a77902p-1, 0x1.b78c7bb0d6000p-4},
{0x1.c8b266d37086dp-1, 0x1.d431332e72000p-4},
{0x1.c5894bd5d5804p-1, 0x1.f0a3171de6000p-4},
{0x1.c26b533bb9f8cp-1, 0x1.067152b914000p-3},
{0x1.bf583eeece73fp-1, 0x1.147858292b000p-3},
{0x1.bc4fd75db96c1p-1, 0x1.2266ecdca3000p-3},
{0x1.b951e0c864a28p-1, 0x1.303d7a6c55000p-3},
{0x1.b65e2c5ef3e2cp-1, 0x1.3dfc33c331000p-3},
{0x1.b374867c9888bp-1, 0x1.4ba366b7a8000p-3},
{0x1.b094b211d304ap-1, 0x1.5933928d1f000p-3},
{0x1.adbe885f2ef7ep-1, 0x1.66acd2418f000p-3},
{0x1.aaf1d31603da2p-1, 0x1.740f8ec669000p-3},
{0x1.a82e63fd358a7p-1, 0x1.815c0f51af000p-3},
{0x1.a5740ef09738bp-1, 0x1.8e92954f68000p-3},
{0x1.a2c2a90ab4b27p-1, 0x1.9bb3602f84000p-3},
{0x1.a01a01393f2d1p-1, 0x1.a8bed1c2c0000p-3},
{0x1.9d79f24db3c1bp-1, 0x1.b5b515c01d000p-3},
{0x1.9ae2505c7b190p-1, 0x1.c2967ccbcc000p-3},
{0x1.9852ef297ce2fp-1, 0x1.cf635d5486000p-3},
{0x1.95cbaeea44b75p-1, 0x1.dc1bd3446c000p-3},
{0x1.934c69de74838p-1, 0x1.e8c01b8cfe000p-3},
{0x1.90d4f2f6752e6p-1, 0x1.f5509c0179000p-3},
{0x1.8e6528effd79dp-1, 0x1.00e6c121fb800p-2},
{0x1.8bfce9fcc007cp-1, 0x1.071b80e93d000p-2},
{0x1.899c0dabec30ep-1, 0x1.0d46b9e867000p-2},
{0x1.87427aa2317fbp-1, 0x1.13687334bd000p-2},
{0x1.84f00acb39a08p-1, 0x1.1980d67234800p-2},
{0x1.82a49e8653e55p-1, 0x1.1f8ffe0cc8000p-2},
{0x1.8060195f40260p-1, 0x1.2595fd7636800p-2},
{0x1.7e22563e0a329p-1, 0x1.2b9300914a800p-2},
{0x1.7beb377dcb5adp-1, 0x1.3187210436000p-2},
{0x1.79baa679725c2p-1, 0x1.377266dec1800p-2},
{0x1.77907f2170657p-1, 0x1.3d54ffbaf3000p-2},
{0x1.756cadbd6130cp-1, 0x1.432eee32fe000p-2},
},
.tab2 = {
{0x1.61000014fb66bp-1, 0x1.e026c91425b3cp-56},
{0x1.63000034db495p-1, 0x1.dbfea48005d41p-55},
{0x1.650000d94d478p-1, 0x1.e7fa786d6a5b7p-55},
{0x1.67000074e6fadp-1, 0x1.1fcea6b54254cp-57},
{0x1.68ffffedf0faep-1, -0x1.c7e274c590efdp-56},
{0x1.6b0000763c5bcp-1, -0x1.ac16848dcda01p-55},
{0x1.6d0001e5cc1f6p-1, 0x1.33f1c9d499311p-55},
{0x1.6efffeb05f63ep-1, -0x1.e80041ae22d53p-56},
{0x1.710000e86978p-1, 0x1.bff6671097952p-56},
{0x1.72ffffc67e912p-1, 0x1.c00e226bd8724p-55},
{0x1.74fffdf81116ap-1, -0x1.e02916ef101d2p-57},
{0x1.770000f679c9p-1, -0x1.7fc71cd549c74p-57},
{0x1.78ffffa7ec835p-1, 0x1.1bec19ef50483p-55},
{0x1.7affffe20c2e6p-1, -0x1.07e1729cc6465p-56},
{0x1.7cfffed3fc9p-1, -0x1.08072087b8b1cp-55},
{0x1.7efffe9261a76p-1, 0x1.dc0286d9df9aep-55},
{0x1.81000049ca3e8p-1, 0x1.97fd251e54c33p-55},
{0x1.8300017932c8fp-1, -0x1.afee9b630f381p-55},
{0x1.850000633739cp-1, 0x1.9bfbf6b6535bcp-55},
{0x1.87000204289c6p-1, -0x1.bbf65f3117b75p-55},
{0x1.88fffebf57904p-1, -0x1.9006ea23dcb57p-55},
{0x1.8b00022bc04dfp-1, -0x1.d00df38e04b0ap-56},
{0x1.8cfffe50c1b8ap-1, -0x1.8007146ff9f05p-55},
{0x1.8effffc918e43p-1, 0x1.3817bd07a7038p-55},
{0x1.910001efa5fc7p-1, 0x1.93e9176dfb403p-55},
{0x1.9300013467bb9p-1, 0x1.f804e4b980276p-56},
{0x1.94fffe6ee076fp-1, -0x1.f7ef0d9ff622ep-55},
{0x1.96fffde3c12d1p-1, -0x1.082aa962638bap-56},
{0x1.98ffff4458a0dp-1, -0x1.7801b9164a8efp-55},
{0x1.9afffdd982e3ep-1, -0x1.740e08a5a9337p-55},
{0x1.9cfffed49fb66p-1, 0x1.fce08c19bep-60},
{0x1.9f00020f19c51p-1, -0x1.a3faa27885b0ap-55},
{0x1.a10001145b006p-1, 0x1.4ff489958da56p-56},
{0x1.a300007bbf6fap-1, 0x1.cbeab8a2b6d18p-55},
{0x1.a500010971d79p-1, 0x1.8fecadd78793p-55},
{0x1.a70001df52e48p-1, -0x1.f41763dd8abdbp-55},
{0x1.a90001c593352p-1, -0x1.ebf0284c27612p-55},
{0x1.ab0002a4f3e4bp-1, -0x1.9fd043cff3f5fp-57},
{0x1.acfffd7ae1ed1p-1, -0x1.23ee7129070b4p-55},
{0x1.aefffee510478p-1, 0x1.a063ee00edea3p-57},
{0x1.b0fffdb650d5bp-1, 0x1.a06c8381f0ab9p-58},
{0x1.b2ffffeaaca57p-1, -0x1.9011e74233c1dp-56},
{0x1.b4fffd995badcp-1, -0x1.9ff1068862a9fp-56},
{0x1.b7000249e659cp-1, 0x1.aff45d0864f3ep-55},
{0x1.b8ffff987164p-1, 0x1.cfe7796c2c3f9p-56},
{0x1.bafffd204cb4fp-1, -0x1.3ff27eef22bc4p-57},
{0x1.bcfffd2415c45p-1, -0x1.cffb7ee3bea21p-57},
{0x1.beffff86309dfp-1, -0x1.14103972e0b5cp-55},
{0x1.c0fffe1b57653p-1, 0x1.bc16494b76a19p-55},
{0x1.c2ffff1fa57e3p-1, -0x1.4feef8d30c6edp-57},
{0x1.c4fffdcbfe424p-1, -0x1.43f68bcec4775p-55},
{0x1.c6fffed54b9f7p-1, 0x1.47ea3f053e0ecp-55},
{0x1.c8fffeb998fd5p-1, 0x1.383068df992f1p-56},
{0x1.cb0002125219ap-1, -0x1.8fd8e64180e04p-57},
{0x1.ccfffdd94469cp-1, 0x1.e7ebe1cc7ea72p-55},
{0x1.cefffeafdc476p-1, 0x1.ebe39ad9f88fep-55},
{0x1.d1000169af82bp-1, 0x1.57d91a8b95a71p-56},
{0x1.d30000d0ff71dp-1, 0x1.9c1906970c7dap-55},
{0x1.d4fffea790fc4p-1, -0x1.80e37c558fe0cp-58},
{0x1.d70002edc87e5p-1, -0x1.f80d64dc10f44p-56},
{0x1.d900021dc82aap-1, -0x1.47c8f94fd5c5cp-56},
{0x1.dafffd86b0283p-1, 0x1.c7f1dc521617ep-55},
{0x1.dd000296c4739p-1, 0x1.8019eb2ffb153p-55},
{0x1.defffe54490f5p-1, 0x1.e00d2c652cc89p-57},
{0x1.e0fffcdabf694p-1, -0x1.f8340202d69d2p-56},
{0x1.e2fffdb52c8ddp-1, 0x1.b00c1ca1b0864p-56},
{0x1.e4ffff24216efp-1, 0x1.2ffa8b094ab51p-56},
{0x1.e6fffe88a5e11p-1, -0x1.7f673b1efbe59p-58},
{0x1.e9000119eff0dp-1, -0x1.4808d5e0bc801p-55},
{0x1.eafffdfa51744p-1, 0x1.80006d54320b5p-56},
{0x1.ed0001a127fa1p-1, -0x1.002f860565c92p-58},
{0x1.ef00007babcc4p-1, -0x1.540445d35e611p-55},
{0x1.f0ffff57a8d02p-1, -0x1.ffb3139ef9105p-59},
{0x1.f30001ee58ac7p-1, 0x1.a81acf2731155p-55},
{0x1.f4ffff5823494p-1, 0x1.a3f41d4d7c743p-55},
{0x1.f6ffffca94c6bp-1, -0x1.202f41c987875p-57},
{0x1.f8fffe1f9c441p-1, 0x1.77dd1f477e74bp-56},
{0x1.fafffd2e0e37ep-1, -0x1.f01199a7ca331p-57},
{0x1.fd0001c77e49ep-1, 0x1.181ee4bceacb1p-56},
{0x1.feffff7e0c331p-1, -0x1.e05370170875ap-57},
{0x1.00ffff465606ep+0, -0x1.a7ead491c0adap-55},
{0x1.02ffff3867a58p+0, -0x1.77f69c3fcb2ep-54},
{0x1.04ffffdfc0d17p+0, 0x1.7bffe34cb945bp-54},
{0x1.0700003cd4d82p+0, 0x1.20083c0e456cbp-55},
{0x1.08ffff9f2cbe8p+0, -0x1.dffdfbe37751ap-57},
{0x1.0b000010cda65p+0, -0x1.13f7faee626ebp-54},
{0x1.0d00001a4d338p+0, 0x1.07dfa79489ff7p-55},
{0x1.0effffadafdfdp+0, -0x1.7040570d66bcp-56},
{0x1.110000bbafd96p+0, 0x1.e80d4846d0b62p-55},
{0x1.12ffffae5f45dp+0, 0x1.dbffa64fd36efp-54},
{0x1.150000dd59ad9p+0, 0x1.a0077701250aep-54},
{0x1.170000f21559ap+0, 0x1.dfdf9e2e3deeep-55},
{0x1.18ffffc275426p+0, 0x1.10030dc3b7273p-54},
{0x1.1b000123d3c59p+0, 0x1.97f7980030188p-54},
{0x1.1cffff8299eb7p+0, -0x1.5f932ab9f8c67p-57},
{0x1.1effff48ad4p+0, 0x1.37fbf9da75bebp-54},
{0x1.210000c8b86a4p+0, 0x1.f806b91fd5b22p-54},
{0x1.2300003854303p+0, 0x1.3ffc2eb9fbf33p-54},
{0x1.24fffffbcf684p+0, 0x1.601e77e2e2e72p-56},
{0x1.26ffff52921d9p+0, 0x1.ffcbb767f0c61p-56},
{0x1.2900014933a3cp+0, -0x1.202ca3c02412bp-56},
{0x1.2b00014556313p+0, -0x1.2808233f21f02p-54},
{0x1.2cfffebfe523bp+0, -0x1.8ff7e384fdcf2p-55},
{0x1.2f0000bb8ad96p+0, -0x1.5ff51503041c5p-55},
{0x1.30ffffb7ae2afp+0, -0x1.10071885e289dp-55},
{0x1.32ffffeac5f7fp+0, -0x1.1ff5d3fb7b715p-54},
{0x1.350000ca66756p+0, 0x1.57f82228b82bdp-54},
{0x1.3700011fbf721p+0, 0x1.000bac40dd5ccp-55},
{0x1.38ffff9592fb9p+0, -0x1.43f9d2db2a751p-54},
{0x1.3b00004ddd242p+0, 0x1.57f6b707638e1p-55},
{0x1.3cffff5b2c957p+0, 0x1.a023a10bf1231p-56},
{0x1.3efffeab0b418p+0, 0x1.87f6d66b152bp-54},
{0x1.410001532aff4p+0, 0x1.7f8375f198524p-57},
{0x1.4300017478b29p+0, 0x1.301e672dc5143p-55},
{0x1.44fffe795b463p+0, 0x1.9ff69b8b2895ap-55},
{0x1.46fffe80475ep+0, -0x1.5c0b19bc2f254p-54},
{0x1.48fffef6fc1e7p+0, 0x1.b4009f23a2a72p-54},
{0x1.4afffe5bea704p+0, -0x1.4ffb7bf0d7d45p-54},
{0x1.4d000171027dep+0, -0x1.9c06471dc6a3dp-54},
{0x1.4f0000ff03ee2p+0, 0x1.77f890b85531cp-54},
{0x1.5100012dc4bd1p+0, 0x1.004657166a436p-57},
{0x1.530001605277ap+0, -0x1.6bfcece233209p-54},
{0x1.54fffecdb704cp+0, -0x1.902720505a1d7p-55},
{0x1.56fffef5f54a9p+0, 0x1.bbfe60ec96412p-54},
{0x1.5900017e61012p+0, 0x1.87ec581afef9p-55},
{0x1.5b00003c93e92p+0, -0x1.f41080abf0ccp-54},
{0x1.5d0001d4919bcp+0, -0x1.8812afb254729p-54},
{0x1.5efffe7b87a89p+0, -0x1.47eb780ed6904p-54},
},
};

#undef N

/* ---- musl src/math/log.c ---- */

/*
 * Double-precision log(x) function.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define T dado_rt__log_data.tab
#define T2 dado_rt__log_data.tab2
#define B dado_rt__log_data.poly1
#define A dado_rt__log_data.poly
#define Ln2hi dado_rt__log_data.ln2hi
#define Ln2lo dado_rt__log_data.ln2lo
#define N (1 << LOG_TABLE_BITS)
#define OFF 0x3fe6000000000000

/* Top 16 bits of a double.  */
static inline uint32_t dado_rt__log__top16(double x)
{
	return dado_rt__asuint64(x) >> 48;
}

double log(double x)
{
	double w, z, r, r2, r3, y, invc, logc, kd, hi, lo;
	uint64_t ix, iz, tmp;
	uint32_t top;
	int k, i;

	ix = dado_rt__asuint64(x);
	top = dado_rt__log__top16(x);
#define LO dado_rt__asuint64(1.0 - 0x1p-4)
#define HI dado_rt__asuint64(1.0 + 0x1.09p-4)
	if (DADO_RT__UNLIKELY(ix - LO < HI - LO)) {
		/* Handle close to 1.0 inputs separately.  */
		/* Fix sign of zero with downward rounding when x==1.  */
		if (DADO_RT__WANT_ROUNDING && DADO_RT__UNLIKELY(ix == dado_rt__asuint64(1.0)))
			return 0;
		r = x - 1.0;
		r2 = r * r;
		r3 = r * r2;
		y = r3 *
		    (B[1] + r * B[2] + r2 * B[3] +
		     r3 * (B[4] + r * B[5] + r2 * B[6] +
			   r3 * (B[7] + r * B[8] + r2 * B[9] + r3 * B[10])));
		/* Worst-case error is around 0.507 ULP.  */
		w = r * 0x1p27;
		double rhi = r + w - w;
		double rlo = r - rhi;
		w = rhi * rhi * B[0]; /* B[0] == -0.5.  */
		hi = r + w;
		lo = r - hi + w;
		lo += B[0] * rlo * (rhi + r);
		y += lo;
		y += hi;
		return dado_rt__eval_as_double(y);
	}
	if (DADO_RT__UNLIKELY(top - 0x0010 >= 0x7ff0 - 0x0010)) {
		/* x < 0x1p-1022 or inf or nan.  */
		if (ix * 2 == 0)
			return dado_rt__math_divzero(1);
		if (ix == dado_rt__asuint64(__builtin_inf())) /* log(inf) == inf.  */
			return x;
		if ((top & 0x8000) || (top & 0x7ff0) == 0x7ff0)
			return dado_rt__math_invalid(x);
		/* x is subnormal, normalize it.  */
		ix = dado_rt__asuint64(x * 0x1p52);
		ix -= 52ULL << 52;
	}

	/* x = 2^k z; where z is in range [OFF,2*OFF) and exact.
	   The range is split into N subintervals.
	   The ith subinterval contains z and c is near its center.  */
	tmp = ix - OFF;
	i = (tmp >> (52 - LOG_TABLE_BITS)) % N;
	k = (int64_t)tmp >> 52; /* arithmetic shift */
	iz = ix - (tmp & 0xfffULL << 52);
	invc = T[i].invc;
	logc = T[i].logc;
	z = dado_rt__asdouble(iz);

	/* log(x) = log1p(z/c-1) + log(c) + k*Ln2.  */
	/* r ~= z/c - 1, |r| < 1/(2*N).  */
	/* rounding error: 0x1p-55/N + 0x1p-66.  */
	r = (z - T2[i].chi - T2[i].clo) * invc;
	kd = (double)k;

	/* hi + lo = r + log(c) + k*Ln2.  */
	w = kd * Ln2hi + logc;
	hi = w + r;
	lo = w - hi + r + kd * Ln2lo;

	/* log(x) = lo + (log1p(r) - r) + hi.  */
	r2 = r * r; /* rounding error: 0x1p-54/N^2.  */
	/* Worst case error if |y| > 0x1p-5:
	   0.5 + 4.13/N + abs-poly-error*2^57 ULP (+ 0.002 ULP without fma)
	   Worst case error if |y| > 0x1p-4:
	   0.5 + 2.06/N + abs-poly-error*2^56 ULP (+ 0.001 ULP without fma).  */
	y = lo + r2 * A[0] +
	    r * r2 * (A[1] + r * A[2] + r2 * (A[3] + r * A[4])) + hi;
	return dado_rt__eval_as_double(y);
}

#undef T
#undef T2
#undef B
#undef A
#undef Ln2hi
#undef Ln2lo
#undef N
#undef OFF
#undef LO
#undef HI

/* ---- musl src/math/log2_data.h ---- */

/*
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define LOG2_TABLE_BITS 6
#define LOG2_POLY_ORDER 7
#define LOG2_POLY1_ORDER 11
struct dado_rt__log2_table {
	double invln2hi;
	double invln2lo;
	double poly[LOG2_POLY_ORDER - 1];
	double poly1[LOG2_POLY1_ORDER - 1];
	struct {
		double invc, logc;
	} tab[1 << LOG2_TABLE_BITS];
	struct {
		double chi, clo;
	} tab2[1 << LOG2_TABLE_BITS];
};


/* ---- musl src/math/log2_data.c ---- */

/*
 * Data for log2.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define N (1 << LOG2_TABLE_BITS)

static const struct dado_rt__log2_table dado_rt__log2_data = {
// First coefficient: 0x1.71547652b82fe1777d0ffda0d24p0
.invln2hi = 0x1.7154765200000p+0,
.invln2lo = 0x1.705fc2eefa200p-33,
.poly1 = {
// relative error: 0x1.2fad8188p-63
// in -0x1.5b51p-5 0x1.6ab2p-5
-0x1.71547652b82fep-1,
0x1.ec709dc3a03f7p-2,
-0x1.71547652b7c3fp-2,
0x1.2776c50f05be4p-2,
-0x1.ec709dd768fe5p-3,
0x1.a61761ec4e736p-3,
-0x1.7153fbc64a79bp-3,
0x1.484d154f01b4ap-3,
-0x1.289e4a72c383cp-3,
0x1.0b32f285aee66p-3,
},
.poly = {
// relative error: 0x1.a72c2bf8p-58
// abs error: 0x1.67a552c8p-66
// in -0x1.f45p-8 0x1.f45p-8
-0x1.71547652b8339p-1,
0x1.ec709dc3a04bep-2,
-0x1.7154764702ffbp-2,
0x1.2776c50034c48p-2,
-0x1.ec7b328ea92bcp-3,
0x1.a6225e117f92ep-3,
},
/* Algorithm:

	x = 2^k z
	log2(x) = k + log2(c) + log2(z/c)
	log2(z/c) = poly(z/c - 1)

where z is in [1.6p-1; 1.6p0] which is split into N subintervals and z falls
into the ith one, then table entries are computed as

	tab[i].invc = 1/c
	tab[i].logc = (double)log2(c)
	tab2[i].chi = (double)c
	tab2[i].clo = (double)(c - (double)c)

where c is near the center of the subinterval and is chosen by trying +-2^29
floating point invc candidates around 1/center and selecting one for which

	1) the rounding error in 0x1.8p10 + logc is 0,
	2) the rounding error in z - chi - clo is < 0x1p-64 and
	3) the rounding error in (double)log2(c) is minimized (< 0x1p-68).

Note: 1) ensures that k + logc can be computed without rounding error, 2)
ensures that z/c - 1 can be computed as (z - chi - clo)*invc with close to a
single rounding error when there is no fast fma for z*invc - 1, 3) ensures
that logc + poly(z/c - 1) has small error, however near x == 1 when
|log2(x)| < 0x1p-4, this is not enough so that is special cased.  */
.tab = {
{0x1.724286bb1acf8p+0, -0x1.1095feecdb000p-1},
{0x1.6e1f766d2cca1p+0, -0x1.08494bd76d000p-1},
{0x1.6a13d0e30d48ap+0, -0x1.00143aee8f800p-1},
{0x1.661ec32d06c85p+0, -0x1.efec5360b4000p-2},
{0x1.623fa951198f8p+0, -0x1.dfdd91ab7e000p-2},
{0x1.5e75ba4cf026cp+0, -0x1.cffae0cc79000p-2},
{0x1.5ac055a214fb8p+0, -0x1.c043811fda000p-2},
{0x1.571ed0f166e1ep+0, -0x1.b0b67323ae000p-2},
{0x1.53909590bf835p+0, -0x1.a152f5a2db000p-2},
{0x1.5014fed61adddp+0, -0x1.9217f5af86000p-2},
{0x1.4cab88e487bd0p+0, -0x1.8304db0719000p-2},
{0x1.49539b4334feep+0, -0x1.74189f9a9e000p-2},
{0x1.460cbdfafd569p+0, -0x1.6552bb5199000p-2},
{0x1.42d664ee4b953p+0, -0x1.56b23a29b1000p-2},
{0x1.3fb01111dd8a6p+0, -0x1.483650f5fa000p-2},
{0x1.3c995b70c5836p+0, -0x1.39de937f6a000p-2},
{0x1.3991c4ab6fd4ap+0, -0x1.2baa1538d6000p-2},
{0x1.3698e0ce099b5p+0, -0x1.1d98340ca4000p-2},
{0x1.33ae48213e7b2p+0, -0x1.0fa853a40e000p-2},
{0x1.30d191985bdb1p+0, -0x1.01d9c32e73000p-2},
{0x1.2e025cab271d7p+0, -0x1.e857da2fa6000p-3},
{0x1.2b404cf13cd82p+0, -0x1.cd3c8633d8000p-3},
{0x1.288b02c7ccb50p+0, -0x1.b26034c14a000p-3},
{0x1.25e2263944de5p+0, -0x1.97c1c2f4fe000p-3},
{0x1.234563d8615b1p+0, -0x1.7d6023f800000p-3},
{0x1.20b46e33eaf38p+0, -0x1.633a71a05e000p-3},
{0x1.1e2eefdcda3ddp+0, -0x1.494f5e9570000p-3},
{0x1.1bb4a580b3930p+0, -0x1.2f9e424e0a000p-3},
{0x1.19453847f2200p+0, -0x1.162595afdc000p-3},
{0x1.16e06c0d5d73cp+0, -0x1.f9c9a75bd8000p-4},
{0x1.1485f47b7e4c2p+0, -0x1.c7b575bf9c000p-4},
{0x1.12358ad0085d1p+0, -0x1.960c60ff48000p-4},
{0x1.0fef00f532227p+0, -0x1.64ce247b60000p-4},
{0x1.0db2077d03a8fp+0, -0x1.33f78b2014000p-4},
{0x1.0b7e6d65980d9p+0, -0x1.0387d1a42c000p-4},
{0x1.0953efe7b408dp+0, -0x1.a6f9208b50000p-5},
{0x1.07325cac53b83p+0, -0x1.47a954f770000p-5},
{0x1.05197e40d1b5cp+0, -0x1.d23a8c50c0000p-6},
{0x1.03091c1208ea2p+0, -0x1.16a2629780000p-6},
{0x1.0101025b37e21p+0, -0x1.720f8d8e80000p-8},
{0x1.fc07ef9caa76bp-1, 0x1.6fe53b1500000p-7},
{0x1.f4465d3f6f184p-1, 0x1.11ccce10f8000p-5},
{0x1.ecc079f84107fp-1, 0x1.c4dfc8c8b8000p-5},
{0x1.e573a99975ae8p-1, 0x1.3aa321e574000p-4},
{0x1.de5d6f0bd3de6p-1, 0x1.918a0d08b8000p-4},
{0x1.d77b681ff38b3p-1, 0x1.e72e9da044000p-4},
{0x1.d0cb5724de943p-1, 0x1.1dcd2507f6000p-3},
{0x1.ca4b2dc0e7563p-1, 0x1.476ab03dea000p-3},
{0x1.c3f8ee8d6cb51p-1, 0x1.7074377e22000p-3},
{0x1.bdd2b4f020c4cp-1, 0x1.98ede8ba94000p-3},
{0x1.b7d6c006015cap-1, 0x1.c0db86ad2e000p-3},
{0x1.b20366e2e338fp-1, 0x1.e840aafcee000p-3},
{0x1.ac57026295039p-1, 0x1.0790ab4678000p-2},
{0x1.a6d01bc2731ddp-1, 0x1.1ac056801c000p-2},
{0x1.a16d3bc3ff18bp-1, 0x1.2db11d4fee000p-2},
{0x1.9c2d14967feadp-1, 0x1.406464ec58000p-2},
{0x1.970e4f47c9902p-1, 0x1.52dbe093af000p-2},
{0x1.920fb3982bcf2p-1, 0x1.651902050d000p-2},
{0x1.8d30187f759f1p-1, 0x1.771d2cdeaf000p-2},
{0x1.886e5ebb9f66dp-1, 0x1.88e9c857d9000p-2},
{0x1.83c97b658b994p-1, 0x1.9a80155e16000p-2},
{0x1.7f405ffc61022p-1, 0x1.abe186ed3d000p-2},
{0x1.7ad22181415cap-1, 0x1.bd0f2aea0e000p-2},
{0x1.767dcf99eff8cp-1, 0x1.ce0a43dbf4000p-2},
},
.tab2 = {
{0x1.6200012b90a8ep-1, 0x1.904ab0644b605p-55},
{0x1.66000045734a6p-1, 0x1.1ff9bea62f7a9p-57},
{0x1.69fffc325f2c5p-1, 0x1.27ecfcb3c90bap-55},
{0x1.6e00038b95a04p-1, 0x1.8ff8856739326p-55},
{0x1.71fffe09994e3p-1, 0x1.afd40275f82b1p-55},
{0x1.7600015590e1p-1, -0x1.2fd75b4238341p-56},
{0x1.7a00012655bd5p-1, 0x1.808e67c242b76p-56},
{0x1.7e0003259e9a6p-1, -0x1.208e426f622b7p-57},
{0x1.81fffedb4b2d2p-1, -0x1.402461ea5c92fp-55},
{0x1.860002dfafcc3p-1, 0x1.df7f4a2f29a1fp-57},
{0x1.89ffff78c6b5p-1, -0x1.e0453094995fdp-55},
{0x1.8e00039671566p-1, -0x1.a04f3bec77b45p-55},
{0x1.91fffe2bf1745p-1, -0x1.7fa34400e203cp-56},
{0x1.95fffcc5c9fd1p-1, -0x1.6ff8005a0695dp-56},
{0x1.9a0003bba4767p-1, 0x1.0f8c4c4ec7e03p-56},
{0x1.9dfffe7b92da5p-1, 0x1.e7fd9478c4602p-55},
{0x1.a1fffd72efdafp-1, -0x1.a0c554dcdae7ep-57},
{0x1.a5fffde04ff95p-1, 0x1.67da98ce9b26bp-55},
{0x1.a9fffca5e8d2bp-1, -0x1.284c9b54c13dep-55},
{0x1.adfffddad03eap-1, 0x1.812c8ea602e3cp-58},
{0x1.b1ffff10d3d4dp-1, -0x1.efaddad27789cp-55},
{0x1.b5fffce21165ap-1, 0x1.3cb1719c61237p-58},
{0x1.b9fffd950e674p-1, 0x1.3f7d94194cep-56},
{0x1.be000139ca8afp-1, 0x1.50ac4215d9bcp-56},
{0x1.c20005b46df99p-1, 0x1.beea653e9c1c9p-57},
{0x1.c600040b9f7aep-1, -0x1.c079f274a70d6p-56},
{0x1.ca0006255fd8ap-1, -0x1.a0b4076e84c1fp-56},
{0x1.cdfffd94c095dp-1, 0x1.8f933f99ab5d7p-55},
{0x1.d1ffff975d6cfp-1, -0x1.82c08665fe1bep-58},
{0x1.d5fffa2561c93p-1, -0x1.b04289bd295f3p-56},
{0x1.d9fff9d228b0cp-1, 0x1.70251340fa236p-55},
{0x1.de00065bc7e16p-1, -0x1.5011e16a4d80cp-56},
{0x1.e200002f64791p-1, 0x1.9802f09ef62ep-55},
{0x1.e600057d7a6d8p-1, -0x1.e0b75580cf7fap-56},
{0x1.ea00027edc00cp-1, -0x1.c848309459811p-55},
{0x1.ee0006cf5cb7cp-1, -0x1.f8027951576f4p-55},
{0x1.f2000782b7dccp-1, -0x1.f81d97274538fp-55},
{0x1.f6000260c450ap-1, -0x1.071002727ffdcp-59},
{0x1.f9fffe88cd533p-1, -0x1.81bdce1fda8bp-58},
{0x1.fdfffd50f8689p-1, 0x1.7f91acb918e6ep-55},
{0x1.0200004292367p+0, 0x1.b7ff365324681p-54},
{0x1.05fffe3e3d668p+0, 0x1.6fa08ddae957bp-55},
{0x1.0a0000a85a757p+0, -0x1.7e2de80d3fb91p-58},
{0x1.0e0001a5f3fccp+0, -0x1.1823305c5f014p-54},
{0x1.11ffff8afbaf5p+0, -0x1.bfabb6680bac2p-55},
{0x1.15fffe54d91adp+0, -0x1.d7f121737e7efp-54},
{0x1.1a00011ac36e1p+0, 0x1.c000a0516f5ffp-54},
{0x1.1e00019c84248p+0, -0x1.082fbe4da5dap-54},
{0x1.220000ffe5e6ep+0, -0x1.8fdd04c9cfb43p-55},
{0x1.26000269fd891p+0, 0x1.cfe2a7994d182p-55},
{0x1.2a00029a6e6dap+0, -0x1.00273715e8bc5p-56},
{0x1.2dfffe0293e39p+0, 0x1.b7c39dab2a6f9p-54},
{0x1.31ffff7dcf082p+0, 0x1.df1336edc5254p-56},
{0x1.35ffff05a8b6p+0, -0x1.e03564ccd31ebp-54},
{0x1.3a0002e0eaeccp+0, 0x1.5f0e74bd3a477p-56},
{0x1.3e000043bb236p+0, 0x1.c7dcb149d8833p-54},
{0x1.4200002d187ffp+0, 0x1.e08afcf2d3d28p-56},
{0x1.460000d387cb1p+0, 0x1.20837856599a6p-55},
{0x1.4a00004569f89p+0, -0x1.9fa5c904fbcd2p-55},
{0x1.4e000043543f3p+0, -0x1.81125ed175329p-56},
{0x1.51fffcc027f0fp+0, 0x1.883d8847754dcp-54},
{0x1.55ffffd87b36fp+0, -0x1.709e731d02807p-55},
{0x1.59ffff21df7bap+0, 0x1.7f79f68727b02p-55},
{0x1.5dfffebfc3481p+0, -0x1.180902e30e93ep-54},
},
};

#undef N

/* ---- musl src/math/log2.c ---- */

/*
 * Double-precision log2(x) function.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define T dado_rt__log2_data.tab
#define T2 dado_rt__log2_data.tab2
#define B dado_rt__log2_data.poly1
#define A dado_rt__log2_data.poly
#define InvLn2hi dado_rt__log2_data.invln2hi
#define InvLn2lo dado_rt__log2_data.invln2lo
#define N (1 << LOG2_TABLE_BITS)
#define OFF 0x3fe6000000000000

/* Top 16 bits of a double.  */
static inline uint32_t dado_rt__log2__top16(double x)
{
	return dado_rt__asuint64(x) >> 48;
}

double log2(double x)
{
	double z, r, r2, r4, y, invc, logc, kd, hi, lo, t1, t2, t3, p;
	uint64_t ix, iz, tmp;
	uint32_t top;
	int k, i;

	ix = dado_rt__asuint64(x);
	top = dado_rt__log2__top16(x);
#define LO dado_rt__asuint64(1.0 - 0x1.5b51p-5)
#define HI dado_rt__asuint64(1.0 + 0x1.6ab2p-5)
	if (DADO_RT__UNLIKELY(ix - LO < HI - LO)) {
		/* Handle close to 1.0 inputs separately.  */
		/* Fix sign of zero with downward rounding when x==1.  */
		if (DADO_RT__WANT_ROUNDING && DADO_RT__UNLIKELY(ix == dado_rt__asuint64(1.0)))
			return 0;
		r = x - 1.0;
		double rhi, rlo;
		rhi = dado_rt__asdouble(dado_rt__asuint64(r) & -1ULL << 32);
		rlo = r - rhi;
		hi = rhi * InvLn2hi;
		lo = rlo * InvLn2hi + r * InvLn2lo;
		r2 = r * r; /* rounding error: 0x1p-62.  */
		r4 = r2 * r2;
		/* Worst-case error is less than 0.54 ULP (0.55 ULP without fma).  */
		p = r2 * (B[0] + r * B[1]);
		y = hi + p;
		lo += hi - y + p;
		lo += r4 * (B[2] + r * B[3] + r2 * (B[4] + r * B[5]) +
			    r4 * (B[6] + r * B[7] + r2 * (B[8] + r * B[9])));
		y += lo;
		return dado_rt__eval_as_double(y);
	}
	if (DADO_RT__UNLIKELY(top - 0x0010 >= 0x7ff0 - 0x0010)) {
		/* x < 0x1p-1022 or inf or nan.  */
		if (ix * 2 == 0)
			return dado_rt__math_divzero(1);
		if (ix == dado_rt__asuint64(__builtin_inf())) /* log(inf) == inf.  */
			return x;
		if ((top & 0x8000) || (top & 0x7ff0) == 0x7ff0)
			return dado_rt__math_invalid(x);
		/* x is subnormal, normalize it.  */
		ix = dado_rt__asuint64(x * 0x1p52);
		ix -= 52ULL << 52;
	}

	/* x = 2^k z; where z is in range [OFF,2*OFF) and exact.
	   The range is split into N subintervals.
	   The ith subinterval contains z and c is near its center.  */
	tmp = ix - OFF;
	i = (tmp >> (52 - LOG2_TABLE_BITS)) % N;
	k = (int64_t)tmp >> 52; /* arithmetic shift */
	iz = ix - (tmp & 0xfffULL << 52);
	invc = T[i].invc;
	logc = T[i].logc;
	z = dado_rt__asdouble(iz);
	kd = (double)k;

	/* log2(x) = log2(z/c) + log2(c) + k.  */
	/* r ~= z/c - 1, |r| < 1/(2*N).  */
	double rhi, rlo;
	/* rounding error: 0x1p-55/N + 0x1p-65.  */
	r = (z - T2[i].chi - T2[i].clo) * invc;
	rhi = dado_rt__asdouble(dado_rt__asuint64(r) & -1ULL << 32);
	rlo = r - rhi;
	t1 = rhi * InvLn2hi;
	t2 = rlo * InvLn2hi + r * InvLn2lo;

	/* hi + lo = r/ln2 + log2(c) + k.  */
	t3 = kd + logc;
	hi = t3 + t1;
	lo = t3 - hi + t1 + t2;

	/* log2(r+1) = r/ln2 + r^2*poly(r).  */
	/* Evaluation is optimized assuming superscalar pipelined execution.  */
	r2 = r * r; /* rounding error: 0x1p-54/N^2.  */
	r4 = r2 * r2;
	/* Worst-case error if |y| > 0x1p-4: 0.547 ULP (0.550 ULP without fma).
	   ~ 0.5 + 2/N/ln2 + abs-poly-error*0x1p56 ULP (+ 0.003 ULP without fma).  */
	p = A[0] + r * A[1] + r2 * (A[2] + r * A[3]) + r4 * (A[4] + r * A[5]);
	y = lo + r2 * p + hi;
	return dado_rt__eval_as_double(y);
}

#undef T
#undef T2
#undef B
#undef A
#undef InvLn2hi
#undef InvLn2lo
#undef N
#undef OFF
#undef LO
#undef HI

/* ---- musl src/math/log10.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/e_log10.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * Return the base 10 logarithm of x.  See log.c for most comments.
 *
 * Reduce x to 2^k (1+f) and calculate r = log(1+f) - f + f*f/2
 * as in log.c, then combine and scale in extra precision:
 *    log10(x) = (f - f*f/2 + r)/log(10) + k*log10(2)
 */

static const double
dado_rt__log10__ivln10hi  = 4.34294481878168880939e-01, /* 0x3fdbcb7b, 0x15200000 */
dado_rt__log10__ivln10lo  = 2.50829467116452752298e-11, /* 0x3dbb9438, 0xca9aadd5 */
dado_rt__log10__log10_2hi = 3.01029995663611771306e-01, /* 0x3FD34413, 0x509F6000 */
dado_rt__log10__log10_2lo = 3.69423907715893078616e-13, /* 0x3D59FEF3, 0x11F12B36 */
dado_rt__log10__Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
dado_rt__log10__Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
dado_rt__log10__Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
dado_rt__log10__Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
dado_rt__log10__Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
dado_rt__log10__Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
dado_rt__log10__Lg7 = 1.479819860511658591e-01;  /* 3FC2F112 DF3E5244 */

double log10(double x)
{
	union {double f; uint64_t i;} u = {x};
	double hfsq,f,s,z,R,w,t1,t2,dk,y,hi,lo,val_hi,val_lo;
	uint32_t hx;
	int k;

	hx = u.i>>32;
	k = 0;
	if (hx < 0x00100000 || hx>>31) {
		if (u.i<<1 == 0)
			return -1/(x*x);  /* log(+-0)=-inf */
		if (hx>>31)
			return (x-x)/0.0; /* log(-#) = NaN */
		/* subnormal number, scale x up */
		k -= 54;
		x *= 0x1p54;
		u.f = x;
		hx = u.i>>32;
	} else if (hx >= 0x7ff00000) {
		return x;
	} else if (hx == 0x3ff00000 && u.i<<32 == 0)
		return 0;

	/* reduce x into [sqrt(2)/2, sqrt(2)] */
	hx += 0x3ff00000 - 0x3fe6a09e;
	k += (int)(hx>>20) - 0x3ff;
	hx = (hx&0x000fffff) + 0x3fe6a09e;
	u.i = (uint64_t)hx<<32 | (u.i&0xffffffff);
	x = u.f;

	f = x - 1.0;
	hfsq = 0.5*f*f;
	s = f/(2.0+f);
	z = s*s;
	w = z*z;
	t1 = w*(dado_rt__log10__Lg2+w*(dado_rt__log10__Lg4+w*dado_rt__log10__Lg6));
	t2 = z*(dado_rt__log10__Lg1+w*(dado_rt__log10__Lg3+w*(dado_rt__log10__Lg5+w*dado_rt__log10__Lg7)));
	R = t2 + t1;

	/* See log2.c for details. */
	/* hi+lo = f - hfsq + s*(hfsq+R) ~ log(1+f) */
	hi = f - hfsq;
	u.f = hi;
	u.i &= (uint64_t)-1<<32;
	hi = u.f;
	lo = f - hi - hfsq + s*(hfsq+R);

	/* val_hi+val_lo ~ log10(1+f) + k*log10(2) */
	val_hi = hi*dado_rt__log10__ivln10hi;
	dk = k;
	y = dk*dado_rt__log10__log10_2hi;
	val_lo = dk*dado_rt__log10__log10_2lo + (lo+hi)*dado_rt__log10__ivln10lo + lo*dado_rt__log10__ivln10hi;

	/*
	 * Extra precision in for adding y is not strictly needed
	 * since there is no very large cancellation near x = sqrt(2) or
	 * x = 1/sqrt(2), but we do it anyway since it costs little on CPUs
	 * with some parallelism and it reduces the error for many args.
	 */
	w = y + val_hi;
	val_lo += (y - w) + val_hi;
	val_hi = w;

	return val_lo + val_hi;
}


/* ---- musl src/math/pow_data.h ---- */

/*
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define POW_LOG_TABLE_BITS 7
#define POW_LOG_POLY_ORDER 8
struct dado_rt__pow_log_table {
	double ln2hi;
	double ln2lo;
	double poly[POW_LOG_POLY_ORDER - 1]; /* First coefficient is 1.  */
	/* Note: the pad field is unused, but allows slightly faster indexing.  */
	struct {
		double invc, pad, logc, logctail;
	} tab[1 << POW_LOG_TABLE_BITS];
};


/* ---- musl src/math/pow_data.c ---- */

/*
 * Data for the log part of pow.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define N (1 << POW_LOG_TABLE_BITS)

static const struct dado_rt__pow_log_table dado_rt__pow_log_data = {
.ln2hi = 0x1.62e42fefa3800p-1,
.ln2lo = 0x1.ef35793c76730p-45,
.poly = {
// relative error: 0x1.11922ap-70
// in -0x1.6bp-8 0x1.6bp-8
// Coefficients are scaled to match the scaling during evaluation.
-0x1p-1,
0x1.555555555556p-2 * -2,
-0x1.0000000000006p-2 * -2,
0x1.999999959554ep-3 * 4,
-0x1.555555529a47ap-3 * 4,
0x1.2495b9b4845e9p-3 * -8,
-0x1.0002b8b263fc3p-3 * -8,
},
/* Algorithm:

	x = 2^k z
	log(x) = k ln2 + log(c) + log(z/c)
	log(z/c) = poly(z/c - 1)

where z is in [0x1.69555p-1; 0x1.69555p0] which is split into N subintervals
and z falls into the ith one, then table entries are computed as

	tab[i].invc = 1/c
	tab[i].logc = round(0x1p43*log(c))/0x1p43
	tab[i].logctail = (double)(log(c) - logc)

where c is chosen near the center of the subinterval such that 1/c has only a
few precision bits so z/c - 1 is exactly representible as double:

	1/c = center < 1 ? round(N/center)/N : round(2*N/center)/N/2

Note: |z/c - 1| < 1/N for the chosen c, |log(c) - logc - logctail| < 0x1p-97,
the last few bits of logc are rounded away so k*ln2hi + logc has no rounding
error and the interval for z is selected such that near x == 1, where log(x)
is tiny, large cancellation error is avoided in logc + poly(z/c - 1).  */
.tab = {
#define A(a, b, c) {a, 0, b, c},
A(0x1.6a00000000000p+0, -0x1.62c82f2b9c800p-2, 0x1.ab42428375680p-48)
A(0x1.6800000000000p+0, -0x1.5d1bdbf580800p-2, -0x1.ca508d8e0f720p-46)
A(0x1.6600000000000p+0, -0x1.5767717455800p-2, -0x1.362a4d5b6506dp-45)
A(0x1.6400000000000p+0, -0x1.51aad872df800p-2, -0x1.684e49eb067d5p-49)
A(0x1.6200000000000p+0, -0x1.4be5f95777800p-2, -0x1.41b6993293ee0p-47)
A(0x1.6000000000000p+0, -0x1.4618bc21c6000p-2, 0x1.3d82f484c84ccp-46)
A(0x1.5e00000000000p+0, -0x1.404308686a800p-2, 0x1.c42f3ed820b3ap-50)
A(0x1.5c00000000000p+0, -0x1.3a64c55694800p-2, 0x1.0b1c686519460p-45)
A(0x1.5a00000000000p+0, -0x1.347dd9a988000p-2, 0x1.5594dd4c58092p-45)
A(0x1.5800000000000p+0, -0x1.2e8e2bae12000p-2, 0x1.67b1e99b72bd8p-45)
A(0x1.5600000000000p+0, -0x1.2895a13de8800p-2, 0x1.5ca14b6cfb03fp-46)
A(0x1.5600000000000p+0, -0x1.2895a13de8800p-2, 0x1.5ca14b6cfb03fp-46)
A(0x1.5400000000000p+0, -0x1.22941fbcf7800p-2, -0x1.65a242853da76p-46)
A(0x1.5200000000000p+0, -0x1.1c898c1699800p-2, -0x1.fafbc68e75404p-46)
A(0x1.5000000000000p+0, -0x1.1675cababa800p-2, 0x1.f1fc63382a8f0p-46)
A(0x1.4e00000000000p+0, -0x1.1058bf9ae4800p-2, -0x1.6a8c4fd055a66p-45)
A(0x1.4c00000000000p+0, -0x1.0a324e2739000p-2, -0x1.c6bee7ef4030ep-47)
A(0x1.4a00000000000p+0, -0x1.0402594b4d000p-2, -0x1.036b89ef42d7fp-48)
A(0x1.4a00000000000p+0, -0x1.0402594b4d000p-2, -0x1.036b89ef42d7fp-48)
A(0x1.4800000000000p+0, -0x1.fb9186d5e4000p-3, 0x1.d572aab993c87p-47)
A(0x1.4600000000000p+0, -0x1.ef0adcbdc6000p-3, 0x1.b26b79c86af24p-45)
A(0x1.4400000000000p+0, -0x1.e27076e2af000p-3, -0x1.72f4f543fff10p-46)
A(0x1.4200000000000p+0, -0x1.d5c216b4fc000p-3, 0x1.1ba91bbca681bp-45)
A(0x1.4000000000000p+0, -0x1.c8ff7c79aa000p-3, 0x1.7794f689f8434p-45)
A(0x1.4000000000000p+0, -0x1.c8ff7c79aa000p-3, 0x1.7794f689f8434p-45)
A(0x1.3e00000000000p+0, -0x1.bc286742d9000p-3, 0x1.94eb0318bb78fp-46)
A(0x1.3c00000000000p+0, -0x1.af3c94e80c000p-3, 0x1.a4e633fcd9066p-52)
A(0x1.3a00000000000p+0, -0x1.a23bc1fe2b000p-3, -0x1.58c64dc46c1eap-45)
A(0x1.3a00000000000p+0, -0x1.a23bc1fe2b000p-3, -0x1.58c64dc46c1eap-45)
A(0x1.3800000000000p+0, -0x1.9525a9cf45000p-3, -0x1.ad1d904c1d4e3p-45)
A(0x1.3600000000000p+0, -0x1.87fa06520d000p-3, 0x1.bbdbf7fdbfa09p-45)
A(0x1.3400000000000p+0, -0x1.7ab890210e000p-3, 0x1.bdb9072534a58p-45)
A(0x1.3400000000000p+0, -0x1.7ab890210e000p-3, 0x1.bdb9072534a58p-45)
A(0x1.3200000000000p+0, -0x1.6d60fe719d000p-3, -0x1.0e46aa3b2e266p-46)
A(0x1.3000000000000p+0, -0x1.5ff3070a79000p-3, -0x1.e9e439f105039p-46)
A(0x1.3000000000000p+0, -0x1.5ff3070a79000p-3, -0x1.e9e439f105039p-46)
A(0x1.2e00000000000p+0, -0x1.526e5e3a1b000p-3, -0x1.0de8b90075b8fp-45)
A(0x1.2c00000000000p+0, -0x1.44d2b6ccb8000p-3, 0x1.70cc16135783cp-46)
A(0x1.2c00000000000p+0, -0x1.44d2b6ccb8000p-3, 0x1.70cc16135783cp-46)
A(0x1.2a00000000000p+0, -0x1.371fc201e9000p-3, 0x1.178864d27543ap-48)
A(0x1.2800000000000p+0, -0x1.29552f81ff000p-3, -0x1.48d301771c408p-45)
A(0x1.2600000000000p+0, -0x1.1b72ad52f6000p-3, -0x1.e80a41811a396p-45)
A(0x1.2600000000000p+0, -0x1.1b72ad52f6000p-3, -0x1.e80a41811a396p-45)
A(0x1.2400000000000p+0, -0x1.0d77e7cd09000p-3, 0x1.a699688e85bf4p-47)
A(0x1.2400000000000p+0, -0x1.0d77e7cd09000p-3, 0x1.a699688e85bf4p-47)
A(0x1.2200000000000p+0, -0x1.fec9131dbe000p-4, -0x1.575545ca333f2p-45)
A(0x1.2000000000000p+0, -0x1.e27076e2b0000p-4, 0x1.a342c2af0003cp-45)
A(0x1.2000000000000p+0, -0x1.e27076e2b0000p-4, 0x1.a342c2af0003cp-45)
A(0x1.1e00000000000p+0, -0x1.c5e548f5bc000p-4, -0x1.d0c57585fbe06p-46)
A(0x1.1c00000000000p+0, -0x1.a926d3a4ae000p-4, 0x1.53935e85baac8p-45)
A(0x1.1c00000000000p+0, -0x1.a926d3a4ae000p-4, 0x1.53935e85baac8p-45)
A(0x1.1a00000000000p+0, -0x1.8c345d631a000p-4, 0x1.37c294d2f5668p-46)
A(0x1.1a00000000000p+0, -0x1.8c345d631a000p-4, 0x1.37c294d2f5668p-46)
A(0x1.1800000000000p+0, -0x1.6f0d28ae56000p-4, -0x1.69737c93373dap-45)
A(0x1.1600000000000p+0, -0x1.51b073f062000p-4, 0x1.f025b61c65e57p-46)
A(0x1.1600000000000p+0, -0x1.51b073f062000p-4, 0x1.f025b61c65e57p-46)
A(0x1.1400000000000p+0, -0x1.341d7961be000p-4, 0x1.c5edaccf913dfp-45)
A(0x1.1400000000000p+0, -0x1.341d7961be000p-4, 0x1.c5edaccf913dfp-45)
A(0x1.1200000000000p+0, -0x1.16536eea38000p-4, 0x1.47c5e768fa309p-46)
A(0x1.1000000000000p+0, -0x1.f0a30c0118000p-5, 0x1.d599e83368e91p-45)
A(0x1.1000000000000p+0, -0x1.f0a30c0118000p-5, 0x1.d599e83368e91p-45)
A(0x1.0e00000000000p+0, -0x1.b42dd71198000p-5, 0x1.c827ae5d6704cp-46)
A(0x1.0e00000000000p+0, -0x1.b42dd71198000p-5, 0x1.c827ae5d6704cp-46)
A(0x1.0c00000000000p+0, -0x1.77458f632c000p-5, -0x1.cfc4634f2a1eep-45)
A(0x1.0c00000000000p+0, -0x1.77458f632c000p-5, -0x1.cfc4634f2a1eep-45)
A(0x1.0a00000000000p+0, -0x1.39e87b9fec000p-5, 0x1.502b7f526feaap-48)
A(0x1.0a00000000000p+0, -0x1.39e87b9fec000p-5, 0x1.502b7f526feaap-48)
A(0x1.0800000000000p+0, -0x1.f829b0e780000p-6, -0x1.980267c7e09e4p-45)
A(0x1.0800000000000p+0, -0x1.f829b0e780000p-6, -0x1.980267c7e09e4p-45)
A(0x1.0600000000000p+0, -0x1.7b91b07d58000p-6, -0x1.88d5493faa639p-45)
A(0x1.0400000000000p+0, -0x1.fc0a8b0fc0000p-7, -0x1.f1e7cf6d3a69cp-50)
A(0x1.0400000000000p+0, -0x1.fc0a8b0fc0000p-7, -0x1.f1e7cf6d3a69cp-50)
A(0x1.0200000000000p+0, -0x1.fe02a6b100000p-8, -0x1.9e23f0dda40e4p-46)
A(0x1.0200000000000p+0, -0x1.fe02a6b100000p-8, -0x1.9e23f0dda40e4p-46)
A(0x1.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0)
A(0x1.0000000000000p+0, 0x0.0000000000000p+0, 0x0.0000000000000p+0)
A(0x1.fc00000000000p-1, 0x1.0101575890000p-7, -0x1.0c76b999d2be8p-46)
A(0x1.f800000000000p-1, 0x1.0205658938000p-6, -0x1.3dc5b06e2f7d2p-45)
A(0x1.f400000000000p-1, 0x1.8492528c90000p-6, -0x1.aa0ba325a0c34p-45)
A(0x1.f000000000000p-1, 0x1.0415d89e74000p-5, 0x1.111c05cf1d753p-47)
A(0x1.ec00000000000p-1, 0x1.466aed42e0000p-5, -0x1.c167375bdfd28p-45)
A(0x1.e800000000000p-1, 0x1.894aa149fc000p-5, -0x1.97995d05a267dp-46)
A(0x1.e400000000000p-1, 0x1.ccb73cdddc000p-5, -0x1.a68f247d82807p-46)
A(0x1.e200000000000p-1, 0x1.eea31c006c000p-5, -0x1.e113e4fc93b7bp-47)
A(0x1.de00000000000p-1, 0x1.1973bd1466000p-4, -0x1.5325d560d9e9bp-45)
A(0x1.da00000000000p-1, 0x1.3bdf5a7d1e000p-4, 0x1.cc85ea5db4ed7p-45)
A(0x1.d600000000000p-1, 0x1.5e95a4d97a000p-4, -0x1.c69063c5d1d1ep-45)
A(0x1.d400000000000p-1, 0x1.700d30aeac000p-4, 0x1.c1e8da99ded32p-49)
A(0x1.d000000000000p-1, 0x1.9335e5d594000p-4, 0x1.3115c3abd47dap-45)
A(0x1.cc00000000000p-1, 0x1.b6ac88dad6000p-4, -0x1.390802bf768e5p-46)
A(0x1.ca00000000000p-1, 0x1.c885801bc4000p-4, 0x1.646d1c65aacd3p-45)
A(0x1.c600000000000p-1, 0x1.ec739830a2000p-4, -0x1.dc068afe645e0p-45)
A(0x1.c400000000000p-1, 0x1.fe89139dbe000p-4, -0x1.534d64fa10afdp-45)
A(0x1.c000000000000p-1, 0x1.1178e8227e000p-3, 0x1.1ef78ce2d07f2p-45)
A(0x1.be00000000000p-1, 0x1.1aa2b7e23f000p-3, 0x1.ca78e44389934p-45)
A(0x1.ba00000000000p-1, 0x1.2d1610c868000p-3, 0x1.39d6ccb81b4a1p-47)
A(0x1.b800000000000p-1, 0x1.365fcb0159000p-3, 0x1.62fa8234b7289p-51)
A(0x1.b400000000000p-1, 0x1.4913d8333b000p-3, 0x1.5837954fdb678p-45)
A(0x1.b200000000000p-1, 0x1.527e5e4a1b000p-3, 0x1.633e8e5697dc7p-45)
A(0x1.ae00000000000p-1, 0x1.6574ebe8c1000p-3, 0x1.9cf8b2c3c2e78p-46)
A(0x1.ac00000000000p-1, 0x1.6f0128b757000p-3, -0x1.5118de59c21e1p-45)
A(0x1.aa00000000000p-1, 0x1.7898d85445000p-3, -0x1.c661070914305p-46)
A(0x1.a600000000000p-1, 0x1.8beafeb390000p-3, -0x1.73d54aae92cd1p-47)
A(0x1.a400000000000p-1, 0x1.95a5adcf70000p-3, 0x1.7f22858a0ff6fp-47)
A(0x1.a000000000000p-1, 0x1.a93ed3c8ae000p-3, -0x1.8724350562169p-45)
A(0x1.9e00000000000p-1, 0x1.b31d8575bd000p-3, -0x1.c358d4eace1aap-47)
A(0x1.9c00000000000p-1, 0x1.bd087383be000p-3, -0x1.d4bc4595412b6p-45)
A(0x1.9a00000000000p-1, 0x1.c6ffbc6f01000p-3, -0x1.1ec72c5962bd2p-48)
A(0x1.9600000000000p-1, 0x1.db13db0d49000p-3, -0x1.aff2af715b035p-45)
A(0x1.9400000000000p-1, 0x1.e530effe71000p-3, 0x1.212276041f430p-51)
A(0x1.9200000000000p-1, 0x1.ef5ade4dd0000p-3, -0x1.a211565bb8e11p-51)
A(0x1.9000000000000p-1, 0x1.f991c6cb3b000p-3, 0x1.bcbecca0cdf30p-46)
A(0x1.8c00000000000p-1, 0x1.07138604d5800p-2, 0x1.89cdb16ed4e91p-48)
A(0x1.8a00000000000p-1, 0x1.0c42d67616000p-2, 0x1.7188b163ceae9p-45)
A(0x1.8800000000000p-1, 0x1.1178e8227e800p-2, -0x1.c210e63a5f01cp-45)
A(0x1.8600000000000p-1, 0x1.16b5ccbacf800p-2, 0x1.b9acdf7a51681p-45)
A(0x1.8400000000000p-1, 0x1.1bf99635a6800p-2, 0x1.ca6ed5147bdb7p-45)
A(0x1.8200000000000p-1, 0x1.214456d0eb800p-2, 0x1.a87deba46baeap-47)
A(0x1.7e00000000000p-1, 0x1.2bef07cdc9000p-2, 0x1.a9cfa4a5004f4p-45)
A(0x1.7c00000000000p-1, 0x1.314f1e1d36000p-2, -0x1.8e27ad3213cb8p-45)
A(0x1.7a00000000000p-1, 0x1.36b6776be1000p-2, 0x1.16ecdb0f177c8p-46)
A(0x1.7800000000000p-1, 0x1.3c25277333000p-2, 0x1.83b54b606bd5cp-46)
A(0x1.7600000000000p-1, 0x1.419b423d5e800p-2, 0x1.8e436ec90e09dp-47)
A(0x1.7400000000000p-1, 0x1.4718dc271c800p-2, -0x1.f27ce0967d675p-45)
A(0x1.7200000000000p-1, 0x1.4c9e09e173000p-2, -0x1.e20891b0ad8a4p-45)
A(0x1.7000000000000p-1, 0x1.522ae0738a000p-2, 0x1.ebe708164c759p-45)
A(0x1.6e00000000000p-1, 0x1.57bf753c8d000p-2, 0x1.fadedee5d40efp-46)
A(0x1.6c00000000000p-1, 0x1.5d5bddf596000p-2, -0x1.a0b2a08a465dcp-47)
},
};

#undef N
#undef A

/* ---- musl src/math/pow.c ---- */

/*
 * Double-precision x^y function.
 *
 * Copyright (c) 2018, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

/*
Worst-case error: 0.54 ULP (~= ulperr_exp + 1024*Ln2*relerr_log*2^53)
relerr_log: 1.3 * 2^-68 (Relative error of log, 1.5 * 2^-68 without fma)
ulperr_exp: 0.509 ULP (ULP error of exp, 0.511 ULP without fma)
*/

#define T dado_rt__pow_log_data.tab
#define A dado_rt__pow_log_data.poly
#define Ln2hi dado_rt__pow_log_data.ln2hi
#define Ln2lo dado_rt__pow_log_data.ln2lo
#define N (1 << POW_LOG_TABLE_BITS)
#define OFF 0x3fe6955500000000

/* Top 12 bits of a double (sign and exponent bits).  */
static inline uint32_t dado_rt__pow__top12(double x)
{
	return dado_rt__asuint64(x) >> 52;
}

/* Compute y+TAIL = log(x) where the rounded result is y and TAIL has about
   additional 15 bits precision.  IX is the bit representation of x, but
   normalized in the subnormal range using the sign bit for the exponent.  */
static inline double dado_rt__pow__log_inline(uint64_t ix, double *tail)
{
	/* double_t for better performance on targets with FLT_EVAL_METHOD==2.  */
	double z, r, y, invc, logc, logctail, kd, hi, t1, t2, lo, lo1, lo2, p;
	uint64_t iz, tmp;
	int k, i;

	/* x = 2^k z; where z is in range [OFF,2*OFF) and exact.
	   The range is split into N subintervals.
	   The ith subinterval contains z and c is near its center.  */
	tmp = ix - OFF;
	i = (tmp >> (52 - POW_LOG_TABLE_BITS)) % N;
	k = (int64_t)tmp >> 52; /* arithmetic shift */
	iz = ix - (tmp & 0xfffULL << 52);
	z = dado_rt__asdouble(iz);
	kd = (double)k;

	/* log(x) = k*Ln2 + log(c) + log1p(z/c-1).  */
	invc = T[i].invc;
	logc = T[i].logc;
	logctail = T[i].logctail;

	/* Note: 1/c is j/N or j/N/2 where j is an integer in [N,2N) and
     |z/c - 1| < 1/N, so r = z/c - 1 is exactly representible.  */
	/* Split z such that rhi, rlo and rhi*rhi are exact and |rlo| <= |r|.  */
	double zhi = dado_rt__asdouble((iz + (1ULL << 31)) & (-1ULL << 32));
	double zlo = z - zhi;
	double rhi = zhi * invc - 1.0;
	double rlo = zlo * invc;
	r = rhi + rlo;

	/* k*Ln2 + log(c) + r.  */
	t1 = kd * Ln2hi + logc;
	t2 = t1 + r;
	lo1 = kd * Ln2lo + logctail;
	lo2 = t1 - t2 + r;

	/* Evaluation is optimized assuming superscalar pipelined execution.  */
	double ar, ar2, ar3, lo3, lo4;
	ar = A[0] * r; /* A[0] = -0.5.  */
	ar2 = r * ar;
	ar3 = r * ar2;
	/* k*Ln2 + log(c) + r + A[0]*r*r.  */
	double arhi = A[0] * rhi;
	double arhi2 = rhi * arhi;
	hi = t2 + arhi2;
	lo3 = rlo * (ar + arhi);
	lo4 = t2 - hi + arhi2;
	/* p = log1p(r) - r - A[0]*r*r.  */
	p = (ar3 * (A[1] + r * A[2] +
		    ar2 * (A[3] + r * A[4] + ar2 * (A[5] + r * A[6]))));
	lo = lo1 + lo2 + lo3 + lo4 + p;
	y = hi + lo;
	*tail = hi - y + lo;
	return y;
}

#undef N
#undef T
#define N (1 << EXP_TABLE_BITS)
#define InvLn2N dado_rt__exp_data.invln2N
#define NegLn2hiN dado_rt__exp_data.negln2hiN
#define NegLn2loN dado_rt__exp_data.negln2loN
#define Shift dado_rt__exp_data.shift
#define T dado_rt__exp_data.tab
#define C2 dado_rt__exp_data.poly[5 - EXP_POLY_ORDER]
#define C3 dado_rt__exp_data.poly[6 - EXP_POLY_ORDER]
#define C4 dado_rt__exp_data.poly[7 - EXP_POLY_ORDER]
#define C5 dado_rt__exp_data.poly[8 - EXP_POLY_ORDER]
#define C6 dado_rt__exp_data.poly[9 - EXP_POLY_ORDER]

/* Handle cases that may overflow or underflow when computing the result that
   is scale*(1+TMP) without intermediate rounding.  The bit representation of
   scale is in SBITS, however it has a computed exponent that may have
   overflown into the sign bit so that needs to be adjusted before using it as
   a double.  (int32_t)KI is the k used in the argument reduction and exponent
   adjustment of scale, positive k here means the result may overflow and
   negative k means the result may underflow.  */
static inline double dado_rt__pow__specialcase(double tmp, uint64_t sbits, uint64_t ki)
{
	double scale, y;

	if ((ki & 0x80000000) == 0) {
		/* k > 0, the exponent of scale might have overflowed by <= 460.  */
		sbits -= 1009ull << 52;
		scale = dado_rt__asdouble(sbits);
		y = 0x1p1009 * (scale + scale * tmp);
		return dado_rt__eval_as_double(y);
	}
	/* k < 0, need special care in the subnormal range.  */
	sbits += 1022ull << 52;
	/* Note: sbits is signed scale.  */
	scale = dado_rt__asdouble(sbits);
	y = scale + scale * tmp;
	if (__builtin_fabs(y) < 1.0) {
		/* Round y to the right precision before scaling it into the subnormal
		   range to avoid double rounding that can cause 0.5+E/2 ulp error where
		   E is the worst-case ulp error outside the subnormal range.  So this
		   is only useful if the goal is better than 1 ulp worst-case error.  */
		double hi, lo, one = 1.0;
		if (y < 0.0)
			one = -1.0;
		lo = scale - y + scale * tmp;
		hi = one + y;
		lo = one - hi + y + lo;
		y = dado_rt__eval_as_double(hi + lo) - one;
		/* Fix the sign of 0.  */
		if (y == 0.0)
			y = dado_rt__asdouble(sbits & 0x8000000000000000);
		/* The underflow exception needs to be signaled explicitly.  */
		dado_rt__fp_force_eval(dado_rt__fp_barrier(0x1p-1022) * 0x1p-1022);
	}
	y = 0x1p-1022 * y;
	return dado_rt__eval_as_double(y);
}

#define SIGN_BIAS (0x800 << EXP_TABLE_BITS)

/* Computes sign*exp(x+xtail) where |xtail| < 2^-8/N and |xtail| <= |x|.
   The sign_bias argument is SIGN_BIAS or 0 and sets the sign to -1 or 1.  */
static inline double dado_rt__pow__exp_inline(double x, double xtail, uint32_t sign_bias)
{
	uint32_t abstop;
	uint64_t ki, idx, top, sbits;
	/* double_t for better performance on targets with FLT_EVAL_METHOD==2.  */
	double kd, z, r, r2, scale, tail, tmp;

	abstop = dado_rt__pow__top12(x) & 0x7ff;
	if (DADO_RT__UNLIKELY(abstop - dado_rt__pow__top12(0x1p-54) >=
			  dado_rt__pow__top12(512.0) - dado_rt__pow__top12(0x1p-54))) {
		if (abstop - dado_rt__pow__top12(0x1p-54) >= 0x80000000) {
			/* Avoid spurious underflow for tiny x.  */
			/* Note: 0 is common input.  */
			double one = DADO_RT__WANT_ROUNDING ? 1.0 + x : 1.0;
			return sign_bias ? -one : one;
		}
		if (abstop >= dado_rt__pow__top12(1024.0)) {
			/* Note: inf and nan are already handled.  */
			if (dado_rt__asuint64(x) >> 63)
				return dado_rt__math_uflow(sign_bias);
			else
				return dado_rt__math_oflow(sign_bias);
		}
		/* Large x is special cased below.  */
		abstop = 0;
	}

	/* exp(x) = 2^(k/N) * exp(r), with exp(r) in [2^(-1/2N),2^(1/2N)].  */
	/* x = ln2/N*k + r, with int k and r in [-ln2/2N, ln2/2N].  */
	z = InvLn2N * x;
	/* z - kd is in [-1, 1] in non-nearest rounding modes.  */
	kd = dado_rt__eval_as_double(z + Shift);
	ki = dado_rt__asuint64(kd);
	kd -= Shift;
	r = x + kd * NegLn2hiN + kd * NegLn2loN;
	/* The code assumes 2^-200 < |xtail| < 2^-8/N.  */
	r += xtail;
	/* 2^(k/N) ~= scale * (1 + tail).  */
	idx = 2 * (ki % N);
	top = (ki + sign_bias) << (52 - EXP_TABLE_BITS);
	tail = dado_rt__asdouble(T[idx]);
	/* This is only a valid scale when -1023*N < k < 1024*N.  */
	sbits = T[idx + 1] + top;
	/* exp(x) = 2^(k/N) * exp(r) ~= scale + scale * (tail + exp(r) - 1).  */
	/* Evaluation is optimized assuming superscalar pipelined execution.  */
	r2 = r * r;
	/* Without fma the worst case error is 0.25/N ulp larger.  */
	/* Worst case error is less than 0.5+1.11/N+(abs poly error * 2^53) ulp.  */
	tmp = tail + r + r2 * (C2 + r * C3) + r2 * r2 * (C4 + r * C5);
	if (DADO_RT__UNLIKELY(abstop == 0))
		return dado_rt__pow__specialcase(tmp, sbits, ki);
	scale = dado_rt__asdouble(sbits);
	/* Note: tmp == 0 or |tmp| > 2^-200 and scale > 2^-739, so there
	   is no spurious underflow here even without fma.  */
	return dado_rt__eval_as_double(scale + scale * tmp);
}

/* Returns 0 if not int, 1 if odd int, 2 if even int.  The argument is
   the bit representation of a non-zero finite floating-point value.  */
static inline int dado_rt__pow__checkint(uint64_t iy)
{
	int e = iy >> 52 & 0x7ff;
	if (e < 0x3ff)
		return 0;
	if (e > 0x3ff + 52)
		return 2;
	if (iy & ((1ULL << (0x3ff + 52 - e)) - 1))
		return 0;
	if (iy & (1ULL << (0x3ff + 52 - e)))
		return 1;
	return 2;
}

/* Returns 1 if input is the bit representation of 0, infinity or nan.  */
static inline int dado_rt__pow__zeroinfnan(uint64_t i)
{
	return 2 * i - 1 >= 2 * dado_rt__asuint64(__builtin_inf()) - 1;
}

double pow(double x, double y)
{
	uint32_t sign_bias = 0;
	uint64_t ix, iy;
	uint32_t topx, topy;

	ix = dado_rt__asuint64(x);
	iy = dado_rt__asuint64(y);
	topx = dado_rt__pow__top12(x);
	topy = dado_rt__pow__top12(y);
	if (DADO_RT__UNLIKELY(topx - 0x001 >= 0x7ff - 0x001 ||
			  (topy & 0x7ff) - 0x3be >= 0x43e - 0x3be)) {
		/* Note: if |y| > 1075 * ln2 * 2^53 ~= 0x1.749p62 then pow(x,y) = inf/0
		   and if |y| < 2^-54 / 1075 ~= 0x1.e7b6p-65 then pow(x,y) = +-1.  */
		/* Special cases: (x < 0x1p-126 or inf or nan) or
		   (|y| < 0x1p-65 or |y| >= 0x1p63 or nan).  */
		if (DADO_RT__UNLIKELY(dado_rt__pow__zeroinfnan(iy))) {
			if (2 * iy == 0)
				return DADO_RT__IS_SIGNALING(x) ? x + y : 1.0;
			if (ix == dado_rt__asuint64(1.0))
				return DADO_RT__IS_SIGNALING(y) ? x + y : 1.0;
			if (2 * ix > 2 * dado_rt__asuint64(__builtin_inf()) ||
			    2 * iy > 2 * dado_rt__asuint64(__builtin_inf()))
				return x + y;
			if (2 * ix == 2 * dado_rt__asuint64(1.0))
				return 1.0;
			if ((2 * ix < 2 * dado_rt__asuint64(1.0)) == !(iy >> 63))
				return 0.0; /* |x|<1 && y==inf or |x|>1 && y==-inf.  */
			return y * y;
		}
		if (DADO_RT__UNLIKELY(dado_rt__pow__zeroinfnan(ix))) {
			double x2 = x * x;
			if (ix >> 63 && dado_rt__pow__checkint(iy) == 1)
				x2 = -x2;
			/* Without the barrier some versions of clang hoist the 1/x2 and
			   thus division by zero exception can be signaled spuriously.  */
			return iy >> 63 ? dado_rt__fp_barrier(1 / x2) : x2;
		}
		/* Here x and y are non-zero finite.  */
		if (ix >> 63) {
			/* Finite x < 0.  */
			int yint = dado_rt__pow__checkint(iy);
			if (yint == 0)
				return dado_rt__math_invalid(x);
			if (yint == 1)
				sign_bias = SIGN_BIAS;
			ix &= 0x7fffffffffffffff;
			topx &= 0x7ff;
		}
		if ((topy & 0x7ff) - 0x3be >= 0x43e - 0x3be) {
			/* Note: sign_bias == 0 here because y is not odd.  */
			if (ix == dado_rt__asuint64(1.0))
				return 1.0;
			if ((topy & 0x7ff) < 0x3be) {
				/* |y| < 2^-65, x^y ~= 1 + y*log(x).  */
				if (DADO_RT__WANT_ROUNDING)
					return ix > dado_rt__asuint64(1.0) ? 1.0 + y :
								    1.0 - y;
				else
					return 1.0;
			}
			return (ix > dado_rt__asuint64(1.0)) == (topy < 0x800) ?
				       dado_rt__math_oflow(0) :
				       dado_rt__math_uflow(0);
		}
		if (topx == 0) {
			/* Normalize subnormal x so exponent becomes negative.  */
			ix = dado_rt__asuint64(x * 0x1p52);
			ix &= 0x7fffffffffffffff;
			ix -= 52ULL << 52;
		}
	}

	double lo;
	double hi = dado_rt__pow__log_inline(ix, &lo);
	double ehi, elo;
	double yhi = dado_rt__asdouble(iy & -1ULL << 27);
	double ylo = y - yhi;
	double lhi = dado_rt__asdouble(dado_rt__asuint64(hi) & -1ULL << 27);
	double llo = hi - lhi + lo;
	ehi = yhi * lhi;
	elo = ylo * lhi + y * llo; /* |elo| < |ehi| * 2^-25.  */
	return dado_rt__pow__exp_inline(ehi, elo, sign_bias);
}

#undef T
#undef A
#undef Ln2hi
#undef Ln2lo
#undef N
#undef OFF
#undef InvLn2N
#undef NegLn2hiN
#undef NegLn2loN
#undef Shift
#undef C2
#undef C3
#undef C4
#undef C5
#undef C6
#undef SIGN_BIAS

/* ---- musl src/math/__rem_pio2_large.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/k_rem_pio2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * __rem_pio2_large(x,y,e0,nx,prec)
 * double x[],y[]; int e0,nx,prec;
 *
 * __rem_pio2_large return the last three digits of N with
 *              y = x - N*pi/2
 * so that |y| < pi/2.
 *
 * The method is to compute the integer (mod 8) and fraction parts of
 * (2/pi)*x without doing the full multiplication. In general we
 * skip the part of the product that are known to be a huge integer (
 * more accurately, = 0 mod 8 ). Thus the number of operations are
 * independent of the exponent of the input.
 *
 * (2/pi) is represented by an array of 24-bit integers in ipio2[].
 *
 * Input parameters:
 *      x[]     The input value (must be positive) is broken into nx
 *              pieces of 24-bit integers in double precision format.
 *              x[i] will be the i-th 24 bit of x. The scaled exponent
 *              of x[0] is given in input parameter e0 (i.e., x[0]*2^e0
 *              match x's up to 24 bits.
 *
 *              Example of breaking a double positive z into x[0]+x[1]+x[2]:
 *                      e0 = ilogb(z)-23
 *                      z  = scalbn(z,-e0)
 *              for i = 0,1,2
 *                      x[i] = floor(z)
 *                      z    = (z-x[i])*2**24
 *
 *
 *      y[]     ouput result in an array of double precision numbers.
 *              The dimension of y[] is:
 *                      24-bit  precision       1
 *                      53-bit  precision       2
 *                      64-bit  precision       2
 *                      113-bit precision       3
 *              The actual value is the sum of them. Thus for 113-bit
 *              precison, one may have to do something like:
 *
 *              long double t,w,r_head, r_tail;
 *              t = (long double)y[2] + (long double)y[1];
 *              w = (long double)y[0];
 *              r_head = t+w;
 *              r_tail = w - (r_head - t);
 *
 *      e0      The exponent of x[0]. Must be <= 16360 or you need to
 *              expand the ipio2 table.
 *
 *      nx      dimension of x[]
 *
 *      prec    an integer indicating the precision:
 *                      0       24  bits (single)
 *                      1       53  bits (double)
 *                      2       64  bits (extended)
 *                      3       113 bits (quad)
 *
 * External function:
 *      double scalbn(), floor();
 *
 *
 * Here is the description of some local variables:
 *
 *      jk      jk+1 is the initial number of terms of ipio2[] needed
 *              in the computation. The minimum and recommended value
 *              for jk is 3,4,4,6 for single, double, extended, and quad.
 *              jk+1 must be 2 larger than you might expect so that our
 *              recomputation test works. (Up to 24 bits in the integer
 *              part (the 24 bits of it that we compute) and 23 bits in
 *              the fraction part may be lost to cancelation before we
 *              recompute.)
 *
 *      jz      local integer variable indicating the number of
 *              terms of ipio2[] used.
 *
 *      jx      nx - 1
 *
 *      jv      index for pointing to the suitable ipio2[] for the
 *              computation. In general, we want
 *                      ( 2^e0*x[0] * ipio2[jv-1]*2^(-24jv) )/8
 *              is an integer. Thus
 *                      e0-3-24*jv >= 0 or (e0-3)/24 >= jv
 *              Hence jv = max(0,(e0-3)/24).
 *
 *      jp      jp+1 is the number of terms in PIo2[] needed, jp = jk.
 *
 *      q[]     double array with integral value, representing the
 *              24-bits chunk of the product of x and 2/pi.
 *
 *      q0      the corresponding exponent of q[0]. Note that the
 *              exponent for q[i] would be q0-24*i.
 *
 *      PIo2[]  double precision array, obtained by cutting pi/2
 *              into 24 bits chunks.
 *
 *      f[]     ipio2[] in floating point
 *
 *      iq[]    integer array by breaking up q[] in 24-bits chunk.
 *
 *      fq[]    final product of x*(2/pi) in fq[0],..,fq[jk]
 *
 *      ih      integer. If >0 it indicates q[] is >= 0.5, hence
 *              it also indicates the *sign* of the result.
 *
 */
/*
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const int dado_rt__rem_pio2_large__init_jk[] = {3,4,4,6}; /* initial value for jk */

/*
 * Table of constants for 2/pi, 396 Hex digits (476 decimal) of 2/pi
 *
 *              integer array, contains the (24*i)-th to (24*i+23)-th
 *              bit of 2/pi after binary point. The corresponding
 *              floating value is
 *
 *                      ipio2[i] * 2^(-24(i+1)).
 *
 * NB: This table must have at least (e0-3)/24 + jk terms.
 *     For quad precision (e0 <= 16360, jk = 6), this is 686.
 */
static const int32_t dado_rt__rem_pio2_large__ipio2[] = {
0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62,
0x95993C, 0x439041, 0xFE5163, 0xABDEBB, 0xC561B7, 0x246E3A,
0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129,
0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C, 0x7026B4, 0x5F7E41,
0x3991D6, 0x398353, 0x39F49C, 0x845F8B, 0xBDF928, 0x3B1FF8,
0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D, 0x367ECF,
0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5,
0xF17B3D, 0x0739F7, 0x8A5292, 0xEA6BFB, 0x5FB11F, 0x8D5D08,
0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3,
0x91615E, 0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880,
0x4D7327, 0x310606, 0x1556CA, 0x73A8C9, 0x60E27B, 0xC08C6B,

};

static const double dado_rt__rem_pio2_large__PIo2[] = {
  1.57079625129699707031e+00, /* 0x3FF921FB, 0x40000000 */
  7.54978941586159635335e-08, /* 0x3E74442D, 0x00000000 */
  5.39030252995776476554e-15, /* 0x3CF84698, 0x80000000 */
  3.28200341580791294123e-22, /* 0x3B78CC51, 0x60000000 */
  1.27065575308067607349e-29, /* 0x39F01B83, 0x80000000 */
  1.22933308981111328932e-36, /* 0x387A2520, 0x40000000 */
  2.73370053816464559624e-44, /* 0x36E38222, 0x80000000 */
  2.16741683877804819444e-51, /* 0x3569F31D, 0x00000000 */
};

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
static int dado_rt__rem_pio2_large(double *x, double *y, int e0, int nx, int prec)
{
	int32_t jz,jx,jv,jp,jk,carry,n,iq[20],i,j,k,m,q0,ih;
	double z,fw,f[20],fq[20],q[20];

	/* initialize jk*/
	jk = dado_rt__rem_pio2_large__init_jk[prec];
	jp = jk;

	/* determine jx,jv,q0, note that 3>q0 */
	jx = nx-1;
	jv = (e0-3)/24;  if(jv<0) jv=0;
	q0 = e0-24*(jv+1);

	/* set up f[0] to f[jx+jk] where f[jx+jk] = ipio2[jv+jk] */
	j = jv-jx; m = jx+jk;
	for (i=0; i<=m; i++,j++)
		f[i] = j<0 ? 0.0 : (double)dado_rt__rem_pio2_large__ipio2[j];

	/* compute q[0],q[1],...q[jk] */
	for (i=0; i<=jk; i++) {
		for (j=0,fw=0.0; j<=jx; j++)
			fw += x[j]*f[jx+i-j];
		q[i] = fw;
	}

	jz = jk;
recompute:
	/* distill q[] into iq[] reversingly */
	for (i=0,j=jz,z=q[jz]; j>0; i++,j--) {
		fw    = (double)(int32_t)(0x1p-24*z);
		iq[i] = (int32_t)(z - 0x1p24*fw);
		z     = q[j-1]+fw;
	}

	/* compute n */
	z  = dado_rt__scalbn(z,q0);       /* actual value of z */
	z -= 8.0*__builtin_floor(z*0.125); /* trim off integer >= 8 */
	n  = (int32_t)z;
	z -= (double)n;
	ih = 0;
	if (q0 > 0) {  /* need iq[jz-1] to determine n */
		i  = iq[jz-1]>>(24-q0); n += i;
		iq[jz-1] -= i<<(24-q0);
		ih = iq[jz-1]>>(23-q0);
	}
	else if (q0 == 0) ih = iq[jz-1]>>23;
	else if (z >= 0.5) ih = 2;

	if (ih > 0) {  /* q > 0.5 */
		n += 1; carry = 0;
		for (i=0; i<jz; i++) {  /* compute 1-q */
			j = iq[i];
			if (carry == 0) {
				if (j != 0) {
					carry = 1;
					iq[i] = 0x1000000 - j;
				}
			} else
				iq[i] = 0xffffff - j;
		}
		if (q0 > 0) {  /* rare case: chance is 1 in 12 */
			switch(q0) {
			case 1:
				iq[jz-1] &= 0x7fffff; break;
			case 2:
				iq[jz-1] &= 0x3fffff; break;
			}
		}
		if (ih == 2) {
			z = 1.0 - z;
			if (carry != 0)
				z -= dado_rt__scalbn(1.0,q0);
		}
	}

	/* check if recomputation is needed */
	if (z == 0.0) {
		j = 0;
		for (i=jz-1; i>=jk; i--) j |= iq[i];
		if (j == 0) {  /* need recomputation */
			for (k=1; iq[jk-k]==0; k++);  /* k = no. of terms needed */

			for (i=jz+1; i<=jz+k; i++) {  /* add q[jz+1] to q[jz+k] */
				f[jx+i] = (double)dado_rt__rem_pio2_large__ipio2[jv+i];
				for (j=0,fw=0.0; j<=jx; j++)
					fw += x[j]*f[jx+i-j];
				q[i] = fw;
			}
			jz += k;
			goto recompute;
		}
	}

	/* chop off zero terms */
	if (z == 0.0) {
		jz -= 1;
		q0 -= 24;
		while (iq[jz] == 0) {
			jz--;
			q0 -= 24;
		}
	} else { /* break z into 24-bit if necessary */
		z = dado_rt__scalbn(z,-q0);
		if (z >= 0x1p24) {
			fw = (double)(int32_t)(0x1p-24*z);
			iq[jz] = (int32_t)(z - 0x1p24*fw);
			jz += 1;
			q0 += 24;
			iq[jz] = (int32_t)fw;
		} else
			iq[jz] = (int32_t)z;
	}

	/* convert integer "bit" chunk to floating-point value */
	fw = dado_rt__scalbn(1.0,q0);
	for (i=jz; i>=0; i--) {
		q[i] = fw*(double)iq[i];
		fw *= 0x1p-24;
	}

	/* compute PIo2[0,...,jp]*q[jz,...,0] */
	for(i=jz; i>=0; i--) {
		for (fw=0.0,k=0; k<=jp && k<=jz-i; k++)
			fw += dado_rt__rem_pio2_large__PIo2[k]*q[i+k];
		fq[jz-i] = fw;
	}

	/* compress fq[] into y[] */
	switch(prec) {
	case 0:
		fw = 0.0;
		for (i=jz; i>=0; i--)
			fw += fq[i];
		y[0] = ih==0 ? fw : -fw;
		break;
	case 1:
	case 2:
		fw = 0.0;
		for (i=jz; i>=0; i--)
			fw += fq[i];
		// TODO: drop excess precision here once double_t is used
		fw = (double)fw;
		y[0] = ih==0 ? fw : -fw;
		fw = fq[0]-fw;
		for (i=1; i<=jz; i++)
			fw += fq[i];
		y[1] = ih==0 ? fw : -fw;
		break;
	case 3:  /* painful */
		for (i=jz; i>0; i--) {
			fw      = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (i=jz; i>1; i--) {
			fw      = fq[i-1]+fq[i];
			fq[i]  += fq[i-1]-fw;
			fq[i-1] = fw;
		}
		for (fw=0.0,i=jz; i>=2; i--)
			fw += fq[i];
		if (ih==0) {
			y[0] =  fq[0]; y[1] =  fq[1]; y[2] =  fw;
		} else {
			y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw;
		}
	}
	return n&7;
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif


/* ---- musl src/math/__rem_pio2.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/e_rem_pio2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 * Optimized by Bruce D. Evans.
 */
/* __rem_pio2(x,y)
 *
 * return the remainder of x rem pi/2 in y[0]+y[1]
 * use __rem_pio2_large() for large x
 */

#define EPS DBL_EPSILON

/*
 * invpio2:  53 bits of 2/pi
 * pio2_1:   first  33 bit of pi/2
 * pio2_1t:  pi/2 - pio2_1
 * pio2_2:   second 33 bit of pi/2
 * pio2_2t:  pi/2 - (pio2_1+pio2_2)
 * pio2_3:   third  33 bit of pi/2
 * pio2_3t:  pi/2 - (pio2_1+pio2_2+pio2_3)
 */
static const double
dado_rt__rem_pio2__toint   = 1.5/EPS,
dado_rt__rem_pio2__pio4    = 0x1.921fb54442d18p-1,
dado_rt__rem_pio2__invpio2 = 6.36619772367581382433e-01, /* 0x3FE45F30, 0x6DC9C883 */
dado_rt__rem_pio2__pio2_1  = 1.57079632673412561417e+00, /* 0x3FF921FB, 0x54400000 */
dado_rt__rem_pio2__pio2_1t = 6.07710050650619224932e-11, /* 0x3DD0B461, 0x1A626331 */
dado_rt__rem_pio2__pio2_2  = 6.07710050630396597660e-11, /* 0x3DD0B461, 0x1A600000 */
dado_rt__rem_pio2__pio2_2t = 2.02226624879595063154e-21, /* 0x3BA3198A, 0x2E037073 */
dado_rt__rem_pio2__pio2_3  = 2.02226624871116645580e-21, /* 0x3BA3198A, 0x2E000000 */
dado_rt__rem_pio2__pio2_3t = 8.47842766036889956997e-32; /* 0x397B839A, 0x252049C1 */

/* caller must handle the case when reduction is not needed: |x| ~<= pi/4 */
static int dado_rt__rem_pio2(double x, double *y)
{
	union {double f; uint64_t i;} u = {x};
	double z,w,t,r,fn;
	double tx[3],ty[2];
	uint32_t ix;
	int sign, n, ex, ey, i;

	sign = u.i>>63;
	ix = u.i>>32 & 0x7fffffff;
	if (ix <= 0x400f6a7a) {  /* |x| ~<= 5pi/4 */
		if ((ix & 0xfffff) == 0x921fb)  /* |x| ~= pi/2 or 2pi/2 */
			goto medium;  /* cancellation -- use medium case */
		if (ix <= 0x4002d97c) {  /* |x| ~<= 3pi/4 */
			if (!sign) {
				z = x - dado_rt__rem_pio2__pio2_1;  /* one round good to 85 bits */
				y[0] = z - dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) - dado_rt__rem_pio2__pio2_1t;
				return 1;
			} else {
				z = x + dado_rt__rem_pio2__pio2_1;
				y[0] = z + dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) + dado_rt__rem_pio2__pio2_1t;
				return -1;
			}
		} else {
			if (!sign) {
				z = x - 2*dado_rt__rem_pio2__pio2_1;
				y[0] = z - 2*dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) - 2*dado_rt__rem_pio2__pio2_1t;
				return 2;
			} else {
				z = x + 2*dado_rt__rem_pio2__pio2_1;
				y[0] = z + 2*dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) + 2*dado_rt__rem_pio2__pio2_1t;
				return -2;
			}
		}
	}
	if (ix <= 0x401c463b) {  /* |x| ~<= 9pi/4 */
		if (ix <= 0x4015fdbc) {  /* |x| ~<= 7pi/4 */
			if (ix == 0x4012d97c)  /* |x| ~= 3pi/2 */
				goto medium;
			if (!sign) {
				z = x - 3*dado_rt__rem_pio2__pio2_1;
				y[0] = z - 3*dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) - 3*dado_rt__rem_pio2__pio2_1t;
				return 3;
			} else {
				z = x + 3*dado_rt__rem_pio2__pio2_1;
				y[0] = z + 3*dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) + 3*dado_rt__rem_pio2__pio2_1t;
				return -3;
			}
		} else {
			if (ix == 0x401921fb)  /* |x| ~= 4pi/2 */
				goto medium;
			if (!sign) {
				z = x - 4*dado_rt__rem_pio2__pio2_1;
				y[0] = z - 4*dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) - 4*dado_rt__rem_pio2__pio2_1t;
				return 4;
			} else {
				z = x + 4*dado_rt__rem_pio2__pio2_1;
				y[0] = z + 4*dado_rt__rem_pio2__pio2_1t;
				y[1] = (z-y[0]) + 4*dado_rt__rem_pio2__pio2_1t;
				return -4;
			}
		}
	}
	if (ix < 0x413921fb) {  /* |x| ~< 2^20*(pi/2), medium size */
medium:
		/* rint(x/(pi/2)) */
		fn = (double)x*dado_rt__rem_pio2__invpio2 + dado_rt__rem_pio2__toint - dado_rt__rem_pio2__toint;
		n = (int32_t)fn;
		r = x - fn*dado_rt__rem_pio2__pio2_1;
		w = fn*dado_rt__rem_pio2__pio2_1t;  /* 1st round, good to 85 bits */
		/* Matters with directed rounding. */
		if (DADO_RT__UNLIKELY(r - w < -dado_rt__rem_pio2__pio4)) {
			n--;
			fn--;
			r = x - fn*dado_rt__rem_pio2__pio2_1;
			w = fn*dado_rt__rem_pio2__pio2_1t;
		} else if (DADO_RT__UNLIKELY(r - w > dado_rt__rem_pio2__pio4)) {
			n++;
			fn++;
			r = x - fn*dado_rt__rem_pio2__pio2_1;
			w = fn*dado_rt__rem_pio2__pio2_1t;
		}
		y[0] = r - w;
		u.f = y[0];
		ey = u.i>>52 & 0x7ff;
		ex = ix>>20;
		if (ex - ey > 16) { /* 2nd round, good to 118 bits */
			t = r;
			w = fn*dado_rt__rem_pio2__pio2_2;
			r = t - w;
			w = fn*dado_rt__rem_pio2__pio2_2t - ((t-r)-w);
			y[0] = r - w;
			u.f = y[0];
			ey = u.i>>52 & 0x7ff;
			if (ex - ey > 49) {  /* 3rd round, good to 151 bits, covers all cases */
				t = r;
				w = fn*dado_rt__rem_pio2__pio2_3;
				r = t - w;
				w = fn*dado_rt__rem_pio2__pio2_3t - ((t-r)-w);
				y[0] = r - w;
			}
		}
		y[1] = (r - y[0]) - w;
		return n;
	}
	/*
	 * all other (large) arguments
	 */
	if (ix >= 0x7ff00000) {  /* x is inf or NaN */
		y[0] = y[1] = x - x;
		return 0;
	}
	/* set z = scalbn(|x|,-ilogb(x)+23) */
	u.f = x;
	u.i &= (uint64_t)-1>>12;
	u.i |= (uint64_t)(0x3ff + 23)<<52;
	z = u.f;
	for (i=0; i < 2; i++) {
		tx[i] = (double)(int32_t)z;
		z     = (z-tx[i])*0x1p24;
	}
	tx[i] = z;
	/* skip zero terms, first term is non-zero */
	while (tx[i] == 0.0)
		i--;
	n = dado_rt__rem_pio2_large(tx,ty,(int)(ix>>20)-(0x3ff+23),i+1,1);
	if (sign) {
		y[0] = -ty[0];
		y[1] = -ty[1];
		return -n;
	}
	y[0] = ty[0];
	y[1] = ty[1];
	return n;
}

#undef EPS

/* ---- musl src/math/__sin.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/k_sin.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* __sin( x, y, iy)
 * kernel sin function on ~[-pi/4, pi/4] (except on -0), pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 * Input iy indicates whether y is 0. (if iy=0, y assume to be 0).
 *
 * Algorithm
 *      1. Since sin(-x) = -sin(x), we need only to consider positive x.
 *      2. Callers must return sin(-0) = -0 without calling here since our
 *         odd polynomial is not evaluated in a way that preserves -0.
 *         Callers may do the optimization sin(x) ~ x for tiny x.
 *      3. sin(x) is approximated by a polynomial of degree 13 on
 *         [0,pi/4]
 *                               3            13
 *              sin(x) ~ x + S1*x + ... + S6*x
 *         where
 *
 *      |sin(x)         2     4     6     8     10     12  |     -58
 *      |----- - (1+S1*x +S2*x +S3*x +S4*x +S5*x  +S6*x   )| <= 2
 *      |  x                                               |
 *
 *      4. sin(x+y) = sin(x) + sin'(x')*y
 *                  ~ sin(x) + (1-x*x/2)*y
 *         For better accuracy, let
 *                   3      2      2      2      2
 *              r = x *(S2+x *(S3+x *(S4+x *(S5+x *S6))))
 *         then                   3    2
 *              sin(x) = x + (S1*x + (x *(r-y/2)+y))
 */

static const double
dado_rt__sin__S1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
dado_rt__sin__S2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
dado_rt__sin__S3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
dado_rt__sin__S4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
dado_rt__sin__S5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
dado_rt__sin__S6  =  1.58969099521155010221e-10; /* 0x3DE5D93A, 0x5ACFD57C */

static double dado_rt__k_sin(double x, double y, int iy)
{
	double z,r,v,w;

	z = x*x;
	w = z*z;
	r = dado_rt__sin__S2 + z*(dado_rt__sin__S3 + z*dado_rt__sin__S4) + z*w*(dado_rt__sin__S5 + z*dado_rt__sin__S6);
	v = z*x;
	if (iy == 0)
		return x + v*(dado_rt__sin__S1 + z*r);
	else
		return x - ((z*(0.5*y - v*r) - y) - v*dado_rt__sin__S1);
}


/* ---- musl src/math/__cos.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/k_cos.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * __cos( x,  y )
 * kernel cos function on [-pi/4, pi/4], pi/4 ~ 0.785398164
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 *
 * Algorithm
 *      1. Since cos(-x) = cos(x), we need only to consider positive x.
 *      2. if x < 2^-27 (hx<0x3e400000 0), return 1 with inexact if x!=0.
 *      3. cos(x) is approximated by a polynomial of degree 14 on
 *         [0,pi/4]
 *                                       4            14
 *              cos(x) ~ 1 - x*x/2 + C1*x + ... + C6*x
 *         where the remez error is
 *
 *      |              2     4     6     8     10    12     14 |     -58
 *      |cos(x)-(1-.5*x +C1*x +C2*x +C3*x +C4*x +C5*x  +C6*x  )| <= 2
 *      |                                                      |
 *
 *                     4     6     8     10    12     14
 *      4. let r = C1*x +C2*x +C3*x +C4*x +C5*x  +C6*x  , then
 *             cos(x) ~ 1 - x*x/2 + r
 *         since cos(x+y) ~ cos(x) - sin(x)*y
 *                        ~ cos(x) - x*y,
 *         a correction term is necessary in cos(x) and hence
 *              cos(x+y) = 1 - (x*x/2 - (r - x*y))
 *         For better accuracy, rearrange to
 *              cos(x+y) ~ w + (tmp + (r-x*y))
 *         where w = 1 - x*x/2 and tmp is a tiny correction term
 *         (1 - x*x/2 == w + tmp exactly in infinite precision).
 *         The exactness of w + tmp in infinite precision depends on w
 *         and tmp having the same precision as x.  If they have extra
 *         precision due to compiler bugs, then the extra precision is
 *         only good provided it is retained in all terms of the final
 *         expression for cos().  Retention happens in all cases tested
 *         under FreeBSD, so don't pessimize things by forcibly clipping
 *         any extra precision in w.
 */

static const double
dado_rt__cos__C1  =  4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
dado_rt__cos__C2  = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
dado_rt__cos__C3  =  2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
dado_rt__cos__C4  = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
dado_rt__cos__C5  =  2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
dado_rt__cos__C6  = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */

static double dado_rt__k_cos(double x, double y)
{
	double hz,z,r,w;

	z  = x*x;
	w  = z*z;
	r  = z*(dado_rt__cos__C1+z*(dado_rt__cos__C2+z*dado_rt__cos__C3)) + w*w*(dado_rt__cos__C4+z*(dado_rt__cos__C5+z*dado_rt__cos__C6));
	hz = 0.5*z;
	w  = 1.0-hz;
	return w + (((1.0-w)-hz) + (z*r-x*y));
}


/* ---- musl src/math/__tan.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/k_tan.c */
/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* __tan( x, y, k )
 * kernel tan function on ~[-pi/4, pi/4] (except on -0), pi/4 ~ 0.7854
 * Input x is assumed to be bounded by ~pi/4 in magnitude.
 * Input y is the tail of x.
 * Input odd indicates whether tan (if odd = 0) or -1/tan (if odd = 1) is returned.
 *
 * Algorithm
 *      1. Since tan(-x) = -tan(x), we need only to consider positive x.
 *      2. Callers must return tan(-0) = -0 without calling here since our
 *         odd polynomial is not evaluated in a way that preserves -0.
 *         Callers may do the optimization tan(x) ~ x for tiny x.
 *      3. tan(x) is approximated by a odd polynomial of degree 27 on
 *         [0,0.67434]
 *                               3             27
 *              tan(x) ~ x + T1*x + ... + T13*x
 *         where
 *
 *              |tan(x)         2     4            26   |     -59.2
 *              |----- - (1+T1*x +T2*x +.... +T13*x    )| <= 2
 *              |  x                                    |
 *
 *         Note: tan(x+y) = tan(x) + tan'(x)*y
 *                        ~ tan(x) + (1+x*x)*y
 *         Therefore, for better accuracy in computing tan(x+y), let
 *                   3      2      2       2       2
 *              r = x *(T2+x *(T3+x *(...+x *(T12+x *T13))))
 *         then
 *                                  3    2
 *              tan(x+y) = x + (T1*x + (x *(r+y)+y))
 *
 *      4. For x in [0.67434,pi/4],  let y = pi/4 - x, then
 *              tan(x) = tan(pi/4-y) = (1-tan(y))/(1+tan(y))
 *                     = 1 - 2*(tan(y) - (tan(y)^2)/(1+tan(y)))
 */

static const double dado_rt__tan__T[] = {
             3.33333333333334091986e-01, /* 3FD55555, 55555563 */
             1.33333333333201242699e-01, /* 3FC11111, 1110FE7A */
             5.39682539762260521377e-02, /* 3FABA1BA, 1BB341FE */
             2.18694882948595424599e-02, /* 3F9664F4, 8406D637 */
             8.86323982359930005737e-03, /* 3F8226E3, E96E8493 */
             3.59207910759131235356e-03, /* 3F6D6D22, C9560328 */
             1.45620945432529025516e-03, /* 3F57DBC8, FEE08315 */
             5.88041240820264096874e-04, /* 3F4344D8, F2F26501 */
             2.46463134818469906812e-04, /* 3F3026F7, 1A8D1068 */
             7.81794442939557092300e-05, /* 3F147E88, A03792A6 */
             7.14072491382608190305e-05, /* 3F12B80F, 32F0A7E9 */
            -1.85586374855275456654e-05, /* BEF375CB, DB605373 */
             2.59073051863633712884e-05, /* 3EFB2A70, 74BF7AD4 */
},
dado_rt__tan__pio4 =       7.85398163397448278999e-01, /* 3FE921FB, 54442D18 */
dado_rt__tan__pio4lo =     3.06161699786838301793e-17; /* 3C81A626, 33145C07 */

static double dado_rt__k_tan(double x, double y, int odd)
{
	double z, r, v, w, s, a;
	double w0, a0;
	uint32_t hx;
	int big, sign;

	DADO_RT__GET_HIGH_WORD(hx,x);
	big = (hx&0x7fffffff) >= 0x3FE59428; /* |x| >= 0.6744 */
	if (big) {
		sign = hx>>31;
		if (sign) {
			x = -x;
			y = -y;
		}
		x = (dado_rt__tan__pio4 - x) + (dado_rt__tan__pio4lo - y);
		y = 0.0;
	}
	z = x * x;
	w = z * z;
	/*
	 * Break x^5*(T[1]+x^2*T[2]+...) into
	 * x^5(T[1]+x^4*T[3]+...+x^20*T[11]) +
	 * x^5(x^2*(T[2]+x^4*T[4]+...+x^22*[T12]))
	 */
	r = dado_rt__tan__T[1] + w*(dado_rt__tan__T[3] + w*(dado_rt__tan__T[5] + w*(dado_rt__tan__T[7] + w*(dado_rt__tan__T[9] + w*dado_rt__tan__T[11]))));
	v = z*(dado_rt__tan__T[2] + w*(dado_rt__tan__T[4] + w*(dado_rt__tan__T[6] + w*(dado_rt__tan__T[8] + w*(dado_rt__tan__T[10] + w*dado_rt__tan__T[12])))));
	s = z * x;
	r = y + z*(s*(r + v) + y) + s*dado_rt__tan__T[0];
	w = x + r;
	if (big) {
		s = 1 - 2*odd;
		v = s - 2.0 * (x + (r - w*w/(w + s)));
		return sign ? -v : v;
	}
	if (!odd)
		return w;
	/* -1.0/(x+r) has up to 2ulp error, so compute it accurately */
	w0 = w;
	DADO_RT__SET_LOW_WORD(w0, 0);
	v = r - (w0 - x);       /* w0+v = r+x */
	a0 = a = -1.0 / w;
	DADO_RT__SET_LOW_WORD(a0, 0);
	return a0 + a*(1.0 + a0*w0 + a0*v);
}


/* ---- musl src/math/sin.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/s_sin.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* sin(x)
 * Return sine function of x.
 *
 * kernel function:
 *      __sin            ... sine function on [-pi/4,pi/4]
 *      __cos            ... cose function on [-pi/4,pi/4]
 *      __rem_pio2       ... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *      in [-pi/4 , +pi/4], and let n = k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          S           C             T
 *          1          C          -S            -1/T
 *          2         -S          -C             T
 *          3         -C           S            -1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

double sin(double x)
{
	double y[2];
	uint32_t ix;
	unsigned n;

	/* High word of x. */
	DADO_RT__GET_HIGH_WORD(ix, x);
	ix &= 0x7fffffff;

	/* |x| ~< pi/4 */
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e500000) {  /* |x| < 2**-26 */
			/* raise inexact if x != 0 and underflow if subnormal*/
			DADO_RT__FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
			return x;
		}
		return dado_rt__k_sin(x, 0.0, 0);
	}

	/* sin(Inf or NaN) is NaN */
	if (ix >= 0x7ff00000)
		return x - x;

	/* argument reduction needed */
	n = dado_rt__rem_pio2(x, y);
	switch (n&3) {
	case 0: return  dado_rt__k_sin(y[0], y[1], 1);
	case 1: return  dado_rt__k_cos(y[0], y[1]);
	case 2: return -dado_rt__k_sin(y[0], y[1], 1);
	default:
		return -dado_rt__k_cos(y[0], y[1]);
	}
}


/* ---- musl src/math/cos.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/s_cos.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* cos(x)
 * Return cosine function of x.
 *
 * kernel function:
 *      __sin           ... sine function on [-pi/4,pi/4]
 *      __cos           ... cosine function on [-pi/4,pi/4]
 *      __rem_pio2      ... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *      in [-pi/4 , +pi/4], and let n = k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          S           C             T
 *          1          C          -S            -1/T
 *          2         -S          -C             T
 *          3         -C           S            -1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

double cos(double x)
{
	double y[2];
	uint32_t ix;
	unsigned n;

	DADO_RT__GET_HIGH_WORD(ix, x);
	ix &= 0x7fffffff;

	/* |x| ~< pi/4 */
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e46a09e) {  /* |x| < 2**-27 * sqrt(2) */
			/* raise inexact if x!=0 */
			DADO_RT__FORCE_EVAL(x + 0x1p120f);
			return 1.0;
		}
		return dado_rt__k_cos(x, 0);
	}

	/* cos(Inf or NaN) is NaN */
	if (ix >= 0x7ff00000)
		return x-x;

	/* argument reduction */
	n = dado_rt__rem_pio2(x, y);
	switch (n&3) {
	case 0: return  dado_rt__k_cos(y[0], y[1]);
	case 1: return -dado_rt__k_sin(y[0], y[1], 1);
	case 2: return -dado_rt__k_cos(y[0], y[1]);
	default:
		return  dado_rt__k_sin(y[0], y[1], 1);
	}
}


/* ---- musl src/math/tan.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/s_tan.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* tan(x)
 * Return tangent function of x.
 *
 * kernel function:
 *      __tan           ... tangent function on [-pi/4,pi/4]
 *      __rem_pio2      ... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *      in [-pi/4 , +pi/4], and let n = k mod 4.
 *      We have
 *
 *          n        sin(x)      cos(x)        tan(x)
 *     ----------------------------------------------------------
 *          0          S           C             T
 *          1          C          -S            -1/T
 *          2         -S          -C             T
 *          3         -C           S            -1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

double tan(double x)
{
	double y[2];
	uint32_t ix;
	unsigned n;

	DADO_RT__GET_HIGH_WORD(ix, x);
	ix &= 0x7fffffff;

	/* |x| ~< pi/4 */
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e400000) { /* |x| < 2**-27 */
			/* raise inexact if x!=0 and underflow if subnormal */
			DADO_RT__FORCE_EVAL(ix < 0x00100000 ? x/0x1p120f : x+0x1p120f);
			return x;
		}
		return dado_rt__k_tan(x, 0.0, 0);
	}

	/* tan(Inf or NaN) is NaN */
	if (ix >= 0x7ff00000)
		return x - x;

	/* argument reduction */
	n = dado_rt__rem_pio2(x, y);
	return dado_rt__k_tan(y[0], y[1], n&1);
}


/* ---- musl src/math/acos.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/e_acos.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* acos(x)
 * Method :
 *      acos(x)  = pi/2 - asin(x)
 *      acos(-x) = pi/2 + asin(x)
 * For |x|<=0.5
 *      acos(x) = pi/2 - (x + x*x^2*R(x^2))     (see asin.c)
 * For x>0.5
 *      acos(x) = pi/2 - (pi/2 - 2asin(sqrt((1-x)/2)))
 *              = 2asin(sqrt((1-x)/2))
 *              = 2s + 2s*z*R(z)        ...z=(1-x)/2, s=sqrt(z)
 *              = 2f + (2c + 2s*z*R(z))
 *     where f=hi part of s, and c = (z-f*f)/(s+f) is the correction term
 *     for f so that f+c ~ sqrt(z).
 * For x<-0.5
 *      acos(x) = pi - 2asin(sqrt((1-|x|)/2))
 *              = pi - 0.5*(s+s*z*R(z)), where z=(1-|x|)/2,s=sqrt(z)
 *
 * Special cases:
 *      if x is NaN, return x itself;
 *      if |x|>1, return NaN with invalid signal.
 *
 * Function needed: sqrt
 */

static const double
dado_rt__acos__pio2_hi = 1.57079632679489655800e+00, /* 0x3FF921FB, 0x54442D18 */
dado_rt__acos__pio2_lo = 6.12323399573676603587e-17, /* 0x3C91A626, 0x33145C07 */
dado_rt__acos__pS0 =  1.66666666666666657415e-01, /* 0x3FC55555, 0x55555555 */
dado_rt__acos__pS1 = -3.25565818622400915405e-01, /* 0xBFD4D612, 0x03EB6F7D */
dado_rt__acos__pS2 =  2.01212532134862925881e-01, /* 0x3FC9C155, 0x0E884455 */
dado_rt__acos__pS3 = -4.00555345006794114027e-02, /* 0xBFA48228, 0xB5688F3B */
dado_rt__acos__pS4 =  7.91534994289814532176e-04, /* 0x3F49EFE0, 0x7501B288 */
dado_rt__acos__pS5 =  3.47933107596021167570e-05, /* 0x3F023DE1, 0x0DFDF709 */
dado_rt__acos__qS1 = -2.40339491173441421878e+00, /* 0xC0033A27, 0x1C8A2D4B */
dado_rt__acos__qS2 =  2.02094576023350569471e+00, /* 0x40002AE5, 0x9C598AC8 */
dado_rt__acos__qS3 = -6.88283971605453293030e-01, /* 0xBFE6066C, 0x1B8D0159 */
dado_rt__acos__qS4 =  7.70381505559019352791e-02; /* 0x3FB3B8C5, 0xB12E9282 */

static double dado_rt__acos__R(double z)
{
	double p, q;
	p = z*(dado_rt__acos__pS0+z*(dado_rt__acos__pS1+z*(dado_rt__acos__pS2+z*(dado_rt__acos__pS3+z*(dado_rt__acos__pS4+z*dado_rt__acos__pS5)))));
	q = 1.0+z*(dado_rt__acos__qS1+z*(dado_rt__acos__qS2+z*(dado_rt__acos__qS3+z*dado_rt__acos__qS4)));
	return p/q;
}

double acos(double x)
{
	double z,w,s,c,df;
	uint32_t hx,ix;

	DADO_RT__GET_HIGH_WORD(hx, x);
	ix = hx & 0x7fffffff;
	/* |x| >= 1 or nan */
	if (ix >= 0x3ff00000) {
		uint32_t lx;

		DADO_RT__GET_LOW_WORD(lx,x);
		if (((ix-0x3ff00000) | lx) == 0) {
			/* acos(1)=0, acos(-1)=pi */
			if (hx >> 31)
				return 2*dado_rt__acos__pio2_hi + 0x1p-120f;
			return 0;
		}
		return 0/(x-x);
	}
	/* |x| < 0.5 */
	if (ix < 0x3fe00000) {
		if (ix <= 0x3c600000)  /* |x| < 2**-57 */
			return dado_rt__acos__pio2_hi + 0x1p-120f;
		return dado_rt__acos__pio2_hi - (x - (dado_rt__acos__pio2_lo-x*dado_rt__acos__R(x*x)));
	}
	/* x < -0.5 */
	if (hx >> 31) {
		z = (1.0+x)*0.5;
		s = __builtin_sqrt(z);
		w = dado_rt__acos__R(z)*s-dado_rt__acos__pio2_lo;
		return 2*(dado_rt__acos__pio2_hi - (s+w));
	}
	/* x > 0.5 */
	z = (1.0-x)*0.5;
	s = __builtin_sqrt(z);
	df = s;
	DADO_RT__SET_LOW_WORD(df,0);
	c = (z-df*df)/(s+df);
	w = dado_rt__acos__R(z)*s+c;
	return 2*(df+w);
}


/* ---- musl src/math/atan.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/s_atan.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* atan(x)
 * Method
 *   1. Reduce x to positive by atan(x) = -atan(-x).
 *   2. According to the integer k=4t+0.25 chopped, t=x, the argument
 *      is further reduced to one of the following intervals and the
 *      arctangent of t is evaluated by the corresponding formula:
 *
 *      [0,7/16]      atan(x) = t-t^3*(a1+t^2*(a2+...(a10+t^2*a11)...)
 *      [7/16,11/16]  atan(x) = atan(1/2) + atan( (t-0.5)/(1+t/2) )
 *      [11/16.19/16] atan(x) = atan( 1 ) + atan( (t-1)/(1+t) )
 *      [19/16,39/16] atan(x) = atan(3/2) + atan( (t-1.5)/(1+1.5t) )
 *      [39/16,INF]   atan(x) = atan(INF) + atan( -1/t )
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double dado_rt__atan__atanhi[] = {
  4.63647609000806093515e-01, /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
  7.85398163397448278999e-01, /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
  9.82793723247329054082e-01, /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
  1.57079632679489655800e+00, /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
};

static const double dado_rt__atan__atanlo[] = {
  2.26987774529616870924e-17, /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
  3.06161699786838301793e-17, /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
  1.39033110312309984516e-17, /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
  6.12323399573676603587e-17, /* atan(inf)lo 0x3C91A626, 0x33145C07 */
};

static const double dado_rt__atan__aT[] = {
  3.33333333333329318027e-01, /* 0x3FD55555, 0x5555550D */
 -1.99999999998764832476e-01, /* 0xBFC99999, 0x9998EBC4 */
  1.42857142725034663711e-01, /* 0x3FC24924, 0x920083FF */
 -1.11111104054623557880e-01, /* 0xBFBC71C6, 0xFE231671 */
  9.09088713343650656196e-02, /* 0x3FB745CD, 0xC54C206E */
 -7.69187620504482999495e-02, /* 0xBFB3B0F2, 0xAF749A6D */
  6.66107313738753120669e-02, /* 0x3FB10D66, 0xA0D03D51 */
 -5.83357013379057348645e-02, /* 0xBFADDE2D, 0x52DEFD9A */
  4.97687799461593236017e-02, /* 0x3FA97B4B, 0x24760DEB */
 -3.65315727442169155270e-02, /* 0xBFA2B444, 0x2C6A6C2F */
  1.62858201153657823623e-02, /* 0x3F90AD3A, 0xE322DA11 */
};

static double dado_rt__atan(double x)
{
	double w,s1,s2,z;
	uint32_t ix,sign;
	int id;

	DADO_RT__GET_HIGH_WORD(ix, x);
	sign = ix >> 31;
	ix &= 0x7fffffff;
	if (ix >= 0x44100000) {   /* if |x| >= 2^66 */
		if (__builtin_isnan(x))
			return x;
		z = dado_rt__atan__atanhi[3] + 0x1p-120f;
		return sign ? -z : z;
	}
	if (ix < 0x3fdc0000) {    /* |x| < 0.4375 */
		if (ix < 0x3e400000) {  /* |x| < 2^-27 */
			if (ix < 0x00100000)
				/* raise underflow for subnormal x */
				DADO_RT__FORCE_EVAL((float)x);
			return x;
		}
		id = -1;
	} else {
		x = __builtin_fabs(x);
		if (ix < 0x3ff30000) {  /* |x| < 1.1875 */
			if (ix < 0x3fe60000) {  /*  7/16 <= |x| < 11/16 */
				id = 0;
				x = (2.0*x-1.0)/(2.0+x);
			} else {                /* 11/16 <= |x| < 19/16 */
				id = 1;
				x = (x-1.0)/(x+1.0);
			}
		} else {
			if (ix < 0x40038000) {  /* |x| < 2.4375 */
				id = 2;
				x = (x-1.5)/(1.0+1.5*x);
			} else {                /* 2.4375 <= |x| < 2^66 */
				id = 3;
				x = -1.0/x;
			}
		}
	}
	/* end of argument reduction */
	z = x*x;
	w = z*z;
	/* break sum from i=0 to 10 aT[i]z**(i+1) into odd and even poly */
	s1 = z*(dado_rt__atan__aT[0]+w*(dado_rt__atan__aT[2]+w*(dado_rt__atan__aT[4]+w*(dado_rt__atan__aT[6]+w*(dado_rt__atan__aT[8]+w*dado_rt__atan__aT[10])))));
	s2 = w*(dado_rt__atan__aT[1]+w*(dado_rt__atan__aT[3]+w*(dado_rt__atan__aT[5]+w*(dado_rt__atan__aT[7]+w*dado_rt__atan__aT[9]))));
	if (id < 0)
		return x - x*(s1+s2);
	z = dado_rt__atan__atanhi[id] - (x*(s1+s2) - dado_rt__atan__atanlo[id] - x);
	return sign ? -z : z;
}


/* ---- musl src/math/atan2.c ---- */

/* origin: FreeBSD /usr/src/lib/msun/src/e_atan2.c */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 *
 */
/* atan2(y,x)
 * Method :
 *      1. Reduce y to positive by atan2(y,x)=-atan2(-y,x).
 *      2. Reduce x to positive by (if x and y are unexceptional):
 *              ARG (x+iy) = arctan(y/x)           ... if x > 0,
 *              ARG (x+iy) = pi - arctan[y/(-x)]   ... if x < 0,
 *
 * Special cases:
 *
 *      ATAN2((anything), NaN ) is NaN;
 *      ATAN2(NAN , (anything) ) is NaN;
 *      ATAN2(+-0, +(anything but NaN)) is +-0  ;
 *      ATAN2(+-0, -(anything but NaN)) is +-pi ;
 *      ATAN2(+-(anything but 0 and NaN), 0) is +-pi/2;
 *      ATAN2(+-(anything but INF and NaN), +INF) is +-0 ;
 *      ATAN2(+-(anything but INF and NaN), -INF) is +-pi;
 *      ATAN2(+-INF,+INF ) is +-pi/4 ;
 *      ATAN2(+-INF,-INF ) is +-3pi/4;
 *      ATAN2(+-INF, (anything but,0,NaN, and INF)) is +-pi/2;
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

static const double
dado_rt__atan2__pi     = 3.1415926535897931160E+00, /* 0x400921FB, 0x54442D18 */
dado_rt__atan2__pi_lo  = 1.2246467991473531772E-16; /* 0x3CA1A626, 0x33145C07 */

double atan2(double y, double x)
{
	double z;
	uint32_t m,lx,ly,ix,iy;

	if (__builtin_isnan(x) || __builtin_isnan(y))
		return x+y;
	DADO_RT__EXTRACT_WORDS(ix, lx, x);
	DADO_RT__EXTRACT_WORDS(iy, ly, y);
	if (((ix-0x3ff00000) | lx) == 0)  /* x = 1.0 */
		return dado_rt__atan(y);
	m = ((iy>>31)&1) | ((ix>>30)&2);  /* 2*sign(x)+sign(y) */
	ix = ix & 0x7fffffff;
	iy = iy & 0x7fffffff;

	/* when y = 0 */
	if ((iy|ly) == 0) {
		switch(m) {
		case 0:
		case 1: return y;   /* atan(+-0,+anything)=+-0 */
		case 2: return  dado_rt__atan2__pi; /* atan(+0,-anything) = pi */
		case 3: return -dado_rt__atan2__pi; /* atan(-0,-anything) =-pi */
		}
	}
	/* when x = 0 */
	if ((ix|lx) == 0)
		return m&1 ? -dado_rt__atan2__pi/2 : dado_rt__atan2__pi/2;
	/* when x is INF */
	if (ix == 0x7ff00000) {
		if (iy == 0x7ff00000) {
			switch(m) {
			case 0: return  dado_rt__atan2__pi/4;   /* atan(+INF,+INF) */
			case 1: return -dado_rt__atan2__pi/4;   /* atan(-INF,+INF) */
			case 2: return  3*dado_rt__atan2__pi/4; /* atan(+INF,-INF) */
			case 3: return -3*dado_rt__atan2__pi/4; /* atan(-INF,-INF) */
			}
		} else {
			switch(m) {
			case 0: return  0.0; /* atan(+...,+INF) */
			case 1: return -0.0; /* atan(-...,+INF) */
			case 2: return  dado_rt__atan2__pi;  /* atan(+...,-INF) */
			case 3: return -dado_rt__atan2__pi;  /* atan(-...,-INF) */
			}
		}
	}
	/* |y/x| > 0x1p64 */
	if (ix+(64<<20) < iy || iy == 0x7ff00000)
		return m&1 ? -dado_rt__atan2__pi/2 : dado_rt__atan2__pi/2;

	/* z = atan(|y/x|) without spurious underflow */
	if ((m&2) && iy+(64<<20) < ix)  /* |y/x| < 0x1p-64, x<0 */
		z = 0;
	else
		z = dado_rt__atan(__builtin_fabs(y/x));
	switch (m) {
	case 0: return z;              /* atan(+,+) */
	case 1: return -z;             /* atan(-,+) */
	case 2: return dado_rt__atan2__pi - (z-dado_rt__atan2__pi_lo); /* atan(+,-) */
	default: /* case 3 */
		return (z-dado_rt__atan2__pi_lo) - dado_rt__atan2__pi; /* atan(-,-) */
	}
}


/* ---- musl src/math/hypot.c ---- */

#define SPLIT (0x1p27 + 1)

static void dado_rt__hypot__sq(double *hi, double *lo, double x)
{
	double xh, xl, xc;

	xc = (double)x*SPLIT;
	xh = x - xc + xc;
	xl = x - xh;
	*hi = (double)x*x;
	*lo = xh*xh - *hi + 2*xh*xl + xl*xl;
}

double hypot(double x, double y)
{
	union {double f; uint64_t i;} ux = {x}, uy = {y}, ut;
	int ex, ey;
	double hx, lx, hy, ly, z;

	/* arrange |x| >= |y| */
	ux.i &= -1ULL>>1;
	uy.i &= -1ULL>>1;
	if (ux.i < uy.i) {
		ut = ux;
		ux = uy;
		uy = ut;
	}

	/* special cases */
	ex = ux.i>>52;
	ey = uy.i>>52;
	x = ux.f;
	y = uy.f;
	/* note: hypot(inf,nan) == inf */
	if (ey == 0x7ff)
		return y;
	if (ex == 0x7ff || uy.i == 0)
		return x;
	/* note: hypot(x,y) ~= x + y*y/x/2 with inexact for small y/x */
	/* 64 difference is enough for ld80 double_t */
	if (ex - ey > 64)
		return x + y;

	/* precise sqrt argument in nearest rounding mode without overflow */
	/* xh*xh must not overflow and xl*xl must not underflow in sq */
	z = 1;
	if (ex > 0x3ff+510) {
		z = 0x1p700;
		x *= 0x1p-700;
		y *= 0x1p-700;
	} else if (ey < 0x3ff-450) {
		z = 0x1p-700;
		x *= 0x1p700;
		y *= 0x1p700;
	}
	dado_rt__hypot__sq(&hx, &lx, x);
	dado_rt__hypot__sq(&hy, &ly, y);
	return z*__builtin_sqrt(ly+lx+hy+hx);
}

#undef SPLIT

/* ---- musl src/math/fma.c ---- */

#define ASUINT64(x) ((union {double f; uint64_t i;}){x}).i
#define ZEROINFNAN (0x7ff-0x3ff-52-1)

struct dado_rt__fma__num { uint64_t m; int e; int sign; };

static struct dado_rt__fma__num dado_rt__fma__normalize(double x)
{
	uint64_t ix = ASUINT64(x);
	int e = ix>>52;
	int sign = e & 0x800;
	e &= 0x7ff;
	if (!e) {
		ix = ASUINT64(x*0x1p63);
		e = ix>>52 & 0x7ff;
		e = e ? e-63 : 0x800;
	}
	ix &= (1ull<<52)-1;
	ix |= 1ull<<52;
	ix <<= 1;
	e -= 0x3ff + 52 + 1;
	return (struct dado_rt__fma__num){ix,e,sign};
}

static void dado_rt__fma__mul(uint64_t *hi, uint64_t *lo, uint64_t x, uint64_t y)
{
	uint64_t t1,t2,t3;
	uint64_t xlo = (uint32_t)x, xhi = x>>32;
	uint64_t ylo = (uint32_t)y, yhi = y>>32;

	t1 = xlo*ylo;
	t2 = xlo*yhi + xhi*ylo;
	t3 = xhi*yhi;
	*lo = t1 + (t2<<32);
	*hi = t3 + (t2>>32) + (t1 > *lo);
}

double fma(double x, double y, double z)
{

	/* normalize so top 10bits and last bit are 0 */
	struct dado_rt__fma__num nx, ny, nz;
	nx = dado_rt__fma__normalize(x);
	ny = dado_rt__fma__normalize(y);
	nz = dado_rt__fma__normalize(z);

	if (nx.e >= ZEROINFNAN || ny.e >= ZEROINFNAN)
		return x*y + z;
	if (nz.e >= ZEROINFNAN) {
		if (nz.e > ZEROINFNAN) /* z==0: x*y is exact-signed, adding +0 would lose a -0 */
			return x*y;
		return z;
	}

	/* mul: r = x*y */
	uint64_t rhi, rlo, zhi, zlo;
	dado_rt__fma__mul(&rhi, &rlo, nx.m, ny.m);
	/* either top 20 or 21 bits of rhi and last 2 bits of rlo are 0 */

	/* align exponents */
	int e = nx.e + ny.e;
	int d = nz.e - e;
	/* shift bits z<<=kz, r>>=kr, so kz+kr == d, set e = e+kr (== ez-kz) */
	if (d > 0) {
		if (d < 64) {
			zlo = nz.m<<d;
			zhi = nz.m>>(64-d);
		} else {
			zlo = 0;
			zhi = nz.m;
			e = nz.e - 64;
			d -= 64;
			if (d == 0) {
			} else if (d < 64) {
				rlo = rhi<<(64-d) | rlo>>d | !!(rlo<<(64-d));
				rhi = rhi>>d;
			} else {
				rlo = 1;
				rhi = 0;
			}
		}
	} else {
		zhi = 0;
		d = -d;
		if (d == 0) {
			zlo = nz.m;
		} else if (d < 64) {
			zlo = nz.m>>d | !!(nz.m<<(64-d));
		} else {
			zlo = 1;
		}
	}

	/* add */
	int sign = nx.sign^ny.sign;
	int samesign = !(sign^nz.sign);
	int nonzero = 1;
	if (samesign) {
		/* r += z */
		rlo += zlo;
		rhi += zhi + (rlo < zlo);
	} else {
		/* r -= z */
		uint64_t t = rlo;
		rlo -= zlo;
		rhi = rhi - zhi - (t < rlo);
		if (rhi>>63) {
			rlo = -rlo;
			rhi = -rhi-!!rlo;
			sign = !sign;
		}
		nonzero = !!rhi;
	}

	/* set rhi to top 63bit of the result (last bit is sticky) */
	if (nonzero) {
		e += 64;
		d = __builtin_clzll(rhi)-1;
		/* note: d > 0 */
		rhi = rhi<<d | rlo>>(64-d) | !!(rlo<<d);
	} else if (rlo) {
		d = __builtin_clzll(rlo)-1;
		if (d < 0)
			rhi = rlo>>1 | (rlo&1);
		else
			rhi = rlo<<d;
	} else {
		/* exact +-0 */
		return x*y + z;
	}
	e -= d;

	/* convert to double */
	int64_t i = rhi; /* i is in [1<<62,(1<<63)-1] */
	if (sign)
		i = -i;
	double r = i; /* |r| is in [0x1p62,0x1p63] */

	if (e < -1022-62) {
		/* result is subnormal before rounding */
		if (e == -1022-63) {
			double c = 0x1p63;
			if (sign)
				c = -c;
			if (r == c) {
				/* min normal after rounding, underflow depends
				   on arch behaviour which can be imitated by
				   a double to float conversion */
				float fltmin = 0x0.ffffff8p-63*FLT_MIN * r;
				return DBL_MIN/FLT_MIN * fltmin;
			}
			/* one bit is lost when scaled, add another top bit to
			   only round once at conversion if it is inexact */
			if (rhi << 53) {
				i = rhi>>1 | (rhi&1) | 1ull<<62;
				if (sign)
					i = -i;
				r = i;
				r = 2*r - c; /* remove top bit */

				/* raise underflow portably, such that it
				   cannot be optimized away */
				{
					double tiny = DBL_MIN/FLT_MIN * r;
					r += (double)(tiny*tiny) * (r-r);
				}
			}
		} else {
			/* only round once when scaled */
			d = 10;
			i = ( rhi>>d | !!(rhi<<(64-d)) ) << d;
			if (sign)
				i = -i;
			r = i;
		}
	}
	return dado_rt__scalbn(r, e);
}

#undef ASUINT64
#undef ZEROINFNAN

/* ---- musl src/math/round.c ---- */

#define EPS DBL_EPSILON
static const double dado_rt__round__toint = 1/EPS;

double round(double x)
{
	union {double f; uint64_t i;} u = {x};
	int e = u.i >> 52 & 0x7ff;
	double y;

	if (e >= 0x3ff+52)
		return x;
	if (u.i >> 63)
		x = -x;
	if (e < 0x3ff-1) {
		/* raise inexact if x!=0 */
		DADO_RT__FORCE_EVAL(x + dado_rt__round__toint);
		return 0*u.f;
	}
	y = x + dado_rt__round__toint - dado_rt__round__toint - x;
	if (y > 0.5)
		y = y + x - 1;
	else if (y <= -0.5)
		y = y + x + 1;
	else
		y = y + x;
	if (u.i >> 63)
		y = -y;
	return y;
}

#undef EPS

/* ---- musl src/math/fmod.c ---- */

double fmod(double x, double y)
{
	union {double f; uint64_t i;} ux = {x}, uy = {y};
	int ex = ux.i>>52 & 0x7ff;
	int ey = uy.i>>52 & 0x7ff;
	int sx = ux.i>>63;
	uint64_t i;

	/* in the followings uxi should be ux.i, but then gcc wrongly adds */
	/* float load/store to inner loops ruining performance and code size */
	uint64_t uxi = ux.i;

	if (uy.i<<1 == 0 || __builtin_isnan(y) || ex == 0x7ff)
		return (x*y)/(x*y);
	if (uxi<<1 <= uy.i<<1) {
		if (uxi<<1 == uy.i<<1)
			return 0*x;
		return x;
	}

	/* normalize x and y */
	if (!ex) {
		for (i = uxi<<12; i>>63 == 0; ex--, i <<= 1);
		uxi <<= -ex + 1;
	} else {
		uxi &= -1ULL >> 12;
		uxi |= 1ULL << 52;
	}
	if (!ey) {
		for (i = uy.i<<12; i>>63 == 0; ey--, i <<= 1);
		uy.i <<= -ey + 1;
	} else {
		uy.i &= -1ULL >> 12;
		uy.i |= 1ULL << 52;
	}

	/* x mod y */
	for (; ex > ey; ex--) {
		i = uxi - uy.i;
		if (i >> 63 == 0) {
			if (i == 0)
				return 0*x;
			uxi = i;
		}
		uxi <<= 1;
	}
	i = uxi - uy.i;
	if (i >> 63 == 0) {
		if (i == 0)
			return 0*x;
		uxi = i;
	}
	for (; uxi>>52 == 0; uxi <<= 1, ex--);

	/* scale result */
	if (ex > 0) {
		uxi -= 1ULL << 52;
		uxi |= (uint64_t)ex << 52;
	} else {
		uxi >>= -ex + 1;
	}
	uxi |= (uint64_t)sx << 63;
	ux.i = uxi;
	return ux.f;
}


#undef EXP_TABLE_BITS
#undef EXP_POLY_ORDER
#undef EXP_USE_TOINT_NARROW
#undef EXP2_POLY_ORDER
#undef LOG_TABLE_BITS
#undef LOG_POLY_ORDER
#undef LOG_POLY1_ORDER
#undef LOG2_TABLE_BITS
#undef LOG2_POLY_ORDER
#undef LOG2_POLY1_ORDER
#undef POW_LOG_TABLE_BITS
#undef POW_LOG_POLY_ORDER

#undef DADO_RT__WANT_ROUNDING
#undef DADO_RT__IS_SIGNALING
#undef DADO_RT__LIKELY
#undef DADO_RT__UNLIKELY
#undef dado_rt__asuint64
#undef dado_rt__asdouble
#undef DADO_RT__EXTRACT_WORDS
#undef DADO_RT__GET_HIGH_WORD
#undef DADO_RT__GET_LOW_WORD
#undef DADO_RT__INSERT_WORDS
#undef DADO_RT__SET_HIGH_WORD
#undef DADO_RT__SET_LOW_WORD
#undef DADO_RT__FORCE_EVAL
