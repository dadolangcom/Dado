//@ types/views 20
//@ ### A name is a view, not identity
//@
//@ Slot names take no part in type identity. A declaration that gives its
//@ positions letters and an anonymous one of the same slots are **one type**,
//@ one arena entry and one emitted C struct: nothing about the name was ever
//@ stored in the shape, so `TypeArena::equal` has nothing to compare. A dump
//@ under `--emit=types` prints both as the bare shape, which is the honest
//@ rendering rather than a lossy one.
//@
//@ **This is the tuple axiom paying out, not a second decision.** Were naming
//@ a position to make a new type, a declaration of three `f32` positions would
//@ stop being the array of three and stop being subscriptable, and the one
//@ construct would have quietly become two. Names are therefore held beside
//@ the shape and handed to a rendering when one asks — which is what
//@ `SlotNames` is: a borrowed function from a declared name to one string per
//@ slot, owned by the crate that knows which declaration is in hand rather
//@ than copied into the arena, where it would be a second copy to go stale.
//@
//@ **The cost is that Dado → C → Dado is not an identity, and the loss is
//@ real.** A declaration whose slots differ survives: `(i32 id, f32 w, u8 t)`
//@ emits a struct of three named members and comes back spelled as written.
//@ One whose slots share a type does not. `(f32 x, f32 y)` emits
//@ `struct { float s[2]; }` and returns as `([2]f32 s)` — because that shape
//@ *is* the array of two, and a struct of two named members has nothing a
//@ computed subscript could reach.
//@
//@ Two repairs were measured and both fail: an overlay through an anonymous
//@ union compiles and the binding generator cannot read it, and taking the
//@ address of the first member and walking is undefined behaviour. So the
//@ loss is stated instead of patched. **Signatures are an identity** — name,
//@ count, order, widths and parameter spellings all return unchanged — and a
//@ declaration is exactly where the round trip stops being one.
