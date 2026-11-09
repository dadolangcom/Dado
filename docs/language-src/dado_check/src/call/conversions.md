//@ types/traits 27
//@ ### The `Allocator` trait
//@
//@ An allocator is two words — a procedure and its state — and
//@ `core:mem`'s `Allocator` is the trait over them: one required method,
//@ `data()`, answering the two words, and a default, `ok()`, asking
//@ whether they carry a procedure. No type names the two words; code that
//@ holds them writes the shape.
//@
//@ ```dado
//@ trait Allocator:
//@     (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) data(self)
//@     bool ok(self): return self.data().proc != nil
//@ ```
//@
//@ **Wherever an allocator is asked, either answers**: the two words — a
//@ rung such as `#default`, a local, a slot, a call that hands them back —
//@ or a value of a declaration that takes `Allocator` on, whose `data()`
//@ is called there, or an `Allocator` trait value, or a `^` of one,
//@ whose `data()` is called through the pair's dispatch — so a
//@ `[]mem.Allocator` of an arena and the heap can each be opened with
//@ `using d:`. That is a verb's last argument, a `using` subject,
//@ `#format`'s last argument, and every parameter, local, slot or return
//@ written as the shape. `Allocator a` as a parameter's whole type is a
//@ binder, instantiated per implementer, and takes implementers only —
//@ so a parameter that must also take a rung is written as the shape.
//@ The two words' slots read as `proc` and `data` wherever they are held.
//@ A trait never crosses to C: an `export` or `foreign` signature takes
//@ the two words.

//@ types/views 25
//@ ### A type is built from one value per slot
//@
//@ A type of two slots or more is built by calling it with one value per
//@ slot: `Vec2(1, 2)` for `type Vec2: f32 x, f32 y`. It is the literal
//@ `(1, 2)` written where a `Vec2` is wanted, so each value enters its
//@ slot as an assignment would — a literal adapts, an `int` or a `float`
//@ adapts to the slot's width, an explicit width only widens, and a
//@ change of kind is written (`Vec2(f32(n), 0)`). The slots may be named
//@ instead, as a call's parameters are: `Vec2(y: 2, x: 1)`, every slot
//@ named once and none passed by position beside them. A wrong count is
//@ `ERR0601`, naming the slots.
//@
//@ ```dado
//@ package doc_sample
//@
//@ type Vec2:
//@     f32 x
//@     f32 y
//@
//@ distinct type Metres: Vec2
//@
//@ public Vec2 half(Vec2 v):
//@     return Vec2(y: v.y / 2, x: v.x / 2)
//@
//@ public Metres origin():
//@     return Metres(0, 0)
//@ ```
//@
//@ A `distinct` type over a shape, a fixed array's name (`type Pair:
//@ [2]f32`, `Pair(1, 2)`), a nested shape and a `foreign` struct are
//@ built the same way, and a constant may be one, its slots passed in
//@ order. **One value is still a conversion:** `Vec2(q)` converts a
//@ `(f32, f32) q`, and a type of one slot is never built from several.
//@ A union is not built from slots either — `Shape(c)` picks the member
//@ by the value's type — and an enum is not a shape of slots.

//@ types/distinct 57
//@ ### An address reads as a number, and only one way
//@
//@ `u64(p)` and `i64(p)` of a `rawptr`, `const` or not, are the address
//@ as a number. Those two widths hold every address a supported target
//@ has, so the conversion means the same everywhere, and the C reads it
//@ through `#c.uintptr_t`, which widens without a word on a 32-bit target.
//@ A typed pointer forgets its type first, `u64(rawptr(&x))`, which is
//@ the step that says so. A narrower width, `u32(p)`, is refused
//@ (`ERR0504`), since an address need not fit it; its help names
//@ `u32(u64(p))`, which narrows a number on purpose.
//@
//@ **The way back is not opened.** A number becomes an address only
//@ through `#c.uintptr_t`, `rawptr(#c.uintptr_t(n))`, where the program
//@ says in C's own type that the number was one: that type is the one C
//@ promises an address survives the round trip through, and `u64` is
//@ Dado's width, which promises nothing of the kind. `rawptr(n)` of a
//@ `u64` is `ERR0504`.
//@
//@ ```dado
//@ package doc_sample
//@
//@ // How far `p` sits past a 64-byte boundary.
//@ public u64 misalignment(const rawptr p):
//@     return u64(p) & 63
//@
//@ // Back to an address, through the one type C promises the trip for.
//@ public rawptr address(u64 n):
//@     return rawptr(#c.uintptr_t(n))
//@ ```
