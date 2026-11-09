//@ flow/conditions 11
//@ ### The conditional value
//@
//@ `a if c else b` is `a` when `c` holds and `b` when it does not, and only
//@ the one taken is evaluated. It is the loosest expression there is and
//@ right-associative, so `a if c else b if d else e` chains to the right
//@ with no parentheses. A newline ends it, so its `else` is on the same
//@ line — except inside brackets, where a line continues until they
//@ close, as it does for any expression: `(a if c` and `else b)` on the
//@ next line is one value. The condition is a `bool`, as every condition
//@ is.
//@
//@ **The two arms are one type**, and nothing is found in between: no
//@ widening, no common type, no conversion supplied. An untyped literal in
//@ one arm — `#LEN` among them — takes the other arm's type, exactly as it
//@ takes a typed sibling's in an array literal, so `n if n > 0 else 0` is
//@ `n`'s type whatever that is. So does an arm of `int` or `float` beside
//@ an explicit width of its kind: the explicit width is the value's type,
//@ on whichever arm it is written. Two arms of two other types are
//@ refused, and the repair is the conversion written on the arm that
//@ should change.
//@
//@ ```dado
//@ package doc_sample
//@
//@ // A folded constant: package-level, outside every `when`, and so
//@ // visible to everything downstream — a `when` included.
//@ const i32 PAGE = 16384 if #OS == #DARWIN else 4096
//@
//@ public i32 limit(i32 base, bool verbose):
//@     return base if base > 0 else PAGE if verbose else 256
//@ ```
//@
//@ It is an expression and not a statement: written on a line of its own it
//@ is a value thrown away, and refused as one. What runs something only
//@ when a condition holds is an `if`.
