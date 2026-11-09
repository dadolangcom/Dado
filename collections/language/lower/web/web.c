/* `#WEB`'s unit: the web runtime (`rt/`, copied from Rust's: bulk memory, heap, console, `%g`, libm,
   threads, exit), then the libc names the emitted C calls, over it. */
#include "web.h"
#include "rt/bulk.c"
#include "rt/heap.c"
DADO_WASM_EXPORT("dado_flush") void dado_rt__web_flush(void);
#include "rt/console.c"
#include "rt/fmt_g.c"
#include "rt/libm.c"
#include "rt/threads.c"
#include "rt/exit.c"

/* The clocks `core:time` reads; the host's calls. */
int64_t dado_rt__web_now_ns(void) { return dado_rt__import_dado_now_ns(); }
int64_t dado_rt__web_wall_ns(void) { return dado_rt__import_dado_wall_ns(); }

struct dado_web_file { int32_t fd; };
static struct dado_web_file dado_web_out = { 1 }, dado_web_err = { 2 };
FILE *const stdout = &dado_web_out;
FILE *const stderr = &dado_web_err;

void *aligned_alloc(size_t align, size_t size) {
    /* The heap aligns to 16; more is asked of no web program yet. */
    (void)align;
    return malloc(size);
}

size_t strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

int strcmp(const char *a, const char *b) {
    while (*a != '\0' && *a == *b) { a++; b++; }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

size_t fwrite(const void *p, size_t size, size_t n, FILE *f) {
    dado_rt__web_fd_write(f->fd, (void *)p, (int64_t)(size * n));
    return n;
}

int fputs(const char *s, FILE *f) {
    dado_rt__web_fd_write(f->fd, (void *)s, (int64_t)strlen(s));
    return 0;
}

int fputc(int c, FILE *f) {
    unsigned char b = (unsigned char)c;
    dado_rt__web_fd_write(f->fd, &b, 1);
    return c;
}

int putchar(int c) { return fputc(c, stdout); }

int fflush(FILE *f) {
    (void)f;
    dado_rt__web_flush();
    return 0;
}

/* The conversions the emitted C spells: %s %.*s %c %d %u %ld %lu %lld %llu
   %zu %x %llx %#llx %g %%. */
static int dado_web_vfprintf(FILE *f, const char *fmt, va_list ap) {
    char buf[64];
    int total = 0;
    for (const char *p = fmt; *p; p++) {
        if (*p != '%') { fputc(*p, f); total++; continue; }
        p++;
        int alt = 0, prec = -1, longs = 0;
        if (*p == '#') { alt = 1; p++; }
        if (*p == '.' && p[1] == '*') { prec = va_arg(ap, int); p += 2; }
        while (*p == 'l') { longs++; p++; }
        if (*p == 'z') { longs = 2; p++; }
        int n = 0;
        if (*p == 's') {
            const char *s = va_arg(ap, const char *);
            size_t k = prec >= 0 ? (size_t)prec : strlen(s);
            fwrite(s, 1, k, f);
            total += (int)k;
            continue;
        } else if (*p == 'c') {
            fputc(va_arg(ap, int), f);
            total++;
            continue;
        } else if (*p == 'd' || *p == 'i') {
            long long v = longs == 2 ? va_arg(ap, long long) : longs == 1 ? va_arg(ap, long) : va_arg(ap, int);
            unsigned long long u = v < 0 ? 0ull - (unsigned long long)v : (unsigned long long)v;
            if (v < 0) buf[n++] = '-';
            char d[20]; int m = 0;
            do { d[m++] = (char)('0' + u % 10); u /= 10; } while (u);
            while (m) buf[n++] = d[--m];
        } else if (*p == 'u' || *p == 'x') {
            unsigned long long u = longs == 2 ? va_arg(ap, unsigned long long) : longs == 1 ? va_arg(ap, unsigned long) : va_arg(ap, unsigned);
            unsigned base = *p == 'x' ? 16 : 10;
            if (alt && u != 0 && base == 16) { buf[n++] = '0'; buf[n++] = 'x'; }
            char d[20]; int m = 0;
            do { d[m++] = "0123456789abcdef"[u % base]; u /= base; } while (u);
            while (m) buf[n++] = d[--m];
        } else if (*p == 'g') {
            n = dado_rt__fmt_g(buf, va_arg(ap, double));
        } else if (*p == '%') {
            buf[n++] = '%';
        } else {
            buf[n++] = '%';
            buf[n++] = *p;
        }
        fwrite(buf, 1, (size_t)n, f);
        total += n;
    }
    return total;
}

int fprintf(FILE *f, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = dado_web_vfprintf(f, fmt, ap);
    va_end(ap);
    return n;
}

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = dado_web_vfprintf(stdout, fmt, ap);
    va_end(ap);
    return n;
}

__attribute__((noreturn)) void abort(void) { dado_rt__web_abort(); }

/* No process to end: what `exit` would say is the trap's. */
__attribute__((noreturn)) void exit(int status) {
    (void)status;
    dado_rt__web_abort();
}

static void (*dado_web_exits[32])(void);
static int dado_web_nexits;

int atexit(void (*f)(void)) {
    if (dado_web_nexits == 32) return -1;
    dado_web_exits[dado_web_nexits++] = f;
    return 0;
}

/* The module's `main`: the program's, then what `atexit` registered. With
   `main(argc, argv)` (DADO_WEB_ARGS), the loader builds argv first through
   the export `dado_argv` (Rust's `host.c` `argv`), from `dado.arg_get`. */
#ifdef DADO_WEB_ARGS
DADO_WASM_IMPORT("dado", "arg_get") int32_t dado_rt__import_dado_arg_get(int32_t, int32_t, int32_t);
DADO_WASM_EXPORT("dado_argv") char **dado_rt__web_argv(int32_t argc);
char **dado_rt__web_argv(int32_t argc) {
    if (argc < 0) return 0;
    char **argv = (char **)calloc((size_t)argc + 1u, sizeof(char *));
    if (argv == 0) return 0;
    for (int32_t i = 0; i < argc; i++) {
        int32_t n = dado_rt__import_dado_arg_get(i, 0, 0);
        if (n < 0) n = 0;
        char *text = (char *)malloc((size_t)n + 1u);
        if (text == 0) return 0;
        int32_t got = dado_rt__import_dado_arg_get(i, (int32_t)(uintptr_t)text, n);
        text[got < 0 ? 0 : (got < n ? got : n)] = 0;
        argv[i] = text;
    }
    return argv;
}
int dado_web_main(int argc, char **argv);
DADO_WASM_EXPORT("main") int dado_wasm_main(int argc, char **argv) {
    int status = dado_web_main(argc, argv);
#else
int dado_web_main(void);
DADO_WASM_EXPORT("main") int dado_wasm_main(void) {
    int status = dado_web_main();
#endif
    while (dado_web_nexits > 0) dado_web_exits[--dado_web_nexits]();
    return status;
}

/* clang's 128-bit multiply, which compiler-rt would supply: by 32-bit halves. */
__int128 __multi3(__int128 a, __int128 b) {
    uint64_t al = (uint64_t)a, ah = (uint64_t)((unsigned __int128)a >> 64);
    uint64_t bl = (uint64_t)b, bh = (uint64_t)((unsigned __int128)b >> 64);
    uint64_t a0 = al & 0xFFFFFFFFu, a1 = al >> 32, b0 = bl & 0xFFFFFFFFu, b1 = bl >> 32;
    uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
    uint64_t mid = (p00 >> 32) + (p01 & 0xFFFFFFFFu) + (p10 & 0xFFFFFFFFu);
    uint64_t hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32) + al * bh + ah * bl;
    uint64_t lo = (mid << 32) | (p00 & 0xFFFFFFFFu);
    return (__int128)(((unsigned __int128)hi << 64) | lo);
}

static void dado_web_put(char **o, size_t *room, const char *s, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (*room > 1) { **o = s[i]; (*o)++; (*room)--; }
    }
}

/* `snprintf` for the two forms the emitted C spells: `%g` and `0x%llx`. */
int snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char tmp[48];
    int n = 0;
    if (fmt[0] == '%' && fmt[1] == 'g' && fmt[2] == '\0') n = dado_rt__fmt_g(tmp, va_arg(ap, double));
    else {
        unsigned long long u = va_arg(ap, unsigned long long);
        char d[20];
        int m = 0;
        tmp[n++] = '0';
        tmp[n++] = 'x';
        do { d[m++] = "0123456789abcdef"[u % 16]; u /= 16; } while (u);
        while (m) tmp[n++] = d[--m];
    }
    va_end(ap);
    char *o = buf;
    size_t room = size;
    dado_web_put(&o, &room, tmp, (size_t)n);
    if (size > 0) *o = '\0';
    return n;
}
