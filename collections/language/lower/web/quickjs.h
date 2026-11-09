/* `#WEB`: the DadoScript engine API, as `rt/web/quickjs.c` declares it (its
   first part, restated: that file is its own unit, `engine.c`). */
#include <stdbool.h>
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

/* What the glue reads of a value, as QuickJS-ng's header spells it. */
#define JS_VALUE_GET_TAG(v) ((int32_t)(v).tag)
#define JS_VALUE_GET_INT(v) ((v).u.int32)
#define JS_VALUE_GET_FLOAT64(v) ((v).u.float64)
#define JS_UNDEFINED ((JSValue){ { .int32 = 0 }, JS_TAG_UNDEFINED })
#define JS_EXCEPTION ((JSValue){ { .int32 = 0 }, JS_TAG_EXCEPTION })

/* The engine's own calls `core:script` restates (defined in quickjs.c). */
void dado_rt__web_dados_hold(int32_t delta);
void dado_rt__web_dados_counters(void *ctx, int64_t *out);
void dado_rt__web_dados_frame(void *ctx);
void dado_rt__web_dados_fn_take(void *into);
void dado_rt__web_dados_map_new(JSContext *ctx, JSValue *out);
void dado_rt__web_dados_map_entries(JSContext *ctx, JSValue *v, JSValue *out);

/* QuickJS-ng calls the port's glue makes that Rust's does not. */
#define JS_ToCString(ctx, v) JS_ToCStringLen2((ctx), (size_t *)0, (v), false)
#define JS_IsString(v) (JS_VALUE_GET_TAG(v) == JS_TAG_STRING || JS_VALUE_GET_TAG(v) == JS_TAG_STRING_ROPE)
static inline int JS_ToInt64(JSContext *ctx, int64_t *pres, JSValue v) {
    int32_t t = JS_VALUE_GET_TAG(v);
    if (t == JS_TAG_INT) *pres = v.u.int32;
    else if (t == JS_TAG_FLOAT64) *pres = v.u.float64 != v.u.float64 ? 0 : (int64_t)v.u.float64;
    else if (t == JS_TAG_BIG_INT) return JS_ToBigInt64(ctx, pres, v);
    else *pres = 0;
    return 0;
}
