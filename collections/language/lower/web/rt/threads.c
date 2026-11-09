/* The `#WEB` runtime: threads, mutexes and condition variables.
 *
 * What `core:threads`' `#WEB` arm calls where a hosted build calls
 * `<pthread.h>`. **Threads exist only in a threaded module** — one compiled
 * with wasm's atomics (`-matomics`, which defines `__wasm_atomics__`) and
 * linked over a shared memory, which `./dado --web-threads` builds. A module
 * without them has one thread: `dadoc` refuses a reached `threads.spawn` there
 * (ERR1033), and what this file compiles to is the one-thread answer of each
 * entry point below.
 *
 * **A thread is a worker on the same memory.** `dado_rt__web_thread_spawn` allocates,
 * from the heap, a record, a stack and a TLS block, then calls the host's
 * `dado.spawn(slot, arg)`: `slot` is the function-table slot of the entry
 * trampoline below and `arg` the record. The loader answers it with a worker
 * (node: `worker_threads`; a page: `Worker`) that instantiates this module on
 * the same shared memory, sets its `__stack_pointer` to the record's first
 * word — the top of the stack allocated here — and calls the slot with the
 * record. **The record's first two words are the loader's contract**: word 0
 * is the stack top, word 1 the state (0 running, 1 returned, 2 trapped — the
 * loader writes 2 when the worker traps, so a join does not wait forever).
 *
 * The trampoline runs `__wasm_init_tls` on the record's TLS block before
 * anything else, so every `_Thread_local` object — the ambient allocator, the
 * channel consumer id, a `@thread_local` variable — is this thread's own and
 * starts at its initializer. It then runs the thread's function, stores state
 * 1 and wakes whoever waits on the word.
 *
 * **The stack is 1 MiB**, `DADO_RT__WEB_STACK`, from the heap, and it is not
 * guarded: an overflow writes into the block below it. The module's main
 * thread is linked with 8 MiB placed first in memory (`--stack-first`), so
 * its overflow traps instead; a function that runs on the main thread can
 * overflow a spawned one's.
 *
 * **`join` waits with `memory.atomic.wait32`**, which a browser's main thread
 * cannot execute (the engine traps it: the page's thread must not block).
 * The host's `dado.can_wait()` answers whether this thread may; where it may
 * not, a join is refused — a sentence on standard error naming why, and
 * `false` — rather than trapped. Node's main thread can wait. Mutexes and
 * condition variables wait the same way, and where waiting is refused they
 * spin (a mutex) or return at once (a condition wait, which may always return
 * spuriously: its caller loops on the predicate).
 *
 * The mutex is the three-state one of U. Drepper, "Futexes Are Tricky"
 * (2011) — 0 free, 1 held, 2 held with waiters — so an uncontended lock
 * and unlock are one atomic each and never call the host. The condition
 * variable is a sequence word: a waiter reads it, releases the mutex and
 * waits while it is unchanged; a signal increments it and wakes one waiter,
 * a broadcast all.
 *
 * Freestanding C11 with clang's wasm builtins; the only headers are
 * `<stdbool.h>`, `<stddef.h>` and `<stdint.h>`.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void *dado_rt__web_thread_spawn(void *(*fn)(void *), void *arg);
bool dado_rt__web_thread_join(void *handle);
int32_t dado_rt__web_thread_cores(void);
void dado_rt__web_thread_yield(void);
void dado_rt__web_thread_sleep(int64_t ns);
void *dado_rt__web_mutex_new(void);
void dado_rt__web_mutex_lock(void *m);
void dado_rt__web_mutex_unlock(void *m);
void dado_rt__web_mutex_free(void *m);
void *dado_rt__web_cond_new(void);
void dado_rt__web_cond_wait(void *c, void *m);
bool dado_rt__web_cond_wait_for(void *c, void *m, int64_t ms);
void dado_rt__web_cond_signal(void *c);
void dado_rt__web_cond_broadcast(void *c);
void dado_rt__web_cond_free(void *c);

/* The host's processor count, at least 1. */
int32_t dado_rt__web_thread_cores(void) {
    int32_t n = dado_rt__import_dado_cores();
    return n >= 1 ? n : 1;
}

#if defined(__wasm_atomics__)

/* How long a yield waits, in nanoseconds. */
#define DADO_RT__WEB_YIELD_NS 1000

/* A spawned thread's stack, in bytes: 1 MiB, smaller than the main thread's 8 MiB. */
#define DADO_RT__WEB_STACK ((size_t)1 << 20)

/* One spawned thread. Words 0 and 1 are read and written by the loader. */
typedef struct {
    uint32_t stack_top;
    int32_t state;
    void *(*fn)(void *);
    void *arg;
    void *stack;
    void *tls_block;
    void *tls;
} dado_rt__web_thread;

#define DADO_RT__WEB_RUNNING 0
#define DADO_RT__WEB_RETURNED 1
#define DADO_RT__WEB_TRAPPED 2

/* Synthesized by `wasm-ld` for a shared memory: sets `__tls_base` to `block`
   and copies the TLS initializers into it. */
extern void __wasm_init_tls(void *block);

/* Whether this thread may wait, asked of the host once per thread: every
   thread but a browser's main thread. -1 is not yet asked. */
static _Thread_local int32_t dado_rt__web_waits = -1;

static bool dado_rt__web_can_wait(void) {
    if (dado_rt__web_waits < 0) {
        dado_rt__web_waits = dado_rt__import_dado_can_wait() != 0;
    }
    return dado_rt__web_waits != 0;
}

/* The entry every worker calls by its table slot, on the stack the loader
   pointed `__stack_pointer` at. */
static void dado_rt__web_thread_main(dado_rt__web_thread *t) {
    __wasm_init_tls(t->tls);
    (void)t->fn(t->arg);
    __atomic_store_n(&t->state, DADO_RT__WEB_RETURNED, __ATOMIC_SEQ_CST);
    (void)__builtin_wasm_memory_atomic_notify(&t->state, UINT32_MAX);
}

static void dado_rt__web_thread_free(dado_rt__web_thread *t) {
    free(t->stack);
    free(t->tls_block);
    free(t);
}

void *dado_rt__web_thread_spawn(void *(*fn)(void *), void *arg) {
    size_t align = __builtin_wasm_tls_align();
    size_t size = __builtin_wasm_tls_size();
    dado_rt__web_thread *t = (dado_rt__web_thread *)malloc(sizeof *t);
    if (t == NULL) {
        return NULL;
    }
    if (align == 0) {
        align = 1;
    }
    t->fn = fn;
    t->arg = arg;
    t->state = DADO_RT__WEB_RUNNING;
    t->stack = malloc(DADO_RT__WEB_STACK);
    t->tls_block = malloc(size + align);
    if (t->stack == NULL || t->tls_block == NULL) {
        dado_rt__web_thread_free(t);
        return NULL;
    }
    t->tls = (void *)(((uintptr_t)t->tls_block + (align - 1)) & ~(uintptr_t)(align - 1));
    t->stack_top = (uint32_t)((uintptr_t)t->stack + DADO_RT__WEB_STACK);
    /* The record is complete before the host can start a worker reading it;
       the host call is not an atomic, so the fence says so. */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    if (dado_rt__import_dado_spawn((int32_t)(uintptr_t)&dado_rt__web_thread_main,
                                 (int32_t)(uintptr_t)t) != 0) {
        dado_rt__web_thread_free(t);
        return NULL;
    }
    return t;
}

bool dado_rt__web_thread_join(void *handle) {
    dado_rt__web_thread *t = (dado_rt__web_thread *)handle;
    int32_t state;
    if (t == NULL) {
        return false;
    }
    if (!dado_rt__web_can_wait()) {
        dado_rt__web_puts(2, "threads.join: this is a browser's main thread, which cannot wait "
                            "(memory.atomic.wait32 traps there), so the join is refused; join "
                            "from a worker, or have the thread set a flag the frame loop reads\n");
        return false;
    }
    while ((state = __atomic_load_n(&t->state, __ATOMIC_SEQ_CST)) == DADO_RT__WEB_RUNNING) {
        (void)__builtin_wasm_memory_atomic_wait32(&t->state, DADO_RT__WEB_RUNNING, -1);
    }
    if (state == DADO_RT__WEB_TRAPPED) {
        /* The thread trapped: natively its abort ended the process, and here
           the joiner ends the program the same way. */
        dado_rt__web_abort();
    }
    dado_rt__web_thread_free(t);
    return true;
}

/* A word nothing ever wakes, for the yield to wait on. */
static int32_t dado_rt__web_never;

/* Give up the processor for a moment. Wasm has no `sched_yield`, and the
   nearest thing a worker has is a wait with a timeout on a word nothing
   wakes: the engine blocks the thread for about the timeout, at least the
   kernel's timer slack (tens of microseconds on Linux), and another thread
   runs. Nothing where this thread may not wait. */
void dado_rt__web_thread_yield(void) {
    if (dado_rt__web_can_wait()) {
        (void)__builtin_wasm_memory_atomic_wait32(&dado_rt__web_never, 0, DADO_RT__WEB_YIELD_NS);
    }
}

/* Wait about `ns` nanoseconds on a word nothing wakes (`core:time.nap`),
   where this thread may; answers at once where it may not, and the caller
   spins on the clock. */
void dado_rt__web_thread_sleep(int64_t ns) {
    if (ns > 0 && dado_rt__web_can_wait()) {
        (void)__builtin_wasm_memory_atomic_wait32(&dado_rt__web_never, 0, ns);
    }
}

/* Wait while `*word == expected`, where this thread may; answers at once
   where it may not. */
static void dado_rt__web_wait(int32_t *word, int32_t expected) {
    if (dado_rt__web_can_wait()) {
        (void)__builtin_wasm_memory_atomic_wait32(word, expected, -1);
    }
}

static int32_t dado_rt__web_cas(int32_t *word, int32_t expected, int32_t desired) {
    (void)__atomic_compare_exchange_n(word, &expected, desired, false, __ATOMIC_SEQ_CST,
                                      __ATOMIC_SEQ_CST);
    return expected;
}

void dado_rt__web_mutex_lock(void *m) {
    int32_t *word = (int32_t *)m;
    int32_t c;
    if (word == NULL) {
        return;
    }
    if ((c = dado_rt__web_cas(word, 0, 1)) == 0) {
        return;
    }
    do {
        if (c == 2 || dado_rt__web_cas(word, 1, 2) != 0) {
            dado_rt__web_wait(word, 2);
        }
    } while ((c = dado_rt__web_cas(word, 0, 2)) != 0);
}

void dado_rt__web_mutex_unlock(void *m) {
    int32_t *word = (int32_t *)m;
    if (word == NULL) {
        return;
    }
    if (__atomic_fetch_sub(word, 1, __ATOMIC_SEQ_CST) != 1) {
        __atomic_store_n(word, 0, __ATOMIC_SEQ_CST);
        (void)__builtin_wasm_memory_atomic_notify(word, 1);
    }
}

void dado_rt__web_cond_wait(void *c, void *m) {
    int32_t *seq = (int32_t *)c;
    int32_t seen;
    if (seq == NULL || m == NULL) {
        return;
    }
    seen = __atomic_load_n(seq, __ATOMIC_SEQ_CST);
    dado_rt__web_mutex_unlock(m);
    dado_rt__web_wait(seq, seen);
    dado_rt__web_mutex_lock(m);
}

/* `cond_wait` for at most `ms` milliseconds: true when woken (or when the
   sequence moved before the wait began), false when the time ran out, `ms`
   is not positive, or this thread may not wait. */
bool dado_rt__web_cond_wait_for(void *c, void *m, int64_t ms) {
    int32_t *seq = (int32_t *)c;
    int32_t seen;
    int32_t woke;
    if (seq == NULL || m == NULL || ms <= 0 || !dado_rt__web_can_wait()) {
        return false;
    }
    seen = __atomic_load_n(seq, __ATOMIC_SEQ_CST);
    dado_rt__web_mutex_unlock(m);
    woke = __builtin_wasm_memory_atomic_wait32(seq, seen, ms * 1000000);
    dado_rt__web_mutex_lock(m);
    return woke != 2;
}

void dado_rt__web_cond_signal(void *c) {
    if (c != NULL) {
        (void)__atomic_fetch_add((int32_t *)c, 1, __ATOMIC_SEQ_CST);
        (void)__builtin_wasm_memory_atomic_notify((int32_t *)c, 1);
    }
}

void dado_rt__web_cond_broadcast(void *c) {
    if (c != NULL) {
        (void)__atomic_fetch_add((int32_t *)c, 1, __ATOMIC_SEQ_CST);
        (void)__builtin_wasm_memory_atomic_notify((int32_t *)c, UINT32_MAX);
    }
}

#else /* one thread */

/* Unreached: `dadoc` refuses a spawn a single-threaded build reaches. */
void *dado_rt__web_thread_spawn(void *(*fn)(void *), void *arg) {
    (void)fn;
    (void)arg;
    return NULL;
}

bool dado_rt__web_thread_join(void *handle) {
    (void)handle;
    return false;
}

/* One thread: nobody to yield to. */
void dado_rt__web_thread_yield(void) {}

/* One thread may not wait on unshared memory: `core:time.nap` spins. */
void dado_rt__web_thread_sleep(int64_t ns) {
    (void)ns;
}

/* With one thread a mutex is never contended: held is a flag. */
void dado_rt__web_mutex_lock(void *m) {
    if (m != NULL) {
        *(int32_t *)m = 1;
    }
}

void dado_rt__web_mutex_unlock(void *m) {
    if (m != NULL) {
        *(int32_t *)m = 0;
    }
}

/* Nobody else can signal: the wait returns at once, spuriously, which a
   condition wait may always do. */
void dado_rt__web_cond_wait(void *c, void *m) {
    (void)c;
    (void)m;
}

void dado_rt__web_cond_signal(void *c) {
    (void)c;
}

/* Nobody else can signal: the time runs out at once. */
bool dado_rt__web_cond_wait_for(void *c, void *m, int64_t ms) {
    (void)c;
    (void)m;
    (void)ms;
    return false;
}

void dado_rt__web_cond_broadcast(void *c) {
    (void)c;
}

#endif

/* A mutex and a condition variable are one word each, zeroed. */
void *dado_rt__web_mutex_new(void) {
    return calloc(1, sizeof(int32_t));
}

void dado_rt__web_mutex_free(void *m) {
    free(m);
}

void *dado_rt__web_cond_new(void) {
    return calloc(1, sizeof(int32_t));
}

void dado_rt__web_cond_free(void *c) {
    free(c);
}
