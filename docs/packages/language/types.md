<!-- dadoc 1.0.0-rc.9 -->
<!-- self-hosted (tools/dadoc) -->
# language:types

Interned types: one row per distinct type, its content hash beside it.
`TypeId`s are handles: compare and key by `hash`, never by id.

## Declarations

197 declarations, 167 public.

* `const u8 C_INT = 0`
* `const u8 C_FLOAT = 1`
* `const u8 C_BOOL = 2`
* `const u8 C_STR = 3`
* `const u8 C_FOREIGN = 4`
* `const u8 C_FN = 5`
* `const u8 C_TYPE = 6`
* `const u8 C_TUPLE = 7`
* `const u8 C_WINDOW = 8`
* `const u8 C_CALLER = 9`
* `const u8 C_UNION = 10`
* `type Consts`
* `void Consts.free(^self)`
* `Hash Consts.hash(^self, u32 c)`
* `[]u32 Consts.list(^self, u32 c)`
* `u32 Consts.make(^self, ^Types t, u8 k, u32 a, u32 b, u64 w0, u64 w1, []u32 xs)` — `xs` may be a window into this table.
* `u32 Consts.int(^self, ^Types t, u64 lo, i64 hi)` — An integer as an i128: `hi` is its top half, sign included.
* `u32 Consts.small(^self, ^Types t, i64 v)`
* `u32 Consts.float_bits(^self, ^Types t, u64 bits)`
* `u32 Consts.boolean(^self, ^Types t, bool v)`
* `u32 Consts.str(^self, ^Types t, string8 written)`
* `u32 Consts.of_type(^self, ^Types t, u32 ty)`
* `u32 Consts.tuple(^self, ^Types t, []u32 slots)`
* `u32 Consts.union_(^self, ^Types t, u32 at, u32 x)`
* `u32 Consts.fn_ref(^self, ^Types t, string8 pkg, string8 name, bool in_c)`
* `void put_u64(^ref []char8 out, u64 n)`
* `void display(^Types t, u32 x, ^ref []char8 out, bool by_name = false)` — `x` written out; `by_name` stops at a declared type's name.
* `type Hash`
* `bool Hash.same(self, Hash o)`
* `i32 Hash.order(self, Hash o)` — Orders hashes, for sorting by content: -1, 0 or 1.
* `type Hasher` — A running hash. `bytes(a); bytes(b)` is `bytes(a ++ b)`; `text` and the…
* `void Hasher.byte(^self, u8 b)`
* `void Hasher.word(^self, u64 x)` — Eight bytes, least significant first.
* `void Hasher.bytes(^self, string8 s)`
* `void Hasher.text(^self, string8 s)` — Its length, then its bytes.
* `void Hasher.hash(^self, Hash h)`
* `Hash Hasher.finish(self)`
* `Hasher hasher(u64 seed = 0)`
* `Hash hash_of(string8 s, u64 seed = 0)` — The hash of `s`'s bytes alone (MurmurHash3_x64_128 with `seed`).
* `const u32 NONE = 0xFFFFFFFF`
* `type Interner`
* `void Interner.free(^self)`
* `i64 Interner.count(^self)`
* `string8 Interner.key(^self, u32 id)`
* `u32 Interner.find(^self, string8 k)` — The handle of `k`, or `NONE`.
* `u32 Interner.intern(^self, string8 k)`
* `void put32(^ref []char8 k, u32 x)` — Key building: rows are encoded as bytes, four per `u32`.
* `void put64(^ref []char8 k, u64 x)`
* `u32 get32(string8 k, i64 at)`
* `type Layout`
* `bool laid(^Types t, u32 x)` — Whether `x` has a C layout; then `size_of`, `align_of` and `offsets` answer.
* `u64 size_of(^Types t, u32 x)`
* `u64 align_of(^Types t, u32 x)`
* `[]u64 offsets(^Types t, u32 x)` — One offset per member: a tuple's slots, a run's pointer and length, a…
* `type Pool($T)`
* `void Pool.free(^self)`
* `u32 Pool.open(^self, i64 n)` — Starts list `count()` with room for `n`; `add` fills it.
* `void Pool.add(^self, T x)`
* `u32 Pool.put(^self, []T xs)` — `xs` may be a window into this pool.
* `[]T Pool.get(^self, u32 id)`
* `const u8 P_VOID = 0`
* `const u8 P_BOOL = 1`
* `const u8 P_I8 = 2`
* `const u8 P_I16 = 3`
* `const u8 P_I32 = 4`
* `const u8 P_I64 = 5`
* `const u8 P_U8 = 6`
* `const u8 P_U16 = 7`
* `const u8 P_U32 = 8`
* `const u8 P_U64 = 9`
* `const u8 P_F32 = 10`
* `const u8 P_F64 = 11`
* `const u8 P_RAWPTR = 12`
* `const u8 P_BYTE = 13`
* `const u8 P_INT = 14`
* `const u8 P_FLOAT = 15`
* `const u8 P_CHAR8 = 16`
* `const u8 P_CHAR16 = 17`
* `const u8 P_CHAR32 = 18`
* `const u8 PRIMS = 19`
* `const u8 K_VOID = 0` — Kinds of value a prim holds.
* `const u8 K_BOOL = 1`
* `const u8 K_SINT = 2`
* `const u8 K_UINT = 3`
* `const u8 K_FLOAT = 4`
* `const u8 K_PTR = 5`
* `const u8 K_CHAR = 6`
* `type PrimRow` — `bits` is the width (0 for `void`; a pointer's is the target's word).
* `const [19]PrimRow PRIM = ( PrimRow("void", P_VOID, K_VOID, 0, false), PrimRow("bool", P_BOOL, K_BOOL, 8, false), PrimRow("i8", P_I8, K_SINT, 8, false), PrimRow("i16", P_I16, K_SINT, 16, false), PrimRow("i32", P_I32, K_SINT, 32, false), PrimRow("i64", P_I64, K_SINT, 64, false), PrimRow("u8", P_U8, K_UINT, 8, false), PrimRow("u16", P_U16, K_UINT, 16, false), PrimRow("u32", P_U32, K_UINT, 32, false), PrimRow("u64", P_U64, K_UINT, 64, false), PrimRow("f32", P_F32, K_FLOAT, 32, false), PrimRow("f64", P_F64, K_FLOAT, 64, false), PrimRow("rawptr", P_RAWPTR, K_PTR, 0, false), PrimRow("byte", P_U8, K_UINT, 8, false), PrimRow("int", P_I64, K_SINT, 64, true), PrimRow("float", P_F64, K_FLOAT, 64, true), PrimRow("char8", P_CHAR8, K_CHAR, 8, false), PrimRow("char16", P_CHAR16, K_CHAR, 16, false), PrimRow("char32", P_CHAR32, K_CHAR, 32, false), )`
* `u8 prim_named(string8 s)` — The prim spelled `s`, or `PRIMS`.
* `u8 integer_bits(u8 p)` — Integer bits of the canonical prim, 0 when it is no integer (`char` is).
* `bool is_signed(u8 p)`
* `bool is_numeric(u8 p)`
* `bool in_int(u8 p)` — The categories a `where` names: `int`, `float`, `scalar`.
* `bool in_float(u8 p)`
* `bool in_scalar(u8 p)`
* `const u8 CS_SIGNED = 0` — C's own scalars, `#c.<name>`.
* `const u8 CS_UNSIGNED = 1`
* `const u8 CS_IMPL_INT = 2`
* `const u8 CS_FLOAT = 3`
* `const u8 CS_BOOL = 4`
* `const u8 CS_CHARACTER = 5`
* `type CScalar` — `size` 0 is the target's word; 255 has no size Dado knows.
* `const [22]CScalar C_SCALARS = ( CScalar("char", "char", CS_IMPL_INT, 1), CScalar("schar", "signed char", CS_SIGNED, 1), CScalar("uchar", "unsigned char", CS_UNSIGNED, 1), CScalar("short", "short", CS_SIGNED, 2), CScalar("ushort", "unsigned short", CS_UNSIGNED, 2), CScalar("int", "int", CS_SIGNED, 4), CScalar("uint", "unsigned int", CS_UNSIGNED, 4), CScalar("long", "long", CS_SIGNED, 255), CScalar("ulong", "unsigned long", CS_UNSIGNED, 255), CScalar("llong", "long long", CS_SIGNED, 8), CScalar("ullong", "unsigned long long", CS_UNSIGNED, 8), CScalar("float", "float", CS_FLOAT, 4), CScalar("double", "double", CS_FLOAT, 8), CScalar("ldouble", "long double", CS_FLOAT, 255), CScalar("bool", "_Bool", CS_BOOL, 1), CScalar("size_t", "size_t", CS_UNSIGNED, 0), CScalar("ptrdiff_t", "ptrdiff_t", CS_SIGNED, 0), CScalar("intptr_t", "intptr_t", CS_SIGNED, 0), CScalar("uintptr_t", "uintptr_t", CS_UNSIGNED, 0), CScalar("wchar_t", "wchar_t", CS_CHARACTER, 255), CScalar("char16_t", "char16_t", CS_CHARACTER, 255), CScalar("char32_t", "char32_t", CS_CHARACTER, 255), )`
* `const u8 T_PRIM = 0`
* `const u8 T_TUPLE = 1`
* `const u8 T_SLICE = 2`
* `const u8 T_REF = 3`
* `const u8 T_PTR = 4`
* `const u8 T_MAP = 5`
* `const u8 T_BUILTIN = 6`
* `const u8 T_FUNC = 7`
* `const u8 T_DISTINCT = 8`
* `const u8 T_MATRIX = 9`
* `const u8 T_SOA = 10`
* `const u8 T_QUAL = 11`
* `const u8 T_ALIGNED = 12`
* `const u8 T_OPAQUE = 13`
* `const u8 T_CSCALAR = 14`
* `const u8 T_UNION = 15`
* `const u8 T_NAMED = 16`
* `const u8 T_TYPE = 17`
* `const u8 T_CHANNEL = 18`
* `const u8 T_TRAIT = 19`
* `const u32 F_NONE = 0`
* `const u32 F_PLAIN = 1`
* `const u32 F_TYPED = 2`
* `const [4]string8 QUALS = ("const", "volatile", "restrict", "_Atomic")` — Qualifier bits, `CQual::ALL` order.
* `const u32 STRING8 = 19` — Fixed handles, interned first by `new_types`.
* `const u32 CSTRING = 20`
* `const u32 TYPE = 21`
* `const u32 STRING16 = 22`
* `const u32 STRING32 = 23`
* `type Types`
* `void Types.free(^self)`
* `u32 Types.str(^self, string8 s)`
* `string8 Types.text(^self, u32 s)`
* `i64 Types.count(^self)`
* `[]u32 Types.list(^self, u32 t)`
* `i64 Types.count_of(^self, u32 t)`
* `Hash Types.hash(^self, u32 t)`
* `u32 Types.make(^self, u8 g, u32 a, u32 b, []u32 xs)` — The row `(g, a, b, xs)`, interned. `xs` may be a window into this table.
* `u32 Types.prim(^self, u8 p)`
* `u32 Types.tuple(^self, []u32 elems)`
* `u32 Types.slice_of(^self, u32 e)`
* `u32 Types.ref_of(^self, u32 e)`
* `u32 Types.ptr_of(^self, u32 e)`
* `u32 Types.map_of(^self, u32 k, u32 v)`
* `u32 Types.func(^self, u32 ret, u32 failure, []u32 params)` — `params` with the error type first when `failure` is `F_TYPED`.
* `[]u32 Types.params(^self, u32 f)` — A FUNC row's parameters (its error type, when typed, left out).
* `u32 Types.error_of(^self, u32 f)` — A FUNC row's error type, or NONE.
* `u32 Types.builtin(^self, string8 name)`
* `u32 Types.opaque(^self, string8 name)`
* `u32 Types.distinct_of(^self, string8 tag, u32 under)`
* `u32 Types.named(^self, string8 pkg, string8 name)` — A declared type: one row per `(package, name)`, its body set once known.
* `void Types.define(^self, u32 t, u32 body, []u32 slot_names)`
* `[]u32 Types.slot_names(^self, u32 t)`
* `u32 Types.resolve(^self, u32 t)` — Through declared names, qualifiers and matrices (`[R,C]T` is the…
* `u32 Types.lres(^self, u32 t)` — `resolve` for layout: an `@align` keeps its row.
* `u32 Types.qual(^self, u32 q, u32 inner)` — `inner` under the qualifier bits `q`, merged with its own.
* `u32 Types.quals(^self, u32 t)` — The qualifier bits on `t` itself (through names), and `t` without them.
* `u32 Types.bare(^self, u32 t)`
* `bool Types.uniform(^self, u32 t, ^u32 elem, ^i64 count)` — The element and count of a tuple of two or more equal slots.
* `[]u32 Types.slots(^self, u32 t)` — A shape's slots through its tags: a tuple's elements (a `#soa`'s…
* `Types new_types(u64 word = 8)` — A table with the prims, `string8`, `cstring`, `type` and the wide strings at their fixed…
* `u8 text_unit(u32 r)` — A string's unit (`P_CHAR8`, `P_CHAR16`, `P_CHAR32`) for resolved `r`, else `PRIMS`.
* `bool is_text(u32 r)`
