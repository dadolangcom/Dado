/* The `#WEB` runtime: bulk memory.
 *
 * `memset`, `memcpy`, `memmove` and `memcmp` with C's exact prototypes. A
 * `#WEB` program has no C library, and the optimizer calls these four whatever
 * the program wrote: an aggregate zeroed or copied at `-O2` becomes a call to
 * `memset`/`memcpy`, and a byte comparison loop becomes `memcmp` (or `bcmp`,
 * which is why `bcmp` is here too). So they are always present, and
 * `wasm-ld --gc-sections` drops the ones a module does not reach.
 *
 * **`no_builtin` on every body** (`DADO_RT__NO_BUILTIN`; gcc spells it differently). Each is a loop, and the loop-idiom pass
 * recognises exactly these loops and replaces them with a call to the function
 * that contains them: an unconditional self-call, a stack overflow on the first
 * use. `__attribute__((no_builtin))` turns the recognition off for the one
 * function; `-ffreestanding` alone does not promise it, and the attribute keeps
 * the file correct when it is compiled without that flag, as the native tests
 * compile it. The self-call's absence is checked by disassembly in
 * `tests/webrt/run.sh`.
 *
 * **Bulk-memory instructions.** When the translation unit is compiled with
 * `-mbulk-memory` (`__wasm_bulk_memory__`), `memset`, `memcpy` and `memmove`
 * are `__builtin_memset` / `__builtin_memmove`, which clang 18 lowers to one
 * `memory.fill` / `memory.copy` for every size, constant or not (measured;
 * `tests/webrt/run.sh` disassembles both builds and fails on any call from one
 * of these functions to itself, so a compiler that stopped doing so is caught
 * rather than trusted). Without the feature the same builtin is lowered to a
 * call of the function being defined, so the loops below are used instead:
 * a word at a time once both pointers can be aligned, a byte at a time
 * otherwise. `memory.copy` has `memmove`'s semantics, so `memcpy` shares it.
 * `memcmp` has no instruction and is a loop either way.
 *
 * Freestanding C11; the only headers are `<stddef.h>` and `<stdint.h>`.
 */

#include <stddef.h>
#include <stdint.h>

void *memset(void *dest, int c, size_t n);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);
int bcmp(const void *a, const void *b, size_t n);

/* A word the loops move at a time. `may_alias` because it reads and writes
 * bytes of any declared type. */
typedef uintptr_t __attribute__((may_alias)) dado_rt__word;

/* clang spells "do not recognise this loop as a library call" `no_builtin`;
 * gcc does not know that attribute and spells it as turning off the pass that
 * does the recognising. */
#if defined(__clang__)
#define DADO_RT__NO_BUILTIN __attribute__((no_builtin))
#elif defined(__GNUC__)
#define DADO_RT__NO_BUILTIN __attribute__((optimize("no-tree-loop-distribute-patterns")))
#else
#define DADO_RT__NO_BUILTIN
#endif

#define DADO_RT__WORD sizeof(dado_rt__word)
#define DADO_RT__MISALIGNED(p) (((uintptr_t)(p)) & (DADO_RT__WORD - 1))

DADO_RT__NO_BUILTIN void *memset(void *dest, int c, size_t n) {
#if defined(__wasm_bulk_memory__)
    return __builtin_memset(dest, c, n);
#else
    unsigned char *d = (unsigned char *)dest;
    unsigned char b = (unsigned char)c;
    while (n > 0 && DADO_RT__MISALIGNED(d)) {
        *d++ = b;
        n--;
    }
    if (n >= DADO_RT__WORD) {
        dado_rt__word w = (dado_rt__word)b * (dado_rt__word)(UINTPTR_MAX / 255u);
        dado_rt__word *dw = (dado_rt__word *)(void *)d;
        while (n >= DADO_RT__WORD) {
            *dw++ = w;
            n -= DADO_RT__WORD;
        }
        d = (unsigned char *)dw;
    }
    while (n > 0) {
        *d++ = b;
        n--;
    }
    return dest;
#endif
}

#if defined(__wasm_bulk_memory__)
/* `memory.copy` is defined for overlapping ranges, so it is `memmove` and
 * therefore also a correct `memcpy`. */
static inline void dado_rt__copy(void *dest, const void *src, size_t n) {
    __builtin_memmove(dest, src, n);
}
#else
/* Forward: `d` below `s`, or the ranges disjoint. Word-wide when the two
 * pointers share an alignment, which is the common case for the optimizer's
 * aggregate copies. */
DADO_RT__NO_BUILTIN static void dado_rt__copy_up(unsigned char *d, const unsigned char *s,
                                                        size_t n) {
    if (DADO_RT__MISALIGNED(d) == DADO_RT__MISALIGNED(s)) {
        while (n > 0 && DADO_RT__MISALIGNED(d)) {
            *d++ = *s++;
            n--;
        }
        dado_rt__word *dw = (dado_rt__word *)(void *)d;
        const dado_rt__word *sw = (const dado_rt__word *)(const void *)s;
        while (n >= DADO_RT__WORD) {
            *dw++ = *sw++;
            n -= DADO_RT__WORD;
        }
        d = (unsigned char *)dw;
        s = (const unsigned char *)sw;
    }
    while (n > 0) {
        *d++ = *s++;
        n--;
    }
}

/* Backward: `d` above `s` and overlapping. */
DADO_RT__NO_BUILTIN static void dado_rt__copy_down(unsigned char *d, const unsigned char *s,
                                                          size_t n) {
    d += n;
    s += n;
    if (DADO_RT__MISALIGNED(d) == DADO_RT__MISALIGNED(s)) {
        while (n > 0 && DADO_RT__MISALIGNED(d)) {
            *--d = *--s;
            n--;
        }
        dado_rt__word *dw = (dado_rt__word *)(void *)d;
        const dado_rt__word *sw = (const dado_rt__word *)(const void *)s;
        while (n >= DADO_RT__WORD) {
            *--dw = *--sw;
            n -= DADO_RT__WORD;
        }
        d = (unsigned char *)dw;
        s = (const unsigned char *)sw;
    }
    while (n > 0) {
        *--d = *--s;
        n--;
    }
}
#endif

DADO_RT__NO_BUILTIN void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
#if defined(__wasm_bulk_memory__)
    dado_rt__copy(dest, src, n);
#else
    dado_rt__copy_up((unsigned char *)dest, (const unsigned char *)src, n);
#endif
    return dest;
}

DADO_RT__NO_BUILTIN void *memmove(void *dest, const void *src, size_t n) {
#if defined(__wasm_bulk_memory__)
    dado_rt__copy(dest, src, n);
#else
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    /* Compared as integers: relational `<` between pointers into two objects
     * is undefined, and the whole point of `memmove` is that they may be one. */
    if ((uintptr_t)d - (uintptr_t)s >= n) {
        dado_rt__copy_up(d, s, n);
    } else {
        dado_rt__copy_down(d, s, n);
    }
#endif
    return dest;
}

DADO_RT__NO_BUILTIN int memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *x = (const unsigned char *)a;
    const unsigned char *y = (const unsigned char *)b;
    for (; n > 0; n--, x++, y++) {
        if (*x != *y) {
            return (int)*x - (int)*y;
        }
    }
    return 0;
}

/* clang turns an equality-only `memcmp` into `bcmp`; C only asks that the
 * answer be zero exactly when the bytes are equal. */
DADO_RT__NO_BUILTIN int bcmp(const void *a, const void *b, size_t n) {
    return memcmp(a, b, n);
}

#undef DADO_RT__WORD
#undef DADO_RT__MISALIGNED
#undef DADO_RT__NO_BUILTIN
