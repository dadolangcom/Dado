/* `#WEB` (no C library): the libc names the emitted C calls, with C's
   prototypes, over the web runtime (`web.c`, the program's web unit). */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#define DADO_WEB 1
#define DADO_WASM_EXPORT(name) __attribute__((export_name(name)))
#define DADO_WASM_IMPORT(module, name) __attribute__((import_module(module), import_name(name)))
#define main dado_web_main
DADO_WASM_IMPORT("dado", "seed") int64_t dado_rt__import_dado_seed(void);
DADO_WASM_IMPORT("dado", "write") void dado_rt__import_dado_write(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dado", "now_ns") int64_t dado_rt__import_dado_now_ns(void);
DADO_WASM_IMPORT("dado", "wall_ns") int64_t dado_rt__import_dado_wall_ns(void);
DADO_WASM_IMPORT("dado", "spawn") int32_t dado_rt__import_dado_spawn(int32_t, int32_t);
DADO_WASM_IMPORT("dado", "can_wait") int32_t dado_rt__import_dado_can_wait(void);
DADO_WASM_IMPORT("dado", "cores") int32_t dado_rt__import_dado_cores(void);
DADO_WASM_IMPORT("dado", "exit") void dado_rt__import_dado_exit(int32_t);
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *old, size_t size);
void free(void *old);
void *aligned_alloc(size_t align, size_t size);
void *memset(void *dst, int byte, size_t n);
void *memcpy(void *restrict dst, const void *restrict src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
int memcmp(const void *a, const void *b, size_t n);
int strcmp(const char *a, const char *b);
size_t strlen(const char *s);
double sqrt(double x); double floor(double x); double ceil(double x); double trunc(double x);
double fabs(double x); double copysign(double x, double y); double exp(double x); double log(double x);
double log2(double x); double log10(double x); double pow(double x, double y); double sin(double x);
double cos(double x); double tan(double x); double acos(double x); double atan2(double y, double x);
double hypot(double x, double y); double fma(double x, double y, double z); double round(double x);
double fmod(double x, double y);
#define isnan(x) __builtin_isnan(x)
#define isinf(x) __builtin_isinf(x)
#define isfinite(x) __builtin_isfinite(x)
#define signbit(x) __builtin_signbit(x)
#define INFINITY __builtin_inff()
#define NAN __builtin_nanf("")
#define HUGE_VAL __builtin_huge_val()
typedef struct dado_web_file FILE;
extern FILE *const stdout;
extern FILE *const stderr;
int fprintf(FILE *f, const char *format, ...);
int printf(const char *format, ...);
int fputs(const char *s, FILE *f);
int fputc(int c, FILE *f);
int putchar(int c);
size_t fwrite(const void *p, size_t size, size_t n, FILE *f);
int fflush(FILE *f);
int snprintf(char *buf, size_t size, const char *format, ...);
__attribute__((noreturn)) void abort(void);
__attribute__((noreturn)) void exit(int status);
int atexit(void (*f)(void));
int64_t dado_rt__web_fd_write(int32_t fd, void *bytes, int64_t count);
void dado_rt__web_flush(void);
int64_t dado_rt__web_now_ns(void);
int64_t dado_rt__web_wall_ns(void);
int32_t dado_rt__web_thread_cores(void);
void *dado_rt__web_thread_spawn(void *(*fn)(void *), void *arg);
_Bool dado_rt__web_thread_join(void *handle);
void dado_rt__web_thread_yield(void);
void dado_rt__web_thread_sleep(int64_t ns);
void *dado_rt__web_mutex_new(void);
void dado_rt__web_mutex_lock(void *m);
void dado_rt__web_mutex_unlock(void *m);
void dado_rt__web_mutex_free(void *m);
void *dado_rt__web_cond_new(void);
void dado_rt__web_cond_wait(void *c, void *m);
_Bool dado_rt__web_cond_wait_for(void *c, void *m, int64_t ms);
void dado_rt__web_cond_signal(void *c);
void dado_rt__web_cond_broadcast(void *c);
void dado_rt__web_cond_free(void *c);
void dado_rt__web_exit_status(int32_t status);
