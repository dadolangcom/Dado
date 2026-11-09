//@ types/tuples 10
//@ ### Everything is a tuple
//@
//@ Function arguments, structs, arrays, unions, enums and multiple returns
//@ are one construct, and that is the axiom the rest of the model is
//@ derived from. A language carrying six of them owes six answers to
//@ equality, to layout, to destructuring and to what a generic binding may
//@ stand for, and gets them out of step — which is where most of a type
//@ system's special cases come from. There is one answer here because
//@ there is one thing to answer about.
//@
//@ **A one-slot tuple is strictly its element**, collapsed at construction
//@ rather than seen through by every reader afterwards. So no one-slot
//@ wrapper exists to be compared, dumped or mangled two ways, and a
//@ declaration of one named position over an `i32` *is* `i32`. Keeping the
//@ wrapper and teaching each consumer to erase it writes one rule into
//@ dozens of places and gets it wrong in one of them.
//@
//@ **The one name a one-slot declaration gives names the whole element**,
//@ there being nothing else for it to name. With `type Outer: (Inner r)`,
//@ `o.r` is the `Outer` seen as its `Inner`: `o.r.a` is the element's
//@ slot `a`, `o.r = v` stores the whole value, and `o.a` reads the same
//@ slot, since `Outer` is `Inner`. A slot holding a `ref T` or a `^T`
//@ names the reference, and reached through a pointer it still does:
//@ with `type W: (^Stream s)`, a `^W` is a pointer to a pointer, and
//@ `p.s` (a `^self` method's `self.s` among them) dereferences once, to
//@ the `W`, where `p.n` goes on to the `Stream`'s own slot `n`.
//@
//@ ```dado
//@ type Cfg: ([4]f64 thickness)
//@
//@ // thickness is the whole [4]f64, so a computed subscript reaches
//@ // every element.
//@ public f64 at(Cfg c, i64 e):
//@     return c.thickness[e]
//@ ```
//@
//@ **An array is a tuple whose slots share a type.** `TypeArena::array` is
//@ `TypeArena::tuple` over N copies of one id and nothing else, so `[N]T`
//@ is not a second kind of type with rules of its own: `[0]T` is the
//@ empty tuple and `[1]T` is its element, both by the collapse above
//@ rather than by a case written here.
//@
//@ Being subscriptable is then a consequence and not a grant. Every slot
//@ is the same width, so slot k sits k widths from the front and a
//@ computed subscript is arithmetic the layout has already licensed. A
//@ declaration whose slots differ has no such spacing, which is the whole
//@ of why it is not subscriptable — the refusal falls out of the model
//@ instead of sitting beside it as a rule.
//@
//@ ```dado
//@ type Rgb: (f32 r, f32 g, f32 b)
//@
//@ // One declaration, both reads: Rgb is [3]f32, so a written name and a
//@ // computed subscript reach the same three slots.
//@ public f32 channel(Rgb c, i32 i):
//@     return c.r + c[i]
//@ ```
//@
//@ **Under `#soa` a position is a lane.** `#soa` over `[N]E` holds one run
//@ per slot of `E`, so where `E` is itself a run, position k of element i
//@ is lane k at i: `c[i][k] = v`, `c[i][k] += v` and `&c[i][k]` reach
//@ that lane exactly as `c[i].x` does, at a written position or a computed
//@ one. What the layout does not have is the element in one piece, so
//@ `&c[i]`, a window `c[i][a..<b]` and `for &x in c[i]` are refused
//@ (`ERR0522`), and `c[i]` as a value is a copy gathered out of the lanes.
//@
//@ **The cost is paid in the emitted name.** A mangled name spells every
//@ slot, so twenty of a six-slot declaration mangles to 612 characters and
//@ has to be cut back to the 63 C guarantees, with a hash on the end to
//@ keep two long names from becoming one.
