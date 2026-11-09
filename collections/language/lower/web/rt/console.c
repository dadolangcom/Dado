/* The `#WEB` runtime: the console, the reports, and the trap.
 *
 * A web program's standard output and standard error are one host call,
 * `dado.write(fd, pointer, length)`, imported by the compiler
 * (`dado_rt__import_dado_write`, declared above this file from the compiler's
 * import table). This file buffers in front of it, so a program printing a
 * character at a time does not cross into the host a character at a time:
 *
 *   * fd 1 is handed over when its buffer is full, when `main` returns (the
 *     loader calls the exported `dado_rt__web_flush`) and before a trap;
 *     and once that flush has run, at the end of every write: nothing flushes
 *     it again, so what is written after `main` returned — by a Dado function
 *     a page's callback calls, a partial line as well as a whole one — would
 *     otherwise wait for 4 KiB more and be lost with the page;
 *   * fd 2 is handed over at each newline, and when full — a report is read
 *     by a person as it happens, and nothing after a trap is written.
 *
 * `dado_rt__web_fd_write` is what `core:io`'s `#WEB` sinks call, so it is what
 * `core:fmt`'s printing functions reach. The rest is what the compiler's own
 * lowerings call on `#WEB` where a hosted build calls `<stdio.h>`: the
 * messages of `#panic`, `#assert`, `#expect` and `#breakpoint`, and the reports
 * of a failed bounds, division or walk check, all to fd 2 — and then
 * `dado_rt__web_abort`, which flushes both descriptors and executes wasm's
 * `unreachable`. The loader maps that trap to exit status 134, SIGABRT's
 * shell status, so a trap means on the web what `abort` means natively.
 *
 * `dado_rt__web_report` is the one formatted write, and it knows exactly the
 * conversions the compiler's reports spell — `%s`, `%lld`, `%llu`, `%#llx`
 * and `%%` — so a report reads byte for byte as `fprintf(stderr, …)` writes it
 * on a hosted build. It is not a `printf`, and nothing a program writes
 * reaches it as a format.
 *
 * **Threads.** In a threaded module (`__wasm_atomics__`) the console is one
 * lock's: every write, flush and report holds it, so a line one thread writes
 * is not interleaved with another's, as `<stdio.h>`'s own lock keeps a
 * `fwrite` or an `fprintf` whole. The lock is **recursive** — a report holds
 * it across the writes it is made of — and a spin, for the heap's reason: a
 * browser's main thread writes and may not wait. A thread is told apart by
 * the address of a `_Thread_local` byte, which each thread's own TLS block
 * gives it. In a single-threaded module the lock is nothing.
 *
 * Freestanding C11; the only headers are `<stdarg.h>`, `<stdbool.h>`,
 * `<stddef.h>` and `<stdint.h>`.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int64_t dado_rt__web_fd_write(int32_t fd, void *bytes, int64_t count);
void dado_rt__web_flush(void);
void dado_rt__web_puts(int32_t fd, const char *text);
void dado_rt__web_putc(int32_t fd, int byte);
void dado_rt__web_report(const char *format, ...);
__attribute__((noreturn)) void dado_rt__web_abort(void);

/* Bytes held per descriptor before they are handed to the host. */
#define DADO_RT__CONSOLE_CAP 4096u

static struct {
    unsigned char bytes[2][DADO_RT__CONSOLE_CAP];
    uint32_t used[2];
    /* Whether fd 1 is handed over at the end of each write: set by the first
       flush, when `main` has returned. */
    bool lines;
} dado_rt__console;

#if defined(__wasm_atomics__)
static _Thread_local unsigned char dado_rt__console_me;
static uintptr_t dado_rt__console_owner;
static uint32_t dado_rt__console_depth;

static void dado_rt__console_lock(void) {
    uintptr_t me = (uintptr_t)&dado_rt__console_me;
    uintptr_t free_word = 0;
    if (__atomic_load_n(&dado_rt__console_owner, __ATOMIC_RELAXED) == me) {
        dado_rt__console_depth++;
        return;
    }
    while (!__atomic_compare_exchange_n(&dado_rt__console_owner, &free_word, me, false,
                                        __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
        free_word = 0;
    }
    dado_rt__console_depth = 1;
}

static void dado_rt__console_unlock(void) {
    if (--dado_rt__console_depth == 0) {
        __atomic_store_n(&dado_rt__console_owner, (uintptr_t)0, __ATOMIC_RELEASE);
    }
}
#else
static void dado_rt__console_lock(void) {}
static void dado_rt__console_unlock(void) {}
#endif

/* Hand descriptor `fd`'s held bytes to the host. */
static void dado_rt__console_drain(int32_t fd) {
    uint32_t i = (uint32_t)(fd - 1);
    if (dado_rt__console.used[i] != 0) {
        dado_rt__import_dado_write(fd, (int32_t)(uintptr_t)dado_rt__console.bytes[i],
                                 (int32_t)dado_rt__console.used[i]);
        dado_rt__console.used[i] = 0;
    }
}

/* `count` bytes to descriptor 1 or 2 — any other descriptor is 2 — answering
   `count`: the host takes every byte it is handed. */
int64_t dado_rt__web_fd_write(int32_t fd, void *bytes, int64_t count) {
    const unsigned char *from = (const unsigned char *)bytes;
    uint32_t i;
    int64_t at;
    if (fd != 1) {
        fd = 2;
    }
    i = (uint32_t)(fd - 1);
    if (from == (const unsigned char *)0 || count <= 0) {
        return 0;
    }
    dado_rt__console_lock();
    for (at = 0; at < count; at++) {
        dado_rt__console.bytes[i][dado_rt__console.used[i]++] = from[at];
        if (dado_rt__console.used[i] == DADO_RT__CONSOLE_CAP || (fd == 2 && from[at] == '\n')) {
            dado_rt__console_drain(fd);
        }
    }
    if (fd == 1 && dado_rt__console.lines) {
        dado_rt__console_drain(1);
    }
    dado_rt__console_unlock();
    return count;
}

/* Both descriptors, standard output first. Exported: the loader calls it
   once `main` has returned, and from then on standard output is handed over
   at the end of every write. */
void dado_rt__web_flush(void) {
    dado_rt__console_lock();
    dado_rt__console.lines = true;
    dado_rt__console_drain(1);
    dado_rt__console_drain(2);
    dado_rt__console_unlock();
}

void dado_rt__web_puts(int32_t fd, const char *text) {
    int64_t n = 0;
    while (text[n] != '\0') {
        n++;
    }
    (void)dado_rt__web_fd_write(fd, (void *)(uintptr_t)text, n);
}

void dado_rt__web_putc(int32_t fd, int byte) {
    unsigned char b = (unsigned char)byte;
    (void)dado_rt__web_fd_write(fd, &b, 1);
}

/* `value` in `base` (10 or 16), to fd 2. */
static void dado_rt__console_digits(unsigned long long value, unsigned base) {
    char digits[24];
    int n = 0;
    do {
        digits[n++] = "0123456789abcdef"[value % base];
        value /= base;
    } while (value != 0);
    while (n > 0) {
        dado_rt__web_putc(2, digits[--n]);
    }
}

/* A report to fd 2, under exactly the conversions the compiler's reports
   spell. */
void dado_rt__web_report(const char *format, ...) {
    va_list args;
    const char *at = format;
    dado_rt__console_lock();
    va_start(args, format);
    while (*at != '\0') {
        if (*at != '%') {
            dado_rt__web_putc(2, *at++);
            continue;
        }
        at++;
        if (*at == '%') {
            dado_rt__web_putc(2, '%');
            at++;
        } else if (*at == 's') {
            dado_rt__web_puts(2, va_arg(args, const char *));
            at++;
        } else if (at[0] == 'l' && at[1] == 'l' && at[2] == 'd') {
            long long v = va_arg(args, long long);
            unsigned long long magnitude = (unsigned long long)v;
            if (v < 0) {
                dado_rt__web_putc(2, '-');
                magnitude = 0ull - magnitude;
            }
            dado_rt__console_digits(magnitude, 10);
            at += 3;
        } else if (at[0] == 'l' && at[1] == 'l' && at[2] == 'u') {
            dado_rt__console_digits(va_arg(args, unsigned long long), 10);
            at += 3;
        } else if (at[0] == '#' && at[1] == 'l' && at[2] == 'l' && at[3] == 'x') {
            unsigned long long v = va_arg(args, unsigned long long);
            /* C's `#` flag writes no prefix for zero. */
            if (v != 0) {
                dado_rt__web_puts(2, "0x");
            }
            dado_rt__console_digits(v, 16);
            at += 4;
        } else {
            /* Not a conversion the compiler spells: written as it stands. */
            dado_rt__web_putc(2, '%');
        }
    }
    va_end(args);
    dado_rt__console_unlock();
}

/* `abort`, on the web: what is held is handed over, then `unreachable`. */
__attribute__((noreturn)) void dado_rt__web_abort(void) {
    dado_rt__web_flush();
    __builtin_trap();
}

#undef DADO_RT__CONSOLE_CAP
