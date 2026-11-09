/* `#WEB`: the DadoScript engine (Rust's `rt/web/quickjs.c`), a unit of its own
   (it defines the types `quickjs.h` restates), over the host's `dados` calls. */
#include "web.h"
DADO_WASM_IMPORT("dados", "ctx_new") void dado_rt__import_dados_ctx_new(int32_t);
DADO_WASM_IMPORT("dados", "ctx_free") void dado_rt__import_dados_ctx_free(int32_t);
DADO_WASM_IMPORT("dados", "dup") void dado_rt__import_dados_dup(int32_t);
DADO_WASM_IMPORT("dados", "free") void dado_rt__import_dados_free(int32_t);
DADO_WASM_IMPORT("dados", "string") void dado_rt__import_dados_string(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "string16") void dado_rt__import_dados_string16(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "bigint") void dado_rt__import_dados_bigint(int32_t, int64_t, int32_t);
DADO_WASM_IMPORT("dados", "array") void dado_rt__import_dados_array(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "map_new") void dado_rt__import_dados_map_new(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "map_entries") void dado_rt__import_dados_map_entries(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "to_bigint") int32_t dado_rt__import_dados_to_bigint(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "to_cstring") int32_t dado_rt__import_dados_to_cstring(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "to_utf16") int32_t dado_rt__import_dados_to_utf16(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "is") int32_t dado_rt__import_dados_is(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "length") int32_t dado_rt__import_dados_length(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "global") void dado_rt__import_dados_global(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "get") void dado_rt__import_dados_get(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "set") int32_t dado_rt__import_dados_set(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "define") int32_t dado_rt__import_dados_define(int32_t, int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "get_index") void dado_rt__import_dados_get_index(int32_t, int32_t, int64_t, int32_t);
DADO_WASM_IMPORT("dados", "set_index") int32_t dado_rt__import_dados_set_index(int32_t, int32_t, int64_t, int32_t);
DADO_WASM_IMPORT("dados", "read_run") int32_t dado_rt__import_dados_read_run(int32_t, int32_t, int32_t, int32_t, int64_t);
DADO_WASM_IMPORT("dados", "array_from") void dado_rt__import_dados_array_from(int32_t, int32_t, int32_t, int64_t, int32_t);
DADO_WASM_IMPORT("dados", "call") void dado_rt__import_dados_call(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "construct") void dado_rt__import_dados_construct(int32_t, int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "cfunction") void dado_rt__import_dados_cfunction(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "exception") void dado_rt__import_dados_exception(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "throw") void dado_rt__import_dados_throw(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "error") void dado_rt__import_dados_error(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "module_new") int32_t dado_rt__import_dados_module_new(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "module_export") int32_t dado_rt__import_dados_module_export(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "module_set") int32_t dado_rt__import_dados_module_set(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "eval_module") void dado_rt__import_dados_eval_module(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "load_module") void dado_rt__import_dados_load_module(int32_t, int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "namespace") void dado_rt__import_dados_namespace(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "promise_state") int32_t dado_rt__import_dados_promise_state(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "promise_result") void dado_rt__import_dados_promise_result(int32_t, int32_t, int32_t);
DADO_WASM_IMPORT("dados", "hold") void dado_rt__import_dados_hold(int32_t);
DADO_WASM_IMPORT("dados", "counters") void dado_rt__import_dados_counters(int32_t, int32_t);
DADO_WASM_IMPORT("dados", "frame") void dado_rt__import_dados_frame(int32_t);
DADO_WASM_IMPORT("dados", "fn_take") void dado_rt__import_dados_fn_take(int32_t);
#include "rt/quickjs.c"
