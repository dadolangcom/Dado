//@ toys/dialect 28
//@ ### A class embeds a type
//@
//@ A class may embed a `type` by naming it as a member, on a line of its
//@ own, as a `type` body embeds one: `Vec2` in a class body makes each named
//@ slot of `Vec2` a field of the class, so a method reads and writes them as
//@ it does any field, bare or through `self` (`self.x`), and from outside the
//@ class `ship.x` reaches one. The type may be one the class, or the class
//@ around it, declares, or a Dado declaration the script names, `Vec2` of
//@ its package or `entities.Vec2` of an import. The fields start at their
//@ slots' zeros.
//@
//@ The type's name, as a member, is the embedded value whole: `Vec2`, or
//@ `self.Vec2` and `ship.Vec2`, reads a copy of the slots as one `Vec2`,
//@ and `Vec2 = v` writes each of them — a plain `=`, on its own line, as Dado
//@ stores an embedded member. A Dado type's `self` methods are promoted:
//@ `ship.length()` calls `Vec2.length` with a copy of the slots, as Dado
//@ calls a promoted method. A `^self` method writes through a pointer, and
//@ there is no `Vec2` object inside a class to point at, so calling one is
//@ refused at the call; write the change in a method of the class and store
//@ the member whole. A method a trait's default gives the type, which the
//@ type does not write, is not reached from a script yet (`ERR0001`).
//@
//@ A class is never embedded: inherit one with `extends`, or hold it in a
//@ field. A collision is refused, never an override: a slot or a promoted
//@ method named like a field or a method of the class, one it inherits, or
//@ a slot of another type it embeds, is refused, naming both. Embedding
//@ takes on no trait: a class that embeds an implementer of `Drawable` is
//@ not one, and a class's `using` is answered by the methods it writes. A
//@ type whose slots are not all named is not embedded, since a field is
//@ reached by its name; hold it in a field instead. A host reaches a
//@ class's own fields through a handle, and an embedded type's slots not
//@ yet.
