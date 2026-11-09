//@ types/methods 30
//@ ### Methods
//@
//@ A `type` declaration's body may hold **methods**: functions written
//@ with the declaration, called on a value of it. A member whose name is
//@ followed by `(` is a method; every other member is a slot.
//@
//@ ```dado
//@ type Vec2:
//@     f32 x
//@     f32 y
//@     f32 len2(self):
//@         return self.x * self.x + self.y * self.y
//@     void move(^self, f32 dx):
//@         self.x += dx
//@ ```
//@
//@ A method is **keyed on the declaration**, never on the shape: `Vector2`
//@ and `Size` over one `(f32, f32)` each have the methods their own body
//@ writes, and `v.describe()` asks the declaration the value was typed with. A
//@ value's declaration travels wherever its type does, and its methods
//@ with it; a `view` renames slots in its own scope and touches no
//@ method. Inside a method, `self` is viewed by the declaration that wrote
//@ the method, whatever the caller's view called its slots.
//@
//@ Every method takes a receiver, `self` or `^self`, written first with no
//@ type; a function of no receiver is a package-level function. `Self`
//@ names the declaration inside its own body. A slot and a method may not
//@ share a name, because `v.name` answers exactly one of them, and a
//@ method is no more visible than its type — an unmarked method is
//@ exactly as visible. A `union`, `cunion`, `enum`, `foreign` or channel
//@ declaration holds no methods: declare a `type` over it and write them
//@ there.
//@
//@ A method is a function in every other way. `Vec2.len2` names it as a
//@ value — `f32(Vec2)`, or `void(^Vec2, f32)` for `^self` — and it is
//@ emitted as a direct function, dropped when nothing reaches it.

//@ types/methods 32
//@ ### Embedding promotes methods
//@
//@ A declaration that embeds another by name takes its methods on for
//@ calls: `type Sprite: (Vec2, i32 id)` makes `s.len2()` call `Vec2.len2`
//@ on a `Vec2` rebuilt from the positions `Vec2`'s slots were spliced
//@ into. Embedding splices slots, so there is no `Vec2` object inside a
//@ `Sprite` for a `^self` method to point at: a promoted `^self` method is
//@ refused at the call, naming the member to hold instead. Two embedded
//@ members bringing one method name, or a method and a slot, are refused
//@ at the declaration, as two slots are; a method the embedder writes
//@ itself collides with a promoted one rather than hiding it.
//@
//@ The member is named whole by the name its slots are qualified with:
//@ `s.Vec2` is a `Vec2` read from those positions and `s.Vec2 = v`
//@ stores each of them, but `&s.Vec2` is refused for the same reason.
//@
//@     Vec2 base(Sprite s): return s.Vec2
//@
//@ Where the embedded type is declared does not matter: one from another
//@ package (`type Desk: (staff.Person, i64 floor)`) or lower in the file
//@ splices the same slots, its methods are promoted, and `d.Person` names
//@ it whole.
//@
//@ A **redeclaration** `type W: V` has `V`'s slots and view and none of
//@ its methods: `W`'s methods are what `W`'s body writes. `V.len(w)`
//@ still calls `V`'s, since a parameter takes any value of its shape.

//@ types/methods 34
//@ ### Calling a method
//@
//@ `recv.name(args)` calls the method of that name of the declaration the
//@ receiver was typed with, once no slot of that name answers — a direct call, the same C a
//@ call to a function makes. The receiver is passed first, the way the
//@ method takes it:
//@
//@ | receiver written | `self` | `^self` |
//@ |---|---|---|
//@ | a place: a name, `s[i]`, `p^` | a copy of it | its address |
//@ | a temporary `make()`, a `const` | a copy of it | refused — bind it first |
//@ | a `^T` | a copy of what it points at | passed as is |
//@ | a `ref T` | a copy of what it holds | the pointer it lends |
//@
//@ Through at most one `^` or `ref`: a `^^Vec2` is not a receiver. A
//@ method is called, so `v.len` with no call is refused and names the
//@ function, `Vec2.len`. In a template, calling a method on a value of `$T`
//@ asks `T`'s declaration, so the template is instantiated once per
//@ declaration and each calls its own method.
