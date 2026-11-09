<!-- dadoc 1.0.0-rc.10 -->
<!-- self-hosted (tools/dadoc) -->
# language:check

Allocators: the two words, `#default`/`#heap`/`#caller` as values, a
verb's allocator argument, `using`, `#new`, and `#caller`'s hand-over.

## Declarations

1506 declarations, 921 public.

* `const u8 B_AL_GET = 90`
* `const u8 B_AL_HEAP = 91`
* `const u8 B_AL_PUSH = 92`
* `const u8 B_AL_POP = 93`
* `const u8 B_AL_NEW = 94`
* `const u8 B_AL_COPY = 95`
* `const u8 B_AL_FORMAT = 96`
* `` const string8 AL_REPAIR = "write an allocator: a value of a declaration that takes `mem.Allocator` on, the two words `(proc, data)` its `.data()` answers, or a rung — `#default`, `#return`, `#caller` or `#heap`" ``
* `const [5]string8 RUNGS = ("#default", "#heap", "#caller", "#stack", "#return")`
* `Val alloc_name(^World w, ^Body b, u32 n, i64 k)` — `RUNGS[k]` where a value is wanted.
* `u32 alloc_arg(^World w, ^Body b, u32 g, string8 verb)` — A verb's allocator argument `g`: a rung constant, or the two words.
* `u32 entry_ambient(^World w, ^Body b, u32 n)` — The ambient this body was entered with: what a `#caller` callee is handed.
* `bool names_caller(^World w, u32 d)` — Whether `d`'s body names `#caller`.
* `void using_suite(^World w, ^Body b, u32 n)` — `using A:` over a suite: the ambient is `A` until the suite is left.
* `u32 push_using(^World w, ^Body b, u32 head, string8 what)` — The ambient saved and `head`'s allocator made it; NONE when reported.
* `bool alloc_trait(^World w, u32 d)`
* `bool allocator_of(^World w, u32 ty)` — Whether `ty` is an `Allocator` trait value (or a `^` to one) or a value…
* `Val alloc_of(^World w, ^Body b, Val v, u32 n)` — An `Allocator` value as the two words its `data()` answers.
* `void pop_using(^World w, ^Body b, u32 saved, u32 n)` — The ambient restored from `saved`.
* `Val new_(^World w, ^Body b, u32 n)` — `#new(T[, alloc])`.
* `Val copy_(^World w, ^Body b, u32 n, Val v, u32 asked)` — `#copy(x[, alloc])`: a `^T` or `ref T` to a `ref T`, a run to a `ref []T`.
* `string8 fn_label(^World w, u32 d)` — `d`'s name as messages spell it: `f`, or `T.m` for a method.
* `bool exported(^World w, u32 d)` — Whether function `d` is written `export`.
* `Val format_(^World w, ^Body b, u32 n, string8 said)` — `#format(args…[, alloc])`: the arguments rendered into a new `ref []char8`.
* `bool rendered(^World w, ^Body b, u32 vn, u32 ty, string8 verb)` — Whether `ty` has a rendering; ERR0504 at `vn` when not.
* `type Local`
* `type Head` — An open body (BLOCK, LOOP, IF) and, for a loop, where `continue` goes.
* `type Body`
* `const u8 V_VAL = 0` — The value of an expression: an operand and its type (NONE: already…
* `const u8 V_LIT = 1`
* `const u8 V_TYPE = 2`
* `const u8 V_PKG = 3`
* `const u8 V_FN = 4`
* `const u8 V_PACK = 5`
* `const u8 V_C = 6`
* `const u8 V_NIL = 7`
* `const u8 V_FLIT = 8`
* `type Val`
* `Val bad()`
* `Val value(u32 opd, u32 ty)`
* `syntax.Ast tree(^World w, ^Body b)`
* `void err(^World w, ^Body b, u32 n, u16 code, string8 msg, string8 help = "", string8 why = "")`
* `void err_tok(^World w, ^Body b, u32 n, u16 code, string8 msg, string8 help = "")`
* `string8 spell(^World w, ^Body b, u32 n)`
* `u32 temp(^World w, ^Body b, u32 ty, u32 n)`
* `u32 open(^World w, ^Body b, u8 o, u32 n, u32 cond, bool is_loop)`
* `void close(^World w, ^Body b, u32 h)`
* `void scope_in(^World w, ^Body b)`
* `void watch_region(^World w, ^Body b, u32 n)` — `--define=`/`--hover=`: the scope just opened covers node `n` (Rust's…
* `void scope_out(^World w, ^Body b)` — Ends a scope: its defers, LNT0412 for each name never read, then the…
* `u32 declare_local(^World w, ^Body b, u32 name, u32 ty, u32 n, u16 flags = ir.S_USER)` — A new local `name` of `ty`, declared by node `n` (its main token the name).
* `u32 find_local(^World w, ^Body b, u32 name, bool read)` — The innermost local `name`, marked read when `read`.
* `bool poisoned(^World w, ^Body b, u32 name)` — Whether the innermost local `name` is one whose declaration failed: its…
* `u32 local_type(^World w, u32 f, u32 sym)` — The type a `type X = T` statement in the open body binds `sym` to, or NONE.
* `void block(^World w, ^Body b, u32 n, bool own = true)` — A suite; `own` false runs it in the scope already open (a body's top…
* `void stmt(^World w, ^Body b, u32 n)`
* `string8 c_text(string8 s)` — A `@c_body` string's C: its quotes off, a multi-line one's first line off.
* `u32 zero(^World w, u32 ty)` — The zero of a type, as an operand: a composite's is the empty tuple.
* `void store(^World w, ^Body b, u32 p, u32 ty, Val v, u32 at, u32 n, string8 site = "the declared shape is")` — `p = v`, `v` converted to `ty` (ERR0504 when it does not fit).
* `u32 spill(^World w, ^Body b, Val v, u32 n)` — `v` as a place (a temporary when it is a constant).
* `Val local_val(^World w, ^Body b, u32 sl)` — A local as a value; a `^self` receiver reads as what it points at.
* `bool copied_self(^World w, ^Body b, u32 sl)` — A `self` receiver taken by value: writes to it are lost (ERR0615).
* `bool in_copied_self(^World w, ^Body b, u32 p)`
* `string8 COPY_LOST(^World w, ^Body b)`
* `string8 self_help(^World w, ^Body b)`
* `string8 src(^World w, ^Body b, u32 n)` — The source text of node `n`.
* `u32 const_member(^World w, u32 ty, i64 depth)` — A `const` slot or member at any depth of `ty`'s shape, not behind an…
* `Val place(^World w, ^Body b, u32 n, bool read)` — A place for an assignment: a local, or a slot of one.
* `u32 cond(^World w, ^Body b, u32 n, string8 what)` — A condition: a `bool`, else ERR0702 naming the construct.
* `void fall_check(^World w, ^Body b, u32 n, bool over_union)` — ERR0711-0713: each `fallthrough` of switch `n`'s arms that cannot fall.
* `bool ends_fall(^World w, ^Body b, u32 s)` — Whether suite `s` ends in `fallthrough`.
* `void bind(^World w, ^Body b, u32 n, u32 ty, u32 from)`
* `u32 arena_call(^World w, ^Body b, u32 e, string8 fname)` — `arena.<fname>(&x)` of `core:mem/arena`: `x`'s name, or NONE.
* `string8 KIND_HELD(string8 what)`
* `bool holds_kind(^World w, u32 t, i64 depth)` — Whether `t` holds a `type` value anywhere.
* `void not_a_value(^World w, ^Body b, u32 n, string8 what)`
* `bool undetermined(^World w, ^Body b, u32 n)` — A value written with no type of its own: literals, `nil`, and arithmetic,…
* `bool holds_str(^World w, ^Body b, u32 n)` — Whether `n` holds a string literal anywhere a literal's type could come from.
* `const u8 POS_VALUE = 0`
* `const u8 POS_CALL = 1`
* `const u8 POS_SPLAT = 2`
* `const u8 POS_PREDICATE = 3`
* `const u8 POS_TYPE = 4`
* `const u8 POS_MARKER = 5`
* `const u8 POS_PACK = 6`
* `const u8 FAM_NONE = 0`
* `const u8 FAM_DIAGNOSTICS = 1`
* `const u8 FAM_LAYOUTS = 2`
* `const u8 DS_SPELLED = 0`
* `const u8 DS_SAME = 1`
* `const u8 DS_ABSENT = 2`
* `const u8 PRINTS = 1` — A verb prints its arguments as `#render` does.
* `const u8 B_BREAKPOINT = 0` — Meanings: a verb (`B_*`, `dado_check::Builtin` order), or what else a name is (`M_*`).
* `const u8 B_PANIC = 1`
* `const u8 B_QUIT = 2`
* `const u8 B_TRANSPOSE = 3`
* `const u8 B_LEN = 4`
* `const u8 B_ASSERT = 5`
* `const u8 B_EXPECT = 6`
* `const u8 B_SIZE = 7`
* `const u8 B_ALIGN_OF = 8`
* `const u8 B_NEW = 9`
* `const u8 B_MAKE = 10`
* `const u8 B_RESIZE = 11`
* `const u8 B_DELETE = 12`
* `const u8 B_COPY = 13`
* `const u8 B_FORMAT = 14`
* `const u8 B_RENDER = 15`
* `const u8 B_APPEND = 16`
* `const u8 B_RESERVE = 17`
* `const u8 B_TRUNCATE = 18`
* `const u8 B_CAP = 19`
* `const u8 B_POPCOUNT = 20`
* `const u8 B_CLZ = 21`
* `const u8 B_CTZ = 22`
* `const u8 B_BSWAP = 23`
* `const u8 B_MUL_WIDE = 24`
* `const u8 B_ADD_OVERFLOW = 25`
* `const u8 B_SUB_OVERFLOW = 26`
* `const u8 B_MUL_OVERFLOW = 27`
* `const u8 B_ROTL = 28`
* `const u8 B_ROTR = 29`
* `const u8 B_ALIGN_FORWARD = 30`
* `const u8 B_ATOMIC_LOAD = 31`
* `const u8 B_ATOMIC_STORE = 32`
* `const u8 B_ATOMIC_EXCHANGE = 33`
* `const u8 B_ATOMIC_COMPARE_EXCHANGE = 34`
* `const u8 B_ATOMIC_FETCH_ADD = 35`
* `const u8 B_ATOMIC_FETCH_SUB = 36`
* `const u8 B_ATOMIC_FETCH_AND = 37`
* `const u8 B_ATOMIC_FETCH_OR = 38`
* `const u8 B_ATOMIC_FETCH_XOR = 39`
* `const u8 B_FENCE = 40`
* `const u8 B_NAMES = 41`
* `const u8 B_SEND = 42`
* `const u8 B_PEEK = 43`
* `const u8 B_DRAIN = 44`
* `const u8 B_RELEASE = 45`
* `const u8 B_SHADER_LOAD = 46`
* `const u8 B_SCRIPT_VM = 47`
* `const u8 B_SCRIPT_RUN = 48`
* `const u8 B_SCRIPT_NEW = 49`
* `const u8 B_SCRIPT_STOP = 50`
* `const u8 B_SCRIPT_PAUSE = 51`
* `const u8 B_SCRIPT_RESUME = 52`
* `const u8 B_SCRIPT_IS_RUNNING = 53`
* `const u8 B_SCRIPT_IS_PAUSED = 54`
* `const u8 B_SCRIPT_PUMP = 55`
* `const u8 B_SCRIPT_RELOAD = 56`
* `const u8 B_SCRIPT_ERROR = 57`
* `const u8 B_TRACE = 58`
* `const u8 B_LOCATION = 59`
* `const u8 M_DEFAULT_ALLOCATOR = 100`
* `const u8 M_STACK_RUNG = 101`
* `const u8 M_RETURN_ALLOCATOR = 102`
* `const u8 M_CALLER_ALLOCATOR = 103`
* `const u8 M_HEAP_ALLOCATOR = 104`
* `const u8 M_FOLDED_LENGTH = 105`
* `const u8 M_SPLAT_SLOTS = 106`
* `const u8 M_TYPE_PREDICATE = 107`
* `const u8 M_LAYOUT = 108`
* `const u8 M_NO_BOUNDS_CHECK = 109`
* `const u8 M_ANY_PACK = 110`
* `const u8 M_TYPE_SPELLING = 111`
* `const u8 M_STATIC_ASSERT = 112`
* `const u8 M_ASSERT_RAISES = 113`
* `const u8 M_CHANNEL = 114`
* `type Builtin` — `hi` 255 takes any number of arguments (a non-verb's arity is its usage's). `sub` is a predicate's or a layout's own number.
* `const string8 NO_ALLOCATOR = "an allocator is memory management, and the DadoScript runtime owns memory; a DadoScript value needs no allocator"`
* `const string8 NO_ATOMIC = "an atomic access is a thread primitive, and a script shares no memory between threads; write it in Dado"`
* `` const string8 NO_CHANNEL = "a channel is a lock-free mailbox between threads, a thread primitive, and a script has no threads; declare it and send into it in Dado, and a script consumes one by `receive(ch, handler)`" ``
* `` const string8 NO_FIT = "DadoScript's `int` wraps at 2^63 and never asks whether an operation fit; write the checked arithmetic in Dado" ``
* `` const string8 NO_HOST_VERB = "a `#script_*` verb is how a Dado host loads and runs a script; a script loads another by its path, `Script.load(path)`, and calls a method of what it answers" ``
* `const string8 NO_LAYOUT = "a layout is representation, which the DadoScript runtime owns; lay it out in Dado"`
* `` const string8 NO_SUCH_TYPE = "no DadoScript type satisfies it, so asking is refused rather than answered `false`" ``
* `` const string8 NO_TRACE = "a script's trace is its `Error`'s: `e.trace` is a `[]Location`, innermost first; the place it is written is `Location()`, as it is `#location()` in Dado" ``
* `` const string8 NO_WIDTH = "the answer depends on an integer's width, and DadoScript's `int` has one width it never writes; write it in Dado" ``
* `` const [88]Builtin BUILTINS = ( Builtin("#default", POS_VALUE, M_DEFAULT_ALLOCATOR, 0, FAM_NONE, 0, 0, 0, "#default", "names the ambient working allocator, as its two words `(proc, data)`", DS_ABSENT, "", NO_ALLOCATOR), Builtin("#stack", POS_VALUE, M_STACK_RUNG, 0, FAM_NONE, 0, 0, 0, "#stack", "puts an allocation in the enclosing frame, and is an allocator argument rather than an allocator value", DS_ABSENT, "", NO_ALLOCATOR), Builtin("#return", POS_VALUE, M_RETURN_ALLOCATOR, 0, FAM_NONE, 0, 0, 0, "#return", "allocates with the caller's lifetime, which is the ambient allocator seen from outside any suite that shadowed it", DS_ABSENT, "", NO_ALLOCATOR), Builtin("#caller", POS_VALUE, M_CALLER_ALLOCATOR, 0, FAM_NONE, 0, 0, 0, "#caller", "allocates with the lifetime the immediate caller was given, which is the ambient that caller saw before any `using` it opened around the call, handed over by a direct call and only to a function that names it", DS_ABSENT, "", NO_ALLOCATOR), Builtin("#heap", POS_VALUE, M_HEAP_ALLOCATOR, 0, FAM_NONE, 0, 0, 0, "#heap", "allocates from the root rung, which is C's heap behind the allocator interface and the nil allocator where a target has none", DS_ABSENT, "", NO_ALLOCATOR), Builtin("#make", POS_CALL, B_MAKE, 0, FAM_NONE, 2, 3, 0, "#make([]T, n[, alloc]) -> ref([]T)", "mints a `ref([]T)` of `n` zeroed elements, from `#default` or from the allocator written last", DS_SPELLED, "xs.resize(n)", "the allocation half is memory management, which the runtime owns, and goes; the sizing half is `resize` on a new `[]T xs = []`, whose new elements are zero"), Builtin("#new", POS_CALL, B_NEW, 0, FAM_NONE, 1, 2, 0, "#new(T[, alloc]) -> ref(T)", "mints a `ref(T)`, zeroed, from `#default` or from the allocator written last", DS_ABSENT, "", "mints a `ref(T)`, which is memory management; a DadoScript value is a value, so write the value, or construct a class, `Name(…)`"), Builtin("#delete", POS_CALL, B_DELETE, 0, FAM_NONE, 1, 2, 0, "#delete(r)", "ends the allocation a reference holds and leaves the place it named `nil`, so a second delete of that place does nothing; a copy of the reference is not nilled, and a second delete through one is a double free nothing detects. Of a script handle `#script_load` answered, it ends the script's engine and everything in it, and nils the place the same way", DS_SPELLED, "m.delete(k)", "over a map key it is the container method `m.delete(k)`; over a reference it ends an allocation, which DadoScript has none of, since the runtime owns memory"), Builtin("#resize", POS_CALL, B_RESIZE, 0, FAM_NONE, 2, 2, 0, "#resize(xs, n) -> ref([]T)", "grows or shrinks the block behind a `ref([]T)` within its own allocator, so it takes no allocator argument, and answers the handle, which may have moved: `xs = #resize(xs, n)`; a `nil` run takes the current allocator, and a growth the allocator declines traps", DS_SPELLED, "xs.resize(n)", "a container method, and new elements are zero; no allocator to name"), Builtin("#copy", POS_CALL, B_COPY, 0, FAM_NONE, 1, 2, 0, "#copy(x[, alloc]) -> ref(T) · ref([]T)", "duplicates a value into a new allocation, shallowly, answering a `ref(T)` for one and a `ref([]T)` for many", DS_SPELLED, "xs.copy()", "`[]T` and `{K: V}` are held by reference in DadoScript, so a copy is asked for, shallowly; `m.copy()` for a map"), Builtin("#format", POS_CALL, B_FORMAT, 0, FAM_NONE, 1, 255, PRINTS, "#format(args…[, alloc]) -> ref []char8", "renders its arguments as `#render` does and answers the bytes instead of writing them, allocated from `#default` or from the allocator written last", DS_SPELLED, "string(v)", "a value becomes text by the conversion shape `T(x)`, and texts join with `+`"), Builtin("#len", POS_CALL, B_LEN, 0, FAM_NONE, 1, 1, 0, "#len(xs) -> i64", "answers how many elements a container holds as an `i64`, whatever the container and wherever the answer stands, taking an inert place because it never runs its operand; over a channel it answers how many messages are queued, from any thread, exact only from the consumer while no producer runs", DS_SPELLED, "xs.len()", "a container method; a `string8`'s `#len` counts bytes, so it is `s.bytes().len()`, because `s.len()` counts UTF-16 code units"), Builtin("#size", POS_CALL, B_SIZE, 0, FAM_NONE, 1, 1, 0, "#size(T) · #size(x) -> u64", "emits a `sizeof`, so the byte count is the C compiler's answer as a `u64` and never folds", DS_ABSENT, "", "a byte count of a representation the DadoScript runtime owns, and it never folds; ask it in Dado"), Builtin("#alignof", POS_CALL, B_ALIGN_OF, 0, FAM_NONE, 1, 1, 0, "#alignof(T) -> u64", "emits an `_Alignof`, so the alignment is the C compiler's answer as a `u64` and never folds", DS_ABSENT, "", "a layout query about a representation the DadoScript runtime owns; ask it in Dado"), Builtin("#assert", POS_CALL, B_ASSERT, 0, FAM_DIAGNOSTICS, 1, 255, PRINTS, "#assert(cond[, message…])", "reports and aborts where its condition is false (on `#NONE`, which has no stream to report on, traps), and returns wherever the claim holds", DS_SPELLED, "assert cond, \"message\"", "a statement word, and a failed assert raises an `Error` a script may catch"), Builtin("#expect", POS_CALL, B_EXPECT, 0, FAM_DIAGNOSTICS, 2, 255, PRINTS, "#expect(cond, message…) -> bool", "reports where its condition is false and answers the claim, handing the decision back to the caller", DS_ABSENT, "", "reports and hands the verdict back, which is a testing library's verb; write `if !cond:` and `print`"), Builtin("#breakpoint", POS_CALL, B_BREAKPOINT, 0, FAM_DIAGNOSTICS, 0, 255, PRINTS, "#breakpoint([message…])", "writes its message to stderr and traps into a debugger", DS_ABSENT, "", "a debugger trap, which an editor's breakpoints serve; a script ordinarily needs none"), Builtin("#transpose", POS_CALL, B_TRANSPOSE, 0, FAM_NONE, 1, 1, 0, "#transpose(m)", "swaps the two levels of a nested tuple, so `[3](i32, f32)` and `([3]i32, [3]f32)` are each other's answer", DS_ABSENT, "", "reshapes a nested tuple's layout, numeric-kernel work that belongs in Dado; write it in Dado"), Builtin("#render", POS_CALL, B_RENDER, 0, FAM_NONE, 2, 2, PRINTS, "#render(v, sink)", "renders a value into the sink it is handed, and answers how many bytes that sink took", DS_SPELLED, "print(v)", "a sink is Dado's output machinery; a script writes to its output with `print`, and makes text with `string(v)`"), Builtin("#append", POS_CALL, B_APPEND, 0, FAM_NONE, 2, 2, 0, "#append(xs, v) · #append(xs, ..ys) -> ref([]T)", "puts one more element on the end of a run, or every element of `..ys`, growing the block where there is no room, and answers the handle, which may have moved: `xs = #append(xs, v)`; a `nil` run takes the current allocator, and a growth the allocator declines traps", DS_SPELLED, "xs.append(v)", "a container method; it answers nothing, since a DadoScript array always has room"), Builtin("#reserve", POS_CALL, B_RESERVE, 0, FAM_NONE, 2, 2, 0, "#reserve(xs, n) -> ref([]T)", "makes room for `n` elements without changing the length, never lowering a capacity, and answers the handle, which may have moved: `xs = #reserve(xs, n)`", DS_ABSENT, "", "capacity is representation, which the DadoScript runtime owns; a script's array grows as it is written"), Builtin("#cap", POS_CALL, B_CAP, 0, FAM_NONE, 1, 1, 0, "#cap(xs) -> i64", "answers how many elements a `ref([]T)`'s block holds, which is at least its length", DS_ABSENT, "", "capacity is representation, which the DadoScript runtime owns; a script asks `xs.len()`"), Builtin("#copy_into", POS_CALL, B_COPY_INTO, 0, FAM_NONE, 2, 2, 0, "#copy_into(dst, src) -> i64", "copies the shorter of two runs of one element type into the first, from its start, overlapping or not, and answers how many elements moved; `dst[i..<j] = src` copies an exact window instead", DS_ABSENT, "", "a script's `xs[i..<j]` answers a new array rather than a window into storage, so there is no existing run to copy into; build the array with its methods"), Builtin("#truncate", POS_CALL, B_TRUNCATE, 0, FAM_NONE, 2, 2, 0, "#truncate(xs, n)", "shortens a `ref([]T)` to `n` elements where it lives, keeping the capacity and asking no allocator; the dropped elements are zeroed and not destroyed, and `n` past the length traps", DS_SPELLED, "xs.resize(n)", "a DadoScript array keeps no capacity, so shortening one is `resize`, the one method that sets a length; a second word for it would be surface with nothing behind it"), Builtin("#popcount", POS_CALL, B_POPCOUNT, 0, FAM_NONE, 1, 1, 0, "#popcount(x)", "counts the set bits of an unsigned integer", DS_ABSENT, "", NO_WIDTH), Builtin("#clz", POS_CALL, B_CLZ, 0, FAM_NONE, 1, 1, 0, "#clz(x)", "counts the zero bits above the highest set one, and answers the operand's width for zero", DS_ABSENT, "", NO_WIDTH), Builtin("#ctz", POS_CALL, B_CTZ, 0, FAM_NONE, 1, 1, 0, "#ctz(x)", "counts the zero bits below the lowest set one, and answers the operand's width for zero", DS_ABSENT, "", NO_WIDTH), Builtin("#bswap", POS_CALL, B_BSWAP, 0, FAM_NONE, 1, 1, 0, "#bswap(x)", "reverses the bytes of a `u16`, `u32` or `u64`, and refuses the `u8` whose swap is the identity", DS_ABSENT, "", "reverses the bytes of a sized integer, which is representation; write it in Dado"), Builtin("#add_fits", POS_CALL, B_ADD_OVERFLOW, 0, FAM_NONE, 2, 2, 0, "#add_fits(a, b) -> (T sum, bool fits)", "answers the sum and whether it fit: the `bool` is true when the operation did **not** overflow, which is what the name says", DS_ABSENT, "", NO_FIT), Builtin("#sub_fits", POS_CALL, B_SUB_OVERFLOW, 0, FAM_NONE, 2, 2, 0, "#sub_fits(a, b) -> (T difference, bool fits)", "answers the difference and whether it fit, with `#add_fits`'s polarity", DS_ABSENT, "", NO_FIT), Builtin("#mul_fits", POS_CALL, B_MUL_OVERFLOW, 0, FAM_NONE, 2, 2, 0, "#mul_fits(a, b) -> (T product, bool fits)", "answers the product and whether it fit, with `#add_fits`'s polarity", DS_ABSENT, "", NO_FIT), Builtin("#LEN", POS_CALL, M_FOLDED_LENGTH, 0, FAM_NONE, 1, 1, 0, "#LEN(xs)", "asks `#len` and asserts the count is in the type, so a count that lives in the value is refused rather than read at run time; the answer is a number with no type of its own, which takes the type its position wants as an integer literal does", DS_SPELLED, "xs.len()", "in DadoScript `len()` folds wherever the count is in the type, so one spelling serves both halves of `#len`"), Builtin("#is_int", POS_PREDICATE, M_TYPE_PREDICATE, 0, FAM_NONE, 1, 1, 0, "where #is_int(T)", "holds in a `where` where the type is in the `int` category", DS_SPELLED, "T is int", "the shared infix `is`; `int` is DadoScript's one integer"), Builtin("#is_float", POS_PREDICATE, M_TYPE_PREDICATE, 1, FAM_NONE, 1, 1, 0, "where #is_float(T)", "holds in a `where` where the type is in the `float` category", DS_SPELLED, "T is float", "the shared infix `is`; `float` is DadoScript's one floating type"), Builtin("#is_number", POS_PREDICATE, M_TYPE_PREDICATE, 2, FAM_NONE, 1, 1, 0, "where #is_number(T)", "holds in a `where` where the type is in the `scalar` category", DS_SPELLED, "T is scalar", "the category `scalar` under its current name, with the shared infix `is`"), Builtin("#is_bool", POS_PREDICATE, M_TYPE_PREDICATE, 3, FAM_NONE, 1, 1, 0, "where #is_bool(T)", "holds in a `where` where the type is `bool`", DS_SPELLED, "T is bool", "the shared infix `is`"), Builtin("#is_char", POS_PREDICATE, M_TYPE_PREDICATE, 4, FAM_NONE, 1, 1, 0, "where #is_char(T)", "holds in a `where` where the type is `char8`, `char16` or `char32`", DS_ABSENT, "", "DadoScript has no character type, so no DadoScript type satisfies it and asking is refused; a character is a one-code-point `string`"), Builtin("#is_string", POS_PREDICATE, M_TYPE_PREDICATE, 5, FAM_NONE, 1, 1, 0, "where #is_string(T)", "holds in a `where` where the type is `string8`", DS_SPELLED, "T is string", "the shared infix `is`; `string` is DadoScript's one string type"), Builtin("#is_slice", POS_PREDICATE, M_TYPE_PREDICATE, 6, FAM_NONE, 1, 1, 0, "where #is_slice(T)", "holds in a `where` where the type is a `[]T` window rather than the two-slot shape it is represented as", DS_SPELLED, "T.is_slice()", "no category name means any `[]E`, so it is a built-in method on the type, and it folds"), Builtin("#is_map", POS_PREDICATE, M_TYPE_PREDICATE, 7, FAM_NONE, 1, 1, 0, "where #is_map(T)", "holds in a `where` where the type is a `{K: V}`", DS_SPELLED, "T.is_map()", "as `#is_slice`: a built-in method on the type"), Builtin("#is_ref", POS_PREDICATE, M_TYPE_PREDICATE, 8, FAM_NONE, 1, 1, 0, "where #is_ref(T)", "holds in a `where` where the type is a `ref(T)` or a `ref([]T)`", DS_ABSENT, "", NO_SUCH_TYPE), Builtin("#is_pointer", POS_PREDICATE, M_TYPE_PREDICATE, 9, FAM_NONE, 1, 1, 0, "where #is_pointer(T)", "holds in a `where` where the type is a `^T`", DS_ABSENT, "", NO_SUCH_TYPE), Builtin("#has_eq", POS_PREDICATE, M_TYPE_PREDICATE, 10, FAM_NONE, 1, 1, 0, "where #has_eq(T)", "holds in a `where` where `==` applies to the type — exactly where `a == a` compiles; a map key asks more, and that is `#is_key`", DS_SPELLED, "T has ==", "the shared `has` form asks the same"), Builtin("#is_key", POS_PREDICATE, M_TYPE_PREDICATE, 11, FAM_NONE, 1, 1, 0, "where #is_key(T)", "holds in a `where` where the type may key a `{K: V}` — its `==` is stable and Dado lays it out, so a float, a pointer, a slice, a `#c` scalar and any shape holding one do not hold", DS_SPELLED, "T.is_key()", "asks DadoScript's own key rule, which refuses mutable containers, so it is a built-in method on the type"), Builtin("#is_declared", POS_PREDICATE, M_TYPE_PREDICATE, 12, FAM_NONE, 1, 1, 0, "where #is_declared(T)", "holds in a `where` where the call bound the type by a declared name rather than as a shape written out; asking it instantiates the template once per declaration, which is how a template carries a declaration to one it calls", DS_ABSENT, "", "steers instantiation per declaration, and DadoScript generics are erased where they are emitted; write the template in Dado"), Builtin("#mul_wide", POS_CALL, B_MUL_WIDE, 0, FAM_NONE, 2, 2, 0, "#mul_wide(a, b) -> (i64 hi, u64 lo)", "answers the full 128-bit product of two `i64`s as the pair `(i64 hi, u64 lo)`", DS_ABSENT, "", "a 128-bit product is a width DadoScript never writes; write it in Dado"), Builtin("#panic", POS_CALL, B_PANIC, 0, FAM_DIAGNOSTICS, 1, 255, PRINTS, "#panic(message…)", "reports and aborts, and never returns", DS_SPELLED, "fail \"message\"", "a script never ends its host: DadoScript fails, and the failure can be caught"), Builtin("#rotl", POS_CALL, B_ROTL, 0, FAM_NONE, 2, 2, 0, "#rotl(x, n)", "turns a bit pattern left by `n` places, the bits leaving the top arriving at the bottom, with the count taken modulo the width", DS_ABSENT, "", NO_WIDTH), Builtin("#rotr", POS_CALL, B_ROTR, 0, FAM_NONE, 2, 2, 0, "#rotr(x, n)", "turns a bit pattern right by `n` places, with `#rotl`'s count rule", DS_ABSENT, "", NO_WIDTH), Builtin("#align_forward", POS_CALL, B_ALIGN_FORWARD, 0, FAM_NONE, 2, 2, 0, "#align_forward(p, n)", "answers the first address at or after a pointer that is a multiple of `n`, where a written `n` is a power of two", DS_ABSENT, "", "pointer arithmetic, and a script holds no address; write it in Dado"), Builtin("#atomic_load", POS_CALL, B_ATOMIC_LOAD, 0, FAM_NONE, 2, 2, 0, "#atomic_load(p, order) -> T", "reads the value at a `^T` atomically, under the ordering it is given", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_store", POS_CALL, B_ATOMIC_STORE, 0, FAM_NONE, 3, 3, 0, "#atomic_store(p, v, order)", "writes a value through a `^T` atomically, and answers `void`", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_exchange", POS_CALL, B_ATOMIC_EXCHANGE, 0, FAM_NONE, 3, 3, 0, "#atomic_exchange(p, v, order) -> T", "writes a value through a `^T` atomically and answers the value that was there", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_compare_exchange", POS_CALL, B_ATOMIC_COMPARE_EXCHANGE, 0, FAM_NONE, 6, 6, 0, "#atomic_compare_exchange(p, expected, desired, weak, success, failure) -> (T observed, bool swapped)", "writes `desired` through a `^T` where what is there is `expected`, and answers `(T observed, bool swapped)` either way", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_fetch_add", POS_CALL, B_ATOMIC_FETCH_ADD, 0, FAM_NONE, 3, 3, 0, "#atomic_fetch_add(p, v, order) -> T", "adds to what is at a `^T` atomically and answers the value that was there **before**", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_fetch_sub", POS_CALL, B_ATOMIC_FETCH_SUB, 0, FAM_NONE, 3, 3, 0, "#atomic_fetch_sub(p, v, order) -> T", "subtracts from what is at a `^T` atomically and answers the value that was there before", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_fetch_and", POS_CALL, B_ATOMIC_FETCH_AND, 0, FAM_NONE, 3, 3, 0, "#atomic_fetch_and(p, v, order) -> T", "ands into what is at a `^T` atomically and answers the value that was there before", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_fetch_or", POS_CALL, B_ATOMIC_FETCH_OR, 0, FAM_NONE, 3, 3, 0, "#atomic_fetch_or(p, v, order) -> T", "ors into what is at a `^T` atomically and answers the value that was there before", DS_ABSENT, "", NO_ATOMIC), Builtin("#atomic_fetch_xor", POS_CALL, B_ATOMIC_FETCH_XOR, 0, FAM_NONE, 3, 3, 0, "#atomic_fetch_xor(p, v, order) -> T", "xors into what is at a `^T` atomically and answers the value that was there before", DS_ABSENT, "", NO_ATOMIC), Builtin("#fence", POS_CALL, B_FENCE, 0, FAM_NONE, 1, 1, 0, "#fence(order)", "orders the accesses around it without being an access itself, so it takes an ordering and no location", DS_ABSENT, "", NO_ATOMIC), Builtin("#soa", POS_TYPE, M_LAYOUT, 0, FAM_LAYOUTS, 0, 0, 0, "#soa [N]T", "lays a `[N]T` out as one run per slot of the element, which is a layout and not a rewrite", DS_ABSENT, "", NO_LAYOUT), Builtin("#aos", POS_TYPE, M_LAYOUT, 1, FAM_LAYOUTS, 0, 0, 0, "#aos [N]T", "lays a `[N]T` out as one run of whole elements, which is the arrangement it already has", DS_ABSENT, "", NO_LAYOUT), Builtin("#no_bounds", POS_MARKER, M_NO_BOUNDS_CHECK, 0, FAM_NONE, 0, 0, 0, "xs[#no_bounds i]", "marks the operand it stands in front of — a subscript's index, or the value of a store into a bitfield — as one whose check the author is opting out of", DS_ABSENT, "", "opts one operation out of a safety check, and DadoScript is safe to the extent a language can be; write the unchecked loop in Dado"), Builtin("#slots", POS_SPLAT, M_SPLAT_SLOTS, 0, FAM_NONE, 1, 1, 0, "..#slots(v) · for name, value in #slots(v):", "names a value's slots: as a splat's operand, the positions `..` splices out of its one argument — a tuple's elements, and a tagged value as one; as what a `for` walks, the slots its declaration names, the body written out once per slot with each slot's name and value; refuses a `#soa`, whose slots are lanes", DS_SPELLED, "..v", "`f(..#slots(v))` is `f(..v)`: the splat is shared; in a `for` header, the walk over a value's slots is the built-in function `slots`, `for name, value in slots(v)`, over an instance of a class, `name` a `string` and `value` an `any`"), Builtin("#any", POS_PACK, M_ANY_PACK, 0, FAM_NONE, 0, 0, 0, "..#any xs", "written after a parameter's `..` in place of the element type: the pack takes whatever arguments the call writes, each of its own type, and the function is instantiated once per sequence of argument types", DS_SPELLED, "..any xs", "a pack of whatever the call writes is a variadic of checked `any`, boxed rather than instantiated per argument type"), Builtin("#names", POS_CALL, B_NAMES, 0, FAM_NONE, 1, 1, 0, "#names(T) · #names(v)", "answers the names a declared type's view binds to its slots, as `string8` constants, and refuses a shape no declaration was named for", DS_SPELLED, "T.names()", "a question about a type is a built-in method on it, and it folds"), Builtin("#spell", POS_CALL, M_TYPE_SPELLING, 0, FAM_NONE, 1, 1, 0, "#spell(T) -> string8", "folds a type — one written out, a `type` binding, or a type value — to its spelling as a `string8` constant, the one `--emit=types` and every diagnostic print", DS_SPELLED, "T.spell()", "a question about a type is a built-in method on it, and it folds"), Builtin("#static_assert", POS_CALL, M_STATIC_ASSERT, 0, FAM_NONE, 2, 2, 0, "#static_assert(COND, \"message\")", "written at package level, `#static_assert(COND, \"message\")`: folds `COND` as a `when` condition is folded, and refuses the build with the message, and where each operand's value came from, when it is `false`", DS_ABSENT, "", "a build invariant folded by the compiler is a compiler query; check at run time with `assert`, or state the invariant in Dado"), Builtin("#assert_raises", POS_CALL, M_ASSERT_RAISES, 0, FAM_NONE, 0, 255, 0, "#assert_raises · #assert_raises(f, g)", "written alone on the line above a function, or above a method in its type's body, `#assert_raises`, or at package level as a list of functions, `#assert_raises(dot, cross)`: promises that the function or method, or each named function of this package, and every function it reaches (a generic one at each instantiation), raises into a script — no `foreign` call but `<math.h>`'s correctly rounded `sqrt` and `sqrtf`, no `export`, `@symbol` or `@c_body`, no `#align_forward`, atomic, `#fence` or `#no_bounds`, and no value whose type is an address (`rawptr`, `cstring`, a `#c.*` scalar, an opaque C type, a C qualifier) — and refuses the build naming the first node that does not, the chain of calls that reached it, and the repair", DS_ABSENT, "", "a script's functions are script already; the promise that a Dado function raises into one is written in the Dado package that declares it"), Builtin("#channel", POS_CALL, M_CHANNEL, 0, FAM_NONE, 1, 1, 0, "#channel(N) name(params) · #channel(latest) name(params)", "declares a channel, `#channel(N) name(params)` at package level or as a member of a `type`: a typed, bounded, lock-free mailbox of `N` messages, or of the newest one only with `#channel(latest)`, many producers and one consumer, whose zero bytes are the empty channel", DS_ABSENT, "", NO_CHANNEL), Builtin("#send", POS_CALL, B_SEND, 0, FAM_NONE, 1, 255, 0, "#send(ch, args…)", "hands a message to a channel from any thread without blocking, its arguments checked as a call's; a queue's `#send` fails when the queue is full and is written `try #send(…) else CODE` or with an `else:` suite, a `latest` channel's never fails, and a `ref` argument moves to the consumer", DS_ABSENT, "", NO_CHANNEL), Builtin("#peek", POS_CALL, B_PEEK, 0, FAM_NONE, 1, 1, 0, "for args… in #peek(ch):", "walks what a channel holds without consuming it, written at the head of a `for` and binding each message's parameters", DS_ABSENT, "", NO_CHANNEL), Builtin("#drain", POS_CALL, B_DRAIN, 0, FAM_NONE, 1, 2, 0, "for args… in #drain(ch[, n]): · #drain(ch)", "walks what a channel holds and consumes it, at most `n` with `#drain(ch, n)`, committing when the loop exits by any edge; alone as a statement it discards what is queued now and frees every `ref` in it", DS_ABSENT, "", NO_CHANNEL), Builtin("#shader_load", POS_CALL, B_SHADER_LOAD, 0, FAM_NONE, 1, 1, 0, "#shader_load(Unit) · #shader_load(pkg.Unit)", "loads a compiled DadoGL shader unit — `#shader_load(Lit)` for `Lit.dadogl` in this package, `#shader_load(pkg.Lit)` for one in an imported package — answering a `core:shader` `Shader`; DadoGL is reserved and not built, so it is refused `ERR0001`", DS_ABSENT, "", "loads a DadoGL unit, and DadoGL is reserved and not built; it has no DadoScript answer until DadoGL has one"), Builtin("#script_vm", POS_CALL, B_SCRIPT_VM, 0, FAM_NONE, 0, 1, 0, "try VM vm = #script_vm([alloc][, out: s][, err: e][, on_error: p])", "makes a VM — `try VM vm = #script_vm()` — the engine a program's scripts run in, on the allocator written (`#heap` when none is), for the thread that made it; failable, and `#delete(vm)` ends it and every instance in it. `out:` and `err:`, named after the allocator, are the `Sink`s its scripts' `print` and their failures' text go to — standard output and standard error when left out — and `on_error:` is its failure policy, `script.QUIT` (the default: an untried call through a handle that fails ends the program) or `script.CONTINUE` (it answers its zero value and the program goes on). Its runs come only from the thread that made it (`script.THREAD` from another); its stop, pause, resume and queries from any", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_run", POS_CALL, B_SCRIPT_RUN, 0, FAM_NONE, 1, 255, 0, "#script_run(g, args…) -> i64", "runs an instance's `ready` — `i64 status = #script_run(g)`, or `#script_run(g, 60, \"Ann\")` for a `ready` that takes arguments, each checked as a method call's are — as a program's `main` is run: it waits for it, is never failable, and answers `ready`'s `int`, 0 when `ready` returns nothing, and for a failure escaping it the status a failing `main` exits with (its code when the low eight bits are nonzero, 70 when they are 0), its text on the error sink. A class with no `ready` is refused. Every other method is called through the handle, `g.hit(3)`; the retired `#script_run(s, \"hit\", 3)` is refused naming it", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_new", POS_CALL, B_SCRIPT_NEW, 0, FAM_NONE, 2, 255, 0, "try pkg.Class g = #script_new(vm, pkg.Class, args…)", "makes an instance of a class in a VM — `try npc.Goblin g = #script_new(vm, npc.Goblin, 3, 4.0)` — passing its `init` the arguments after the class, each checked to cross as a method call's are, and answers a handle typed by the class, which member calls (`g.hit(3)`) and `#script_run` go through and `#delete` releases; the VM loads the class's module the first time. Failable: a failing `init` is the call's, with the script's code or one of `core:script`'s reserved codes (`script.STALE`, `script.THREAD`, `script.OTHER_VM`, …), and `#script_error()` reads its text", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_stop", POS_CALL, B_SCRIPT_STOP, 0, FAM_NONE, 2, 2, 0, "#script_stop(vm, timeout_ms)", "stops the script running in a VM — `#script_stop(vm, 100)`, from any thread — asking it first, as an `Error` its `defer`s and `try` see, then forcing it once the timeout's milliseconds pass, which runs no more of it; `#script_stop(vm, 0)` forces at once, and the timeout is always written. The VM's engine then ends, its values released and its heap freed, and a later call through any handle on it fails `script.STOPPED`; the call it stopped answers `script.STOPPED` under `try`, and its type's zero value without one", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_pause", POS_CALL, B_SCRIPT_PAUSE, 0, FAM_NONE, 1, 1, 0, "#script_pause(vm)", "pauses the run in progress in a VM at its next loop or call until `#script_resume`, from any thread; a paused VM is busy, and a call or a `#script_new` on it fails `script.BUSY`", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_resume", POS_CALL, B_SCRIPT_RESUME, 0, FAM_NONE, 1, 1, 0, "#script_resume(vm)", "lets a run `#script_pause` held in a VM go on, from any thread", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_is_running", POS_CALL, B_SCRIPT_IS_RUNNING, 0, FAM_NONE, 1, 1, 0, "#script_is_running(vm) -> bool", "answers whether a run is in progress in a VM, asked from any thread, paused or not", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_is_paused", POS_CALL, B_SCRIPT_IS_PAUSED, 0, FAM_NONE, 1, 1, 0, "#script_is_paused(vm) -> bool", "answers whether a VM is paused, asked from any thread", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_pump", POS_CALL, B_SCRIPT_PUMP, 0, FAM_NONE, 1, 1, 0, "#script_pump(vm) -> i64", "delivers every message waiting for a VM's channel handlers — `i64 n = #script_pump(vm)` — the functions its scripts bound to the program's channels with `receive`, each on the VM's thread, in the order sent, and answers how many; a VM it cannot enter answers a negative reserved code (`script.THREAD`, `script.STALE`, `script.STOPPED`), its text on the error sink. The host's frame boundary: a call into a script delivers them as it returns too", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_reload", POS_CALL, B_SCRIPT_RELOAD, 0, FAM_NONE, 2, 3, 0, "try #script_reload(vm, Class[, dir])", "swaps new code in under a class a VM has loaded — `try #script_reload(vm, Goblin)` reads `reload/<package>/Goblin.js` under the working directory, `#script_reload(vm, Goblin, dir)` the same path under `dir`, the module `dadoc --emit=reload` wrote — after checking it against what this build bound: every Dado function it calls bound with the same signature, every method this program runs kept with its signature, every field this program reads or writes through a handle kept with its type, every other field it keeps of the same type; the class is any the build carries, and the VM must have made an instance of it. Every instance keeps its fields' values, a new field starts at its initializer, `init` does not run again, and each instance a handle holds runs its `reloaded()` method when it has one. Failable: on any miss the code running now stays, and `#script_error()` names every miss. Called from inside the script's own run, the new code is checked at once and takes over when that run leaves. Refused in a `--release` build unless it is built with `--reload`", DS_ABSENT, "", NO_HOST_VERB), Builtin("#script_error", POS_CALL, B_SCRIPT_ERROR, 0, FAM_NONE, 0, 0, 0, "#script_error() -> string8", "answers the text of the failure the `try` around a `#script_vm`, a `#script_new`, a method called through a handle or a `#script_reload` caught — the script's `Error` and its trace, or the runtime's sentence, what the error sink was given — as a `string8` borrowed until the `else` it is written in ends (`#format(#script_error())` keeps a copy); legal only in that `else`", DS_ABSENT, "", NO_HOST_VERB), Builtin("#trace", POS_CALL, B_TRACE, 0, FAM_NONE, 0, 0, 0, "#trace() -> ref []Location", "answers where the program is — `ref []Location t = #trace()` — as a run of `Location`s, innermost first: the place `#trace()` is written, then the place of each call the run is inside, each with its file, line, column and function (`main.update`), minted on the ambient allocator, which the caller frees with `#delete(t)`. A build with the full call stack (`dado test`, `--debug`, `--full-trace`) knows each call from it, up to 4,096 calls deep (a deeper call is counted, not kept); any other build answers the one place it is written, the first element either way", DS_ABSENT, "", NO_TRACE), Builtin("#location", POS_CALL, B_LOCATION, 0, FAM_NONE, 0, 0, 0, "#location() -> Location", "answers where it is written — `Location here = #location()` — as one `Location`: its file, line, column and function (`main.update`), the place `#trace()` names for itself. A constant of static strings, the same in every build and on every target: nothing is allocated and nothing is freed. A `#trace()` with no call stack (a build without `--full-trace`) answers one element, equal to `#location()` written at the same place. As a parameter's default, `Location at = #location()`, it is the caller's place: each call that leaves the argument out passes its own. In a `const` or package variable's initializer it folds to the place it is written", DS_SPELLED, "Location()", "a place is a `Location`, and `Location()` with no argument is where it is written, the same four fields"), Builtin("#release", POS_CALL, B_RELEASE, 0, FAM_NONE, 1, 1, 0, "#release(ch)", "a statement by a channel's consumer thread giving the channel up, so the next thread to walk it becomes its consumer: a hand-off, checked in a debug build and compiled to nothing without assertions", DS_ABSENT, "", NO_CHANNEL), ) ``
* `const [13]string8 PREDICATES = ("#is_int", "#is_float", "#is_number", "#is_bool", "#is_char", "#is_string", "#is_slice", "#is_map", "#is_ref", "#is_pointer", "#has_eq", "#is_key", "#is_declared")` — Type predicates, in `sub` order.
* `const u8 R_ACCEPTED = 0` — Spellings retired, each with what replaced it and the repair.
* `const u8 R_SILENT = 1`
* `const u8 R_REFUSED = 2`
* `type Retired`
* `` const string8 CHANNEL_REPAIR = "the construct was renamed before 1.0.0 and only its words changed: `#signal` is `#channel`, `#emit` is `#send` and a script's `connect` is `receive`, with the same meaning, the same rules and the same layout, so rename it and change nothing else" ``
* `` const string8 FITS_REPAIR = "the name now says what the `bool` answers — `true` when the result fits, as it always was. Only the name changed, so every existing use keeps its meaning under the new one: rename it and change no branch" ``
* `` const string8 PRINT_REPAIR = "the printing verbs are a library now: `import \"core:fmt\"` and write the call qualified. They take the same arguments in the same order, rendered one after another with nothing between them, and they write through `core:io`'s standard-output sink. `fmt.write(out, …)` and `fmt.writeln(out, …)` take a `Sink` first where some other destination is wanted. They exist only where `#OS` is not `#NONE`, because a freestanding target has no stream to write to. **A type is the one argument that does not carry over**: the builtin substituted a type's spelling at the call and a function cannot take a type at all (ERR0913), so `println(i32)` is `fmt.println(#spell(i32))` — `#spell` folds a type to the same spelling the builtin printed" ``
* `` const string8 VERB_REPAIR = "every builtin is spelled with a `#`, and the sigil is what says the compiler owns the name. The bare word is an ordinary name and nothing here declares it — write the `#` spelling for the builtin, or declare a function of this name and the call reaches it" ``
* `` const [23]Retired RETIRED = ( Retired("context.allocator", "#default", "write `#default`, which is the same allocator and is immutable within a scope", R_REFUSED), Retired("context.temp", "runtime.temp()", "the scratch allocator is a library and no longer a slot of an ambient container: `import \"core:runtime\"`, configure it once with `runtime.init_scratch`, and open a scope with `using runtime.temp():` — inside it `#default` is the arena and `#return` is still the caller's, so a survivor needs no escape analysis. Where the old spelling was a trailing allocator argument, `runtime.temp()` goes in the same position", R_REFUSED), Retired("context", "#default", "there is no container left to reify, because `#default` is the allocator itself — write `auto a = #default` — the two words `(proc, data)` — where a whole context used to be taken", R_REFUSED), Retired("print", "fmt.print", PRINT_REPAIR, R_REFUSED), Retired("println", "fmt.println", PRINT_REPAIR, R_REFUSED), Retired("#add_overflow", "#add_fits", FITS_REPAIR, R_REFUSED), Retired("#sub_overflow", "#sub_fits", FITS_REPAIR, R_REFUSED), Retired("#mul_overflow", "#mul_fits", FITS_REPAIR, R_REFUSED), Retired("#signal", "#channel", CHANNEL_REPAIR, R_REFUSED), Retired("#emit", "#send", CHANNEL_REPAIR, R_REFUSED), Retired("connect", "receive", CHANNEL_REPAIR, R_REFUSED), Retired("make", "#make", VERB_REPAIR, R_REFUSED), Retired("new", "#new", VERB_REPAIR, R_REFUSED), Retired("delete", "#delete", VERB_REPAIR, R_REFUSED), Retired("resize", "#resize", VERB_REPAIR, R_REFUSED), Retired("copy", "#copy", VERB_REPAIR, R_REFUSED), Retired("format", "#format", VERB_REPAIR, R_REFUSED), Retired("len", "#len", VERB_REPAIR, R_REFUSED), Retired("size", "#size", VERB_REPAIR, R_REFUSED), Retired("assert", "#assert", VERB_REPAIR, R_REFUSED), Retired("expect", "#expect", VERB_REPAIR, R_REFUSED), Retired("breakpoint", "#breakpoint", VERB_REPAIR, R_REFUSED), Retired("transpose", "#transpose", VERB_REPAIR, R_REFUSED), ) ``
* `const u8 Q_OS = 0` — The `#UPPER` enumerations and the queries that answer one member.
* `const u8 Q_ARCH = 1`
* `const u8 Q_ENDIAN = 2`
* `const u8 Q_ORDERING = 3`
* `type Member`
* `` const string8 NO_ARCH = "the CPU architecture is the target's machinery, and a script branches on the operating system or the web; ask `#ARCH` in Dado and give the script a Dado function that answers what it needs" ``
* `const string8 NO_ENDIAN = "byte order is a fact about representation, and a script never sees memory; ask it in Dado"`
* `const string8 NO_ORDERING = "a memory ordering is a thread primitive, and a script shares no memory between threads; write the atomic in Dado"`
* `` const string8 NO_TOOLCHAIN_OS = "a toolchain, not a host a script runs on: a host there answers `Platform.UNKNOWN_OS`, and a script on the web asks `Platform.WEB`" ``
* `` const string8 PLATFORM_MEMBER = "a member of `Platform`: `#UPPER` is `Platform.UPPER`, one mechanical rule, and it folds so `when` can use it" ``
* `const [26]Member MEMBERS = ( Member("#LINUX", Q_OS, 10, DS_SPELLED, "Platform.LINUX", PLATFORM_MEMBER), Member("#DARWIN", Q_OS, 20, DS_SPELLED, "Platform.DARWIN", PLATFORM_MEMBER), Member("#WINDOWS", Q_OS, 30, DS_SPELLED, "Platform.WINDOWS", PLATFORM_MEMBER), Member("#FREEBSD", Q_OS, 40, DS_SPELLED, "Platform.FREEBSD", PLATFORM_MEMBER), Member("#OPENBSD", Q_OS, 50, DS_SPELLED, "Platform.OPENBSD", PLATFORM_MEMBER), Member("#NETBSD", Q_OS, 60, DS_SPELLED, "Platform.NETBSD", PLATFORM_MEMBER), Member("#WASI", Q_OS, 70, DS_ABSENT, "", NO_TOOLCHAIN_OS), Member("#EMSCRIPTEN", Q_OS, 75, DS_ABSENT, "", NO_TOOLCHAIN_OS), Member("#NONE", Q_OS, 80, DS_ABSENT, "", "a target with no operating system has no DadoScript runtime, so no script runs there to ask"), Member("#WEB", Q_OS, 85, DS_SPELLED, "Platform.WEB", PLATFORM_MEMBER), Member("#UNKNOWN_OS", Q_OS, 90, DS_SPELLED, "Platform.UNKNOWN_OS", PLATFORM_MEMBER), Member("#X86_64", Q_ARCH, 10, DS_ABSENT, "", NO_ARCH), Member("#X86", Q_ARCH, 20, DS_ABSENT, "", NO_ARCH), Member("#AARCH64", Q_ARCH, 30, DS_ABSENT, "", NO_ARCH), Member("#ARM", Q_ARCH, 40, DS_ABSENT, "", NO_ARCH), Member("#RISCV64", Q_ARCH, 50, DS_ABSENT, "", NO_ARCH), Member("#WASM32", Q_ARCH, 60, DS_ABSENT, "", NO_ARCH), Member("#UNKNOWN_ARCH", Q_ARCH, 70, DS_ABSENT, "", NO_ARCH), Member("#LITTLE", Q_ENDIAN, 10, DS_ABSENT, "", NO_ENDIAN), Member("#BIG", Q_ENDIAN, 20, DS_ABSENT, "", NO_ENDIAN), Member("#UNKNOWN_ENDIAN", Q_ENDIAN, 30, DS_ABSENT, "", NO_ENDIAN), Member("#RELAXED", Q_ORDERING, 10, DS_ABSENT, "", NO_ORDERING), Member("#ACQUIRE", Q_ORDERING, 20, DS_ABSENT, "", NO_ORDERING), Member("#RELEASE", Q_ORDERING, 30, DS_ABSENT, "", NO_ORDERING), Member("#ACQ_REL", Q_ORDERING, 40, DS_ABSENT, "", NO_ORDERING), Member("#SEQ_CST", Q_ORDERING, 50, DS_ABSENT, "", NO_ORDERING), )`
* `const [5]string8 QUERIES = ("#OS", "#ARCH", "#ENDIAN", "#TRIPLE", "#DADO_VERSION")` — The queries: `#OS`, `#ARCH`, `#ENDIAN`, and two answered by a string.
* `const [5]string8 LADDER = ("#stack", "#default", "#return", "#caller", "#heap")` — The allocator ladder, outermost last.
* `i64 builtin(string8 name)` — The row spelled `name`, or -1.
* `i64 retired(string8 spelling)` — The retirement row of `spelling`, or -1.
* `Val fn_value(^World w, ^Body b, Val v, u32 n)` — A function as a value: its body queued, its C name a constant.
* `[]u32 call_args(^World w, ^Body b, u32 n)` — The argument nodes, `E_ARG`s, of call `n`.
* `Val call(^World w, ^Body b, u32 n, u32 want, bool effect)`
* `u32 arg_value(^World w, ^Body b, u32 g)`
* `string8 debug_f64(f64 x)` — `x` as Rust's `{:?}` writes it: the shortest digits, `1e30`, `-1.0`.
* `u32 method(^World w, u32 ty, u32 name, ^u32 at)` — Method `name` of type `ty` (through a pointer), or NONE; `at` is the slot…
* `Val indirect(^World w, ^Body b, u32 n, Val c)` — A call through a function value: positional arguments, one per parameter.
* `` const string8 SLOTS_WHERE = "`#slots` names a value's slots, so it is written in one of two places, and this is neither" ``
* `i64 axis_of(char8 c)`
* `string8 arity_said(string8 said, Builtin r, i64 got)`
* `bool c_bool(^World w, u32 r)` — `#c.bool`, C's `_Bool`.
* `const u8 B_CH_SEND = 80`
* `const u8 B_CH_BEGIN = 81`
* `const u8 B_CH_HEAD = 82`
* `const u8 B_CH_READY = 83`
* `const u8 B_CH_FIELD = 84`
* `const u8 B_CH_COMMIT = 85`
* `const u8 B_CH_LEN = 86`
* `const u8 B_CH_RELEASE = 87`
* `type ChWalk` — A walk open around the statement being checked; `at` spells a member…
* `type Commit` — A consuming walk's commit (or a peek's close), made at each exit.
* `u32 chan_of(^World w, u32 ty)`
* `u32 chan_type(^World w, u32 d, u32 f, u32 n, string8 q)` — The channel type of `T_CHANNEL` node `n` in file `f`, named `q`.
* `u32 held_chan(^World w, u32 ty, i64 depth = 0)` — The first channel `ty` holds in its own storage, or NONE.
* `void chan_by_value(^World w, u32 f, u32 lo, u32 hi, u32 ty, string8 what)` — ERR0531 for a signature's parameter or return (`what`) holding a channel.
* `bool chan_copied(^World w, ^Body b, u32 ty, Val v, u32 vn)` — ERR0531 for value `v` (node `vn`) copied into a `ty`.
* `bool chan_bound(^World w, ^Body b, u32 bn, u32 ty, string8 help)` — ERR0531 for a binding `bn` that copies a `ty` (a walk's element, a union arm's member).
* `u8 chan_verb(^World w, ^Body b, u32 n)` — The verb a call `n` spells (`#send`, `#peek`, `#drain`, `#release`, the…
* `Val chan_call(^World w, ^Body b, u32 n, u8 m, string8 said, bool stmt)` — `#send`, `#peek`, `#drain`, `#release` as a value (`stmt`: on a line of…
* `void commit(^World w, ^Body b, Commit c)`
* `Val chan_len(^World w, ^Body b, u32 n, Val v, u32 vn)` — `#len(ch)`.
* `bool chan_iterated(^World w, ^Body b, u32 it, u32 ty)` — ERR0720: `for x in ch`.
* `void chan_loop(^World w, ^Body b, u32 n, []u32 binds, u32 it)` — `for … in #peek(ch)` / `#drain(ch[, n])`: each entry queued when the walk…
* `bool holds(^World w, u32 f, u32 base, u32 n)` — Whether condition `n` of file `f` holds; ERR0001 when it does not fold.
* `void unenumerated_os(^World w, u32 f, u32 base, []u32 conds)` — ERR0908 (Rust's `unenumerated_os`): a chain branching on `#OS` under a…
* `Val flat_name(^World w, ^Body b, u32 n, u32 sym)` — A bare name in a `when` condition: the one package constant of that…
* `void when_stmt(^World w, ^Body b, u32 n)` — `when` in a body: the arm whose condition holds, else the `else`.
* `Val platform(^World w, string8 s)` — `#OS` and the other queries, `#LINUX` and the other members: constants of…
* `u32 query_type(^World w, u8 q)`
* `void static_asserts(^World w)` — Package-level `#static_assert(COND, "message")`: folded as a `when`
* `const string8 TARGET_REPAIR = "build for a target the assertion accepts, or change the assertion if it is the one that is wrong"`
* `void refused_said_once(^World w)` — A package refusing the build with its own `#static_assert` (ERR0930) says…
* `const u8 B_COPY_INTO = 130` — dst = how many elements moved from view args[1] into view args[0].
* `void copy_assign(^World w, ^Body b, u32 n, u32 pn, u32 vn)` — `dst[…] = src`.
* `Val copy_into(^World w, ^Body b, u32 n)` — `#copy_into(dst, src)`: the count, which folds when both lengths do.
* `void enum_body(^World w, u32 d, u32 ty)` — An enum: its backing (a tag over it when `distinct`), members counted on…
* `void fenum_body(^World w, u32 d, u32 ty)` — A `foreign` enum: its backing; each member is the enumerator C names.
* `u32 global_type(^World w, u32 d)` — A constant's or variable's type; its value lands in `decls[d].value`.
* `u32 const_value(^World w, u32 d, u32 vn, u32 want, bool var_, string8 site = "", bool ds = false)` — The value of `vn` as a `want` (NONE: a literal), folded; NONE when…
* `void tagless(^World w, ^Body b, u32 ty, u32 c, u32 vn, ^ref []u32 said)` — ERR0915: a folded union value whose member shares its sort with another.
* `const string8 SLICE_UNFOLDED = "a slice, which windows storage that exists when the program runs"`
* `u32 folded(^World w, ^Body b, u32 lo, u32 x)` — The constant operand `x` holds when the ops from `lo` on fold, or NONE.
* `Val const_val(^World w, ^Body b, u32 d, u32 n)` — A declaration's value as an expression.
* `Val konst_val(^World w, ^Body b, u32 c, u32 ty, u32 n)` — A constant as an operand; a tuple is set into a temporary first.
* `Val global(^World w, ^Body b, u32 d)` — A package variable as a place: its global in this body's table.
* `Val foreign_var(^World w, ^Body b, u32 d)` — A `foreign` object as a place: a global the header defines, by its C name.
* `bool build_const(^World w, u32 d)` — Whether `d` is a `@const`.
* `u32 overridden(^World w, u32 d, u32 ty, u32 cv)` — `@const` `d`'s value after the build's say: `cv` (its written value) or…
* `Val local_override(^World w, ^Body b, u32 n, u32 ty, Val v)` — A body's `@const` (node `n`): its value `v` after the build's say.
* `void defines_after(^World w)` — After the program: each `-D` that reaches none (ERR1101), or a bare one…
* `type Said`
* `type Dia`
* `` const string8 BANG_WHY = "the failable mark: `!T` is `T` and `!R(A)` is `R(A)`, because any DadoScript function may fail; the prefix `!x`, logical not, is the same in both" ``
* `` const string8 UNBUILT_TYPE_HELP = "write one of the types a script holds: `int`, `float`, `bool`, `string`, `any`, `[]T`, `[N]T`, `{K: V}`, a tuple, a class, a trait, an enum, a function type, `async T`, `Error` or `Location`" ``
* `` const string8 NO_SHARPS = "DadoScript has no `#` spellings — a `#` name is a compiler query or machinery Dado makes visible — so this is written in Dado" ``
* `` const string8 SINKS_WHY = "a script's sinks are its host's to scope — a script's `print` writes to the `out:` the Dado call into it put in force — so a script scopes none" ``
* `ref []string8 NOARGS` — Never grown: an empty argument list.
* `` const string8 LEN_OVER_TEXT = "a DadoScript string is UTF-16 code units, so `s.len()` would count those: the rewrite is `s.bytes().len()`, the bytes Dado's `#len` counted" ``
* `bool dialects(^World w)` — The pass over every file: whether the program stops here.
* `` const string8 WIDTH_INT = "a width is Dado's to write; DadoScript's one integer is `int`, exactly `i64`, and a Dado slot of this type reads as `int` and is narrowed into on a store" ``
* `bool spelling(string8 s, ^u8 k, ^string8 sp, ^string8 why)` — What DadoScript writes for the Dado spelling `s`: false when nothing is said.
* `string8 table_help(^World w, string8 s, []string8 args)` — The help a Dado spelling's row gives, its placeholders filled from `args`.
* `const [13]string8 KEPT = ("is", "has", "int", "float", "scalar", "bool", "string", "any", "fail", "assert", "print", "Platform", "defer")`
* `u32 under(^types.Types t, u32 ty)` — Through declared names and `distinct` tags to the shape below.
* `bool tagged(^World w, u32 ty)`
* `bool is_int(^World w, u32 ty)`
* `bool is_float(^World w, u32 ty)`
* `u8 float_bits(^World w, u32 ty)` — A float type's width in bits (C's floating scalars count as 64).
* `bool is_num(^World w, u32 ty)`
* `f64 fval(u64 bits)`
* `u64 fbits(f64 v)`
* `u32 fconst(^World w, f64 v, u32 ty)` — A float constant of type `ty`, rounded to its width.
* `f64 lit_f(Val v)` — A literal's value as a float.
* `u8 int_prim(^World w, u32 ty)` — An integer type's prim (canonical), or PRIMS.
* `u8 int_prim_of(^types.Types t, u32 ty)`
* `string8 decimal(u64 lo, i64 hi)` — `(lo, hi)` as a decimal, the i128 a literal carries.
* `u32 final_ty(^World w, Val v)` — An untyped value's own type: `i32`, or `i64` past it.
* `Val finish(^World w, ^Body b, Val v, u32 n)`
* `bool nil_fits(^World w, u32 ty)` — Where `nil` is a value: windows, handles, pointers, text.
* `bool lends(^World w, u32 from, u32 to)` — A `ref []T` lends its elements as a `[]T`, a `ref []char8` as text.
* `string8 hex(u64 v, i64 width)` — `v` in upper-case hex, at least `width` digits.
* `bool fits_type(^World w, Val v, u32 ty)` — Whether `v` can be written where `ty` is wanted.
* `bool same_shape(^World w, u32 x, u32 y)` — A declared shape is its shape under `ref`, `[]` and `^` too (C2-11).
* `bool same_q(^World w, u32 x, u32 y)`
* `u32 pq(^World w, u32 t)` — A `rawptr`'s qualifiers, which are its pointee's (`restrict` is not).
* `bool top_const(^World w, u32 t)` — A top-level `const` (on a `rawptr` it is the pointee's).
* `bool adds_quals(^World w, u32 to, u32 from)` — `from` stands where `to` is wanted by gaining qualifiers one level down:
* `bool elem_const(^World w, u32 to, u32 from)` — A window's element `to` is `from` with `const` added.
* `bool widens(^World w, u32 from, u32 to)` — An integer that converts without loss: same signedness and no narrower,…
* `Val coerce(^World w, ^Body b, Val v, u32 ty, u32 at, string8 site = "the declared shape is")` — `v` as a `ty`: a literal wrapped (LNT0505 when it does not fit), an integer…
* `void mismatch(^World w, ^Body b, u32 at, string8 site, u32 want, u32 got)`
* `string8 ro_help(^World w, u32 got, u32 want)` — A read-only window `got` where a writable `want` is asked for.
* `void cstring_help(^World w, u32 from, u32 to, ^string8 help, ^string8 why)` — The repair for text crossing to or from a `cstring`.
* `string8 one_help(^World w, u32 ty)`
* `Val expr(^World w, ^Body b, u32 n, u32 want, bool effect = false)`
* `Val name(^World w, ^Body b, u32 n)`
* `bool in_const_init(^World w, ^Body b)` — Whether `b` is a package constant's initializer.
* `u32 vis_of(^World w, u32 d)` — A declaration's written visibility (a `foreign` item's is its block's).
* `Val decl_val(^World w, ^Body b, u32 d, u32 n)` — What declaration `d` is as a value.
* `Val field_of(^World w, ^Body b, u32 n, Val base)` — `base.name`, the base already checked.
* `u32 common(^World w, ^Body b, Val l, Val r, u32 n, string8 op)` — Two integer operands' common type; ERR0504 when there is none.
* `Val binary(^World w, ^Body b, u32 n, string8 op, Val lv, u32 ln, u32 rn, u32 want)` — `l op r`; `lv` is the left operand when already checked (`+=`).
* `i64 arity(^World w, u32 ty)`
* `bool is_char(^World w, u32 ty)`
* `Val combine(^World w, ^Body b, u32 n, string8 op, u8 sub, Val l, Val r, u32 at, u32 rn)` — `l op r` over checked operands; a tuple side goes slot by slot (a…
* `string8 embedded(^World w, ^Body b, u32 n)` — ERR0518: an `auto` local's type spelled by two visible declarations that…
* `string8 embedded_help(^World w, ^Body b, u32 n)`
* `u32 ptrdiff(^World w)`
* `bool calls_in(^World w, ^Body b, u32 n)` — Whether evaluating `n` may call a function (and so change a place read before it).
* `Val pinned(^World w, ^Body b, Val v, u32 later, u32 at)` — `v` read now, before `later` runs: a place copied when `later` may call.
* `bool cnl_same(^World w, u32 x, u32 y)` — One shape but for a header's named member lists (at least one met).
* `bool cnl_eq(^World w, u32 x, u32 y, ^bool met)`
* `bool tevals(^World w, ^Body b, u32 n)` — ERR0912: a compilation evaluates at most 4096 type values (Rust's…
* `bool plain_char(^World w, u32 ty)` — C's plain `char`, whose signedness is the implementation's.
* `Val base_expr(^World w, ^Body b, u32 q)` — The base of `q.name`: an import of this file named `q` wins over a package…
* `type Defer` — A pending `defer`: its statement, scope, `defer fail`, and the locals it sees.
* `void unwind(^World w, ^Body b, u32 depth, bool failing, bool pop = false)` — Runs the defers of scopes deeper than `depth`, innermost first; `pop`
* `bool failable_call(^World w, ^Body b, u32 n, string8 callee, u32 code)` — A failable call at node `n`, its code in `code`: refused (ERR0611) unless…
* `void failing(^World w, ^Body b, u32 n, u32 code)` — What a failure inside the `try` being made does: pass `code` on, fail…
* `void guarded(^World w, ^Body b, u32 n, u32 bad_)` — A guarded read or create verb at `n`: `bad` (a bool place) set is the failure.
* `u32 code_type(^World w, u32 fty)` — The type a failable call's code is bound as: its `E` for `E!T`, else `i32`.
* `void fail_(^World w, ^Body b, u32 n)` — `fail CODE`.
* `u32 named_const(^World w, u32 f, u32 n, ^Body b)` — The package constant expression `n` of file `f` names (NONE: none, or a…
* `u32 alias_of(^World w, u32 d)` — The constant `d` is defined as, through every `const A = B` (one code…
* `` const string8 TRY_SHAPE = "`try` makes one failable call or map read: `try T v = f(…)`, `try v = f(…)`, `try f(…)` or `try V v = m[k] else …` — and this statement has no call or read where its value goes" ``
* `void try_(^World w, ^Body b, u32 n)` — `try <stmt> [else …]`: the statement inside a BLOCK its suite leaves to.
* `bool suite_leaves(^World w, ^Body b, u32 n)` — Whether a suite leaves its scope on every path.
* `void defer_(^World w, ^Body b, u32 n)` — `defer <stmt>`: refused when it would leave the scope; run at each exit.
* `u32 fails_with(^World w, ^Body b)` — The enum this body fails with (`E!T`), or NONE.
* `bool read_only(^World w, ^Body b, u32 pn)` — ERR0606: a typed failure's binding is read-only.
* `u32 failure_enum(^World w, ^Body b, u32 s, u32 ty)` — The enum a `switch` scrutinee `s` is a typed failure binding of, or NONE.
* `void closed_over(^World w, ^Body b, u32 n, u32 s, u32 et, []u32 seen)` — A failure `switch` with no `else`: closed when it names every member,…
* `u32 rt_pkg(^World w)` — The `core:script` package, or NONE.
* `bool is_vm(^World w, u32 ty)`
* `bool handle_ty(^World w, u32 ty)` — A VM or a class's handle: a `void *` in C.
* `Val host_verb(^World w, ^Body b, u32 n, u8 m, string8 said)` — A host verb, `m` its meaning.
* `Val host_delete(^World w, ^Body b, u32 n, Val v, u32 vn)` — `#delete(vm)` or `#delete(g)`: the handle closed and its place nilled.
* `u32 s_method_named(^World w, u32 ci, string8 name)` — Method `name` of class `ci`, or NONE.
* `bool h_fixed(^World w, u32 s)`
* `string8 h_article(string8 s)`
* `Val host_member(^World w, ^Body b, u32 n, u32 cn, Val base, u32 ci)` — `g.m(args…)` through a handle of class `ci`: pushes, then a run.
* `Val host_emit(^World w, ^Body b, u32 n, u32 h, string8 sn, []u32 ps, []u32 vals)` — `g.sig.emit(…)` from Dado: the values pushed (`ps`: the signal's script…
* `void host_push(^World w, ^Body b, u32 n, u32 h, Val v, string8 pn)` — `v` pushed onto handle `h` by `core:script`'s `pn`.
* `Val host_run(^World w, ^Body b, u32 n, u32 h, string8 e)` — A run of the instance's method `e`, taking what was pushed.
* `Val host_control(^World w, ^Body b, u32 n, u8 m)` — The VM-wide verbs: `#script_stop`, `_pause`, `_resume`, `_is_running`,…
* `void h_rest(^World w, ^Body b, u32 n, i64 from)`
* `i64 n_errors(^World w)` — Errors said so far.
* `const u32 SINK_MARK = 1`
* `bool using_sinks(^World w, ^Body b, u32 head, bool after, ^[2]u32 sv)` — A `using`'s `out:`/`err:` (Rust's `using_sinks`): names and types; with…
* `void sinks_pop(^World w, ^Body b, Commit c)` — The sinks a scope saved, put back.
* `void sinks_after(^World w, ^Body b, u32 n, u32 head)` — Sinks scoped after call `n`, which checked clean: it must be one into a…
* `bool vm_scope(^World w, ^Body b, u32 n)` — `#script_vm(…) using out: …`: ERR1211.
* `const [30]string8 HRUN_RT = ("read_same_string", "mix_string", "mix_float", "refused_back", "drop_string", "read_string", "read_same_length", "lend_host", "unlend_host", "lent_changed", "lent_refused", "clear_exception", "mix", "out_of_memory", "begin_run", "begin_new", "push_value", "call_entered", "construct_entered", "answer_int", "answer_float", "answer_bool", "answer_string", "answer_of", "answer_refused", "end_run", "quit_untried", "dup_value", "give_int", "give_string")` — What a run helper calls, from `core:script`.
* `Val host_fn(^World w, ^Body b, u32 n, u32 cn, u32 pk, ^bool done)` — `pkg.f(args…)` from Dado, `pkg` holding scripts and no Dado `f`: a…
* `const [9]string8 FN_RT = ("push_int", "push_float", "push_bool", "push_string", "call_function", "call_function_int", "call_function_float", "call_function_bool", "call_function_string")` — What a package function's helper calls, from `core:script`.
* `Val host_get(^World w, ^Body b, u32 n, Val base, ^bool done)` — `g.f` from Dado: a field read through a handle (`get_*_or_quit`); `done`
* `void h_put(^World w, ^Body b, u32 n, u32 pn, u32 vn, string8 op)` — A handle's field as a place: `g.f op= v` is ERR1218 (two runs); a store…
* `bool h_misplaced(^World w, ^Body b, u32 pn, string8 what)` — ERR1218 for a handle's field in a group assignment or under `&`.
* `bool host_put(^World w, ^Body b, u32 n, u32 pn, u32 vn, string8 op)` — `g.f = v` from Dado, `g` a local handle: the value pushed and stored…
* `bool h_web(^World w, ^Body b, u32 n, u8 k, string8 what)` — ERR1225: what a web build cannot do with a script.
* `const u8 BK_TYPE = 0`
* `const u8 BK_LEN = 1`
* `const u8 BK_PACK = 2`
* `const u8 BK_TRAIT = 3`
* `const [2]string8 CATEGORIES = ("int", "float")`
* `u32 owner(^World w, u32 d)` — The declaration whose binders an instance of `d` is over: a generic type…
* `[]u32 binders(^World w, u32 d)` — The names `d`'s head binds, in order (gathered once).
* `u8 binder_kind(^World w, u32 d, i64 i)`
* `u32 trait_param(^World w, u32 f, u32 n)` — A trait written as a parameter's whole type (or under its `^`): the…
* `bool templated(^World w, u32 d)` — A template: a function binding more than a pack, or a generic type's…
* `bool packed_only(^World w, u32 d)`
* `u32 bound(^World w, u32 sym, ^u8 k)` — What `sym` is bound to in the current instance, or NONE; `k` its kind.
* `u32 len_row(^World w, i64 n)`
* `u32 sig(^World w, u32 d, u32 env)` — A template's signature under `env`, resolved once per instance.
* `u32 instance(^World w, u32 d, []u32 args, u32 f, u32 n)` — `Pool(i32)`: generic type `d` over `args`, its body resolved once; `n`
* `string8 binder_use(^World w, u32 d)` — `Bad(T)`: generic `d` written over its own binders.
* `bool derived(^World w, u32 env, u32 par)` — Whether instance `env` is built out of `par`'s: a binding holding one of…
* `u32 env_of(^World w, u32 ty)` — The environment of instance type `ty` (through a pointer), or NONE.
* `void generic_rules(^World w, u32 d)` — What a generic declaration says about all its instantiations: its…
* `u32 applied(^World w, u32 f, u32 n, u32 d, []u32 ps)` — `G(A, B)` in a type position: ERR0538 unless one type per binder.
* `string8 generic_head(^World w, u32 d, i64 given)`
* `Val generic(^World w, ^Body b, u32 n, u32 d)` — A call of template `d`: each argument binds what its parameter's written…
* `Val emit_call(^World w, ^Body b, u32 n, u32 d, u32 fty, u32 cname, Val recv, []Val vals, []Val packed)` — The call op: `dst = call fn(recv, vals…, packed…)`.
* `bool mentions(^World w, u32 d, u32 n)` — Whether written type `n` of `d`'s file names one of `d`'s binders.
* `bool foreign_struct(^World w, u32 t)`
* `string8 abbreviate(string8 s)` — `s` cut to 48 characters, as a diagnostic shows a shape.
* `string8 bindings(^World w, u32 d, u32 env)` — `N=3, T=i32`: an instance's bindings, lengths then categories then types,…
* `u32 where_of(^World w, u32 d)` — A template's `where` condition, or NONE.
* `u32 decl_named(^World w, u32 f, u32 n)` — The declaration expression `n` of file `f` names, through a package.
* `u32 ptn(^const syntax.Ast a, []u32 ps, i64 i)` — Parameter `i` of `ps` is `type T` (or groups with one: `type T, U`):
* `bool kind_at(^const syntax.Ast a, []u32 ps, i64 i)`
* `u32 job(^World w, u32 d, u32 pack)` — The C name of declaration `d`'s body over `pack`, queued the first time.
* `string8 pkg_key(^World w, u32 p)` — `_` doubled, so `a_b` and `a__b` stay apart from the separators.
* `string8 mangle(string8 s)`
* `bool entry_decl(^World w, u32 d)` — The program's entry: `main` of the main package, or in a `@test` build…
* `void program(^World w)` — Every body reached from `main`, checked in queue order. ERR0301 when the…
* `void unlint(^World w)` — LNT0412 is for a program that otherwise checks.
* `string8 shape_c_name(^types.Types t, u32 r)` — The C name the lowering gives shape `r` (a tuple, array, union, function…
* `void foreign_mangled(^World w)`
* `const u8 B_MAP_SLOT = 60`
* `const u8 B_MAP_GET = 61`
* `const u8 B_MAP_HAS = 62`
* `const u8 B_MAP_REMOVE = 63`
* `const u8 B_MAP_FREE = 64`
* `const u8 B_MAP_COPY = 65`
* `const u8 B_MAP_MAKE = 66`
* `const u8 B_MAP_CAP = 67`
* `const u8 B_MAP_LIVE = 68`
* `const u8 B_MAP_KEY = 69`
* `const u8 B_MAP_VAL = 70`
* `const u8 B_MAP_WALK = 71`
* `const u8 B_MAP_LEN = 72`
* `const u8 B_SCAN = 73`
* `const u8 B_MAP_GEN = 74`
* `type Walk` — A map walk open in a body: the place walked, its key binding's slot.
* `bool is_map(^World w, u32 ty)`
* `string8 unkeyable(^World w, u32 k)` — Why `k` cannot key a map, or "".
* `bool keyed_ok(^World w, u32 f, u32 base, u32 lo, u32 hi, u32 ty, string8 what, bool use, i64 depth = 0)` — ERR0801 for each map `ty` reaches whose key cannot be one; `use`: said…
* `u32 map_written(^World w, ^Body b, u32 n)` — The map type an expression written as a type denotes (`{K: V}` in an…
* `Val map_lit(^World w, ^Body b, u32 n, u32 want)` — `{k: v, …}` and `{}`: the zero map with each entry inserted.
* `u32 map_slot(^World w, ^Body b, u32 m, u32 mt, u32 k, u32 n)` — The value slot for key `k` in the map at place `m`, inserted when absent.
* `Val map_index(^World w, ^Body b, u32 n, Val base, bool writing)` — `m[k]`: a place (an insert) when written, else `(V, bool)`.
* `Val in_(^World w, ^Body b, u32 n)` — `k in m`, and `x in xs` over a run (one scan).
* `void map_loop(^World w, ^Body b, u32 n, []u32 binds, u32 it, Val v)` — `for k in m`, `for k, v in m`, `for k, &v in m`: each live entry of the…
* `Val map_verb(^World w, ^Body b, u32 n, u8 m, Val v, u32 vn)` — `#len(m)`, `#delete(m)`, `#delete(m, k)`, `#copy(m)`, `#make({K: V}, n)`.
* `Val map_make(^World w, ^Body b, u32 n, u32 mt)`
* `type MathRow`
* `const [29]MathRow MATH = (MathRow("#sin", 1, false, "sin", "Math.sin", "sin"), MathRow("#cos", 1, false, "cos", "Math.cos", "cos"), MathRow("#tan", 1, false, "tan", "Math.tan", "tan"), MathRow("#asin", 1, false, "asin", "Math.asin", "asin"), MathRow("#acos", 1, false, "acos", "Math.acos", "acos"), MathRow("#atan", 1, false, "atan", "Math.atan", "atan"), MathRow("#atan2", 2, false, "atan2", "Math.atan2", "atan2"), MathRow("#sinh", 1, false, "sinh", "Math.sinh", "sinh"), MathRow("#cosh", 1, false, "cosh", "Math.cosh", "cosh"), MathRow("#tanh", 1, false, "tanh", "Math.tanh", "tanh"), MathRow("#asinh", 1, false, "asinh", "Math.asinh", "asinh"), MathRow("#acosh", 1, false, "acosh", "Math.acosh", "acosh"), MathRow("#atanh", 1, false, "atanh", "Math.atanh", "atanh"), MathRow("#pow", 2, false, "pow", "Math.pow", "pow"), MathRow("#sqrt", 1, false, "sqrt", "Math.sqrt", "sqrt"), MathRow("#cbrt", 1, false, "cbrt", "Math.cbrt", "cbrt"), MathRow("#exp", 1, false, "exp", "Math.exp", "exp"), MathRow("#exp2", 1, false, "exp2", "2 ** ", "exp2"), MathRow("#log", 1, false, "log", "Math.log", "log"), MathRow("#log2", 1, false, "log2", "Math.log2", "log2"), MathRow("#log10", 1, false, "log10", "Math.log10", "log10"), MathRow("#fma", 3, false, "fma", "", "fma"), MathRow("#floor", 1, false, "floor", "Math.floor", "floor"), MathRow("#ceil", 1, false, "ceil", "Math.ceil", "ceil"), MathRow("#round", 1, false, "round", "", "round"), MathRow("#trunc", 1, false, "trunc", "Math.trunc", "trunc"), MathRow("#abs", 1, true, "fabs", "Math.abs", "abs"), MathRow("#min", 2, true, "fmin", "Math.min", "min"), MathRow("#max", 2, true, "fmax", "Math.max", "max"))`
* `const u8 B_MATH = 100` — dst = MATH[sub - B_MATH](args…), over `ty`.
* `i64 math_row(string8 s)`
* `i64 math_global(string8 s)` — The row of DadoScript global `s`, or -1.
* `Val math_(^World w, ^Body b, u32 n, i64 k)`
* `u32 matrix_type(^World w, u32 f, u32 n)`
* `u32 matrix_of(^World w, u32 e, u32 rows, u32 cols)`
* `u32 matrix(^World w, u32 ty)` — The matrix row under names, qualifiers and tags, or NONE.
* `u32 melem(^World w, u32 m)` — A matrix row's element type.
* `u32 matrix_col(^World w, u32 m)` — The column vector `[C]T` a matrix row takes on its right.
* `Val matrix_op(^World w, ^Body b, u32 n, string8 op, Val l, Val r, u32 at, u32 rn, ^bool ok)` — `l op r` when a side is a matrix and the tag answers it; `ok` false to…
* `Val transpose_(^World w, ^Body b, u32 n, []u32 args, string8 said)` — `#transpose(m)`: a matrix's two dimensions swapped (it keeps its tag), or…
* `u32 shape_of(^World w, u32 f, u32 n)` — `typeof(p)`: the type of an inert place; `shapeof(p)` its shape (no name,…
* `type WV` — A `where` value: k 0 not read here, 1 bool, 2 arity, 3 type.
* `void where_check(^World w, u32 d)` — A `where` refused where it is declared (ERR0920 and its staged forms):
* `WV wv(^World w, u32 d, u32 x, u32 env)` — `where` expression `x` of template `d` under bindings `env`.
* `bool may_name(^World w, u32 f, u32 n)` — Whether expression `n` may name a declaration: a name that is no builtin…
* `bool same_type(^World w, u32 f, u32 ea, u32 a, u32 eb, u32 b)` — `is` between types: by declaration when both operands may name one.
* `bool one_type(^World w, u32 a, u32 b, i64 depth)` — One type: the same after declared names, element by element.
* `bool op_applies(^World w, u32 t, string8 op)` — Whether operator `op` (as `has` spells it) applies between two `t`s.
* `Val is_expr(^World w, ^Body b, u32 n)` — `x is T` in a body: a union value's tag, else answered from types.
* `Val has_expr(^World w, ^Body b, u32 n)` — `T has +` in a body: not built (as Rust).
* `bool fx_eq(^World w, u32 a, u32 b, i64 depth)` — One layout where only a transparent `foreign` struct and its shape differ.
* `Val names_call(^World w, ^Body b, u32 n)` — `#names(T)`, `#names(v)`: the names a declared type's view (or a `view`
* `void shader_units(^World w)` — A `.dadogl` file of a program's own package: reserved, refused at the…
* `Val shader_load(^World w, ^Body b, u32 n)` — `#shader_load(Lit)`: DadoGL is reserved, so every call is refused.
* `type Lib`
* `const [103]Lib LIBS = ( Lib("C11", "<complex.h>", "CMPLX CMPLXF CMPLXL cabs cabsf cabsl cacos cacosf cacosh cacoshf cacoshl cacosl carg cargf cargl"), Lib("C11", "<complex.h>", "casin casinf casinh casinhf casinhl casinl catan catanf catanh catanhf catanhl catanl ccos ccosf"), Lib("C11", "<complex.h>", "ccosh ccoshf ccoshl ccosl cexp cexpf cexpl cimag cimagf cimagl clog clogf clogl complex conj"), Lib("C11", "<complex.h>", "conjf conjl cpow cpowf cpowl cproj cprojf cprojl creal crealf creall csin csinf csinh csinhf"), Lib("C11", "<complex.h>", "csinhl csinl csqrt csqrtf csqrtl ctan ctanf ctanh ctanhf ctanhl ctanl imaginary"), Lib("C11", "<stdio.h>", "FILE clearerr fclose feof ferror fflush fgetc fgetpos fgets fopen fpos_t fprintf fputc fputs"), Lib("C11", "<stdio.h>", "fread freopen fscanf fseek fsetpos ftell fwrite getc getchar gets perror printf putc putchar"), Lib("C11", "<stdio.h>", "puts remove rename rewind scanf setbuf setvbuf snprintf sprintf sscanf stderr stdin stdout tmpfile"), Lib("C11", "<stdio.h>", "tmpnam ungetc vfprintf vfscanf vprintf vscanf vsnprintf vsprintf vsscanf"), Lib("C11", "<threads.h>", "ONCE_FLAG_INIT call_once once_flag thread_local"), Lib("C11", "<stdlib.h>", "abort abs aligned_alloc at_quick_exit atexit atof atoi atol atoll bsearch calloc div div_t exit"), Lib("C11", "<stdlib.h>", "free getenv labs ldiv ldiv_t llabs lldiv lldiv_t malloc mblen mbstowcs mbtowc qsort quick_exit"), Lib("C11", "<stdlib.h>", "rand realloc srand strtod strtof strtol strtold strtoll strtoul strtoull system wcstombs wctomb"), Lib("C11", "<math.h>", "acos acosf acosh acoshf acoshl acosl asin asinf asinh asinhf asinhl asinl atan atan2 atan2f"), Lib("C11", "<math.h>", "atan2l atanf atanh atanhf atanhl atanl cbrt cbrtf cbrtl ceil ceilf ceill copysign copysignf"), Lib("C11", "<math.h>", "copysignl cos cosf cosh coshf coshl cosl double_t erf erfc erfcf erfcl erff erfl exp exp2 exp2f"), Lib("C11", "<math.h>", "exp2l expf expl expm1 expm1f expm1l fabs fabsf fabsl fdim fdimf fdiml float_t floor floorf floorl"), Lib("C11", "<math.h>", "fma fmaf fmal fmax fmaxf fmaxl fmin fminf fminl fmod fmodf fmodl fpclassify frexp frexpf frexpl"), Lib("C11", "<math.h>", "hypot hypotf hypotl ilogb ilogbf ilogbl isfinite isgreater isgreaterequal isinf isless islessequal"), Lib("C11", "<math.h>", "islessgreater isnan isnormal isunordered ldexp ldexpf ldexpl lgamma lgammaf lgammal llrint llrintf"), Lib("C11", "<math.h>", "llrintl llround llroundf llroundl log log10 log10f log10l log1p log1pf log1pl log2 log2f log2l"), Lib("C11", "<math.h>", "logb logbf logbl logf logl lrint lrintf lrintl lround lroundf lroundl math_errhandling modf"), Lib("C11", "<math.h>", "modff modfl nan nanf nanl nearbyint nearbyintf nearbyintl nextafter nextafterf nextafterl nexttoward"), Lib("C11", "<math.h>", "nexttowardf nexttowardl pow powf powl remainder remainderf remainderl remquo remquof remquol"), Lib("C11", "<math.h>", "rint rintf rintl round roundf roundl scalbln scalblnf scalblnl scalbn scalbnf scalbnl signbit"), Lib("C11", "<math.h>", "sin sinf sinh sinhf sinhl sinl sqrt sqrtf sqrtl tan tanf tanh tanhf tanhl tanl tgamma tgammaf"), Lib("C11", "<math.h>", "tgammal trunc truncf truncl"), Lib("C11", "<stdalign.h>", "alignas alignof"), Lib("C11", "<time.h>", "asctime clock clock_t ctime difftime gmtime localtime mktime strftime time time_t timespec_get"), Lib("C11", "<assert.h>", "assert static_assert"), Lib("C11", "<stdbool.h>", "bool false true"), Lib("C11", "<wchar.h>", "btowc fgetwc fgetws fputwc fputws fwide fwprintf fwscanf getwc getwchar mbrlen mbrtowc mbsinit"), Lib("C11", "<wchar.h>", "mbsrtowcs mbstate_t putwc putwchar swprintf swscanf ungetwc vfwprintf vfwscanf vswprintf vswscanf"), Lib("C11", "<wchar.h>", "vwprintf vwscanf wcrtomb wcscat wcschr wcscmp wcscoll wcscpy wcscspn wcsftime wcslen wcsncat"), Lib("C11", "<wchar.h>", "wcsncmp wcsncpy wcspbrk wcsrchr wcsrtombs wcsspn wcsstr wcstod wcstof wcstok wcstol wcstold"), Lib("C11", "<wchar.h>", "wcstoll wcstoul wcstoull wcsxfrm wctob wint_t wmemchr wmemcmp wmemcpy wmemmove wmemset wprintf"), Lib("C11", "<wchar.h>", "wscanf"), Lib("C11", "<uchar.h>", "c16rtomb c32rtomb char16_t char32_t mbrtoc16 mbrtoc32"), Lib("C11", "<errno.h>", "errno"), Lib("C11", "<fenv.h>", "feclearexcept fegetenv fegetexceptflag fegetround feholdexcept fenv_t feraiseexcept fesetenv"), Lib("C11", "<fenv.h>", "fesetexceptflag fesetround fetestexcept feupdateenv fexcept_t"), Lib("C11", "<inttypes.h>", "imaxabs imaxdiv imaxdiv_t strtoimax strtoumax wcstoimax wcstoumax"), Lib("C11", "<stdint.h>", "int16_t int32_t int64_t int8_t int_fast16_t int_fast32_t int_fast64_t int_fast8_t int_least16_t"), Lib("C11", "<stdint.h>", "int_least32_t int_least64_t int_least8_t intmax_t intptr_t uint16_t uint32_t uint64_t uint8_t"), Lib("C11", "<stdint.h>", "uint_fast16_t uint_fast32_t uint_fast64_t uint_fast8_t uint_least16_t uint_least32_t uint_least64_t"), Lib("C11", "<stdint.h>", "uint_least8_t uintmax_t uintptr_t"), Lib("C11", "<ctype.h>", "isalnum isalpha isblank iscntrl isdigit isgraph islower isprint ispunct isspace isupper isxdigit"), Lib("C11", "<ctype.h>", "tolower toupper"), Lib("C11", "<wctype.h>", "iswalnum iswalpha iswblank iswcntrl iswctype iswdigit iswgraph iswlower iswprint iswpunct iswspace"), Lib("C11", "<wctype.h>", "iswupper iswxdigit towctrans towlower towupper wctrans wctrans_t wctype wctype_t"), Lib("C11", "<setjmp.h>", "jmp_buf longjmp setjmp"), Lib("C11", "<stdatomic.h>", "kill_dependency"), Lib("C11", "<locale.h>", "localeconv setlocale"), Lib("C11", "<stddef.h>", "max_align_t offsetof ptrdiff_t size_t wchar_t"), Lib("C11", "<string.h>", "memchr memcmp memcpy memmove memset strcat strchr strcmp strcoll strcpy strcspn strerror strlen"), Lib("C11", "<string.h>", "strncat strncmp strncpy strpbrk strrchr strspn strstr strtok strxfrm"), Lib("C11", "<stdnoreturn.h>", "noreturn"), Lib("C11", "<signal.h>", "raise sig_atomic_t signal"), Lib("C11", "<stdarg.h>", "va_arg va_copy va_end va_list va_start"), Lib("POSIX", "<sys/socket.h>", "accept bind connect getpeername getsockname getsockopt listen recv recvfrom recvmsg send sendmsg"), Lib("POSIX", "<sys/socket.h>", "sendto setsockopt shutdown socket socketpair"), Lib("POSIX", "<unistd.h>", "access alarm chdir chown close confstr dup dup2 environ execl execle execlp execv execve execvp"), Lib("POSIX", "<unistd.h>", "fchdir fchown fdatasync fexecve fork fpathconf fsync ftruncate getcwd getegid geteuid getgid"), Lib("POSIX", "<unistd.h>", "gethostname getlogin getopt getpgid getpgrp getpid getppid getsid getuid isatty lchown link"), Lib("POSIX", "<unistd.h>", "linkat lseek nice optarg opterr optind optopt pathconf pause pipe pread pwrite read readlink"), Lib("POSIX", "<unistd.h>", "rmdir setgid setpgid setsid setuid sleep symlink sync sysconf truncate ttyname unlink unlinkat"), Lib("POSIX", "<unistd.h>", "usleep write"), Lib("POSIX", "<dirent.h>", "alphasort closedir dirfd fdopendir opendir readdir readdir_r rewinddir scandir seekdir telldir"), Lib("POSIX", "<time.h>", "asctime_r clock_getres clock_gettime clock_nanosleep clock_settime ctime_r daylight gmtime_r"), Lib("POSIX", "<time.h>", "localtime_r nanosleep strptime timer_create timer_delete timer_gettime timer_settime timezone"), Lib("POSIX", "<time.h>", "tzname tzset"), Lib("POSIX", "<libgen.h>", "basename dirname"), Lib("POSIX", "<strings.h>", "bcmp bcopy bzero ffs index rindex strcasecmp strncasecmp"), Lib("POSIX", "<sys/types.h>", "blkcnt_t blksize_t dev_t gid_t ino_t mode_t nlink_t off_t pid_t ssize_t suseconds_t uid_t useconds_t"), Lib("POSIX", "<termios.h>", "cfmakeraw tcgetattr tcsetattr"), Lib("POSIX", "<sys/stat.h>", "chmod fchmod fstat fstatat futimens lstat mkdir mkdirat mkfifo mknod stat umask utimensat"), Lib("POSIX", "<syslog.h>", "closelog openlog setlogmask syslog"), Lib("POSIX", "<fcntl.h>", "creat fcntl open openat"), Lib("POSIX", "<stdio.h>", "ctermid dprintf fdopen fileno flockfile fmemopen fseeko ftello funlockfile getc_unlocked getchar_unlocked"), Lib("POSIX", "<stdio.h>", "getdelim getline open_memstream pclose popen putc_unlocked putchar_unlocked vdprintf"), Lib("POSIX", "<dlfcn.h>", "dlclose dlerror dlopen dlsym"), Lib("POSIX", "<stdlib.h>", "drand48 erand48 grantpt initstate jrand48 lrand48 mkdtemp mkstemp mrand48 nrand48 ptsname putenv"), Lib("POSIX", "<stdlib.h>", "rand_r random realpath setenv setstate srand48 srandom unlockpt unsetenv"), Lib("POSIX", "<fnmatch.h>", "fnmatch"), Lib("POSIX", "<netdb.h>", "freeaddrinfo gai_strerror getaddrinfo getnameinfo"), Lib("POSIX", "<grp.h>", "getgrgid getgrnam"), Lib("POSIX", "<sys/resource.h>", "getpriority getrlimit getrusage setpriority setrlimit"), Lib("POSIX", "<pwd.h>", "getpwnam getpwnam_r getpwuid getpwuid_r"), Lib("POSIX", "<sys/time.h>", "gettimeofday select utimes"), Lib("POSIX", "<glob.h>", "glob globfree"), Lib("POSIX", "<arpa/inet.h>", "htonl htons inet_addr inet_ntoa inet_ntop inet_pton ntohl ntohs"), Lib("POSIX", "<sys/ioctl.h>", "ioctl"), Lib("POSIX", "<signal.h>", "kill killpg psignal sigaction sigaddset sigaltstack sigdelset sigemptyset sigfillset sigismember"), Lib("POSIX", "<signal.h>", "sigprocmask sigqueue sigsuspend sigwait"), Lib("POSIX", "<string.h>", "memccpy stpcpy stpncpy strdup strerror_r strndup strnlen strsignal strtok_r"), Lib("POSIX", "<sys/mman.h>", "mlock mmap mprotect msync munlock munmap shm_open shm_unlink"), Lib("POSIX", "<poll.h>", "poll"), Lib("POSIX", "<sys/select.h>", "pselect"), Lib("POSIX", "<sys/uio.h>", "readv writev"), Lib("POSIX", "<regex.h>", "regcomp regerror regexec regfree"), Lib("POSIX", "<sys/utsname.h>", "uname"), Lib("POSIX", "<utime.h>", "utime"), Lib("POSIX", "<sys/wait.h>", "wait waitid waitpid"), )`
* `type Prefix` — A prefix C, POSIX or this compiler reserves; `follow` 1: a lowercase…
* `` const [16]Prefix PREFIXES = ( Prefix("dado_", 0, "this compiler"), Prefix("DADO_", 0, "this compiler"), Prefix("atomic_", 1, "C11's `<stdatomic.h>`"), Prefix("memory_order_", 1, "C11's `<stdatomic.h>`"), Prefix("ATOMIC_", 2, "C11's `<stdatomic.h>`"), Prefix("cnd_", 1, "C11's `<threads.h>`"), Prefix("mtx_", 1, "C11's `<threads.h>`"), Prefix("thrd_", 1, "C11's `<threads.h>`"), Prefix("tss_", 1, "C11's `<threads.h>`"), Prefix("aio_", 0, "POSIX's `<aio.h>`"), Prefix("lio_", 0, "POSIX's `<aio.h>`"), Prefix("mq_", 0, "POSIX's `<mqueue.h>`"), Prefix("posix_", 0, "POSIX, for all of its headers"), Prefix("pthread_", 0, "POSIX's `<pthread.h>`"), Prefix("sched_", 0, "POSIX's `<sched.h>`"), Prefix("sem_", 0, "POSIX's `<semaphore.h>`"), ) ``
* `const string8 REPLACES = "an export is defined at file scope of the one translation unit that also holds Dado's runtime and every included header, so it replaces the library's symbol for the whole program — the runtime's own calls included — or collides with the header's declaration of it"`
* `string8 reserved_by(string8 s, ^string8 why, ^bool lib)` — Who reserves C symbol `s`, or ""; `why` is the consequence and `lib`
* `void export_name(^World w, u32 d)` — ERR1007: an export's C name is someone else's.
* `void export_rules(^World w)` — What `export` may stand on, and the names an export may not claim (ERR1005,…
* `const u8 RK_NONE = 0`
* `const u8 RK_FIXED = 1`
* `const u8 RK_SLICE = 2`
* `const u8 RK_REF = 3`
* `const u8 RK_TEXT = 4`
* `const u8 RK_ONE = 5`
* `const u8 RK_PTR = 6`
* `type Run`
* `Run run_shape(^World w, u32 ty)`
* `bool is_handle(^World w, u32 ty)` — A `ref T` or `ref []T`.
* `bool one_ref(^World w, u32 r)` — A `ref T` of one `T` (not a run).
* `bool is_ref(^World w, u32 ty)` — The `ref []T` of slice type `s`, and the view it lends.
* `i64 leaves(^World w, u32 ty)` — How many slots a shape holds, through nested tuples.
* `u32 array_type(^World w, u32 f, u32 n)` — `[N]T`: a tuple of N `T`s, `[1]T` its element (A2).
* `u32 length(^World w, ^Body b, u32 p, Run k, u32 n)` — A run's length as an `i64` operand.
* `u32 position(^World w, ^Body b, u32 n, string8 what = "an index")` — An index or bound: an integer, as an `i64` operand; NONE when reported.
* `Val index_(^World w, ^Body b, u32 n)` — `xs[i]`: a slot of a tuple by a constant, else a checked position.
* `bool marker_ok(^World w, ^Body b, u32 m, string8 what)` — The `#` name in front of a checked operand (token `m`): whether it waives…
* `Val index_in(^World w, ^Body b, u32 n, Val base, bool past, bool writing)`
* `Val window(^World w, ^Body b, u32 n)` — `xs[lo..<hi]`, `xs[:]`: a view, `string8` over text.
* `Val deref(^World w, ^Body b, u32 n)` — `p^`: the place a pointer points at.
* `Val address(^World w, ^Body b, u32 n)` — `&x`: the address of a place; a unit of text has none.
* `bool written_one(^World w, u32 f, u32 tn)` — A `[1]T` declaration keeps its run of one for windows.
* `Val array_lit(^World w, ^Body b, u32 n, u32 want)` — `[a, b, c]`: a fixed array; where a `[]T` is wanted, a view of one in the…
* `void run_loop(^World w, ^Body b, u32 n, []u32 binds, u32 it)` — `for x in xs`, `for x, i in xs`, `for &x in xs`: the length once, then…
* `void arity_here(^World w, u32 f, u32 tok)` — ERR0918: a length binder `$N` written outside a signature.
* `u32 text_named(string8 s)` — `string8`, `string16`, `string32` by name, else `NONE`.
* `u32 text_of(u32 u)` — The string whose unit is `u`.
* `` const string8 STRING_KIND_WHY = "`string` is the kind of a string literal and not a type: a `\"…\"` has no width until it is placed" `` — Why a written `string` is no type; each door appends its repair.
* `u32 imported_decl(^World w, u32 f, u32 q, u32 n)` — `pkg.NAME`: the declaration an import of file `f` named by `q` binds, or NONE.
* `const u8 B_SAWAIT = 162`
* `u32 sa_type(^World w, u32 inner)` — `async T`.
* `u32 sa_inner(^World w, u32 t)` — What `async T` answers, or NONE when `t` is none.
* `u32 sa_held(^World w, u32 ty, i64 depth = 0)` — The `async T` `ty` holds (itself, or in an element, a map or a slot), or NONE.
* `bool sa_refused(^World w, u32 f, u32 m, bool generic)` — ERR1330 for an `async` `init` or generic method (`m` its node in file `f`).
* `void sa_await(^World w, ^Body b, u32 n)` — `await <stmt>`.
* `Val sa_awaited(^World w, ^Body b, u32 n, u32 want)` — The awaited value `n` (its expected type `want`): its `T`.
* `bool sa_returned(^World w, ^Body b, u32 vn, Val v)` — `return x` where `x` is the `async T` of the method's `T`: ERR1330.
* `void sa_discarded(^World w, ^Body b, u32 s, u32 e, u32 ty)` — A call answering `async T` alone on a line: ERR1330.
* `bool sa_deferred(^World w, ^Body b, u32 s)` — `defer await …`: ERR1330.
* `bool sa_operator(^World w, ^Body b, u32 n, string8 op, u32 lt, u32 rt)` — An operator over `async T`: ERR0607; false when refused.
* `bool sa_printed(^World w, ^Body b, u32 g, u32 ty, string8 verb)` — `print` of a value holding `async T`: ERR1330; false when refused.
* `u32 sa_kw(^World w, u32 f, u32 m)` — Where the `async` heading method node `m` of file `f` begins.
* `bool st_async_self(^World w, u32 m)` — A trait method `async Self m(…)`: not built.
* `u32 sc_top_super(^World w, u32 d)` — The first `super(…)` statement of method `d`'s own body, if it is `init`.
* `Val sc_call(^World w, ^Body b, u32 n, u32 c)` — `super(…)` (`c` NONE) or `super.m(…)` (`c` the callee).
* `Val sc_value(^World w, ^Body b, u32 n, u32 sn)` — `super` read as a value or a field: ERR1310.
* `void sc_enter(^World w, ^Body b)` — Before `init`'s body: with no `super(…)`, the parent's `init` runs first…
* `bool sc_upcast(^World w, u32 got, u32 want)` — Whether an instance of class type `got` is one of `want`: `want` is in its chain.
* `void sc_rules(^World w)` — A subclass's field naming an inherited one (ERR0409), and an override of…
* `const u8 B_SARRAY = 146`
* `const u8 B_SMAP = 147`
* `const u8 B_SINDEX = 148`
* `const u8 B_SSTORE = 149`
* `const u8 B_SMETHOD = 150`
* `u8 s_kind(^World w, u32 ty)` — What a container type is: 1 an array, 2 a map, 3 a string, 0 neither.
* `bool s_tup(^World w, u32 ty)` — A fixed array or a tuple: a value of slots.
* `u32 s_held(^World w, u32 ty, i64 depth)` — The first array or map a fixed array or a tuple of `ty` holds.
* `bool s_cont_op(^World w, ^Body b, u32 n, string8 op, u32 lt, u32 rt, u32 ln, u32 rn)` — ERR0607 for an operator over an array or a map, or `==` over a shape…
* `bool s_searchable(^World w, ^Body b, u32 n, u32 et, string8 what)` — ERR0607 for a search by `==` over elements of `et` that have none.
* `Val s_array(^World w, ^Body b, u32 n, u32 want)` — `[a, b, …]`: its element type from what is wanted, else its first element.
* `Val s_map(^World w, ^Body b, u32 n, u32 want)` — `{}` and `{k: v, …}`.
* `Val s_index(^World w, ^Body b, u32 n)` — `c[i]`: an element, a map's value, a string's character.
* `void s_store(^World w, ^Body b, u32 n, u32 pn, u32 vn, string8 op)` — `c[i] = v` and `c[i] op= v`.
* `bool s_for_in(^World w, ^Body b, u32 n, []u32 binds, u32 it)` — `for x in c:` and `for k, v in m:`: an `O_LOOP` of sub 1 (an array), 2 (a…
* `type SRow`
* `const [49]SRow SROWS = (SRow(1, "decode_utf8", "", 'S'), SRow(1, "resize", "I", 'V'), SRow(3, "replace_all", "SS", 'S'), SRow(3, "code_point_at", "I", 'I'), SRow(3, "to_int", "", 'I'), SRow(3, "to_float", "", 'F'), SRow(3, "bytes", "", 'Y'), SRow(1, "len", "", 'I'), SRow(1, "is_empty", "", 'B'), SRow(1, "append", "E", 'V'), SRow(1, "insert", "IE", 'V'), SRow(1, "fill", "E", 'V'), SRow(1, "sort", "", 'V'), SRow(1, "reverse", "", 'V'), SRow(1, "copy", "", 'R'), SRow(1, "join", "S", 'S'), SRow(1, "contains", "E", 'B'), SRow(1, "index_of", "E", 'I'), SRow(1, "last_index_of", "E", 'I'), SRow(1, "append_array", "A", 'V'), SRow(1, "remove_at", "I", 'E'), SRow(1, "pop", "", 'E'), SRow(1, "pop_front", "", 'E'), SRow(1, "front", "", 'E'), SRow(1, "back", "", 'E'), SRow(1, "clear", "", 'V'), SRow(2, "len", "", 'I'), SRow(2, "is_empty", "", 'B'), SRow(2, "has", "K", 'B'), SRow(2, "delete", "K", 'B'), SRow(2, "get", "KE", 'E'), SRow(2, "clear", "", 'V'), SRow(2, "keys", "", 'Q'), SRow(2, "values", "", 'W'), SRow(2, "copy", "", 'R'), SRow(3, "len", "", 'I'), SRow(3, "is_empty", "", 'B'), SRow(3, "contains", "S", 'B'), SRow(3, "starts_with", "S", 'B'), SRow(3, "ends_with", "S", 'B'), SRow(3, "split", "S", 'L'), SRow(3, "trim", "", 'S'), SRow(3, "trim_left", "", 'S'), SRow(3, "trim_right", "", 'S'), SRow(3, "to_upper", "", 'S'), SRow(3, "to_lower", "", 'S'), SRow(3, "index_of", "S", 'I'), SRow(3, "last_index_of", "S", 'I'), SRow(3, "repeat", "I", 'S'))`
* `Val s_value_call(^World w, ^Body b, u32 n, u32 cn, ^bool done)` — `c.m(args…)` on an array, a map or a string; `bad()` with `done` unset…
* `type SSpell` — Another language's spelling, or a method DadoScript chose not to have, and…
* `` const [76]SSpell SSPELL = (SSpell("SMTF", "size", "`len()`"), SSpell("SMTF", "length", "`len()`"), SSpell("SMT", "count", "`len()` counts every element; to count some, `filter` and then `len()`"), SSpell("S", "push", "`append(v)`"), SSpell("S", "push_back", "`append(v)`"), SSpell("S", "add", "`append(v)`"), SSpell("S", "extend", "`append_array(vs)`"), SSpell("S", "concat", "`append_array(vs)` on a `copy()`"), SSpell("S", "pop_back", "`pop()`"), SSpell("S", "shift", "`pop_front()`"), SSpell("S", "unshift", "`insert(0, v)`"), SSpell("S", "push_front", "`insert(0, v)`"), SSpell("S", "remove", "`remove_at(i)`, with the position from `index_of(v)`"), SSpell("S", "erase", "`remove_at(i)`"), SSpell("S", "pop_at", "`remove_at(i)`"), SSpell("S", "splice", "`remove_at(i)` or `insert(i, v)`"), SSpell("S", "has", "`contains(v)`"), SSpell("ST", "includes", "`contains(v)`"), SSpell("ST", "find", "`index_of(v)`"), SSpell("ST", "indexOf", "`index_of(v)`"), SSpell("ST", "rfind", "`last_index_of(v)`"), SSpell("ST", "lastIndexOf", "`last_index_of(v)`"), SSpell("S", "find_custom", "`find_if(p)`"), SSpell("S", "findIndex", "`find_if(p)`"), SSpell("S", "some", "`any(p)`"), SSpell("S", "every", "`all(p)`"), SSpell("SM", "duplicate", "`copy()`"), SSpell("SM", "clone", "`copy()`"), SSpell("S", "sort_custom", "`sort(less)`"), SSpell("S", "slice", "a slice, `xs[a..<b]`, which is a new array"), SSpell("S", "at", "an index, `xs[i]`"), SSpell("S", "get", "an index, `xs[i]`"), SSpell("S", "set", "a store, `xs[i] = v`"), SSpell("M", "contains", "`has(k)`"), SSpell("M", "contains_key", "`has(k)`"), SSpell("M", "has_key", "`has(k)`"), SSpell("M", "erase", "`delete(k)`"), SSpell("M", "remove", "`delete(k)`"), SSpell("M", "set", "a store, `m[k] = v`"), SSpell("M", "put", "a store, `m[k] = v`"), SSpell("M", "insert", "a store, `m[k] = v`"), SSpell("M", "merge", "a loop: `for k, v in other: m[k] = v`"), SSpell("M", "entries", "a loop: `for k, v in m:`"), SSpell("M", "items", "a loop: `for k, v in m:`"), SSpell("T", "startsWith", "`starts_with(p)`"), SSpell("T", "begins_with", "`starts_with(p)`"), SSpell("T", "endsWith", "`ends_with(p)`"), SSpell("T", "has", "`contains(s)`"), SSpell("T", "strip", "`trim()`"), SSpell("T", "strip_edges", "`trim()`"), SSpell("T", "trimStart", "`trim_left()`"), SSpell("T", "lstrip", "`trim_left()`"), SSpell("T", "trimEnd", "`trim_right()`"), SSpell("T", "rstrip", "`trim_right()`"), SSpell("T", "toUpperCase", "`to_upper()`"), SSpell("T", "upper", "`to_upper()`"), SSpell("T", "toLowerCase", "`to_lower()`"), SSpell("T", "lower", "`to_lower()`"), SSpell("T", "replace", "`replace_all(a, b)`"), SSpell("T", "replaceAll", "`replace_all(a, b)`"), SSpell("T", "substring", "a slice, `s[a..<b]`"), SSpell("T", "substr", "a slice, `s[a..<b]`"), SSpell("T", "slice", "a slice, `s[a..<b]`"), SSpell("T", "charAt", "an index, `s[i]`"), SSpell("T", "at", "an index, `s[i]`"), SSpell("T", "chars", "a loop over the code points: `for c in s:`"), SSpell("T", "codePointAt", "`code_point_at(i)`"), SSpell("T", "unicode_at", "`code_point_at(i)`"), SSpell("T", "ord", "`code_point_at(i)`"), SSpell("T", "to_utf8_buffer", "`bytes()`"), SSpell("T", "encode", "`bytes()`"), SSpell("T", "parseInt", "`to_int()`"), SSpell("T", "parseFloat", "`to_float()`"), SSpell("T", "join", "`join(sep)` on the `[]string` of the parts: `parts.join(s)`"), SSpell("B", "get_string_from_utf8", "`decode_utf8()`"), SSpell("B", "decode", "`decode_utf8()`")) ``
* `const [26]string8 SEQ_M = ("len", "is_empty", "append", "append_array", "insert", "pop", "pop_front", "remove_at", "clear", "resize", "front", "back", "contains", "index_of", "last_index_of", "reverse", "sort", "filter", "map", "reduce", "any", "all", "find_if", "copy", "fill", "join")`
* `const [9]string8 MAP_M = ("len", "is_empty", "has", "get", "delete", "clear", "keys", "values", "copy")`
* `const [19]string8 STR_M = ("len", "is_empty", "contains", "starts_with", "ends_with", "index_of", "last_index_of", "split", "trim", "trim_left", "trim_right", "to_upper", "to_lower", "replace_all", "repeat", "bytes", "code_point_at", "to_int", "to_float")`
* `const u8 B_SSLICE = 161`
* `Val s_slice(^World w, ^Body b, u32 n)` — `xs[a..<b]` and `s[a..=b]`: a new array, or a string.
* `const u8 K_CLASS = 15`
* `type SClass`
* `const [5]string8 S_BUILTINS = ("print", "println", "string", "int", "float")`
* `bool ds_file(^World w, u32 f)`
* `bool ds_at(^World w, u32 f, u32 n)` — Whether node `n` of file `f` is DadoScript: a `.dados` file, or inside a…
* `bool ds_off(^World w, u32 f, u32 at)` — Whether offset `at` of file `f` is DadoScript.
* `void s_declare(^World w, u32 pkg, u32 f)` — A `.dados` file's class: named by its header, else its basename.
* `void s_declare_block(^World w, u32 pkg, u32 f, u32 n, string8 outer = "")` — A `class` block in a `.dado` file: `[extends, using…, items…]`.
* `string8 s_qual(^World w, u32 d)` — `Class.Name` of a class member `d`.
* `void s_enum_body(^World w, u32 d, u32 ty)` — A script's enum: closed, its members their names (JS strings).
* `string8 s_short(^World w, u32 d)` — A class's name without its outer classes.
* `u32 s_inner(^World w, u32 f, u32 n, u32 pkg)` — Name `n` of file `f` as an inner class of a class around it, or NONE.
* `u32 s_member_type(^World w, u32 cd, u32 sym)` — `Class.T`, a type written in class `cd`: its declaration, or NONE.
* `bool s_enum(^World w, u32 ty)` — Whether `ty` is a script's enum.
* `u32 s_enum_zero(^World w, u32 ty)` — Its first member.
* `ref []string8 s_enum_names(^World w, u32 d, []u32 seen)` — A script enum's members in order, but those in `seen` (konsts).
* `string8 s_ticked(ref []string8 xs)`
* `Val s_no_member(^World w, ^Body b, u32 n, u32 d)` — ERR0410: `Mood.Happy`.
* `u8 s_switch_on(^World w, ^Body b, u32 at, u32 ty)` — What a script's `switch` is over: 1 its enum, 2 `int` or `string`, 3 refused.
* `void s_case_wrong(^World w, ^Body b, u32 vn, u32 ty, u32 got)`
* `void s_unnamed(^World w, ^Body b, u32 n, u32 ty, []u32 seen)` — LNT0731 over a script's enum, `else` or not.
* `u32 s_parent(^World w, u32 ci)` — The class `ci` extends (NONE: `Script`), ERR1310 for a Dado type or a cycle.
* `void s_all_fields(^World w, u32 ci, ^ref []u32 files, ^ref []u32 nodes)` — Every field of class `ci`, inherited ones first: (file, node) pairs.
* `u32 s_class_row(^World w, u32 d)` — The class's row: its fields, by name, as one named shape.
* `u32 s_type_name(^World w, u32 f, u32 n)` — A script's own type names; NONE-1 when `s` is none of them.
* `i64 s_field_ix(^World w, ^Body b, u32 sym)` — The field `sym` of the body's class: its index, or -1.
* `Val s_self(^World w, ^Body b, u32 n)`
* `Val s_field_use(^World w, ^Body b, u32 n, u32 sym, i64 i)` — A field read or written through `self`, with the initializer order rule.
* `bool s_hidden(^World w, ^Body b, u32 n, u32 oc, u32 mf, u32 mn)` — ERR0413: member `mn` of file `mf`, of class `oc`, private beyond here.
* `Val s_name(^World w, ^Body b, u32 n)` — A bare name in a script body.
* `u32 s_method_of(^World w, ^Body b, u32 sym)`
* `u32 s_method_in(^World w, u32 ci, u32 sym)` — Method `sym` of class `ci` or its ancestors, or NONE.
* `Val s_field(^World w, ^Body b, u32 n)` — `self.x` and `obj.x`.
* `const u8 B_SPROP = 154`
* `void s_try(^World w, ^Body b, u32 n)` — A script's `try S else e:`: the statement in a JS `try`, the suite its…
* `Val s_place(^World w, ^Body b, u32 n, bool read)` — A place: a local, or a field of `self`.
* `Val s_call(^World w, ^Body b, u32 n, u32 want)` — A call in a script body.
* `const u8 B_SANY = 156` — `any`'s dynamic verbs (`B_SANY`, `b` the kind): 0 `globalThis`, 1 a…
* `Val s_dyn(^World w, ^Body b, u32 n, u32 kind, u32 c, []u32 xs)`
* `void s_any_store(^World w, ^Body b, u32 n, Val base, u32 key, u32 vn, string8 op)` — `a.name = v` and `a[k] = v` through an `any` (kind 6).
* `bool s_dyn_root(^World w, ^Body b, u32 c)` — Whether field `c`'s base reaches back to `globalThis` or an `any` local.
* `bool s_any_op(^World w, ^Body b, u32 n, string8 op, Val l, Val r, u32 ln, u32 rn)` — ERR0607 for an operator over an `any`; false when refused.
* `bool s_args(^World w, ^Body b, u32 n, u32 m, string8 mn, ^ref []u32 opds)` — The arguments of a call of method `m` (named `mn` in messages), checked…
* `ref []u32 s_mparams(^World w, u32 ft)` — A method type's parameters past `self`, copied.
* `bool s_args_of(^World w, ^Body b, u32 n, []u32 ps, u32 mf, []u32 qs, string8 mn, ^ref []u32 opds)` — Arguments against parameter types `ps`, named by nodes `qs` of file `mf`.
* `Val s_method(^World w, ^Body b, u32 n, u32 m, u32 inst = NONE)` — Method `m` called on `inst` (NONE: `self`).
* `Val s_string_op(^World w, ^Body b, u32 n, string8 op, Val l, Val r, u32 ln, u32 rn)` — `+`, `==`, `!=` over strings; any other operator is refused.
* `string8 s_conv_help(^World w, ^Body b, u32 want, u32 got, u32 at)` — The help a script's mismatch gives: the conversion it never makes itself.
* `void s_fields(^World w, ^Body b)` — The class's field initializers, in order, each reading the fields above it.
* `void s_hooks(^World w, u32 d)` — ERR1304: `init` and `ready`'s fixed signatures.
* `const u8 B_SIG_CONNECT = 140` — IR verbs over a signal: args are the instance, then the receiver or the values.
* `const u8 B_SIG_DISCONNECT = 141`
* `const u8 B_SIG_EMIT = 142`
* `const u8 B_PRINT = 143`
* `const u8 B_PRINTLN = 144`
* `const u8 B_SNEW = 151`
* `u32 s_signal(^World w, u32 ci, u32 sym)` — Signal `sym` of class `ci`: its node, or NONE.
* `u32 s_class_of_ty(^World w, u32 ty)` — The class a value of type `ty` is an instance of, or NONE.
* `Val s_not_a_value(^World w, ^Body b, u32 n, u32 sym)` — ERR1362: a signal named anywhere but as `.connect`/`.disconnect`/`.emit`'s receiver.
* `u64 s_signal_ref(^World w, ^Body b, u32 x)` — The instance and signal a verb's receiver `x` names (script and Dado…
* `Val s_signal_call(^World w, ^Body b, u32 n, u64 ref_, string8 verb)` — `died.connect(f)`, `died.disconnect(f)`, `died.emit(…)`, either dialect.
* `string8 s_platform(^World w, u32 f, u32 n, string8 x, bool call, i64 nargs)` — `Platform`: its members and `os()` are strings fixed when the script is…
* `bool s_is_platform(^World w, ^Body b, u32 f, u32 n)` — Whether `n` names `Platform` (nothing of the script's own takes the name).
* `string8 s_fold(^World w, u32 f, u32 n)` — A script's `when` condition, folded: "T"/"F", a literal's text, or ""…
* `u32 s_when_arm(^World w, u32 f, u32 n)` — The arm a script's `when` takes (NONE: none, or refused).
* `const u8 B_SJS = 157`
* `bool s_fn(^World w, u32 ty)` — Closures: a lambda as a value, a method read as one (bound to its…
* `Val s_lambda_expr(^World w, ^Body b, u32 g, u32 want)`
* `Val s_method_val(^World w, ^Body b, u32 n, u32 inst, u32 m)`
* `Val s_to_location(^World w, ^Body b, Val v, u32 at)` — A function where a `Location` is written: where it begins (`B_SLOC`:
* `Val s_located(^World w, ^Body b, u32 n, u32 f, u32 at, string8 fname)`
* `const u8 B_SLOC = 158`
* `string8 s_fn_help(^World w, ^Body b, u32 want, u32 got, u32 at)` — The help where a function meets another type.
* `u32 s_lambda_value(^World w, ^Body b, u32 g, []u32 lps, u32 ret)` — A lambda's value: `O_BLOCK` sub 1, its body checked over `lps`.
* `bool crosses(^World w, u32 dado, u32 script, ^string8 why)` — The one crossing relation between a Dado type and a script type (either…
* `bool js_path(string8 sp)`
* `u64 js_key(u32 f, u32 x, u64 tag)`
* `string8 utf8_of(u32 c)` — Code point `c` as UTF-8.
* `const u8 B_SDADO = 152`
* `const u8 B_SNARROW = 153`
* `const u8 SK_NONE = 0`
* `const u8 SK_INT = 1`
* `const u8 SK_FLOAT = 2`
* `const u8 SK_BOOL = 3`
* `const u8 SK_STR = 4`
* `const u8 SK_OWNED = 5`
* `const u8 SK_RUN = 6`
* `const u8 SK_INST = 7`
* `const u8 SK_C32 = 8`
* `const u8 SK_C16 = 9`
* `const u8 SK_CB = 10`
* `const u8 SK_TUP = 11`
* `const u8 SK_MAP = 12`
* `const u8 SK_FNV = 13`
* `const u8 SK_UNION = 14`
* `const u8 SK_ENUM = 15`
* `const u8 SK_WIDE = 16`
* `const [12]string8 SEAM_WIDE_RT = ("read_units", "drop_units", "give_units", "mix_units", "read_points", "drop_points", "give_points", "mix_points", "read_cstring", "drop_cstring", "give_cstring", "mix_cstring")` — The `core:script` glue a wide string crosses by.
* `bool sk_str(u8 k)` — A kind crossing as the script's `string`.
* `const [24]string8 SEAM_RUN_RT = ("new_array", "put", "give_float", "give_bool", "read_int", "read_float", "read_bool", "read_string", "read_length", "element", "free_value", "scratch", "unscratch", "put_in", "array_length", "same_object", "read_exactly", "lend_script", "unlend_script", "lent_written", "mix", "clear_exception", "read_owned_string", "read_same_length")` — What a run's glue calls, from `core:script`.
* `const [16]string8 SEAM_RT = ("typed_entered", "typed_left", "take_int", "take_float", "take_bool", "take_string", "drop_string", "give_int", "give_string", "give_owned_string", "raised", "failed", "take_char32", "take_char16", "give_char32", "give_char16")` — The helpers a thunk calls, from `core:script`.
* `u8 seam_kind(^World w, u32 ty, bool ret)` — How a Dado type crosses (`ret`: as an answer).
* `u32 s_soa_lane(^World w, u32 r)` — The first lane of `#soa` row `r` that is no scalar or string, or NONE.
* `u32 fnv_sty(^World w, u32 r)` — The script function type Dado function type `r` crosses as: scalar and…
* `void fnv_bind(^World w, u32 r)` — Each function the program names as a value of Dado type `r`, bound so a…
* `bool fnv_settle(^World w)` — After the bodies: every function named as a value of a type that crosses…
* `void seam_maps(^World w)` — What a map's glue calls, kept live.
* `u32 run_elem(^World w, u32 r, bool ret)` — A `[]U` (with `ret`, a `ref []U` too): `U`, else NONE.
* `u32 seam_sty(^World w, u32 ty, bool ret)` — The script type Dado type `ty` crosses as.
* `u32 s_sview(^World w, u32 ty)` — A Dado shape as the script's own: its slots' script types.
* `bool s_tup_fits(^World w, u32 got, u32 want)` — Whether a script value of `got` fits tuple type `want` slot by slot: the…
* `bool s_slot_fits(^World w, u32 g, u32 x)`
* `u32 sk_script(^World w, u8 k)`
* `Val s_dado_call(^World w, ^Body b, u32 n, u32 d, u32 recv = NONE)` — A script's call of Dado function `d`, written `c(args…)`.
* `u32 seam_rt(^World w, string8 nm)` — The C name of `core:script`'s `nm`, kept live.
* `u32 seam_c(^World w, string8 nm)` — The C name `seam_rt` kept for `nm`, or NONE.
* `Val host_connect(^World w, ^Body b, u32 n, u32 h, u32 ci, u32 sg, u32 d, bool on, u32 p)` — `g.sig.connect(f)` from Dado: the class's module gets `$connect_sig$f`,…
* `string8 seam_name(^World w, u32 d)` — The name a bound Dado function is imported by: `f`, or `T$m` for a method.
* `const u8 B_SRECV = 155`
* `Val s_receive(^World w, ^Body b, u32 n)` — `receive(ch, handler)` in a script (Rust's `dados_receive`): `ch` a…
* `const [5]string8 RECV_RT = ("receive_channel", "deliver_one", "give_int", "give_float", "give_bool")` — What a channel's drain and `receive` call, from `core:script`.
* `u8 cb_form(^World w, u32 ty)` — A callback's form: 1 a bare function type, 2 a `(proc, rawptr data)`
* `u32 cb_fn(^World w, u32 ty)` — A callback's function type, its parameters past a pair's `data`.
* `string8 cb_refused(^World w, u32 ty, ^u32 part)` — Why callback type `ty` does not cross ("" when it does); `part` the…
* `u32 cb_script(^World w, u32 ty, ^ref []u32 ps)` — A callback's script parameters (`ps`) and answer.
* `const [14]string8 CB_RT = ("thunk_entered", "thunk_left", "take_callback", "drop_callback", "take_bare_callback", "callback_begin", "callback_begin_bare", "callback_call", "callback_failed", "callback_end", "give_bytes", "read_int", "read_float", "free_value")` — What a callback's glue calls, from `core:script`.
* `void cb_escapes(^World w)` — Rust's escape check (`script_callbacks`), once every body is checked: a…
* `string8 s_uncross(^World w, u32 ty, bool answer, ^string8 step, ^u32 part, ^string8 rep)` — Why Dado type `ty` does not cross (Rust's `seam_or_refused`), "" when it…
* `Val s_view_make(^World w, ^Body b, u32 n, u32 td, ^bool done)` — A Dado type with a narrow integer slot constructed in a script: built over…
* `type SEmbed` — One `type` a class embeds: its line, the class's own fields above it, the…
* `type SField` — A field of a class: its own (`emb` -1), or slot `slot` of embedding `emb`.
* `ref []SField s_fields_of(^World w, u32 ci)` — Every field of class `ci`, inherited ones first, in written order.
* `u32 se_field_ty(^World w, SField x)` — The script type of field `x`, NONE for an own field (resolved from its node).
* `void se_all(^World w)` — Each class's embeddings declared (their errors said), once.
* `void se_ready(^World w, u32 ci)`
* `u32 se_member(^World w, u32 ci, u32 sym, ^i64 first, ^bool full)` — The member `sym` class `ci` or a parent embeds, read whole: its type, and…
* `Val se_read(^World w, ^Body b, u32 n, u32 ty, i64 first, bool full, u32 base, u32 bty)` — Member `ty` (first slot `first`) read whole.
* `bool se_assign(^World w, ^Body b, u32 n, u32 pn, u32 rhs, string8 op)` — `place = value` where the place is an embedded member named whole: each…
* `u32 se_promoted(^World w, u32 ci, u32 sym, ^u32 mt, ^i64 first, ^bool full, ^u32 owner)` — The promoted method `sym` of class `ci` or a parent: the method, the type…
* `string8 se_answers(^World w, u32 ci, u32 sym)` — ERR0536's help when a promoted method has the name a trait asks.
* `Val se_call(^World w, ^Body b, u32 n, u32 cn, u32 m, u32 mt, i64 first, bool full, u32 owner, u32 base, u32 bty)` — A promoted method called (`cn` its name's node), on `self` when `base` is…
* `void se_promotions(^World w)` — ERR0407 for two embeddings bringing one method; ERR0534 for a class's own…
* `void sx_publish(^World w)` — Every class's published surface, checked.
* `bool s_published_fn(^World w, u32 ci, u32 m)` — Whether method `m` of class `ci` is published as a package function: it reaches no `self`.
* `const u8 B_SERR = 159`
* `string8 sf_fname(^World w, ^Body b)` — `pkg.Class.method` of the body being checked, "" outside a method.
* `string8 s_mpath(^World w, u32 m)` — Method `m` as Rust names it: `pkg.Class.m`, or `pkg.Class.Trait.m` for a…
* `Val sf_here(^World w, ^Body b, u32 n)` — `Location()`: where `n` is.
* `Val sf_error(^World w, ^Body b, u32 n)` — `Error(message)`, `Error(message, code)`, `Error(message, code, at)`.
* `bool sf_here_default(^World w, u32 f, u32 x)` — Whether default `x` of file `f` is `Location()`.
* `Val sf_default(^World w, ^Body b, u32 n, u32 mf, u32 q, u32 pt)` — Parameter `q`'s default (file `mf`) at call `n`, as `pt`.
* `void sf_defaults(^World w, ^Body b)` — A method's `Location()` default whose parameter is no `Location`: ERR0504.
* `const u8 B_SASSERT = 160`
* `bool sf_code_fits(^World w, ^Body b, Val v, u32 n)` — A failure's literal code: nonzero and an `i32` (ERR0715, ERR0539).
* `void sf_fail(^World w, ^Body b, u32 n)` — `fail x` in a script: sub 0 a code, 2 a message, 3 an `Error`.
* `void sf_assert(^World w, ^Body b, u32 n)` — `assert cond` / `assert cond, message`.
* `bool sf_tried(^World w, ^Body b, u32 s)` — Whether `try` statement `s` tries a call or an index.
* `void sf_untried(^World w, ^Body b, u32 s)` — ERR0716 for a `try` over statement `s` with nothing that can fail.
* `bool sf_left(^World w, ^Body b, i64 from)` — Whether an `any` leaves through a check in the ops from `from` on (a `try` can catch it).
* `Val sf_location(^World w, ^Body b, u32 n)` — `Location(m)` and `Location(m, k)`: where method `m` begins, or `k` lines into it.
* `Val sf_load(^World w, ^Body b, u32 n)` — `Script.load(path, ..args)`, checked; a valid one is not built (ERR0001).
* `const u8 B_SLOAD = 163` — `Script.load(path, ..args)`: args the path and the mod's init arguments.
* `type SGLab` — Instance labels: job, then (file << 32 | node) of the call and its text.
* `bool sg_asks(^World w, u32 f, u32 x)` — Whether `x` asks a bound name `T is …`/`T has …`.
* `i8 sg_pred(^World w, u32 f, u32 x, ^u32 at, ^string8 why)` — A `where` or `when` over the bindings in hand (`w.env`): 1, 0, or -1…
* `Val sg_call(^World w, ^Body b, u32 n, u32 m)` — `m(…)` or `self.m(…)` of generic method `m`.
* `Val sg_elsewhere(^World w, ^Body b, u32 n, u32 at, u32 m)` — ERR0001: generic method `m` reached on an instance, from a subclass or through `super`.
* `void sg_label(^World w, u32 j, i64 mark)` — Job `j`'s diagnostics from `mark` on carry its asking call's label.
* `u32 layout_type(^World w, u32 f, u32 n)`
* `u32 soa_row(^types.Types t, u32 ty)` — The `#soa` row under names, qualifiers and tags, or NONE.
* `bool is_soa(^World w, u32 ty)`
* `bool soa_run_named(^World w, ^Body b, u32 n, u32 ty)` — `s.x` over a `#soa`: ERR0525 at the base.
* `u32 soa_elem(^World w, u32 ty)`
* `u32 soa_whole(^World w, ^ir.Fn f, u32 x)` — The `#soa` type whose element place `x` names whole (its last step…
* `u32 before_last(^World w, ^ir.Fn f, u32 x)` — The type of place `x` before its last step, or NONE (no steps).
* `i64 bits_of(^World w, ^ir.Fn f, u32 x)` — The bit width of the foreign bitfield place `x` names, or -1.
* `i64 field_bits(^World w, u32 t, u32 i)`
* `void bits_store(^World w, ^Body b, Val v, u32 ty, i64 bw, u32 at, bool waived)` — `v` stored into a bitfield of `bw` bits: a constant that does not fit is…
* `string8 spell_src(^World w, ^Body b, u32 n)`
* `Val sr_names(^World w, ^Body b, u32 n, u32 c, ^bool done)` — `T.names()`; `done` unset when `T` is no type.
* `string8 sr_other(^World w, ^Body b, ^string8 through)` — What else a bare `slots(…)` could call here, or "".
* `bool sr_header(^World w, ^Body b, u32 it)` — Whether `it` is a header's `slots(…)`.
* `void sr_walk(^World w, ^Body b, u32 n, []u32 binds, u32 it)` — `for name, value in slots(v)` / `for value in slots(v)`.
* `Val sr_elsewhere(^World w, ^Body b, u32 n)` — `slots(v)` anywhere but a header: ERR1350.
* `bool s_decl_ds(^World w, u32 d)` — Whether declaration `d` is a script's.
* `u32 s_denum(^World w, u32 ty)` — The Dado enum declaration type `ty` names, or NONE.
* `u32 s_ddist(^World w, u32 ty)` — A `distinct` row of Dado's own under `ty` (no handle, enum or `#` tag),…
* `u32 s_erase(^World w, u32 ty)` — `ty` with Dado's `distinct` tags taken off.
* `u32 s_dunion(^World w, u32 ty)` — The union row of Dado's own `ty` declares, or NONE.
* `u32 s_dview(^World w, u32 ty)` — The script's type of Dado type `ty`.
* `u32 s_dmember(^World w, u32 m)` — A Dado union's member as the script holds it.
* `string8 s_union_refused(^World w, u32 ty, ^u32 part)` — Why Dado union `ty` does not cross (its member `part`), or "".
* `u32 s_tdecl(^World w, u32 ty)` — The declaration type `ty` names, or NONE.
* `u32 s_dado_of(^World w, u32 st)` — The Dado type script type `st` (a union's or a `distinct` type's view)
* `i64 s_umember(^World w, u32 st, u32 dt)` — The member of a script's union `st` Dado type `dt` crosses into: its…
* `string8 s_cshown(^World w, u32 ty)` — `ty` as Rust shows it beside a class: a handle as `pkg.Class`.
* `const u8 B_SENUM_OF = 164`
* `const u8 B_SENUM_INT = 165`
* `string8 s_ename(^World w, u32 d, i64 v)` — The name value `v` of Dado enum `d` crosses as (the first member declared…
* `Val s_evalue(^World w, u32 ty, i64 v)` — Dado enum `ty`'s value `v` as a script holds it: its member's name, else…
* `Val s_eknst(^World w, Val cv, u32 ty)` — A constant of Dado enum `ty` as a script holds it, or `cv`.
* `Val s_enum_make(^World w, ^Body b, u32 n, u32 ty)` — `Dir(n)` in a script: the value of Dado enum `ty` whose number is `n`.
* `Val s_enum_int(^World w, ^Body b, u32 n, Val v)` — `int(d)` of a value of Dado enum `d`'s type.
* `u32 s_ezero(^World w, u32 ty)` — The zero of Dado enum `ty` as a script holds it.
* `type STrait`
* `u32 st_of_decl(^World w, u32 d)` — The script trait declaration `d` is, or NONE.
* `u32 st_of_ty(^World w, u32 ty, ^bool self_)` — The trait a value of `ty` is (a trait value or a default's `Self`); `self_` set for `Self`.
* `void st_register(^World w, u32 ci, u32 d)` — Registers trait node `n` of class `ci` (its declaration `d`).
* `bool st_default(^World w, u32 m)` — Whether method decl `m` is a default (written with a body).
* `ref []u32 st_family(^World w, u32 t)` — The trait and its ancestors.
* `ref []u32 st_methods(^World w, u32 t)` — The methods trait `t` prescribes, its own first, one per name.
* `string8 st_sig_written(^World w, u32 m, u32 itself)` — Method `m`'s signature, `Self` as `itself` (NONE: written `Self`).
* `string8 s_written(^World w, u32 t)` — A script type as a script writes it (Rust's `dados_type_written`).
* `u32 s_outer(^World w, u32 ci)` — The class a class block `ci` is written in, or NONE.
* `bool st_adopts(^World w, u32 ci, u32 t)` — Whether class `ci` (its chain) takes trait `t` on, directly or through a trait extending it.
* `bool st_fits(^World w, u32 got, u32 want)` — Whether a value of `got` stands where trait-typed `want` is asked.
* `void st_declare(^World w)` — Every trait's head and methods, then each class's `using`: conformance and defaults.
* `Val st_call(^World w, ^Body b, u32 n, u32 cn, u32 t, bool self_, u32 inst)` — `v.m(…)` through a trait value or a default's `self` (`inst` its operand).
* `Val st_field(^World w, ^Body b, u32 n, u32 t, bool self_)` — `v.f` of a trait value or a default's `self`: ERR0410.
* `bool st_no_zero(^World w, u32 ty)` — Whether a value of `ty` has no zero: an instance, a trait value, or a tuple holding one.
* `void st_needs(^World w, u32 f, u32 n, u32 ty)` — A slot of `ty` named by node `n` (file `f`) declared with no value: ERR1310.
* `u32 s_path_decl(^World w, u32 f, []u32 parts)` — The declaration a written path names from file `f` — `Name`,…
* `ref []u32 s_tpath(^World w, u32 f, u32 n)` — A `T_PATH`'s parts.
* `ref []u32 s_epath(^World w, u32 f, u32 n)` — An expression path's parts (`a.b.c` of names), or empty.
* `u32 dloc(^World w)` — Dado's `Location`, named on first mention (a script's is `w.sloc`).
* `string8 here_fn(^World w, ^Body b)` — The function body `b` is in, as a trace names it: `pkg.f`, `pkg.Type.m`;…
* `Val caller_place(^World w, ^Body b, u32 n)` — A default `#location()` at call `n`: the call's own place.
* `bool location_default(^World w, u32 f, u32 x)` — Whether node `x` of file `f` is `#location()` itself.
* `u32 location_inside(^World w, u32 f, u32 x)` — A `#location()` under `x` (file `f`), or NONE.
* `void location_nested(^World w, u32 f, u32 at)` — A parameter's default with a `#location()` inside a larger value: ERR0901.
* `Val here_call(^World w, ^Body b, u32 n, u8 m)` — `#location()`, and `#trace()`: a `ref []Location` from the ambient…
* `type Need` — One method a trait prescribes: the trait declaring it and its node.
* `ref []Need needs(^World w, u32 t)` — What trait `t` prescribes, its parents' first. ERR0535 for an `extends`
* `bool reaches(^World w, u32 from, u32 to, i64 depth)`
* `void trait_head(^World w, u32 t)` — A trait's head, once: ERR0535 for an `extends` naming a type, and for a…
* `string8 head_text(^World w, u32 f, u32 n)` — `void draw(self)`: a method's head as written.
* `void conform(^World w, u32 d, u32 ty, u32 env = NONE)` — Type `d` (row `ty`) takes on each trait its head names: ERR0535, ERR0536,…
* `bool implements(^World w, u32 ty, u32 t)` — Whether the type declaration behind `ty` takes trait `t` on.
* `string8 trait_spelled(^World w, u32 t, u32 f, u32 here)` — How file `f` spells trait `t`: bare in its own package, through `f`'s…
* `string8 pkg_label(^World w, u32 p)` — A package as a repair names it: its name.
* `bool visible(^World w, u32 d, u32 f)` — Whether declaration `d` is visible from file `f`.
* `string8 an_implementer(^World w, u32 t, u32 f)` — How file `f` spells some declaration taking `t` on, the first by…
* `string8 using_repair(^World w, u32 t, u32 dd, u32 f, string8 fname, string8 verb)` — The repair for a value whose declaration `dd` (NONE: a shape) does not…
* `u32 decl_of(^World w, u32 ty)` — The type declaration behind `ty`, or NONE.
* `bool takes_on(^World w, ^Body b, u32 d, u32 t, Val v, u32 at)` — ERR0919: what binds a trait parameter is a declaration that takes it on.
* `string8 default_note(^World w, u32 d, u32 of, u32 at)` — A default's provenance: the `using` that gave it, then the asking call.
* `bool shapeless(^World w, ^Body b, u32 base, u32 n)` — ERR0410: a default body reaches `self` only through its trait's methods.
* `u32 using_decl(^World w, u32 f, u32 u)` — The declaration a `using` names: `T`, or `pkg.T` through this file's imports.
* `type Arm` — One arm of a dispatch: an implementer's number, its method's C name,…
* `type Dispatch`
* `bool is_trait(^World w, u32 ty)`
* `u32 impl_number(^World w, u32 ty)` — Implementer `ty`'s number in every trait value (1 and up; 0 is empty).
* `u32 trait_decl(^World w, u32 ty)`
* `Val to_trait(^World w, ^Body b, Val v, u32 ty, u32 at)` — `v` as trait value `ty`: a place of an implementer, or a pointer to one.
* `Val trait_data(^World w, ^Body b, Val base, u32 n)` — `v.data()` over an `Allocator` trait value: its two words.
* `Val trait_call(^World w, ^Body b, u32 n, u32 cn, Val base)` — `v.m(args)` over a trait value: the dispatch function for `(trait, m)`.
* `bool settle_dispatches(^World w)` — Every dispatch's arms, once the bodies are checked: each implementer of…
* `string8 some_implementer(^World w, u32 t)` — An implementer of trait `t` to name in a repair, or "".
* `bool unforwarded(^World w, ^Body b, u32 n, u32 d, u32 env, []u32 args, []u32 ps)` — ERR0922: inside an instance holding a binder as a shape, a call handing it…
* `void union_body(^World w, u32 d, u32 ty)` — Declaration `d`'s members, checked, as the union under `ty`.
* `u32 union_of(^World w, u32 ty)` — The union row under `ty`, or NONE (a `cunion`'s too: it has slots).
* `u32 raw_of(^types.Types t, u32 ty)` — The `cunion` row under `ty`, or NONE.
* `i64 member_at(^World w, u32 u, u32 ty)` — Member `ty`'s place in union row `u`, or -1.
* `string8 members_text(^World w, u32 u)`
* `Val union_make(^World w, ^Body b, u32 n, u32 ty, u32 vn)` — `U(x)`: `x` tagged as the member it is.
* `Val narrow(^World w, ^Body b, u32 n)` — `v.(T)`: member `T` and whether the tag says so, as `(T, bool)`.
* `void union_switch(^World w, ^Body b, u32 n, Val v)` — A `switch` over a union: an arm per member named, its binding the member.
* `string8 raw_members(^World w, u32 rw)` — A `cunion`'s members as `` `T name` ``, joined.
* `const string8 DADO_VERSION = "1.0.0-rc.10"` — What `#DADO_VERSION` answers.
* `const u32 NONE = 0xFFFFFFFF`
* `const u8 K_FN = 0`
* `const u8 K_FFN = 1`
* `const u8 K_FCONST = 2`
* `const u8 K_FTYPE = 3`
* `const u8 K_TYPE = 4`
* `const u8 K_ENUM = 5`
* `const u8 K_CONST = 6`
* `const u8 K_VAR = 7`
* `const u8 K_METHOD = 8`
* `const u8 K_OTHER = 9`
* `const u8 K_FSTRUCT = 10`
* `const u8 K_FVAR = 11`
* `const u8 K_TRAIT = 12`
* `const u8 K_FBIND = 13` — A bound `foreign` block (`foreign sys "stdio.h":`): its items are `sys.x`.
* `const u8 K_UNION = 14`
* `const u32 BOUND_PKG = 0x80000000`
* `type Decl` — `ty` is a function's FUNC row, a constant's type, a type's own row; a…
* `type Job` — A body to check: a declaration, the instance it is over (`instance.dado`;…
* `type World`
* `ref []u64 g_watch_reg` — The watch's scope regions by depth (lo << 32 | hi + 1, 0: none) and the…
* `ref []WLocal g_watch_names`
* `{u32: u8} g_keyed_clean`
* `type WLocal` — One binding a watched scope holds: kind 0 local, 1 parameter, 2 const.
* `type WPoint` — `dadoc query` (tools/dadoc/inspect.dado): many watched bytes of the file…
* `ref []WPoint g_watch_pts`
* `ref []WLocal g_wp_names`
* `bool g_wr_on` — `dadoc query peek`: every scope overlapping `g_wr_lo..<g_wr_hi` (when…
* `u32 g_wr_lo`
* `u32 g_wr_hi`
* `ref []WLocal g_wr_names`
* `type HRun` — A call from Dado into a script carrying runs (Rust's container helper):
* `World world(^index.Program p, string8 os = "UNKNOWN_OS", string8 arch = "UNKNOWN_ARCH", string8 endian = "UNKNOWN_ENDIAN")`
* `u32 tstr(^World w, string8 s)` — `s` as a type-table string, through a held copy (the table may move).
* `string8 name_of(^World w, u32 sym)` — A held copy: views into the tables do not survive their growth.
* `string8 shown(^World w, u32 ty)` — A type as messages spell it.
* `string8 named(^World w, u32 ty)` — A type by its declared name, as most messages spell it.
* `bool nameless(^World w, u32 ty)` — The declaration `name` of package `pkg`, or NONE; the interface job runs…
* `u32 lookup(^World w, u32 pkg, u32 name)`
* `u32 bare(^World w, u32 f, u32 n)` — Name `n` of file `f`: this package's declaration, or the one a dot import…
* `string8 either([]string8 xs)`
* `void dot_clashes(^World w, u32 pkg)` — ERR0310: a declaration of a file that a dot import of it already names.
* `void declare(^World w, u32 pkg)` — The interface job's listing: every item, `when` arms decided.
* `bool valued(u8 k)`
* `bool typeish(u8 k)` — Whether a declaration kind names a type.
* `bool c_array(^World w, u32 ty)` — A C array: Dado's `[N]T`, which C passes and names differently.
* `bool foreign_kind(u8 k)`
* `bool is_fstruct(^types.Types t, u32 ty)` — Whether `ty` is (a name for) a `foreign` type the header declares.
* `u64 align_n(^World w, u32 f, u32 an)` — `@align(n)`'s n (ERR1026); 0 when refused.
* `u32 one_row(^types.Types t, u32 ty)` — The header struct of one slot `ty` names (A2 makes it its slot in Dado,…
* `u32 slot_ty(^types.Types t, u32 ty, u32 i)` — Slot `i`'s type of `ty`, a one-slot header struct's its body.
* `string8 fbound_name(^World w, u32 d)` — A `foreign` declaration's name, through its bound block's (`other.Handle`).
* `void say(^World w, u32 f, u32 base, u32 lo, u32 hi, u16 code, string8 msg, string8 help, string8 why = "")` — A diagnostic over `lo..hi` of file `f`, relative to `base`.
* `bool builtin_tname(^World w, u32 f, u32 lo, u32 hi, string8 pre, string8 name, string8 what, u8 door, u32 pkg = NONE, u8 phase = diag.PH_INTERFACE)` — The same over node `n`'s main token.
* `u64 name_span(^World w, u32 f, u32 n)` — A token's span, a leading `$` left off.
* `void drop_quiet(^World w, i64 from)` — What passes after `w.d.list[from]` said inside a body only `all_bodies`
* `void say_tok(^World w, u32 f, u32 base, u32 n, u16 code, string8 msg, string8 help, string8 why = "")`
* `u32 resolve(^World w, u32 d)` — A declaration's signature, resolved once; a cycle answers what is known…
* `u32 fn_type(^World w, u32 d)` — A function's or method's FUNC row, under the current instance.
* `bool instance_head(^World w, u32 f, u32 n)` — Whether written `R(…)` names a generic type's instance.
* `type Embed` — A spliced (embedded) member: `outer` holds `inner`'s slots from `at`.
* `const [37]string8 C_KEYWORDS = ("auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Bool", "_Complex", "_Imaginary")`
* `u32 method_named(^World w, u32 d, string8 s)`
* `[]u32 params(^World w, u32 d)` — The parameter nodes of a function declaration.
* `bool thread_local(^World w, u32 d)` — The `Sink` shape, named on first mention.
* `u32 amode(^World w)` — `AllocatorMode`: `i32` under a name (structural), members 0 to 3.
* `const [4]string8 MODES = ("Alloc", "Resize", "Free", "FreeAll")`
* `u32 alloc_ty(^World w)` — The allocator shape, `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data)`.
* `u32 sink(^World w)`
* `u32 resolve_type(^World w, u32 f, u32 n)` — A written type. Unknown names are ERR0401 and answer NONE.
* `void foreign_facts(^World w)` — The `foreign` clauses of every loaded file, once each: ERR1011 (one macro…
* `void file_blocks(^World w, u32 f, ^ref []u32 out)` — The `foreign` blocks of file `f`, through the `when` arms that hold.
* `u32 c_name(^World w, u32 d)` — A foreign declaration's C name: its `@c("…")` / written string, else its name.
* `void reach_foreign(^World w, u32 f, u32 n)` — Notes foreign block `n` of file `f` as reached.
* `bool reaches_header(^World w, string8 h)` — Whether a reached `foreign` block names header `h`.
* `string8 foreign_heads(^World w)` — What the reached `foreign` blocks put before the prelude: `define`s, then…
* `string8 c_facts(^World w)` — The build manifest's facts about the C a program brings: `inline-c <file>…
* `type Spawn` — A `threads.spawn` call: its body's declaration (NONE outside one) and where.
* `type Where`
* `Where where(^World w, u32 f, u32 item, u32 n)` — Where node `n` of file `f` starts (`item` is the node it is relative to).
* `string8 nearest(string8 wrong, []string8 pool)` — The one name of `pool` near enough to `wrong` to offer (a third of the…
* `bool bytes_less(string8 x, string8 y)`
* `string8 vis_word(u32 v)`
* `i64 bitfield(^World w, u32 f, u32 m, u32 mt, string8 tn)` — A foreign member's width `: W` (ERR1023 when C could not lay it out so); -1 when refused.
* `void detached(^World w, u32 f, u32 n, string8 under)` — ERR0524: names in a member list no binding views (under `[N]`, `^`, …).
* `bool flex(^World w, u32 f, u32 n)` — `[_]T`, C's flexible array member.
* `bool flex_beside(^World w, u32 f, u32 n, u32 e)` — A header struct ending in `[_]T` laid out beside another (ERR1022).
* `u32 c_named_list(^World w, u32 f, u32 tn, string8 tag)` — A header struct's member list written with a name, `(i32 x, …) inner`: a…
* `u32 cnl_shape(^World w, u32 t)` — `t` resolved, a header's named member list taken for its shape.
* `u32 fs_row(^World w, u32 ty)` — The header struct row `ty` names (through Dado names and qualifiers), or NONE.
* `u32 fpos_refused(^World w, u32 ty)` — Rust's `foreign_position_refused`: the shape of a header struct (or a…
* `u32 fwin_refused(^World w, u32 ty)` — Rust's `foreign_window_refused`: no window over a header struct's slots…
* `bool fs_nominal(^World w, u32 ty)` — A header struct with no Dado shape to be one type with (bits, a flexible tail).
* `bool two_spellings(^World w, u32 x, u32 y)` — Pointees `x` and `y` are one type spelled twice in C: a header's…
* `u32 incomplete_in(^World w, u32 ty, i64 depth)` — The `@incomplete` foreign type `ty` holds by value (a run's elements…
* `bool inc_refuse(^World w, u32 f, u32 lo, u32 hi, u32 ty, string8 what, u32 base = 0)` — ERR1002 where `what` (written `lo..hi` of file `f`) holds an `@incomplete` type by value.
* `bool unbroken(^World w, u32 x, u32 to, ^ref []u32 seen)` — Whether `x` reaches named row `to` through names, pointers, windows, maps…
* `bool anon_find(^World w, u32 ty, string8 s, ^ref []u32 path)` — The slot path to `s` through type `ty`'s anonymous members, into `path`.
* `bool anon_node(^World w, u32 f, u32 tn, string8 s, ^ref []u32 path)` — The same through member list node `tn` of file `f` (nested lists recursed).
* `bool contract_decl(^World w, u32 d)` — A `contract`: a `type` of function references, tagged.
* `bool contract_slots(^World w, u32 d, bool say_)` — Every slot a function reference; ERR0520 said where one is not (`say`).
* `u32 contract_reached(^World w, u32 ty, bool top, ^ref []u32 seen)` — The contract declaration `ty` reaches, as written (`top`: `ty` itself counts).
* `bool contract_refused(^World w, u32 f, u32 base, u32 lo, u32 hi, u32 ty, string8 what)` — ERR0921: a contract written where a value is stored (`what`, over `lo..hi`).
