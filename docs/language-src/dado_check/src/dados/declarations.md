//@ toys/dialect 27
//@ ### Types and unions
//@
//@ A class may declare a `type`, spelled as Dado spells one: named slots,
//@ one per line — `type Vec2:` over `float x` and `float y` — or the shape
//@ after the colon, `type Pair: [2]float`, `type Hp: int`. A class and a
//@ type answer different questions. A class is held by reference:
//@ assigning, passing and returning one hand over the same instance, `==`
//@ asks whether two names hold the same instance, and it has no operators
//@ but its methods. A type is a value: assigning, passing and returning one
//@ copy it, `==` compares its slots, and its operators are Dado's for its
//@ shape — a type whose slots share one type is that fixed array, so `Vec2`
//@ is a `[2]float` with its `+` and its `* 2.0`, and a type of mixed slots
//@ is the tuple of them. A type has no identity and extends nothing. Whether
//@ a value is copied or shared is decided by the word that declares it,
//@ `class` or `type`, never by where the value is kept.
//@
//@ A slot holds what a script holds: an `int`, a `float`, a `bool`, a
//@ `string`, a fixed array, a `[]T`, a map, an enum, a tuple or another
//@ type. A width, `ref` and a pointer are Dado's (`ERR1301`). A slot holding
//@ a `[]T` or a map holds that container by reference, so a copy of the
//@ type shares it, as a Dado type holding a `ref []T` does: after `u = t`,
//@ `u.items.append(3)` is seen through `t.items`, and `u.n = 5` is not seen
//@ through `t.n`.
//@
//@ A type is built as Dado builds one, by the same code: from one value per
//@ slot, `Vec2(1.0, 2.0)` or with the slots named, `Vec2(y: 2.0, x: 1.0)`;
//@ by a tuple or array literal where the type is wanted, `Vec2 v = (1.0,
//@ 2.0)` or `Vec2 w = [3.0, 4.0]`; or by the conversion of one value of its
//@ shape, `Vec2(q)`. A slot is read and written by its name, `v.x` and
//@ `v.x = 3.0`, and by its position where the type is a fixed array,
//@ `v[0]`; a store changes the value the name holds and no copy of it. A field or a local written without a value holds each
//@ slot's zero, and `Vec2.names()` lists the slot names.
//@
//@ A `union` is Dado's too: `union Shape: (Circle, Rect, string)` is a value
//@ that is exactly one of its member types, which are distinct types. A
//@ value becomes one by the conversion `Shape(c)`, which picks the member by
//@ the value's type, and is read by Dado's narrowing `switch`: each arm
//@ names a member's type and may bind the value as that type, `case Circle
//@ k:`, and a `switch` naming every member needs no `else`. A field of a
//@ union written without a value holds its first member's zero.
//@
//@ Types, unions, enums and traits are declared in a class or an inner
//@ class. Each is named by its bare name inside the class that declares it
//@ and that class's inner classes, through that class's name in the outer
//@ class — `Board.Cell` — and through its whole path in the package,
//@ `Game.Board.Cell`. A package that imports it writes the import's name
//@ first, `npc.Game.Vec2`; a written type is at most three names, so one an
//@ inner class declares is not reached from another package yet. A
//@ class's inner classes, types, unions, enums and traits share one set of
//@ names. A type, a union or an enum holds no declaration, so nesting ends
//@ there, and a class inside an inner class is still refused. A
//@ declaration of the package or the class answers a name DadoScript also
//@ has built in, `Location` or `Error`, in a written type and in a call
//@ alike.
//@
//@ A type crosses between Dado and a script as the Dado shape it is, each
//@ way: `type Point:` over `float x` and `float y` is Dado's `(f64 x, f64
//@ y)`, which a Dado `Vec2` of two `f32` slots adapts to slot by slot, and a
//@ type of mixed slots crosses as a tuple does. A union of a script's
//@ crosses into a Dado call of its method as the member Dado's value
//@ crosses into, and an enum of a script's, which has no number, into none
//@ (`ERR1201`). A script's type is data: a
//@ method written on one, a trait taken on with `using` and binders are not
//@ built yet (`ERR0001`), since behaviour written in a script lives in
//@ classes.
