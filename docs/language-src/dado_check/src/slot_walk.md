//@ flow/iteration 21
//@ ### A value's slots, walked at compile time
//@
//@ `for name, slot in #slots(a):` walks the slots of the declaration `a`'s
//@ type was written with, in the order it declares them. It is not a loop:
//@ the body is written out once per slot and each copy is checked on its
//@ own, so `slot` has that slot's own type — no box, no tag, no dispatch at
//@ run time — and `name` is that slot's name as a `string8` constant, the
//@ text `#names` gives for the same position.
//@
//@ ```dado
//@ package doc_sample
//@
//@ type Point: (i32 x, i32 y)
//@
//@ // Written out twice: once with `name` = "x" and `slot` = `p.x`, once
//@ // with "y" and `p.y`.
//@ public i32 sum_except(Point p, string8 skipped):
//@     i32 total = 0
//@     for name, slot in #slots(p):
//@         if name != skipped:
//@             total += slot
//@     return total
//@ ```
//@
//@ Two bindings are the name and the value, a map's order; one binding is the
//@ value. The value is a copy, as a `for` over an array binds one, and
//@ `for name, &slot in #slots(a)` binds each slot's address instead, which
//@ is how a reader fills a value in. With no loop left in the program there
//@ is nothing for `break` or `continue` to leave, so one aimed at the walk is
//@ refused; guard the rest of the body with an `if`.
//@
//@ The point is generic code. A template walking `#slots(a)` is
//@ instantiated once per declaration, and calling itself on a slot's value
//@ instantiates it at that slot's type. Inside a template a value with no
//@ slots — a number, a string, a slice — walks zero times, so the same body
//@ renders a leaf and walks a declared type, and one function serializes
//@ every declared type there is. Written where the type is fixed, the same
//@ walk is refused, because it could only ever do nothing. So are a pointer
//@ (walk `p^`), a union, whose members are alternatives rather than parts,
//@ and a shape no declaration named, which has no names to give.
