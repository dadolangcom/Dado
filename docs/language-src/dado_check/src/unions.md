//@ types/distinct 60
//@ ### A union is one of its members, and a member converts into it
//@
//@ `union Extent: (Whole, Pending, ToClose)` is exactly one of three types,
//@ with a tag saying which. A member is a **type**, not a name: the
//@ discriminant is the member's type, so members are pairwise distinct
//@ types — two `type` declarations over one shape are one type, and
//@ `distinct` is how two members that hold the same shape are told apart.
//@
//@ **A value of a member's type converts into the union.** `Extent(w)`
//@ writes it; and where the union is the declared type — a `return` from a
//@ function answering it, a local, an assignment, an argument, a slot of a
//@ construction or a literal — the conversion is supplied, as a reference
//@ lends its view there: `return Whole(n)` from an `Error!Extent` function.
//@ It is supplied only where the value's type *is* one member's type, so
//@ the member is the one `Extent(w)` would pick and no other could be. A
//@ value that would only become a member's type by adapting (an `int`, a
//@ narrower width) is not converted, and nor is a member of a member: a
//@ union inside a union is two conversions, and the inner one is written,
//@ `return Outer(Inner(v))` or `return Inner(v)`.
//@
//@ **A member that carries nothing is a `distinct` over a byte**: a shape
//@ with no slots has no value to hold (`ERR0506`), so a case with no data is
//@ written `distinct type Idle: u8`, built as `Idle(0)`, and matched by
//@ `case Idle:`. A union's storage is its largest member's, so the byte
//@ costs nothing.
//@
//@ ```dado
//@ package doc_sample
//@
//@ distinct type Idle: u8
//@ distinct type Got: (i32 n)
//@
//@ union Event: (Idle, Got)
//@
//@ public Event poll(i32 n):
//@     if n == 0:
//@         return Idle(0)
//@     return Got(n)
//@
//@ public i32 size(Event e):
//@     switch e:
//@         case Idle:
//@             return 0
//@         case Got g:
//@             return g.n
//@ ```
