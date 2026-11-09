<!-- dadoc 1.0.0-rc.9 -->
<!-- self-hosted (tools/dadoc) -->
# language:ir

The IR: typed, structured, flat, one `Fn` per function, method, instance,
default body or script function. Monomorphic, not SSA. Outside references
are `TypeId`, `StrId`, `ConstId` and the function's own `refs` (package +
mangled name). Definitions for P3's subset; the rest of the op set joins
with its family.

## Declarations

130 declarations, 126 public.

* `u32 konst(u32 c)`
* `bool is_const(u32 opd)`
* `u32 slot(^Fn f, u32 ty, u32 name, u16 flags, u32 node)`
* `u32 at(^Fn f, u32 s)` — The place that is slot `s` (or a global, `ROOT_GLOBAL | g`).
* `u32 step(^Fn f, u32 p, u8 st, u32 val, u32 ty)` — `p` and one more step.
* `u32 op(^Fn f, u8 o, u8 sub, u32 ty, u32 node, u32 dst, u32 a, u32 b = NONE, u32 c = NONE)`
* `void arg(^Fn f, u32 opd, u8 pass = PASS_VALUE)` — An operand of the last op.
* `[]u32 args(^Fn f, u32 i)`
* `u32 count(^Fn f)`
* `void close(^Fn f, u32 head)` — Ends a head's body (an IF's else, after `otherwise`).
* `void otherwise(^Fn f, u32 head)` — An IF's then-body ends here and its else-body starts.
* `u32 span(^Fn f, u32 i)` — A head's whole length.
* `u32 fn_ref(^Fn f, u32 pkg, u32 name, u8 flags)` — The function reference `(pkg, name)`, added once.
* `u32 verify(^Fn f)` — Structure holds: every body ends inside its parent, operands name slots,…
* `const u32 NONE = 0xFFFFFFFF`
* `const u8 O_BLOCK = 0` — Ops. A body is the ops after its head, `len` of them; `break n` and…
* `const u8 O_LOOP = 1`
* `const u8 O_IF = 2`
* `const u8 O_BREAK = 3`
* `const u8 O_CONTINUE = 4`
* `const u8 O_RETURN = 5`
* `const u8 O_RETURN_FAIL = 6`
* `const u8 O_SET = 7`
* `const u8 O_UN = 8`
* `const u8 O_BIN = 9`
* `const u8 O_CONV = 10`
* `const u8 O_CHECK = 11`
* `const u8 O_MAKE = 12`
* `const u8 O_CALL = 13`
* `const u8 O_CALL_INDIRECT = 14`
* `const u8 O_INTRINSIC = 15`
* `const u8 O_C_INLINE = 16`
* `const u8 O_ADDR = 17`
* `const u8 O_SCOPE_END = 18`
* `const u8 O_VIEW = 19`
* `const u8 O_MOVE = 20`
* `const u8 O_QUIT = 21`
* `const u8 MV_FREE = 0` — `sub` of O_MOVE.
* `const u8 MV_APPEND = 1`
* `const u8 MV_RESIZE = 2`
* `const u8 MV_REBIND = 3`
* `const u8 MV_INSERT = 4`
* `const u8 MV_REMOVE = 5`
* `const u8 MV_SEND = 6`
* `const u8 MV_DRAIN = 7`
* `const u8 MV_RESET = 8`
* `const u8 U_NEG = 0` — `sub` of O_UN and O_BIN.
* `const u8 U_NOT = 1`
* `const u8 U_BITNOT = 2`
* `const u8 B_ADD = 0`
* `const u8 B_SUB = 1`
* `const u8 B_MUL = 2`
* `const u8 B_DIV = 3`
* `const u8 B_REM = 4`
* `const u8 B_AND = 5`
* `const u8 B_OR = 6`
* `const u8 B_XOR = 7`
* `const u8 B_SHL = 8`
* `const u8 B_SHR = 9`
* `const u8 B_EQ = 10`
* `const u8 B_NE = 11`
* `const u8 B_LT = 12`
* `const u8 B_LE = 13`
* `const u8 B_GT = 14`
* `const u8 B_GE = 15`
* `const u8 CV_WIDEN = 0` — `sub` of O_CONV: the closed list, so far the slice's.
* `const u8 CV_WRAP = 1`
* `const u8 CV_INT_FLOAT = 2`
* `const u8 CV_FLOAT_INT = 3`
* `const u8 CV_FLOAT_WIDTH = 4`
* `const u8 CV_TO_RAWPTR = 5`
* `const u8 CV_FROM_RAWPTR = 6`
* `const u8 CV_LEND = 7`
* `const u8 CV_CSTRING = 8`
* `const u8 CV_FROM_CSTRING = 9`
* `const u8 CV_RETAG = 10`
* `const u8 CV_CROSS = 11`
* `const u8 CV_UNION = 12`
* `const u8 CV_TAG = 13`
* `const u8 CV_MEMBER = 14`
* `const u8 CV_TAIL = 15`
* `const u8 CK_BOUNDS = 0` — `sub` of O_CHECK.
* `const u8 CK_WINDOW = 1`
* `const u8 CK_NARROW = 2`
* `const u8 CK_BITS = 3`
* `const u8 CK_OVERFLOW = 4`
* `const u8 CK_STALE = 5`
* `const u8 CK_FLOAT_INT = 6`
* `const u8 CK_SENT = 7`
* `const u8 F_PURE = 1` — Op facts.
* `const u8 F_MAY_TRAP = 2`
* `const u8 E_READS = 1` — O_C_INLINE effects.
* `const u8 E_WRITES = 2`
* `const u8 E_DIVERGES = 4`
* `const u32 OPD_CONST = 0x80000000` — An operand is a place, or a constant with `OPD_CONST` set.
* `const u32 ROOT_GLOBAL = 0x80000000` — A place's root is a slot, or a global with `ROOT_GLOBAL` set. Steps:
* `const u8 ST_FIELD = 0`
* `const u8 ST_INDEX = 1`
* `const u8 ST_DEREF = 2`
* `const u16 S_PARAM = 1` — Slot flags.
* `const u16 S_USER = 2`
* `const u16 S_TEMP = 4`
* `const u16 S_RECEIVER = 8`
* `const u16 S_COPY = 16`
* `const u16 S_ADDR_TAKEN = 32`
* `const u16 S_READ = 64`
* `const u16 S_ONE = 128`
* `const u16 S_WATCH = 256`
* `const u16 S_CONST = 512`
* `const u8 PASS_VALUE = 0` — Argument passing.
* `const u8 PASS_POINTER = 1`
* `const u8 PASS_REF = 2`
* `const u8 PASS_LENT = 3`
* `const u8 R_DIVERGES = 1` — A function reference's flags.
* `const u8 R_FAILABLE = 2`
* `const u8 R_RETURNLESS = 4`
* `const u8 R_FOREIGN = 8`
* `const u8 D_DADO = 0`
* `const u8 D_SCRIPT = 1`
* `const u8 H_ENTRY = 1` — Function header flags.
* `const u8 H_NAMES_CALLER = 2`
* `const u8 H_FAILABLE = 4`
* `const u8 H_BROKEN = 8` — Its check raised an error: analysis skips it.
* `type Fn` — One function's IR. `node` is item-relative (from the item's first node),…
* `void Fn.free(^self)`
* `void print(^Fn f, ^ref []char8 out)`
