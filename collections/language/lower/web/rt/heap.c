/* The `#WEB` runtime: the heap.
 *
 * `malloc`, `calloc`, `realloc` and `free`, with C's prototypes, over the
 * module's linear memory. The region starts at `__heap_base` (the linker's
 * symbol for the first byte past the data and the stack) and ends at the
 * memory's current size; when nothing fits, the memory grows with
 * `__builtin_wasm_memory_grow(0, pages)`, and a refused grow is a `NULL`
 * answer — failure is a value, never a trap.
 *
 * `aligned_alloc` is not defined: every block here is aligned to 16
 * (`mem.MAX_ALIGN`), and the compiler's root heap serves a type aligned beyond
 * that by over-allocating through `calloc`/`realloc` and keeping the address
 * this file handed out in the word in front of its block.
 *
 * **The algorithm is TLSF** (two-level segregated fit: M. Masmano, I. Ripoll,
 * A. Crespo, J. Real, "TLSF: a new dynamic memory allocator for real-time
 * systems", ECRTS 2004), written for this file. Free blocks are kept in
 * `FL x SL` segregated lists — a first level per power of two, 32 linear
 * second-level classes inside each — with a bitmap per level, so finding a
 * list that is guaranteed to fit is two bit scans, and allocate, free and
 * coalesce are O(1): no loop in either path depends on how much is allocated.
 * A request is rounded up to the next class boundary before the search
 * (good-fit), which bounds the internal waste of a block to 1/32 of its size;
 * immediate coalescing with both physical neighbours bounds the external
 * fragmentation. No pause, no compaction: a DadoScript host runs a frame loop.
 *
 * **Layout.** Every block has a 16-byte header — the physically previous
 * block and this block's payload size, whose two low bits say whether the
 * block and its previous neighbour are free — and a payload that is a
 * multiple of 16 and at least 16. A free block keeps its list links in the
 * first 16 payload bytes. A region ends in a zero-size used sentinel, so the
 * last block has a next neighbour like every other. Payloads are therefore
 * aligned to 16 wherever the region starts, once the start is rounded up.
 *
 * **Threads.** `dado_rt__heap_lock` and `dado_rt__heap_unlock` bracket every
 * entry point. In a single-threaded module they are empty. In a threaded one
 * (`-matomics`, which defines `__wasm_atomics__`) they are a **spin lock** on
 * one word: test-and-test-and-set, an acquiring exchange and a releasing
 * store. A spin and not a waiting lock, for two reasons: a browser's main
 * thread may allocate and may not wait (`memory.atomic.wait32` traps there),
 * so a lock every thread takes must be one every thread can take; and what
 * it guards is short — TLSF's allocate and free are O(1), and the one long
 * path, a `memory.grow`, is rare and bounded by the engine. Nothing else here
 * changes.
 *
 * **Native testing.** Compiled with `DADO_RT_HEAP_STATIC_BYTES` defined, the
 * same algorithm takes its "linear memory" from a static array of that many
 * bytes: `__heap_base` becomes the array's start (plus
 * `DADO_RT_HEAP_STATIC_OFFSET`, to exercise the rounding) and memory growth is
 * emulated in 64 KiB pages over the array, refusing past its end — so the
 * growth and refusal paths run natively under ASan and UBSan too. Under ASan,
 * free payloads and the slack past each request are poisoned. With
 * `DADO_RT_HEAP_CHECK` defined, `dado_rt__heap_check` walks every region and
 * every list and answers 0 when every invariant holds, and
 * `dado_rt__heap_stats` reports the footprint.
 *
 * Freestanding C11; the only headers are `<stddef.h>` `<stdint.h>`
 * `<stdbool.h>`.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void *memset(void *dest, int c, size_t n);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);

/* ── the one place the threaded build changes ──────────────────────────── */

#if defined(__wasm__) && defined(__wasm_atomics__)
static int32_t dado_rt__heap_word;

static inline void dado_rt__heap_lock(void) {
    while (__atomic_exchange_n(&dado_rt__heap_word, 1, __ATOMIC_ACQUIRE) != 0) {
        while (__atomic_load_n(&dado_rt__heap_word, __ATOMIC_RELAXED) != 0) {
        }
    }
}

static inline void dado_rt__heap_unlock(void) {
    __atomic_store_n(&dado_rt__heap_word, 0, __ATOMIC_RELEASE);
}
#else
static inline void dado_rt__heap_lock(void) {}
static inline void dado_rt__heap_unlock(void) {}
#endif

/* ── parameters ────────────────────────────────────────────────────────── */

#define DADO_RT__ALIGN_LOG2 4u
#define DADO_RT__ALIGN ((size_t)1 << DADO_RT__ALIGN_LOG2) /* mem.MAX_ALIGN */
#define DADO_RT__HDR ((size_t)16)                         /* a block's header */
#define DADO_RT__MIN ((size_t)16)                         /* the least payload */
#define DADO_RT__SL_LOG2 5u
#define DADO_RT__SL_COUNT (1u << DADO_RT__SL_LOG2)
#define DADO_RT__FL_SHIFT (DADO_RT__SL_LOG2 + DADO_RT__ALIGN_LOG2)
#define DADO_RT__SMALL ((size_t)1 << DADO_RT__FL_SHIFT)
/* One past the largest first-level index: blocks are below 2^32 on a 32-bit
 * target and below 2^40 on a 64-bit one (the native tests). */
#define DADO_RT__FL_MAX (SIZE_MAX > 0xffffffffu ? 40u : 32u)
#define DADO_RT__FL_COUNT (DADO_RT__FL_MAX - DADO_RT__FL_SHIFT + 1u)
/* The largest request served; above it `malloc` answers NULL. */
#define DADO_RT__MAX_REQ ((size_t)1 << (DADO_RT__FL_MAX - 1u))
#define DADO_RT__PAGE ((size_t)65536)

#define DADO_RT__FREE ((size_t)1)
#define DADO_RT__PREV_FREE ((size_t)2)
#define DADO_RT__FLAGS ((size_t)3)

_Static_assert(DADO_RT__ALIGN >= _Alignof(max_align_t), "the heap under-aligns max_align_t");
_Static_assert(DADO_RT__MIN >= 2 * sizeof(void *), "a free payload cannot hold its links");
_Static_assert(DADO_RT__HDR >= 2 * sizeof(void *) && DADO_RT__HDR % DADO_RT__ALIGN == 0,
               "the header does not keep payloads aligned");
_Static_assert(DADO_RT__FL_COUNT <= 32, "the first-level bitmap is 32 bits");

typedef struct dado_rt__block {
    struct dado_rt__block *prev_phys; /* NULL for a region's first block */
    size_t size;                     /* payload bytes | FREE | PREV_FREE */
} dado_rt__block;

typedef struct dado_rt__links {
    dado_rt__block *next;
    dado_rt__block *prev;
} dado_rt__links;

static struct {
    bool ready;
    uint32_t fl_map;
    uint32_t sl_map[DADO_RT__FL_COUNT];
    dado_rt__block *lists[DADO_RT__FL_COUNT][DADO_RT__SL_COUNT];
    unsigned char *end;        /* one past the newest region's sentinel */
    dado_rt__block *sentinel;   /* the newest region's sentinel */
    size_t claimed;            /* bytes of every region, sentinels included */
#if defined(DADO_RT_HEAP_CHECK)
    unsigned char *regions[256]; /* each region's first byte, for the walker */
    unsigned regions_len;
#endif
} dado_rt__tlsf;

/* ── the memory underneath ─────────────────────────────────────────────── */

#if defined(DADO_RT_HEAP_STATIC_BYTES)

#if !defined(DADO_RT_HEAP_STATIC_OFFSET)
#define DADO_RT_HEAP_STATIC_OFFSET 0
#endif
#if !defined(DADO_RT_HEAP_STATIC_INITIAL_PAGES)
#define DADO_RT_HEAP_STATIC_INITIAL_PAGES 1
#endif

static _Alignas(16) unsigned char dado_rt__arena[DADO_RT_HEAP_STATIC_BYTES];
static size_t dado_rt__arena_pages = DADO_RT_HEAP_STATIC_INITIAL_PAGES;
/* Pages the next emulated grow skips before it hands out memory, so a test
 * can make a grow land somewhere not contiguous with the region before it —
 * what happens on the web when something other than this heap grows memory. */
size_t dado_rt__heap_test_gap_pages;

static unsigned char *dado_rt__heap_base(void) {
    return dado_rt__arena + DADO_RT_HEAP_STATIC_OFFSET;
}
static size_t dado_rt__memory_pages(void) {
    return dado_rt__arena_pages;
}
static size_t dado_rt__memory_grow(size_t pages) {
    size_t cap = sizeof dado_rt__arena / DADO_RT__PAGE;
    size_t gap = dado_rt__heap_test_gap_pages;
    if (pages > cap || gap > cap - pages || dado_rt__arena_pages > cap - pages - gap) {
        return SIZE_MAX;
    }
    dado_rt__arena_pages += gap;
    size_t old = dado_rt__arena_pages;
    dado_rt__arena_pages += pages;
    return old;
}
static unsigned char *dado_rt__page_address(size_t page) {
    return dado_rt__arena + page * DADO_RT__PAGE;
}

#else

extern unsigned char __heap_base;

static unsigned char *dado_rt__heap_base(void) {
    return &__heap_base;
}
static size_t dado_rt__memory_pages(void) {
    return __builtin_wasm_memory_size(0);
}
static size_t dado_rt__memory_grow(size_t pages) {
    return __builtin_wasm_memory_grow(0, pages);
}
static unsigned char *dado_rt__page_address(size_t page) {
    return (unsigned char *)(uintptr_t)(page * DADO_RT__PAGE);
}

#endif

/* ── ASan poisoning (native tests only) ────────────────────────────────── */

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define DADO_RT__ASAN 1
#endif
#endif
#if defined(__SANITIZE_ADDRESS__)
#define DADO_RT__ASAN 1
#endif

#if defined(DADO_RT__ASAN) && defined(DADO_RT_HEAP_STATIC_BYTES)
void __asan_poison_memory_region(void const volatile *addr, size_t size);
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
#define DADO_RT__POISON(p, n) __asan_poison_memory_region((p), (n))
#define DADO_RT__UNPOISON(p, n) __asan_unpoison_memory_region((p), (n))
#else
#define DADO_RT__POISON(p, n) ((void)(p), (void)(n))
#define DADO_RT__UNPOISON(p, n) ((void)(p), (void)(n))
#endif

/* ── block arithmetic ──────────────────────────────────────────────────── */

static inline size_t dado_rt__size(const dado_rt__block *b) {
    return b->size & ~DADO_RT__FLAGS;
}
static inline bool dado_rt__is_free(const dado_rt__block *b) {
    return (b->size & DADO_RT__FREE) != 0;
}
static inline bool dado_rt__prev_is_free(const dado_rt__block *b) {
    return (b->size & DADO_RT__PREV_FREE) != 0;
}
static inline void dado_rt__set_size(dado_rt__block *b, size_t size) {
    b->size = size | (b->size & DADO_RT__FLAGS);
}
static inline void dado_rt__set_flag(dado_rt__block *b, size_t flag, bool on) {
    b->size = on ? (b->size | flag) : (b->size & ~flag);
}
static inline unsigned char *dado_rt__payload(dado_rt__block *b) {
    return (unsigned char *)b + DADO_RT__HDR;
}
static inline dado_rt__block *dado_rt__from_payload(void *p) {
    return (dado_rt__block *)(void *)((unsigned char *)p - DADO_RT__HDR);
}
static inline dado_rt__block *dado_rt__next_phys(dado_rt__block *b) {
    return (dado_rt__block *)(void *)(dado_rt__payload(b) + dado_rt__size(b));
}
static inline dado_rt__links *dado_rt__links_of(dado_rt__block *b) {
    return (dado_rt__links *)(void *)dado_rt__payload(b);
}

/* Make the header (and, for a free block, the links) of a block about to be
 * written at `b` addressable. */
static inline void dado_rt__claim_header(dado_rt__block *b) {
    DADO_RT__UNPOISON(b, DADO_RT__HDR + DADO_RT__MIN);
}

/* ── the two-level index ───────────────────────────────────────────────── */

static inline unsigned dado_rt__fls(size_t v) { /* v != 0 */
#if SIZE_MAX > 0xffffffffu
    return 63u - (unsigned)__builtin_clzll((unsigned long long)v);
#else
    return 31u - (unsigned)__builtin_clz((unsigned)v);
#endif
}

static inline void dado_rt__mapping(size_t size, unsigned *fl, unsigned *sl) {
    if (size < DADO_RT__SMALL) {
        *fl = 0;
        *sl = (unsigned)(size >> DADO_RT__ALIGN_LOG2);
    } else {
        unsigned t = dado_rt__fls(size);
        *sl = (unsigned)(size >> (t - DADO_RT__SL_LOG2)) ^ DADO_RT__SL_COUNT;
        *fl = t - (DADO_RT__FL_SHIFT - 1u);
    }
}

/* The class a search starts at: the request rounded up to the next class
 * boundary, so that every block in that class or above fits it. */
static inline void dado_rt__mapping_search(size_t size, unsigned *fl, unsigned *sl) {
    if (size >= DADO_RT__SMALL) {
        size += ((size_t)1 << (dado_rt__fls(size) - DADO_RT__SL_LOG2)) - 1u;
    }
    dado_rt__mapping(size, fl, sl);
}

static void dado_rt__insert(dado_rt__block *b) {
    unsigned fl, sl;
    dado_rt__mapping(dado_rt__size(b), &fl, &sl);
    dado_rt__block *head = dado_rt__tlsf.lists[fl][sl];
    dado_rt__links *l = dado_rt__links_of(b);
    l->next = head;
    l->prev = NULL;
    if (head != NULL) {
        dado_rt__links_of(head)->prev = b;
    }
    dado_rt__tlsf.lists[fl][sl] = b;
    dado_rt__tlsf.fl_map |= 1u << fl;
    dado_rt__tlsf.sl_map[fl] |= 1u << sl;
}

static void dado_rt__remove(dado_rt__block *b) {
    unsigned fl, sl;
    dado_rt__mapping(dado_rt__size(b), &fl, &sl);
    dado_rt__links *l = dado_rt__links_of(b);
    if (l->next != NULL) {
        dado_rt__links_of(l->next)->prev = l->prev;
    }
    if (l->prev != NULL) {
        dado_rt__links_of(l->prev)->next = l->next;
    } else {
        dado_rt__tlsf.lists[fl][sl] = l->next;
        if (l->next == NULL) {
            dado_rt__tlsf.sl_map[fl] &= ~(1u << sl);
            if (dado_rt__tlsf.sl_map[fl] == 0) {
                dado_rt__tlsf.fl_map &= ~(1u << fl);
            }
        }
    }
}

/* A free block of at least `size` payload bytes, still on its list, or NULL. */
static dado_rt__block *dado_rt__find(size_t size) {
    unsigned fl, sl;
    dado_rt__mapping_search(size, &fl, &sl);
    if (fl >= DADO_RT__FL_COUNT) {
        return NULL;
    }
    uint32_t sl_map = dado_rt__tlsf.sl_map[fl] & (~(uint32_t)0 << sl);
    if (sl_map == 0) {
        uint32_t fl_map = fl + 1u < 32u ? dado_rt__tlsf.fl_map & (~(uint32_t)0 << (fl + 1u)) : 0u;
        if (fl_map == 0) {
            return NULL;
        }
        fl = (unsigned)__builtin_ctz(fl_map);
        sl_map = dado_rt__tlsf.sl_map[fl];
    }
    sl = (unsigned)__builtin_ctz(sl_map);
    return dado_rt__tlsf.lists[fl][sl];
}

/* ── free-block bookkeeping ────────────────────────────────────────────── */

/* Mark `b` free, tell its next neighbour, poison the payload past the links,
 * and put it on its list. The caller has already merged it. */
static void dado_rt__release(dado_rt__block *b) {
    dado_rt__set_flag(b, DADO_RT__FREE, true);
    dado_rt__block *next = dado_rt__next_phys(b);
    next->prev_phys = b;
    dado_rt__set_flag(next, DADO_RT__PREV_FREE, true);
    DADO_RT__UNPOISON(dado_rt__payload(b), DADO_RT__MIN);
    DADO_RT__POISON(dado_rt__payload(b) + DADO_RT__MIN, dado_rt__size(b) - DADO_RT__MIN);
    dado_rt__insert(b);
}

/* Merge the used-to-be block `b` with a free neighbour on either side, then
 * release the result. */
static void dado_rt__free_block(dado_rt__block *b) {
    if (dado_rt__prev_is_free(b)) {
        dado_rt__block *prev = b->prev_phys;
        dado_rt__remove(prev);
        dado_rt__set_size(prev, dado_rt__size(prev) + DADO_RT__HDR + dado_rt__size(b));
        b = prev;
    }
    dado_rt__block *next = dado_rt__next_phys(b);
    if (dado_rt__is_free(next)) {
        dado_rt__remove(next);
        dado_rt__set_size(b, dado_rt__size(b) + DADO_RT__HDR + dado_rt__size(next));
    }
    dado_rt__release(b);
}

/* Cut `b` (used) down to `size` payload bytes when what is left over can be
 * a block of its own; the remainder is freed, merging with a free next. */
static void dado_rt__trim(dado_rt__block *b, size_t size) {
    size_t have = dado_rt__size(b);
    if (have < size + DADO_RT__HDR + DADO_RT__MIN) {
        return;
    }
    dado_rt__block *rest = (dado_rt__block *)(void *)(dado_rt__payload(b) + size);
    dado_rt__claim_header(rest);
    rest->prev_phys = b;
    rest->size = have - size - DADO_RT__HDR; /* used, previous used */
    dado_rt__set_size(b, size);
    dado_rt__free_block(rest);
}

/* Take the free block `b` off its list, mark it used, and trim it to `size`. */
static void *dado_rt__take(dado_rt__block *b, size_t size, size_t request) {
    dado_rt__remove(b);
    dado_rt__set_flag(b, DADO_RT__FREE, false);
    dado_rt__block *next = dado_rt__next_phys(b);
    dado_rt__set_flag(next, DADO_RT__PREV_FREE, false);
    dado_rt__trim(b, size);
    unsigned char *p = dado_rt__payload(b);
    DADO_RT__UNPOISON(p, request);
    DADO_RT__POISON(p + request, dado_rt__size(b) - request);
    return p;
}

/* ── regions ───────────────────────────────────────────────────────────── */

static inline unsigned char *dado_rt__align_up(unsigned char *p) {
    uintptr_t a = (uintptr_t)p;
    return (unsigned char *)(uintptr_t)((a + (DADO_RT__ALIGN - 1u)) & ~(uintptr_t)(DADO_RT__ALIGN - 1u));
}

/* Lay a new region over [start, end): one free block and a sentinel. Answers
 * false when the span is too small to hold them. */
static bool dado_rt__add_region(unsigned char *start, unsigned char *end) {
    start = dado_rt__align_up(start);
    if (end <= start || (size_t)(end - start) < 2u * DADO_RT__HDR + DADO_RT__MIN) {
        return false;
    }
    size_t span = (size_t)(end - start) & ~(DADO_RT__ALIGN - 1u);
    dado_rt__block *b = (dado_rt__block *)(void *)start;
    dado_rt__claim_header(b);
    b->prev_phys = NULL;
    b->size = span - 2u * DADO_RT__HDR;
    dado_rt__block *s = dado_rt__next_phys(b);
    dado_rt__claim_header(s);
    s->prev_phys = b;
    s->size = 0;
    dado_rt__tlsf.end = start + span;
    dado_rt__tlsf.sentinel = s;
    dado_rt__tlsf.claimed += span;
#if defined(DADO_RT_HEAP_CHECK)
    if (dado_rt__tlsf.regions_len < sizeof dado_rt__tlsf.regions / sizeof dado_rt__tlsf.regions[0]) {
        dado_rt__tlsf.regions[dado_rt__tlsf.regions_len++] = start;
    }
#endif
    dado_rt__release(b);
    return true;
}

static void dado_rt__init(void) {
    dado_rt__tlsf.ready = true;
    unsigned char *end = dado_rt__page_address(dado_rt__memory_pages());
    (void)dado_rt__add_region(dado_rt__heap_base(), end);
}

/* Grow the memory by enough for a block of `size` payload bytes to be found:
 * at least that, at least an eighth of what is claimed, whole pages. Answers false
 * when the memory refuses. */
static bool dado_rt__grow(size_t size) {
    /* A search rounds the request up to its class boundary (at most a 32nd
     * of it), so the new block must reach that boundary to be found. */
    size_t slack = size / 16u + 3u * DADO_RT__HDR + DADO_RT__ALIGN;
    if (size > SIZE_MAX - slack) {
        return false;
    }
    size_t want = size + slack;
    size_t eighth = dado_rt__tlsf.claimed / 8u;
    if (want < eighth) {
        want = eighth;
    }
    size_t pages = want / DADO_RT__PAGE + (want % DADO_RT__PAGE != 0);
    size_t old = dado_rt__memory_grow(pages);
    if (old == SIZE_MAX) {
        return false;
    }
    unsigned char *start = dado_rt__page_address(old);
    unsigned char *end = start + pages * DADO_RT__PAGE;
    if (dado_rt__tlsf.sentinel != NULL && start == dado_rt__tlsf.end) {
        /* Contiguous: the old sentinel becomes the new block's header, and
         * a new sentinel closes the region. */
        dado_rt__block *b = dado_rt__tlsf.sentinel;
        size_t added = pages * DADO_RT__PAGE;
        b->size = (b->size & DADO_RT__PREV_FREE) | (added - DADO_RT__HDR);
        dado_rt__block *s = dado_rt__next_phys(b);
        dado_rt__claim_header(s);
        s->prev_phys = b;
        s->size = 0;
        dado_rt__tlsf.end = end;
        dado_rt__tlsf.sentinel = s;
        dado_rt__tlsf.claimed += added;
        dado_rt__free_block(b);
        return true;
    }
    return dado_rt__add_region(start, end);
}

/* ── the C interface ───────────────────────────────────────────────────── */

/* The payload a request of `n` bytes is given: rounded up to the alignment,
 * never below the least payload. 0 when the request is too large. */
static inline size_t dado_rt__adjust(size_t n) {
    if (n > DADO_RT__MAX_REQ) {
        return 0;
    }
    size_t size = (n + (DADO_RT__ALIGN - 1u)) & ~(DADO_RT__ALIGN - 1u);
    return size < DADO_RT__MIN ? DADO_RT__MIN : size;
}

static void *dado_rt__malloc(size_t n) {
    size_t size = dado_rt__adjust(n);
    if (size == 0) {
        return NULL;
    }
    if (!dado_rt__tlsf.ready) {
        dado_rt__init();
    }
    dado_rt__block *b = dado_rt__find(size);
    if (b == NULL) {
        if (!dado_rt__grow(size)) {
            return NULL;
        }
        b = dado_rt__find(size);
        if (b == NULL) {
            return NULL;
        }
    }
    return dado_rt__take(b, size, n);
}

void *malloc(size_t n) {
    dado_rt__heap_lock();
    void *p = dado_rt__malloc(n);
    dado_rt__heap_unlock();
    return p;
}

void *calloc(size_t count, size_t size) {
    if (size != 0 && count > SIZE_MAX / size) {
        return NULL;
    }
    size_t n = count * size;
    dado_rt__heap_lock();
    void *p = dado_rt__malloc(n);
    dado_rt__heap_unlock();
    if (p != NULL) {
        memset(p, 0, n);
    }
    return p;
}

void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    dado_rt__heap_lock();
    dado_rt__block *b = dado_rt__from_payload(ptr);
    dado_rt__free_block(b);
    dado_rt__heap_unlock();
}

/* `realloc(p, 0)` answers a least-size block and frees `p` (it never answers
 * NULL for a zero size except when the memory is exhausted); a refused
 * `realloc` leaves `p` exactly as it was. */
void *realloc(void *ptr, size_t n) {
    if (ptr == NULL) {
        return malloc(n);
    }
    size_t size = dado_rt__adjust(n);
    if (size == 0) {
        return NULL;
    }
    dado_rt__heap_lock();
    dado_rt__block *b = dado_rt__from_payload(ptr);
    size_t have = dado_rt__size(b);
    if (size <= have) {
        dado_rt__trim(b, size);
        DADO_RT__UNPOISON(ptr, n);
        DADO_RT__POISON((unsigned char *)ptr + n, dado_rt__size(b) - n);
        dado_rt__heap_unlock();
        return ptr;
    }
    dado_rt__block *next = dado_rt__next_phys(b);
    if (dado_rt__is_free(next) && have + DADO_RT__HDR + dado_rt__size(next) >= size) {
        dado_rt__remove(next);
        dado_rt__set_size(b, have + DADO_RT__HDR + dado_rt__size(next));
        dado_rt__block *after = dado_rt__next_phys(b);
        after->prev_phys = b;
        dado_rt__set_flag(after, DADO_RT__PREV_FREE, false);
        dado_rt__trim(b, size);
        DADO_RT__UNPOISON(ptr, n);
        DADO_RT__POISON((unsigned char *)ptr + n, dado_rt__size(b) - n);
        dado_rt__heap_unlock();
        return ptr;
    }
    void *q = dado_rt__malloc(n);
    if (q != NULL) {
        /* `have < n` here, so the whole old payload fits the new block. */
        DADO_RT__UNPOISON(ptr, have);
        memcpy(q, ptr, have);
        dado_rt__free_block(b);
    }
    dado_rt__heap_unlock();
    return q;
}

/* ── the walker (tests only) ───────────────────────────────────────────── */

#if defined(DADO_RT_HEAP_CHECK)
/* 0 when every invariant holds; otherwise a small number naming the first
 * that does not. */
int dado_rt__heap_check(void);
int dado_rt__heap_check(void) {
    size_t free_blocks = 0;
    size_t claimed = 0;
    for (unsigned r = 0; r < dado_rt__tlsf.regions_len; r++) {
        dado_rt__block *b = (dado_rt__block *)(void *)dado_rt__tlsf.regions[r];
        dado_rt__block *prev = NULL;
        if (b->prev_phys != NULL) {
            return 1;
        }
        for (;;) {
            if (b->prev_phys != prev) {
                return 2;
            }
            if (prev != NULL && dado_rt__prev_is_free(b) != dado_rt__is_free(prev)) {
                return 3;
            }
            if (dado_rt__size(b) == 0) {
                if (dado_rt__is_free(b)) {
                    return 4;
                }
                break;
            }
            if (dado_rt__size(b) % DADO_RT__ALIGN != 0 || dado_rt__size(b) < DADO_RT__MIN) {
                return 5;
            }
            if (((uintptr_t)dado_rt__payload(b)) % DADO_RT__ALIGN != 0) {
                return 6;
            }
            if (dado_rt__is_free(b)) {
                if (prev != NULL && dado_rt__is_free(prev)) {
                    return 7; /* two free neighbours were not merged */
                }
                free_blocks++;
            }
            prev = b;
            b = dado_rt__next_phys(b);
        }
        claimed += (size_t)((unsigned char *)b + DADO_RT__HDR - dado_rt__tlsf.regions[r]);
    }
    if (claimed != dado_rt__tlsf.claimed) {
        return 8;
    }
    size_t listed = 0;
    for (unsigned fl = 0; fl < DADO_RT__FL_COUNT; fl++) {
        if (((dado_rt__tlsf.fl_map >> fl) & 1u) != (dado_rt__tlsf.sl_map[fl] != 0)) {
            return 9;
        }
        for (unsigned sl = 0; sl < DADO_RT__SL_COUNT; sl++) {
            dado_rt__block *b = dado_rt__tlsf.lists[fl][sl];
            if (((dado_rt__tlsf.sl_map[fl] >> sl) & 1u) != (b != NULL)) {
                return 10;
            }
            dado_rt__block *prev = NULL;
            for (; b != NULL; b = dado_rt__links_of(b)->next) {
                unsigned f, s;
                dado_rt__mapping(dado_rt__size(b), &f, &s);
                if (!dado_rt__is_free(b) || f != fl || s != sl || dado_rt__links_of(b)->prev != prev) {
                    return 11;
                }
                prev = b;
                listed++;
            }
        }
    }
    return listed == free_blocks ? 0 : 12;
}

/* The bytes claimed from memory (every region, headers and sentinels
 * included), the bytes of free payload, and the largest free payload. */
void dado_rt__heap_stats(size_t *claimed, size_t *free_bytes, size_t *largest);
void dado_rt__heap_stats(size_t *claimed, size_t *free_bytes, size_t *largest) {
    size_t total = 0, big = 0;
    for (unsigned r = 0; r < dado_rt__tlsf.regions_len; r++) {
        dado_rt__block *b = (dado_rt__block *)(void *)dado_rt__tlsf.regions[r];
        for (; dado_rt__size(b) != 0; b = dado_rt__next_phys(b)) {
            if (dado_rt__is_free(b)) {
                total += dado_rt__size(b);
                big = dado_rt__size(b) > big ? dado_rt__size(b) : big;
            }
        }
    }
    *claimed = dado_rt__tlsf.claimed;
    *free_bytes = total;
    *largest = big;
}
#endif

#undef DADO_RT__POISON
#undef DADO_RT__UNPOISON
