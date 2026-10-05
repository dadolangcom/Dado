<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:script

core:script — the native runtime a DadoScript program runs on: QuickJS-ng, the
fork, embedded.

    import "core:script"
    import "core:mem/libc"

    try script.Engine e = script.create(libc.allocator()) else code:
        return code
    defer script.destroy(e)
    script.add_module(e, "counter", COUNTER_JS)      // an embedded ES module
    try script.Handle m = script.load(e, "counter") else code:
        return code                                  // its text is on stderr
    script.arg_i64(e, 40)
    try script.Handle c = script.construct(e, m, "Counter") else code:
        return code
    script.arg_i64(e, 2)
    try script.call_method(e, c, "add") else code:
        return code
    try i64 n = script.result_i64(e) else code:
        return code

**What this package is.** The seam the generated glue of `#script_vm`,
`#script_new`, `#script_run` and member calls through a handle calls: an
engine on the host allocator, a registry of embedded ES modules that the engine's module loader reads by name, `print` and
`println` on the engine's output sink, and an uncaught exception's text on
its error sink; and the interrupt hook, with stopping and pausing a running
script (`stop.dado`); the sink stack each run pushes and pops
(`sinks.dado`); and the reserved codes a verb answers and the failure's text
`#script_error()` reads (`failure.dado`). It is not all of the runtime
library: the print suite's rendering rules arrive with the row that owns
them.

**The engine is a toolchain component, never compiled in a program's
build.** `quickjs.h` is not in this directory: it is the header of the
archive `./dado` links, built once per target triple, compiler and flags from
the pinned upstream and the fork's patch series (`tools/quickjs/build.sh`).
The build learns that it needs the archive from the compiler: a program whose
live set holds this package gets an `engine quickjs` line in its build
manifest, and the driver answers it with the archive's directory on the
include path and the archive on the link line. A program that never reaches
this package links nothing and stays one translation unit.

**Hand-restated, not generated, and checked all the same.** Every other
binding of a C library in this tree is `bindgen`'s output over a header in
the tree. This header is the output of the patch series applied to a pinned
upstream, and it lives beside the archive in the cache: a copy committed
here would be a second source of truth for a header that has one. So the
declarations below are the handful this package calls, restated by hand,
and the emitter's assertions check each against the real header on every
build — every signature's C type by `_Generic`, every restated struct's
layout by `sizeof`/`offsetof`, and every constant's value comes from C.

**No scripting on `#NONE`; on the web, the page's engine.** QuickJS-ng
needs a hosted C library even with its allocator routed through Dado's, so a
freestanding build that reaches this package is refused: the `foreign` block
names a header outside C's freestanding set (`ERR1032`), and the driver
refuses an `engine` line on such a target by the same reason. On the web a
script runs on the page's own engine and a web build links none:
`quickjs.h` is a header the compiler's web runtime supplies, implementing
every function below over the JavaScript engine beside the module, so this
package compiles unchanged there; its scripts are files beside the module,
never source an engine compiles.

**Values cross as scalars, and `int` in its tiers.** A DadoScript `int` is
exactly `i64`, held in the smallest canonical tier that fits: a 32-bit
integer, a double exact to 2⁵³ − 1, or a BigInt beyond. `arg_i64` builds
that tier and `result_i64` reads any of the three, so a value past 2⁵³
crosses both ways exactly. `f64`, `bool` and strings cross as themselves.

## Declarations

536 declarations, 273 public.

* `type Parking` — A script's error a callback met, waiting for the innermost thunk running…
* `JSValue thunk_left(^JSContext ctx, JSValue answer)` — A thunk is done: its depth let go of, and its answer given back — or the…
* `!rawptr take_callback(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param, bool kept)` — A pair's function, argument `i`: held by a handle — the VM's when the…
* `void drop_callback(^JSContext ctx, rawptr h, bool kept)` — The call is over: a handle made for it let go of, so a copy Dado kept…
* `!JSValue take_bare_callback(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param)` — A bare reference's function, argument `i`: checked to be one; the thunk…
* `!^JSContext callback_begin(rawptr h, ^JSValue fn, cstring label)` — A pair's trampoline coming in: the engine its handle names entered, and the…
* `!^JSContext callback_begin_bare(^JSContext ctx, cstring label)` — A bare reference's trampoline coming in: `ctx` is what its slot holds — nil…
* `!JSValue callback_call(^JSContext ctx, JSValue fn, i32 argc, ^JSValue argv, cstring label, bool owed)` — The function called with `argc` arguments, which this takes; its answer, a…
* `i32 callback_failed(^JSContext ctx, cstring label, bool owed)` — The answer a trampoline read refused, or the call's own failure: the…
* `void callback_end(^JSContext ctx)` — The trampoline is done with the engine: a safe point.
* `JSValue give_bytes(^JSContext ctx, rawptr bytes, i64 n)` — A `Sink`'s bytes, as the `string` a script's function takes: bytes out of…
* `type Counters: (i64 calls, i64 runs, i64 bytes_in, i64 bytes_out)` — What crossed a VM's seam since its frame began.
* `Counters counters(VM vm)` — `vm`'s counters since its frame began; all zero for a handle that holds no…
* `void dado_rt__web_dados_counters(rawptr ctx, ^i64 out)`
* `void dado_rt__web_dados_frame(rawptr ctx)`
* `void dado_rt__web_exit_status(i32 status)`
* `const i32 ERROR = -2147483647 - 1` — The script threw something that carries no code, or an `Error` whose code…
* `const i32 STOPPED = -2147483647` — The script was stopped: by `#script_stop`, during the run that answers it,…
* `const i32 STALE = -2147483646` — The handle reaches no script: it was never one, or `#delete` ended it or…
* `const i32 TYPE = -2147483645` — The script answered a value of another type than the entry returns.
* `const i32 NO_MEMORY = -2147483644` — The allocator refused, or one of the engine's tables is full: more than 16…
* `const i32 BUSY = -2147483643` — The engine is paused, or another thread is running it: the run, the…
* `const i32 LENT = -2147483642` — A structure a run lent the script was written, during the run, by a Dado…
* `const i32 RELOAD = -2147483641` — `#script_reload`'s new code was not swapped in — it could not be read, did…
* `const i32 THREAD = -2147483640` — A run, a construction, a field or a reload through a VM from a thread that…
* `const i32 OTHER_VM = -2147483639` — An instance crossed into a script on another VM than its own — an argument…
* `string8 failure_text()` — `#script_error()`: the text of the last failure a verb answered on this…
* `!void call_function(const rawptr table, const rawptr bound, const rawptr signals, i32 index, cstring module, cstring class, cstring function, bool seam)` — The calls, one per answer's type.
* `!i64 call_function_int(const rawptr table, const rawptr bound, const rawptr signals, i32 index, cstring module, cstring class, cstring function, bool seam)`
* `!f64 call_function_float(const rawptr table, const rawptr bound, const rawptr signals, i32 index, cstring module, cstring class, cstring function, bool seam)`
* `!bool call_function_bool(const rawptr table, const rawptr bound, const rawptr signals, i32 index, cstring module, cstring class, cstring function, bool seam)`
* `!ref([]char8) call_function_string(const rawptr table, const rawptr bound, const rawptr signals, i32 index, cstring module, cstring class, cstring function, bool seam)` — A `string` answer is a create verb's, as a member call's: a copy on the…
* `void dado_rt__web_dados_fn_take(rawptr into)`
* `!ref([]char8) function_text(i64 n)`
* `type Slot` — One handle's slot. A free slot is on the free list, by `next`.
* `i64 live_handles()` — How many handles are live, on every engine: what a test reads.
* `void inline_entered(^JSContext ctx)` — An inline crossing beginning while a run lent the script a Dado structure:
* `i32 inline_left(^JSContext ctx)` — After an inline crossing whose callee can reach `#emit`, the any-signal…
* `void dado_rt__web_dados_hold(i32 delta)`
* `^#c.char js_strdup(^JSContext ctx, cstring str)`
* `const i32 QUIT = 0` — The program ends with the failure's status.
* `const i32 CONTINUE = 1` — The call answers its type's zero value, and the program goes on.
* `void pend_policy(i32 policy)` — The next VM's policy, `on_error:`.
* `JSValue JS_EvalFunction(^JSContext ctx, JSValue fun_obj)`
* `JSValue JS_GetModuleNamespace(^JSContext ctx, ^JSModuleDef m)`
* `type Reloading` — What an engine keeps of its reloads: how many it has evaluated, which…
* `!void reload_engine(rawptr vm, cstring module, cstring class, string8 dir, cstring manifest)` — `#script_reload(vm, Class, dir)` under `try`: the class `class` of the…
* `type JSRuntime`
* `type JSContext`
* `type JSModuleDef`
* `cunion JSValueUnion`
* `type JSValue`
* `type JSMallocFunctions`
* `enum @c("JSCFunctionEnum") JSCFunctionEnum`
* `enum @c("JSPromiseStateEnum") JSPromiseStateEnum`
* `const i32 JS_TAG_BIG_INT` — The value tags this package reads. An anonymous enum in the header, so…
* `const i32 JS_TAG_SHORT_BIG_INT`
* `const i32 JS_TAG_INT`
* `const i32 JS_TAG_BOOL`
* `const i32 JS_TAG_UNDEFINED`
* `const i32 JS_TAG_FLOAT64`
* `const i32 JS_TAG_STRING`
* `const i32 JS_TAG_STRING_ROPE`
* `const i32 JS_TAG_OBJECT`
* `const i32 JS_TAG_EXCEPTION`
* `const i32 JS_EVAL_TYPE_MODULE`
* `const i32 JS_EVAL_FLAG_COMPILE_ONLY`
* `const i32 JS_EVAL_FLAG_DADOS` — The fork's flag (patch 0002): code evaluated with it compiles the…
* `^JSRuntime JS_NewRuntime2(^const JSMallocFunctions mf, rawptr opaque)`
* `void JS_FreeRuntime(^JSRuntime rt)`
* `void JS_UpdateStackTop(^JSRuntime rt)`
* `void JS_SetInterruptHandler(^JSRuntime rt, i32(^JSRuntime, rawptr) cb, rawptr opaque)` — The hook the engine calls at its polls (`stop.dado`), and the fork's…
* `const i32 JS_INTERRUPT_RAISE`
* `^JSContext JS_NewContext(^JSRuntime rt)`
* `void JS_FreeContext(^JSContext ctx)`
* `void JS_SetContextOpaque(^JSContext ctx, rawptr opaque)`
* `rawptr JS_GetContextOpaque(^JSContext ctx)`
* `void JS_SetModuleLoaderFunc(^JSRuntime rt, (^#c.char)(^JSContext, cstring, cstring, rawptr) module_normalize, (^JSModuleDef)(^JSContext, cstring, rawptr) module_loader, rawptr opaque)`
* `i32 JS_ExecutePendingJob(^JSRuntime rt, ^^JSContext pctx)`
* `JSValue JS_Eval(^JSContext ctx, cstring input, #c.size_t input_len, cstring filename, i32 eval_flags)`
* `JSValue JS_LoadModule(^JSContext ctx, cstring basename, cstring filename)`
* `JSPromiseStateEnum JS_PromiseState(^JSContext ctx, JSValue promise)`
* `JSValue JS_PromiseResult(^JSContext ctx, JSValue promise)`
* `JSValue JS_GetGlobalObject(^JSContext ctx)`
* `JSValue JS_GetPropertyStr(^JSContext ctx, JSValue this_obj, cstring prop)`
* `i32 JS_SetPropertyStr(^JSContext ctx, JSValue this_obj, cstring prop, JSValue val)`
* `i32 JS_DefinePropertyValueStr(^JSContext ctx, JSValue this_obj, cstring prop, JSValue val, i32 flags)` — A handle's mark (`handles.dado`): a property no script names, kept out…
* `const i32 JS_PROP_WRITABLE`
* `const i32 JS_PROP_CONFIGURABLE`
* `JSValue JS_DupValue(^JSContext ctx, JSValue v)` — A counted reference more: what a handle holds of an instance.
* `bool JS_IsFunction(^JSContext ctx, JSValue val)`
* `JSValue JS_Call(^JSContext ctx, JSValue func_obj, JSValue this_obj, i32 argc, ^JSValue argv)`
* `JSValue JS_CallConstructor(^JSContext ctx, JSValue func_obj, i32 argc, ^JSValue argv)`
* `JSValue JS_NewCFunction2(^JSContext ctx, JSValue(^JSContext, JSValue, i32, ^JSValue) func, cstring name, i32 length, JSCFunctionEnum cproto, i32 magic)`
* `^JSModuleDef JS_NewCModule(^JSContext ctx, cstring name_str, i32(^JSContext, ^JSModuleDef) func)`
* `i32 JS_AddModuleExport(^JSContext ctx, ^JSModuleDef m, cstring name_str)`
* `i32 JS_SetModuleExport(^JSContext ctx, ^JSModuleDef m, cstring export_name, JSValue val)`
* `JSValue JS_GetException(^JSContext ctx)`
* `JSValue JS_Throw(^JSContext ctx, JSValue obj)`
* `bool JS_IsError(JSValue val)`
* `JSValue JS_ThrowReferenceError(^JSContext ctx, cstring fmt, ...)`
* `JSValue JS_ThrowTypeError(^JSContext ctx, cstring fmt, ...)`
* `JSValue JS_ThrowPlainError(^JSContext ctx, cstring fmt, ...)`
* `void JS_FreeValue(^JSContext ctx, JSValue v)`
* `JSValue JS_NewBool(^JSContext ctx, bool val)`
* `JSValue JS_NewInt32(^JSContext ctx, i32 val)`
* `JSValue JS_NewFloat64(^JSContext ctx, f64 val)`
* `JSValue JS_NewBigInt64(^JSContext ctx, i64 v)`
* `JSValue JS_NewStringLen(^JSContext ctx, cstring str1, #c.size_t len1)`
* `i32 JS_ToBigInt64(^JSContext ctx, ^i64 pres, JSValue val)`
* `^const #c.char JS_ToCStringLen2(^JSContext ctx, ^#c.size_t plen, JSValue val1, bool cesu8)`
* `void JS_FreeCString(^JSContext ctx, cstring ptr)` — `const char *` in the header, which is what a `cstring` is in C: a…
* `^const u16 JS_ToCStringLenUTF16(^JSContext ctx, ^#c.size_t plen, JSValue val1)` — A string's UTF-16 code units, unmatched surrogates kept, and a string…
* `void JS_FreeCStringUTF16(^JSContext ctx, ^const u16 ptr)`
* `JSValue JS_NewStringUTF16(^JSContext ctx, ^const u16 buf, #c.size_t len)`
* `JSValue JS_NewArray(^JSContext ctx)` — Arrays, for the values that cross as one: a Dado slice, array or tuple.
* `bool JS_IsArray(JSValue val)`
* `i32 JS_GetLength(^JSContext ctx, JSValue obj, ^i64 pres)`
* `JSValue JS_GetPropertyInt64(^JSContext ctx, JSValue this_obj, i64 idx)`
* `i32 JS_SetPropertyInt64(^JSContext ctx, JSValue this_obj, i64 idx, JSValue val)`
* `bool JS_IsMap(JSValue val)`
* `JSValue JS_ThrowOutOfMemory(^JSContext ctx)`
* `const i32 THREW = 1` — The script threw and nothing caught it; the error's text and stack are on…
* `const i32 NO_SUCH = 2` — No module, export, class or method has that name.
* `type State` — One engine: a QuickJS-ng runtime and its one context, the allocator both…
* `type Engine: ^State` — An engine, as the address of its state.
* `type Handle: i32` — A value the engine holds for its caller: a module's namespace or an object.
* `type Module: (cstring name, cstring source)` — One embedded ES module: the name `import` and `load` reach it by, and its…
* `type Loaded: (cstring name, Handle ns)` — A module a VM has loaded for its host: its name, a constant of the…
* `type Bound: (cstring module, cstring name, JSValue(^JSContext, JSValue, i32, ^JSValue) thunk, i32 arity)` — One Dado function a script calls: the native module `dado:<package>` it is…
* `type Native: (^JSModuleDef def, i64 first)` — A native module made for an engine: its definition, and the row of the…
* `const i32 MAX_MODULES = 64`
* `const i32 MAX_HANDLES = 256`
* `const i32 MAX_ARGS = 16`
* `!Engine create((rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — A new engine on `alloc`, which everything it allocates comes from — the…
* `void destroy(Engine e)` — Release everything the engine holds, the runtime last, and then the…
* `void set_sinks(Engine e, Sink out, Sink err)` — Where `print` and `println` write, and where a failure's text goes.
* `(i64 blocks, i64 bytes) held(Engine e)` — The blocks and bytes the engine's runtime holds on its allocator right now.
* `!void add_module(Engine e, cstring name, cstring source)` — Register the ES module `source` under `name`, for `load` and for any…
* `!Handle load(Engine e, cstring name)` — Import the module `name` and evaluate it (and everything it imports) under…
* `void arg_i64(Engine e, i64 v)` — An `int`, in the smallest canonical tier that holds it.
* `void arg_f64(Engine e, f64 v)`
* `void arg_bool(Engine e, bool v)`
* `void arg_string(Engine e, string8 s)` — The string's bytes, read as UTF-8 and copied into the engine: bytes out…
* `!void call(Engine e, Handle module, cstring function)` — Call the function `module` exports as `function` with the pushed arguments;…
* `!Handle construct(Engine e, Handle module, cstring class)` — Construct `module`'s exported class `class` with the pushed arguments, and…
* `!void call_method(Engine e, Handle object, cstring method)` — Call `object`'s method `method` with the pushed arguments.
* `void release(Engine e, Handle h)` — Let go of a value the engine holds for the caller.
* `!i64 result_i64(Engine e)` — The last answer as an `int`, from whichever tier holds it.
* `!f64 result_f64(Engine e)`
* `!bool result_bool(Engine e)`
* `!string8 result_string(Engine e)` — The last answer as a string, borrowed: it is valid until the next call and…
* `const i32 UNTRIED = 70` — The status a program ends with when a script fails inside a run no `try`
* `!rawptr vm_create((rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc, const rawptr table, const rawptr bound, const rawptr signals)` — A host's `#script_vm(alloc)`: an engine on `alloc`, every row of `table`
* `!rawptr vm_create_on_heap(const rawptr table, const rawptr bound, const rawptr signals)` — `#script_vm()`, the allocator omitted: `#heap`, the long-lived root,…
* `void close(rawptr s)` — `#delete(s)`: on a VM's own handle, the engine and everything in its…
* `!rawptr new_instance(rawptr vm, cstring module, cstring class)` — `#script_new(vm, Class, ..args)` under `try`: an instance of `class`,…
* `rawptr new_instance_or_quit(rawptr vm, cstring module, cstring class, cstring label)` — `#script_new` with no `try` around it: a failure ends the program with the…
* `i64 run_ready(rawptr s, bool answers)` — `#script_run(c)`: the instance's `ready`, with `main`'s rules — blocking,…
* `void push_int(rawptr s, i64 v)` — The next `run`'s arguments, pushed in order, each as its DadoScript type: `int`
* `void push_float(rawptr s, f64 v)`
* `void push_bool(rawptr s, bool v)`
* `void push_string(rawptr s, string8 v)`
* `void push_instance(rawptr s, rawptr v)` — An instance, by its handle: the run checks it once it has entered — a…
* `!void run(rawptr s, cstring entry)` — A member call `g.entry(…)` under `try` (and `#script_run(c)`, `ready`):
* `!i64 run_int(rawptr s, cstring entry)`
* `!f64 run_float(rawptr s, cstring entry)`
* `!bool run_bool(rawptr s, cstring entry)`
* `!ref([]char8) run_string(rawptr s, cstring entry)` — A `string` answer is a create verb's: a copy of the engine's string, on the…
* `!rawptr run_instance(rawptr s, cstring entry, cstring class)` — An instance answered: the handle Dado holds it by — the one it already has…
* `!i64 get_int(rawptr s, cstring field)` — `g.field`, read: the field of the instance the handle holds, read as its…
* `!f64 get_float(rawptr s, cstring field)`
* `!bool get_bool(rawptr s, cstring field)`
* `!ref([]char8) get_string(rawptr s, cstring field)`
* `!rawptr get_instance(rawptr s, cstring field, cstring class)`
* `!void put_field(rawptr s, cstring field)` — `g.field = v`: the one pushed argument written into the field.
* `void run_or_quit(rawptr s, cstring entry)` — A member call with no `try` around it: the same calls, and a failure ends…
* `i64 run_int_or_quit(rawptr s, cstring entry)`
* `f64 run_float_or_quit(rawptr s, cstring entry)`
* `bool run_bool_or_quit(rawptr s, cstring entry)`
* `ref([]char8) run_string_or_quit(rawptr s, cstring entry)` — A stopped run's empty string is still the caller's to `#delete`.
* `rawptr run_instance_or_quit(rawptr s, cstring entry, cstring class)` — A stopped run's instance is nil, a handle that holds no script.
* `i64 get_int_or_quit(rawptr s, cstring field)` — A field read is never under `try`: these are the only reads there are.
* `f64 get_float_or_quit(rawptr s, cstring field)`
* `bool get_bool_or_quit(rawptr s, cstring field)`
* `ref([]char8) get_string_or_quit(rawptr s, cstring field)`
* `rawptr get_instance_or_quit(rawptr s, cstring field, cstring class)`
* `void put_or_quit(rawptr s, cstring field)`
* `!i64 take_int(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param)` — Argument `i` as an `int`, from whichever of its three tiers holds it.
* `!f64 take_float(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param)` — Argument `i` as a `float`; an `int` in any tier is read as the nearest…
* `!bool take_bool(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param)`
* `!string8 take_string(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param)` — Argument `i` as a `string8`: the engine's UTF-8 copy, borrowed until…
* `void drop_string(^JSContext ctx, string8 s)`
* `!char32 take_char32(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param)` — Argument `i` as a `char32`: a string of exactly one code point, which is…
* `!char16 take_char16(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param)` — Argument `i` as a `char16`: a string of exactly one UTF-16 code unit —…
* `JSValue give_int(^JSContext ctx, i64 v)` — An `int`, in the smallest canonical tier that holds it.
* `JSValue give_float(^JSContext ctx, f64 v)`
* `JSValue give_bool(^JSContext ctx, bool v)`
* `JSValue give_string(^JSContext ctx, string8 s)` — A copy of `s`'s bytes, read as UTF-8: the Dado function's string may be…
* `JSValue give_owned_string(^JSContext ctx, ref([]char8) s)` — A Dado function's new string, copied into the engine and then ended on the…
* `JSValue give_char32(^JSContext ctx, char32 c)` — A character as a one-code-point string. A value that is no Unicode scalar…
* `JSValue give_char16(^JSContext ctx, char16 c)` — A `char16` as a string of that one UTF-16 code unit, a lone surrogate…
* `!rawptr take_instance(^JSContext ctx, ^JSValue argv, i32 i, cstring function, cstring param, cstring class)` — Argument `i` as an instance: the handle Dado holds it by (`adopt`), one…
* `JSValue give_instance(^JSContext ctx, cstring function, rawptr h)` — A Dado function's handle answered to the script: the instance itself. A…
* `JSValue give_nothing()` — What a thunk of a Dado function returning nothing answers.
* `JSValue raised()` — What a thunk answers once an exception is pending: the engine's…
* `JSValue failed(^JSContext ctx, cstring function, i32 code)` — A failable Dado function's failure, thrown into the script as an `Error`
* `void thunk_entered(^JSContext ctx)` — The thunk side's counter: a thunk of a bound Dado function is running, so…
* `type Lend: (void(rawptr) settle, rawptr next)` — A lend waiting for its fingerprint: the generated glue's frame begins with…
* `rawptr lend_script(^JSContext ctx, rawptr lend)` — A thunk lending Dado a script's structure for the call: `lend` goes on the…
* `void unlend_script(^JSContext ctx, rawptr was)` — The call returned: a lend still waiting is taken off, and one settled is…
* `rawptr lend_host(^JSContext ctx, rawptr lend)` — A run lending the script a Dado structure: as `lend_script`, on the engine…
* `void unlend_host(^JSContext ctx, rawptr was)`
* `void clear_exception(^JSContext ctx)` — Give back an error a fingerprint's reading left pending: a lend whose…
* `JSValue lent_written(^JSContext ctx, cstring function, cstring param)` — What a thunk throws when a script's structure it lent Dado was written…
* `i32 lent_changed(^JSContext ctx, cstring label, cstring structure)` — What a run says when a Dado structure it lent the script was written during…
* `!i64 read_int(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)` — `v` as an `int`, wherever in an argument or an answer it is: `what` names…
* `!f64 read_float(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)`
* `!bool read_bool(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)`
* `!string8 read_string(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)` — `v` as a `string8`: the engine's UTF-8 copy, borrowed until `drop_string`;…
* `!ref([]char8) read_owned_string(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)` — `v` as a new string a Dado host owns: a copy on the ambient allocator.
* `!char32 read_char32(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)` — `v` as a `char32`: a string of exactly one code point.
* `!char16 read_char16(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)` — `v` as a `char16`: a string of exactly one UTF-16 code unit.
* `!i64 read_length(^JSContext ctx, JSValue v, cstring function, cstring what, ^i64 path, i32 depth)` — How many elements the array `v` holds; anything but an array throws.
* `!void read_exactly(^JSContext ctx, JSValue v, i64 n, cstring function, cstring what, ^i64 path, i32 depth)` — That `v` is an array of exactly `n` elements — a Dado `[N]T` or tuple takes…
* `!JSValue element(^JSContext ctx, JSValue v, i64 i)` — Element `i` of the array `v`, held until `free_value` lets go of it.
* `void free_value(^JSContext ctx, JSValue v)`
* `!rawptr scratch(^JSContext ctx, u64 bytes)` — A block for the call, zeroed, on the engine's allocator; `unscratch` gives…
* `void unscratch(^JSContext ctx, rawptr p, u64 bytes)`
* `i32 out_of_memory(^JSContext ctx)` — What a reader answers when the ambient allocator refuses a block for an…
* `JSValue new_array(^JSContext ctx)` — A new, empty array.
* `!void put(^JSContext ctx, JSValue arr, i64 i, JSValue v)` — `arr[i] = v`, taking `v`. Either one already the exception marker — a…
* `u64 mix(u64 h, u64 bits)` — A fingerprint of a value's bytes, for the lend lock: FNV-1a over 64-bit…
* `u64 mix_float(u64 h, f64 v)`
* `u64 mix_string(u64 h, string8 s)`
* `!^JSContext begin_run(rawptr s)` — The engine the handle holds, entered for a run of its instance, its sinks…
* `!^JSContext begin_new(rawptr vm, cstring module)` — As `begin_run`, for `#script_new`: the VM's handle, entered with `module`
* `void push_value(^JSContext ctx, JSValue v)` — The entered run's argument, built by a generated writer or a `give_`
* `!void call_entered(rawptr s, cstring entry)` — The entered run's call: the method `entry` of the instance `s` holds, with…
* `!rawptr construct_entered(^JSContext ctx, cstring module, cstring class)` — The entered construction: an instance of `class`, exported by the loaded…
* `!i64 answer_int(^JSContext ctx)` — The entered run's scalar answer, as the entry's return type says.
* `!f64 answer_float(^JSContext ctx)`
* `!bool answer_bool(^JSContext ctx)`
* `!ref([]char8) answer_string(^JSContext ctx)` — A copy of the answer on the ambient allocator, as `run_string`'s.
* `JSValue answer_of(^JSContext ctx)` — The entered run's answer, borrowed, for a generated reader.
* `i32 answer_refused(^JSContext ctx, i32 code)` — The answer a generated reader refused: its error, pending, written to the…
* `void end_run(^JSContext ctx)` — The entered run leaving: the last thing its helper does with the engine.
* `void quit_untried(rawptr s, cstring entry, cstring verb, i32 code)` — A container run or construction no `try` was written around, failed: as…
* `i32 exit_status(i32 code)` — What a program ends with when a script's failure reaches a member call or…
* `const i32 MAX_SIGNALS = 64` — How many signals of a program scripts may connect to: a VM's connections…
* `type SignalRow: (cstring name, i32(^JSContext, JSValue, ^i64) deliver, rawptr owner, i64 index)` — One signal a script connects to, as the glue's table lays it out: its…
* `type SignalTable: (rawptr flags, rawptr bits, i64 count, rawptr rows)` — The glue's table: the program's flag word and bitmap, and its rows in…
* `type Connections` — A VM's connections: a handler per row it connected, which rows those are,…
* `JSValue connect_signal(^JSContext ctx, JSValue handler, i64 row)` — A script's `connect(sig, handler)`: the glue's thunk for the signal at row…
* `i32 deliver_one(^JSContext ctx, JSValue handler, i32 argc, ^JSValue argv, cstring signal)` — One message into a handler, from a signal's generated drain: `argv` its…
* `i64 pump_engine(rawptr vm)` — `#script_pump(vm)`: the host's frame boundary — the VM entered, and every…
* `const i32 MAX_NEST = 64` — How deep runs on one engine nest: a run made from a Dado function a script…
* `type SinkStack` — The sinks the runs in progress on an engine displaced, one pair a run, the…
* `Sink scope_out()` — The output sink a `using out:` scope put in force on this thread — nil…
* `void scope_set_out(Sink s)` — `s` in force as this thread's scoped output sink: a scope's own on…
* `Sink scope_err()` — The error sink a `using err:` scope put in force on this thread.
* `void scope_set_err(Sink s)` — `s` in force as this thread's scoped error sink.
* `void pend_out(Sink s)` — The next verb's output sink, `out:`.
* `void pend_err(Sink s)` — The next verb's error sink, `err:`.
* `type Stopper` — What a stop, a pause and the runs of one engine share: the word the hook…
* `void JS_SetInterruptWord(^JSRuntime rt, ^u32 word)`
* `void stop_engine(rawptr vm, i64 timeout)` — `#script_stop(vm, timeout)`: ask the script running on the handle's engine…
* `void pause_engine(rawptr vm)` — `#script_pause(vm)`: the run in progress on the handle's engine waits at its…
* `void resume_engine(rawptr vm)` — `#script_resume(vm)`: a paused engine goes on.
* `bool engine_running(rawptr vm)` — `#script_is_running(vm)`: whether a run is in progress on the handle's…
* `bool engine_paused(rawptr vm)` — `#script_is_paused(vm)`: whether the handle's engine is paused.
* `rawptr typed_entered(^JSContext ctx)` — A typed thunk coming in: counted, its depth raised, and any lend a run…
* `JSValue typed_left(rawptr engine, ^JSContext ctx, JSValue answer)` — A typed thunk leaving with `answer`: `thunk_left` when a script's error…
