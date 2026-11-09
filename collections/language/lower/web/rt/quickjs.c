/* The `#WEB` runtime: the DadoScript engine's API, over the JavaScript engine the
 * module runs beside — "the engine on the browser's engine".
 *
 * Natively a program that hosts DadoScript scripts links QuickJS-ng, and
 * `core:script` and the compiler's script glue call QuickJS-ng's C API. On
 * the web there is no engine to link: a script is JavaScript, running on the
 * page's own engine (node standing in for a page). This file is the API those
 * callers use, declared as QuickJS-ng's header declares it and implemented
 * over host calls in the wasm import module `dados` (`tools/web/dados.mjs`
 * answers them), so `core:script` and every piece of generated glue compile
 * unchanged for `#WEB`. It is spliced into a `#WEB` program only when the
 * program reaches `foreign "quickjs.h"`, in place of the header.
 *
 * **Values.** A `JSValue` keeps the layout `core:script` restates — a payload
 * union and a 64-bit tag — and QuickJS-ng's tag numbers. An immediate (an
 * `int32`, a `float64`, a `bool`, `undefined`, `null`, the exception marker)
 * lives in the value itself. A string, a BigInt, a symbol or an object is the
 * index of a slot in the host's value table, in `u.int32`: the slot holds the
 * JavaScript value and a count of the references C holds, which
 * `JS_DupValue` and `JS_FreeValue` keep (one host call each), so a slot is
 * released — and its index reused — when C's last reference goes, exactly
 * when QuickJS-ng would free it. A module definition is a C pointer
 * (`JS_TAG_MODULE`), held by the context and freed with it.
 *
 * **Allocation.** Everything this file allocates — the runtime, each
 * context, module definitions, every C string an answer is copied into, and
 * the frame a call from JavaScript into C is laid out in — comes from the
 * runtime's `JSMallocFunctions`, as QuickJS-ng's does, so `core:script`'s
 * count of what its engine holds means what it means natively.
 *
 * **C function pointers are function-table slots.** A JavaScript function
 * `JS_NewCFunction2` makes calls the C function through the exported
 * trampoline `dados_call`, which the host hands the slot, the context and a
 * frame it laid out in memory this file allocated (`dados_alloc`): the struct
 * passing and returning — by pointer and through a hidden result pointer in
 * wasm32's C ABI — is done here, in C. Each call has its own frame, so a C
 * function may call into a script that calls another C function. The module
 * normaliser and loader are reached the same way (`dados_normalize`,
 * `dados_load`, `dados_init`), by the host as it links a module.
 *
 * **Modules are files.** A script's modules are never compiled from source on
 * the web: the build writes each beside the module as a `.js` file, and the
 * loader imports them all before `main` runs. `JS_Eval` of a module under
 * `JS_EVAL_FLAG_COMPILE_ONLY` therefore reads no source — it names the module
 * the host already holds — and `JS_LoadModule` links it: the normaliser and
 * the loader are called for it and for everything it imports, a C module's
 * init function runs, and the answer is a settled promise of its namespace.
 * Code evaluated from a string (a script's `JS_Eval`) is refused: a page under
 * a strict Content-Security-Policy cannot evaluate a string.
 *
 * **What has no web form.** `JS_ExecutePendingJob` runs nothing (Dado under
 * wasm cannot drain the page's microtask queue; the compiler refuses a call of
 * an `async` entry from Dado on the web) and `JS_UpdateStackTop` does nothing.
 * The interrupt handler is kept and polled by the scripts themselves (the
 * stop's poll, below).
 *
 * Freestanding C11; the only headers are `<stdarg.h>`, `<stdbool.h>`,
 * `<stddef.h>` and `<stdint.h>`.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct JSRuntime JSRuntime;
typedef struct JSContext JSContext;
typedef struct JSModuleDef JSModuleDef;

typedef union JSValueUnion {
    int32_t int32;
    double float64;
    void *ptr;
    int32_t short_big_int;
} JSValueUnion;

typedef struct JSValue {
    JSValueUnion u;
    int64_t tag;
} JSValue;

typedef struct JSMallocFunctions {
    void *(*js_calloc)(void *opaque, size_t count, size_t size);
    void *(*js_malloc)(void *opaque, size_t size);
    void (*js_free)(void *opaque, void *ptr);
    void *(*js_realloc)(void *opaque, void *ptr, size_t size);
    size_t (*js_malloc_usable_size)(const void *ptr);
} JSMallocFunctions;

/* QuickJS-ng's tag numbers. */
enum {
    JS_TAG_BIG_INT = -9,
    JS_TAG_SYMBOL = -8,
    JS_TAG_STRING = -7,
    JS_TAG_STRING_ROPE = -6,
    JS_TAG_MODULE = -3,
    JS_TAG_FUNCTION_BYTECODE = -2,
    JS_TAG_OBJECT = -1,
    JS_TAG_INT = 0,
    JS_TAG_BOOL = 1,
    JS_TAG_NULL = 2,
    JS_TAG_UNDEFINED = 3,
    JS_TAG_UNINITIALIZED = 4,
    JS_TAG_CATCH_OFFSET = 5,
    JS_TAG_EXCEPTION = 6,
    JS_TAG_SHORT_BIG_INT = 7,
    JS_TAG_FLOAT64 = 8
};

#define JS_EVAL_TYPE_GLOBAL (0 << 0)
#define JS_EVAL_TYPE_MODULE (1 << 0)
#define JS_EVAL_FLAG_COMPILE_ONLY (1 << 5)
/* The fork's flag: on the web the runtime's `__dados_*` functions are what
   the intrinsics are, so it changes nothing here. */
#define JS_EVAL_FLAG_DADOS (1 << 10)

#define JS_PROP_CONFIGURABLE (1 << 0)
#define JS_PROP_WRITABLE (1 << 1)
#define JS_PROP_ENUMERABLE (1 << 2)

/* The fork's interrupt answer that raises what the hook threw. */
#define JS_INTERRUPT_RAISE 2

typedef enum JSCFunctionEnum {
    JS_CFUNC_generic,
    JS_CFUNC_generic_magic,
    JS_CFUNC_constructor
} JSCFunctionEnum;

typedef enum JSPromiseStateEnum {
    JS_PROMISE_NOT_A_PROMISE = -1,
    JS_PROMISE_PENDING = 0,
    JS_PROMISE_FULFILLED = 1,
    JS_PROMISE_REJECTED = 2
} JSPromiseStateEnum;

typedef JSValue JSCFunction(JSContext *ctx, JSValue this_val, int argc, JSValue *argv);
typedef int JSInterruptHandler(JSRuntime *rt, void *opaque);
typedef char *JSModuleNormalizeFunc(JSContext *ctx, const char *module_base_name,
                                    const char *module_name, void *opaque);
typedef JSModuleDef *JSModuleLoaderFunc(JSContext *ctx, const char *module_name, void *opaque);
typedef int JSModuleInitFunc(JSContext *ctx, JSModuleDef *m);

JSRuntime *JS_NewRuntime2(const JSMallocFunctions *mf, void *opaque);
void JS_FreeRuntime(JSRuntime *rt);
void JS_UpdateStackTop(JSRuntime *rt);
void JS_SetInterruptHandler(JSRuntime *rt, JSInterruptHandler *cb, void *opaque);
/* Web only: the engine word the hook reads (`core:script`'s `Stopper.word`),
   which a script on a worker loads at every back-edge (`dados_word`). */
void JS_SetInterruptWord(JSRuntime *rt, uint32_t *word);
JSContext *JS_NewContext(JSRuntime *rt);
void JS_FreeContext(JSContext *ctx);
void JS_SetContextOpaque(JSContext *ctx, void *opaque);
void *JS_GetContextOpaque(JSContext *ctx);
void JS_SetModuleLoaderFunc(JSRuntime *rt, JSModuleNormalizeFunc *module_normalize,
                            JSModuleLoaderFunc *module_loader, void *opaque);
int JS_ExecutePendingJob(JSRuntime *rt, JSContext **pctx);
JSValue JS_Eval(JSContext *ctx, const char *input, size_t input_len, const char *filename,
                int eval_flags);
JSValue JS_EvalFunction(JSContext *ctx, JSValue fun_obj);
JSValue JS_LoadModule(JSContext *ctx, const char *basename, const char *filename);
JSValue JS_GetModuleNamespace(JSContext *ctx, JSModuleDef *m);
JSPromiseStateEnum JS_PromiseState(JSContext *ctx, JSValue promise);
JSValue JS_PromiseResult(JSContext *ctx, JSValue promise);
JSValue JS_GetGlobalObject(JSContext *ctx);
JSValue JS_GetPropertyStr(JSContext *ctx, JSValue this_obj, const char *prop);
int JS_SetPropertyStr(JSContext *ctx, JSValue this_obj, const char *prop, JSValue val);
int JS_DefinePropertyValueStr(JSContext *ctx, JSValue this_obj, const char *prop, JSValue val,
                              int flags);
JSValue JS_DupValue(JSContext *ctx, JSValue v);
void JS_FreeValue(JSContext *ctx, JSValue v);
bool JS_IsFunction(JSContext *ctx, JSValue val);
bool JS_IsError(JSValue val);
bool JS_IsArray(JSValue val);
bool JS_IsMap(JSValue val);
JSValue JS_Call(JSContext *ctx, JSValue func_obj, JSValue this_obj, int argc, JSValue *argv);
JSValue JS_CallConstructor(JSContext *ctx, JSValue func_obj, int argc, JSValue *argv);
JSValue JS_NewCFunction2(JSContext *ctx, JSCFunction *func, const char *name, int length,
                         JSCFunctionEnum cproto, int magic);
JSModuleDef *JS_NewCModule(JSContext *ctx, const char *name_str, JSModuleInitFunc *func);
int JS_AddModuleExport(JSContext *ctx, JSModuleDef *m, const char *name_str);
int JS_SetModuleExport(JSContext *ctx, JSModuleDef *m, const char *export_name, JSValue val);
JSValue JS_GetException(JSContext *ctx);
JSValue JS_Throw(JSContext *ctx, JSValue obj);
JSValue JS_ThrowReferenceError(JSContext *ctx, const char *fmt, ...);
JSValue JS_ThrowTypeError(JSContext *ctx, const char *fmt, ...);
JSValue JS_ThrowPlainError(JSContext *ctx, const char *fmt, ...);
JSValue JS_ThrowOutOfMemory(JSContext *ctx);
JSValue JS_NewBool(JSContext *ctx, bool val);
JSValue JS_NewInt32(JSContext *ctx, int32_t val);
JSValue JS_NewFloat64(JSContext *ctx, double val);
JSValue JS_NewBigInt64(JSContext *ctx, int64_t v);
JSValue JS_NewStringLen(JSContext *ctx, const char *str1, size_t len1);
JSValue JS_NewStringUTF16(JSContext *ctx, const uint16_t *buf, size_t len);
JSValue JS_NewArray(JSContext *ctx);
int JS_ToBigInt64(JSContext *ctx, int64_t *pres, JSValue val);
const char *JS_ToCStringLen2(JSContext *ctx, size_t *plen, JSValue val1, bool cesu8);
void JS_FreeCString(JSContext *ctx, const char *ptr);
const uint16_t *JS_ToCStringLenUTF16(JSContext *ctx, size_t *plen, JSValue val1);
void JS_FreeCStringUTF16(JSContext *ctx, const uint16_t *ptr);
int JS_GetLength(JSContext *ctx, JSValue obj, int64_t *pres);
JSValue JS_GetPropertyInt64(JSContext *ctx, JSValue this_obj, int64_t idx);
int JS_SetPropertyInt64(JSContext *ctx, JSValue this_obj, int64_t idx, JSValue val);
/* A run of scalars across in one host call each way: `kind`
 * names the run's C storage (`dado_rt__web_run_kind`'s numbering, which
 * `dados.read_run` and `dados.array_from` read). The reader answers 1 when it
 * copied the whole run and 0 when the generated glue must read it element by
 * element, which answers or raises as it always did. */
int32_t dado_rt__web_read_run(JSContext *ctx, JSValue v, int32_t kind, void *dst, int64_t n);
JSValue dado_rt__web_array_from(JSContext *ctx, int32_t kind, const void *src, int64_t n);
char *js_strdup(JSContext *ctx, const char *str);

/* The trampolines the host calls in through, exported. */
DADO_WASM_EXPORT("dados_call") void dado_rt__dados_call(JSCFunction *fn, JSContext *ctx,
                                                    JSValue *this_val, int32_t argc,
                                                    JSValue *argv, JSValue *out);
DADO_WASM_EXPORT("dados_alloc") void *dado_rt__dados_alloc(JSContext *ctx, int32_t size);
DADO_WASM_EXPORT("dados_free") void dado_rt__dados_free(JSContext *ctx, void *ptr);
DADO_WASM_EXPORT("dados_normalize") char *dado_rt__dados_normalize(JSContext *ctx, const char *base,
                                                              const char *name);
DADO_WASM_EXPORT("dados_load") JSModuleDef *dado_rt__dados_load(JSContext *ctx, const char *name);
DADO_WASM_EXPORT("dados_init") int32_t dado_rt__dados_init(JSContext *ctx, JSModuleDef *m);
DADO_WASM_EXPORT("dados_interrupt") int32_t dado_rt__dados_interrupt(JSContext *ctx);
DADO_WASM_EXPORT("dados_word") uint32_t *dado_rt__dados_word(JSContext *ctx);

struct JSRuntime {
    JSMallocFunctions mf;
    void *opaque;
    JSInterruptHandler *interrupt;
    void *interrupt_opaque;
    uint32_t *interrupt_word;
    JSModuleNormalizeFunc *normalize;
    JSModuleLoaderFunc *loader;
    void *loader_opaque;
};

struct JSContext {
    JSRuntime *rt;
    void *opaque;
    /* Every module definition made in this context, freed with it. */
    JSModuleDef *defs;
};

struct JSModuleDef {
    JSModuleDef *next;
    JSModuleInitFunc *init;
};

/* The host's error kinds (`dados.error`). */
#define DADO_RT__DADOS_PLAIN 0
#define DADO_RT__DADOS_TYPE 1
#define DADO_RT__DADOS_REFERENCE 2
#define DADO_RT__DADOS_MEMORY 3

/* What the host answers `dados.is` about. */
#define DADO_RT__DADOS_IS_FUNCTION 0
#define DADO_RT__DADOS_IS_ARRAY 1
#define DADO_RT__DADOS_IS_MAP 2
#define DADO_RT__DADOS_IS_ERROR 3

/* A pointer as the `i32` a host call takes, and back. */
#define DADO_RT__DADOS_P(p) ((int32_t)(uintptr_t)(p))

static JSValue dado_rt__dados_make(int64_t tag, int32_t payload) {
    JSValue v;
    v.u.float64 = 0.0;
    v.u.int32 = payload;
    v.tag = tag;
    return v;
}

/* Whether `v` is a slot of the host's value table. */
static bool dado_rt__dados_counted(JSValue v) {
    return v.tag == JS_TAG_STRING || v.tag == JS_TAG_STRING_ROPE || v.tag == JS_TAG_OBJECT ||
           v.tag == JS_TAG_BIG_INT || v.tag == JS_TAG_SYMBOL;
}

static void *dado_rt__dados_malloc(JSRuntime *rt, size_t size) {
    return rt->mf.js_malloc(rt->opaque, size);
}

static void dado_rt__dados_mfree(JSRuntime *rt, void *ptr) {
    if (ptr != NULL) {
        rt->mf.js_free(rt->opaque, ptr);
    }
}

static size_t dado_rt__dados_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

/* ── the runtime and its contexts ─────────────────────────────────────── */

JSRuntime *JS_NewRuntime2(const JSMallocFunctions *mf, void *opaque) {
    JSRuntime *rt = (JSRuntime *)mf->js_calloc(opaque, 1, sizeof(JSRuntime));
    if (rt == NULL) {
        return NULL;
    }
    rt->mf = *mf;
    rt->opaque = opaque;
    rt->interrupt = NULL;
    rt->interrupt_opaque = NULL;
    rt->interrupt_word = NULL;
    rt->normalize = NULL;
    rt->loader = NULL;
    rt->loader_opaque = NULL;
    return rt;
}

void JS_FreeRuntime(JSRuntime *rt) {
    if (rt != NULL) {
        rt->mf.js_free(rt->opaque, rt);
    }
}

void JS_UpdateStackTop(JSRuntime *rt) {
    (void)rt;
}

void JS_SetInterruptHandler(JSRuntime *rt, JSInterruptHandler *cb, void *opaque) {
    rt->interrupt = cb;
    rt->interrupt_opaque = opaque;
}

/* ── the stop's poll ──────────────────────────────────────────────────── */
/* Natively the engine calls the hook at its back-edges. On the web the
   script's own code polls (the check `dado_emit_js` writes into a loop of a
   module written for the web): the host calls `dados_interrupt` for the
   running context, which answers the hook's answer — 0 go on, 1 the
   uncatchable stop, `JS_INTERRUPT_RAISE` the exception the hook threw — and
   a script on a worker reads the word itself first, at `dados_word`. */

void JS_SetInterruptWord(JSRuntime *rt, uint32_t *word) {
    rt->interrupt_word = word;
}

int32_t dado_rt__dados_interrupt(JSContext *ctx) {
    JSRuntime *rt = ctx->rt;
    if (rt->interrupt == NULL) {
        return 0;
    }
    return rt->interrupt(rt, rt->interrupt_opaque);
}

uint32_t *dado_rt__dados_word(JSContext *ctx) {
    return ctx->rt->interrupt_word;
}

JSContext *JS_NewContext(JSRuntime *rt) {
    JSContext *ctx = (JSContext *)rt->mf.js_calloc(rt->opaque, 1, sizeof(JSContext));
    if (ctx == NULL) {
        return NULL;
    }
    ctx->rt = rt;
    ctx->opaque = NULL;
    ctx->defs = NULL;
    dado_rt__import_dados_ctx_new(DADO_RT__DADOS_P(ctx));
    return ctx;
}

void JS_FreeContext(JSContext *ctx) {
    JSModuleDef *m;
    if (ctx == NULL) {
        return;
    }
    dado_rt__import_dados_ctx_free(DADO_RT__DADOS_P(ctx));
    m = ctx->defs;
    while (m != NULL) {
        JSModuleDef *next = m->next;
        dado_rt__dados_mfree(ctx->rt, m);
        m = next;
    }
    dado_rt__dados_mfree(ctx->rt, ctx);
}

void JS_SetContextOpaque(JSContext *ctx, void *opaque) {
    ctx->opaque = opaque;
}

void *JS_GetContextOpaque(JSContext *ctx) {
    return ctx->opaque;
}

void JS_SetModuleLoaderFunc(JSRuntime *rt, JSModuleNormalizeFunc *module_normalize,
                            JSModuleLoaderFunc *module_loader, void *opaque) {
    rt->normalize = module_normalize;
    rt->loader = module_loader;
    rt->loader_opaque = opaque;
}

int JS_ExecutePendingJob(JSRuntime *rt, JSContext **pctx) {
    (void)rt;
    if (pctx != NULL) {
        *pctx = NULL;
    }
    return 0;
}

/* ── values ───────────────────────────────────────────────────────────── */

JSValue JS_DupValue(JSContext *ctx, JSValue v) {
    (void)ctx;
    if (dado_rt__dados_counted(v)) {
        dado_rt__import_dados_dup(v.u.int32);
    }
    return v;
}

/* `core:script`'s count of the Dado structures runs have lent the script,
 * kept by the page: an inline crossing settles the lends only while it is not
 * zero. */
void dado_rt__web_dados_hold(int32_t delta);
void dado_rt__web_dados_hold(int32_t delta) { dado_rt__import_dados_hold(delta); }

/* `core:script`'s crossing counters on the web: the page's counts for a VM
 * (crossings, and the inline sites' string bytes in and out, written as three
 * `int64_t`s at `out`), and the frame boundary that begins them again. */
void dado_rt__web_dados_counters(void *ctx, int64_t *out);
void dado_rt__web_dados_counters(void *ctx, int64_t *out) {
    dado_rt__import_dados_counters(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(out));
}
void dado_rt__web_dados_frame(void *ctx);
void dado_rt__web_dados_frame(void *ctx) { dado_rt__import_dados_frame(DADO_RT__DADOS_P(ctx)); }

/* `core:script`'s copy of a direct call's `string` answer:
 * the page encoded it and holds its bytes until they are written at `into`. */
void dado_rt__web_dados_fn_take(void *into);
void dado_rt__web_dados_fn_take(void *into) { dado_rt__import_dados_fn_take(DADO_RT__DADOS_P(into)); }

/* `core:script`'s maps: a new `Map` at `out`, and the array of a map's
 * `[key, value]` pairs `Array.from` makes. Natively they are made through the
 * engine's global object; a VM's globals here hold neither constructor. */
void dado_rt__web_dados_map_new(JSContext *ctx, JSValue *out);
void dado_rt__web_dados_map_new(JSContext *ctx, JSValue *out) {
    dado_rt__import_dados_map_new(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(out));
}
void dado_rt__web_dados_map_entries(JSContext *ctx, JSValue *v, JSValue *out);
void dado_rt__web_dados_map_entries(JSContext *ctx, JSValue *v, JSValue *out) {
    dado_rt__import_dados_map_entries(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(v),
                                      DADO_RT__DADOS_P(out));
}

void JS_FreeValue(JSContext *ctx, JSValue v) {
    (void)ctx;
    if (dado_rt__dados_counted(v)) {
        dado_rt__import_dados_free(v.u.int32);
    }
}

JSValue JS_NewBool(JSContext *ctx, bool val) {
    (void)ctx;
    return dado_rt__dados_make(JS_TAG_BOOL, val ? 1 : 0);
}

JSValue JS_NewInt32(JSContext *ctx, int32_t val) {
    (void)ctx;
    return dado_rt__dados_make(JS_TAG_INT, val);
}

JSValue JS_NewFloat64(JSContext *ctx, double val) {
    JSValue v;
    (void)ctx;
    v.u.float64 = val;
    v.tag = JS_TAG_FLOAT64;
    return v;
}

JSValue JS_NewBigInt64(JSContext *ctx, int64_t val) {
    JSValue v;
    dado_rt__import_dados_bigint(DADO_RT__DADOS_P(ctx), val, DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_NewStringLen(JSContext *ctx, const char *str1, size_t len1) {
    JSValue v;
    dado_rt__import_dados_string(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(str1), (int32_t)len1,
                               DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_NewStringUTF16(JSContext *ctx, const uint16_t *buf, size_t len) {
    JSValue v;
    dado_rt__import_dados_string16(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(buf), (int32_t)len,
                                 DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_NewArray(JSContext *ctx) {
    JSValue v;
    dado_rt__import_dados_array(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&v));
    return v;
}

int JS_ToBigInt64(JSContext *ctx, int64_t *pres, JSValue val) {
    return dado_rt__import_dados_to_bigint(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&val),
                                         DADO_RT__DADOS_P(pres));
}

const char *JS_ToCStringLen2(JSContext *ctx, size_t *plen, JSValue val1, bool cesu8) {
    int32_t len = 0;
    int32_t at = dado_rt__import_dados_to_cstring(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&val1),
                                                DADO_RT__DADOS_P(&len), cesu8 ? 1 : 0);
    if (plen != NULL) {
        *plen = at == 0 ? 0 : (size_t)len;
    }
    return (const char *)(uintptr_t)at;
}

void JS_FreeCString(JSContext *ctx, const char *ptr) {
    dado_rt__dados_mfree(ctx->rt, (void *)(uintptr_t)ptr);
}

const uint16_t *JS_ToCStringLenUTF16(JSContext *ctx, size_t *plen, JSValue val1) {
    int32_t len = 0;
    int32_t at = dado_rt__import_dados_to_utf16(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&val1),
                                              DADO_RT__DADOS_P(&len));
    if (plen != NULL) {
        *plen = at == 0 ? 0 : (size_t)len;
    }
    return (const uint16_t *)(uintptr_t)at;
}

void JS_FreeCStringUTF16(JSContext *ctx, const uint16_t *ptr) {
    dado_rt__dados_mfree(ctx->rt, (void *)(uintptr_t)ptr);
}

char *js_strdup(JSContext *ctx, const char *str) {
    size_t n = dado_rt__dados_strlen(str);
    char *out = (char *)dado_rt__dados_malloc(ctx->rt, n + 1);
    size_t i;
    if (out == NULL) {
        return NULL;
    }
    for (i = 0; i <= n; i++) {
        out[i] = str[i];
    }
    return out;
}

bool JS_IsFunction(JSContext *ctx, JSValue val) {
    (void)ctx;
    return val.tag == JS_TAG_OBJECT &&
           dado_rt__import_dados_is(DADO_RT__DADOS_P(&val), DADO_RT__DADOS_IS_FUNCTION) != 0;
}

bool JS_IsError(JSValue val) {
    return val.tag == JS_TAG_OBJECT &&
           dado_rt__import_dados_is(DADO_RT__DADOS_P(&val), DADO_RT__DADOS_IS_ERROR) != 0;
}

bool JS_IsArray(JSValue val) {
    return val.tag == JS_TAG_OBJECT &&
           dado_rt__import_dados_is(DADO_RT__DADOS_P(&val), DADO_RT__DADOS_IS_ARRAY) != 0;
}

bool JS_IsMap(JSValue val) {
    return val.tag == JS_TAG_OBJECT &&
           dado_rt__import_dados_is(DADO_RT__DADOS_P(&val), DADO_RT__DADOS_IS_MAP) != 0;
}

int JS_GetLength(JSContext *ctx, JSValue obj, int64_t *pres) {
    return dado_rt__import_dados_length(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&obj),
                                      DADO_RT__DADOS_P(pres));
}

/* ── properties ───────────────────────────────────────────────────────── */

JSValue JS_GetGlobalObject(JSContext *ctx) {
    JSValue v;
    dado_rt__import_dados_global(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_GetPropertyStr(JSContext *ctx, JSValue this_obj, const char *prop) {
    JSValue v;
    dado_rt__import_dados_get(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&this_obj), DADO_RT__DADOS_P(prop),
                            DADO_RT__DADOS_P(&v));
    return v;
}

/* Takes `val`, whether the write succeeds or not. */
int JS_SetPropertyStr(JSContext *ctx, JSValue this_obj, const char *prop, JSValue val) {
    int r = dado_rt__import_dados_set(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&this_obj),
                                    DADO_RT__DADOS_P(prop), DADO_RT__DADOS_P(&val));
    JS_FreeValue(ctx, val);
    return r;
}

/* Takes `val`, whether the definition succeeds or not. */
int JS_DefinePropertyValueStr(JSContext *ctx, JSValue this_obj, const char *prop, JSValue val,
                              int flags) {
    int r = dado_rt__import_dados_define(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&this_obj),
                                       DADO_RT__DADOS_P(prop), DADO_RT__DADOS_P(&val), flags);
    JS_FreeValue(ctx, val);
    return r;
}

JSValue JS_GetPropertyInt64(JSContext *ctx, JSValue this_obj, int64_t idx) {
    JSValue v;
    dado_rt__import_dados_get_index(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&this_obj), idx,
                                  DADO_RT__DADOS_P(&v));
    return v;
}

int32_t dado_rt__web_read_run(JSContext *ctx, JSValue v, int32_t kind, void *dst, int64_t n) {
    return dado_rt__import_dados_read_run(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&v), kind,
                                        DADO_RT__DADOS_P(dst), n);
}

JSValue dado_rt__web_array_from(JSContext *ctx, int32_t kind, const void *src, int64_t n) {
    JSValue v;
    dado_rt__import_dados_array_from(DADO_RT__DADOS_P(ctx), kind, DADO_RT__DADOS_P(src), n,
                                   DADO_RT__DADOS_P(&v));
    return v;
}

/* Takes `val`, whether the write succeeds or not. */
int JS_SetPropertyInt64(JSContext *ctx, JSValue this_obj, int64_t idx, JSValue val) {
    int r = dado_rt__import_dados_set_index(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&this_obj), idx,
                                          DADO_RT__DADOS_P(&val));
    JS_FreeValue(ctx, val);
    return r;
}

/* ── calls ────────────────────────────────────────────────────────────── */

JSValue JS_Call(JSContext *ctx, JSValue func_obj, JSValue this_obj, int argc, JSValue *argv) {
    JSValue v;
    dado_rt__import_dados_call(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&func_obj),
                             DADO_RT__DADOS_P(&this_obj), argc, DADO_RT__DADOS_P(argv),
                             DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_CallConstructor(JSContext *ctx, JSValue func_obj, int argc, JSValue *argv) {
    JSValue v;
    dado_rt__import_dados_construct(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&func_obj), argc,
                                  DADO_RT__DADOS_P(argv), DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_NewCFunction2(JSContext *ctx, JSCFunction *func, const char *name, int length,
                         JSCFunctionEnum cproto, int magic) {
    JSValue v;
    (void)cproto;
    dado_rt__import_dados_cfunction(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(func),
                                  DADO_RT__DADOS_P(name), length, magic, DADO_RT__DADOS_P(&v));
    return v;
}

void dado_rt__dados_call(JSCFunction *fn, JSContext *ctx, JSValue *this_val, int32_t argc,
                       JSValue *argv, JSValue *out) {
    *out = fn(ctx, *this_val, argc, argv);
}

void *dado_rt__dados_alloc(JSContext *ctx, int32_t size) {
    return dado_rt__dados_malloc(ctx->rt, size > 0 ? (size_t)size : 1);
}

void dado_rt__dados_free(JSContext *ctx, void *ptr) {
    dado_rt__dados_mfree(ctx->rt, ptr);
}

/* ── exceptions ───────────────────────────────────────────────────────── */

JSValue JS_GetException(JSContext *ctx) {
    JSValue v;
    dado_rt__import_dados_exception(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&v));
    return v;
}

/* Takes `obj`. */
JSValue JS_Throw(JSContext *ctx, JSValue obj) {
    dado_rt__import_dados_throw(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&obj));
    JS_FreeValue(ctx, obj);
    return dado_rt__dados_make(JS_TAG_EXCEPTION, 0);
}

/* The message of a `JS_Throw…Error`: the conversions `core:script` spells —
   `%s`, `%d`, `%i`, `%u`, `%c` and `%%` — into `out`, truncated at `cap - 1`
   bytes; answers the length. */
static int32_t dado_rt__dados_format(char *out, int32_t cap, const char *fmt, va_list ap) {
    int32_t n = 0;
    const char *f;
    for (f = fmt; *f != '\0'; f++) {
        char digits[12];
        int32_t k = 0;
        const char *s = NULL;
        if (*f != '%') {
            if (n < cap - 1) {
                out[n++] = *f;
            }
            continue;
        }
        f++;
        if (*f == 's') {
            s = va_arg(ap, const char *);
            if (s == NULL) {
                s = "(null)";
            }
        } else if (*f == 'd' || *f == 'i' || *f == 'u') {
            unsigned int u;
            bool negative = false;
            if (*f == 'u') {
                u = va_arg(ap, unsigned int);
            } else {
                int d = va_arg(ap, int);
                negative = d < 0;
                u = negative ? 0u - (unsigned int)d : (unsigned int)d;
            }
            do {
                digits[k++] = (char)('0' + u % 10u);
                u /= 10u;
            } while (u != 0u);
            if (negative && n < cap - 1) {
                out[n++] = '-';
            }
            while (k > 0 && n < cap - 1) {
                out[n++] = digits[--k];
            }
            continue;
        } else if (*f == 'c') {
            if (n < cap - 1) {
                out[n++] = (char)va_arg(ap, int);
            }
            continue;
        } else if (*f == '%') {
            if (n < cap - 1) {
                out[n++] = '%';
            }
            continue;
        } else {
            if (*f == '\0') {
                break;
            }
            continue;
        }
        while (*s != '\0' && n < cap - 1) {
            out[n++] = *s++;
        }
    }
    out[n] = '\0';
    return n;
}

static JSValue dado_rt__dados_throw_error(JSContext *ctx, int32_t kind, const char *fmt,
                                        va_list ap) {
    char text[1024];
    int32_t n = dado_rt__dados_format(text, (int32_t)sizeof text, fmt, ap);
    dado_rt__import_dados_error(DADO_RT__DADOS_P(ctx), kind, DADO_RT__DADOS_P(text), n);
    return dado_rt__dados_make(JS_TAG_EXCEPTION, 0);
}

JSValue JS_ThrowReferenceError(JSContext *ctx, const char *fmt, ...) {
    JSValue v;
    va_list ap;
    va_start(ap, fmt);
    v = dado_rt__dados_throw_error(ctx, DADO_RT__DADOS_REFERENCE, fmt, ap);
    va_end(ap);
    return v;
}

JSValue JS_ThrowTypeError(JSContext *ctx, const char *fmt, ...) {
    JSValue v;
    va_list ap;
    va_start(ap, fmt);
    v = dado_rt__dados_throw_error(ctx, DADO_RT__DADOS_TYPE, fmt, ap);
    va_end(ap);
    return v;
}

JSValue JS_ThrowPlainError(JSContext *ctx, const char *fmt, ...) {
    JSValue v;
    va_list ap;
    va_start(ap, fmt);
    v = dado_rt__dados_throw_error(ctx, DADO_RT__DADOS_PLAIN, fmt, ap);
    va_end(ap);
    return v;
}

JSValue JS_ThrowOutOfMemory(JSContext *ctx) {
    dado_rt__import_dados_error(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_MEMORY,
                              DADO_RT__DADOS_P("out of memory"), 13);
    return dado_rt__dados_make(JS_TAG_EXCEPTION, 0);
}

/* ── modules ──────────────────────────────────────────────────────────── */

/* A module definition, held by `ctx` until it is freed, and made known to
   the host under `name`: a C module when `init` is not null, else the module
   of that name the host holds — and null, with an exception pending, when
   it holds none. */
static JSModuleDef *dado_rt__dados_module(JSContext *ctx, const char *name, JSModuleInitFunc *init) {
    JSModuleDef *m = (JSModuleDef *)dado_rt__dados_malloc(ctx->rt, sizeof(JSModuleDef));
    if (m == NULL) {
        (void)JS_ThrowOutOfMemory(ctx);
        return NULL;
    }
    m->init = init;
    m->next = ctx->defs;
    ctx->defs = m;
    if (dado_rt__import_dados_module_new(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(m),
                                       DADO_RT__DADOS_P(name), init != NULL ? 1 : 0) == 0) {
        return NULL;
    }
    return m;
}

JSModuleDef *JS_NewCModule(JSContext *ctx, const char *name_str, JSModuleInitFunc *func) {
    return dado_rt__dados_module(ctx, name_str, func);
}

int JS_AddModuleExport(JSContext *ctx, JSModuleDef *m, const char *name_str) {
    return dado_rt__import_dados_module_export(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(m),
                                             DADO_RT__DADOS_P(name_str));
}

/* Takes `val`. */
int JS_SetModuleExport(JSContext *ctx, JSModuleDef *m, const char *export_name, JSValue val) {
    int r = dado_rt__import_dados_module_set(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(m),
                                           DADO_RT__DADOS_P(export_name), DADO_RT__DADOS_P(&val));
    JS_FreeValue(ctx, val);
    return r;
}

JSValue JS_Eval(JSContext *ctx, const char *input, size_t input_len, const char *filename,
                int eval_flags) {
    JSModuleDef *m;
    JSValue v;
    (void)input;
    (void)input_len;
    if ((eval_flags & JS_EVAL_TYPE_MODULE) == 0 || (eval_flags & JS_EVAL_FLAG_COMPILE_ONLY) == 0) {
        return JS_ThrowPlainError(ctx,
                                  "`%s` is code to evaluate from a string, and on the web a "
                                  "script's modules are files the page loads: nothing is "
                                  "evaluated from a string",
                                  filename);
    }
    m = dado_rt__dados_module(ctx, filename, NULL);
    if (m == NULL) {
        return dado_rt__dados_make(JS_TAG_EXCEPTION, 0);
    }
    v.u.float64 = 0.0;
    v.u.ptr = m;
    v.tag = JS_TAG_MODULE;
    return v;
}

JSValue JS_EvalFunction(JSContext *ctx, JSValue fun_obj) {
    JSValue v;
    if (fun_obj.tag != JS_TAG_MODULE) {
        JS_FreeValue(ctx, fun_obj);
        return JS_ThrowTypeError(ctx, "not a compiled module");
    }
    dado_rt__import_dados_eval_module(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(fun_obj.u.ptr),
                                    DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_LoadModule(JSContext *ctx, const char *basename, const char *filename) {
    JSValue v;
    dado_rt__import_dados_load_module(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(basename),
                                    DADO_RT__DADOS_P(filename), DADO_RT__DADOS_P(&v));
    return v;
}

JSValue JS_GetModuleNamespace(JSContext *ctx, JSModuleDef *m) {
    JSValue v;
    dado_rt__import_dados_namespace(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(m), DADO_RT__DADOS_P(&v));
    return v;
}

JSPromiseStateEnum JS_PromiseState(JSContext *ctx, JSValue promise) {
    if (promise.tag != JS_TAG_OBJECT) {
        return JS_PROMISE_NOT_A_PROMISE;
    }
    return (JSPromiseStateEnum)dado_rt__import_dados_promise_state(DADO_RT__DADOS_P(ctx),
                                                                 DADO_RT__DADOS_P(&promise));
}

JSValue JS_PromiseResult(JSContext *ctx, JSValue promise) {
    JSValue v;
    dado_rt__import_dados_promise_result(DADO_RT__DADOS_P(ctx), DADO_RT__DADOS_P(&promise),
                                       DADO_RT__DADOS_P(&v));
    return v;
}

char *dado_rt__dados_normalize(JSContext *ctx, const char *base, const char *name) {
    JSRuntime *rt = ctx->rt;
    if (rt->normalize != NULL) {
        return rt->normalize(ctx, base, name, rt->loader_opaque);
    }
    return js_strdup(ctx, name);
}

JSModuleDef *dado_rt__dados_load(JSContext *ctx, const char *name) {
    JSRuntime *rt = ctx->rt;
    if (rt->loader == NULL) {
        (void)JS_ThrowReferenceError(ctx, "could not load module '%s'", name);
        return NULL;
    }
    return rt->loader(ctx, name, rt->loader_opaque);
}

int32_t dado_rt__dados_init(JSContext *ctx, JSModuleDef *m) {
    return m->init != NULL ? m->init(ctx, m) : 0;
}
