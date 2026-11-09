//@ procs/defaults 40
//@ ### A default is call-site sugar and not part of the type
//@
//@ A parameter may carry a default, and the value is folded at the
//@ declaration: it may name a constant and it may not name a call, nor
//@ another parameter — a parameter is not in scope where its neighbour's
//@ default is written, and that is a scope fact rather than an ordering one.
//@ The one default that is not folded there is `Location at = #location()`,
//@ which is the place of each call that leaves it out (see *Where it is
//@ written: `#location()`*).
//@
//@ `nil` is a default for a parameter whose type takes it — a window, a
//@ pointer, a `ref`, a function — and it folds to that type's zero: the
//@ empty window for `[]f64 xs = nil`, the null pointer for
//@ `^Node next = nil`. No address is a constant, but a zero is, and the
//@ zero is what is folded.
//@
//@ ```dado
//@ package doc_sample
//@
//@ public i64 total([]const i64 xs = nil):
//@     i64 sum = 0
//@     for x in xs:
//@         sum += x
//@     return sum
//@
//@ public i64 nothing():
//@     return total()
//@ ```
//@
//@ Defaults are **trailing-only**, and the refusal is at the declaration
//@ rather than at some later call. A positional call fills slots left to
//@ right, so a gap in the middle would be a slot the call could not say it
//@ had skipped. A *named* call may omit any defaulted parameter it likes,
//@ because a named call never had to count.
//@
//@ **A default is not part of the function's type**, which is the part worth
//@ knowing before it surprises somebody: a function declared with two
//@ parameters, one of them defaulted, has a two-parameter type, and binding
//@ it to a one-parameter function reference is refused. The sugar exists at
//@ the call site and nowhere else — the emitted function has no idea.

//@ procs/packs 50
//@ ### A pack is a calling convention, not a type
//@
//@ A parameter may take the rest of the arguments. Two spellings, and the
//@ difference is what the caller may write.
//@
//@ A homogeneous pack takes any number of one type. `#any` takes whatever
//@ the call writes, each argument keeping its own type — and the function is
//@ instantiated once per *sequence* of argument types, so the body is
//@ compiled knowing exactly what it was handed. Neither one is a runtime
//@ list and neither allocates: the storage is a compound literal in the
//@ caller's own block.
//@
//@ **A pack is not part of a function's type.** It cannot be written in a
//@ function reference, and it cannot cross into a `foreign` signature —
//@ where it would have to mean C's own variadic convention, which is a
//@ different thing that this language does not promise to match.
//@
//@ ```dado
//@ package doc_sample
//@
//@ // `#len` over a pack is the count the call wrote, known at compile time.
//@ public i64 count(..#any xs):
//@     return #len(xs)
//@ ```
//@
//@ One parameter may be the pack and it is the last.
