//@ flow/iteration 20
//@ ### Iteration is stride, not a concept
//@
//@ There is no iterator in this language: no trait to implement, no hook to
//@ provide, nothing a program can make walkable. What may be walked is
//@ decided by one closed question — *does this shape have one element type*
//@ — and the answer comes from the layout.
//@
//@ That is the tuple axiom paying out again. An array is a tuple whose slots
//@ share a type, so every element sits the same distance from the one before
//@ it, and a loop variable has a type to have. A shape whose slots differ
//@ has no single element type, so there is nothing for the loop variable to
//@ be; the refusal is a fact about the shape rather than a feature nobody
//@ wrote yet.
//@
//@ A slice walks, a string walks as its code units, a map walks as key and
//@ value, and a value that is not a tuple at all walks once as itself — that
//@ last one falling out of *a one-slot tuple is its element* rather than
//@ being a case. A channel walks as its parameters, destructured, through
//@ `#peek` or `#drain` — any number of them, and one name binds the whole
//@ message as its tuple.
//@
//@ ```dado
//@ package doc_sample
//@
//@ // `Vec2` is two slots of one type, so it is `[2]f32` and it walks.
//@ type Vec2: (f32 x, f32 y)
//@
//@ public f32 total(Vec2 v):
//@     f32 sum = 0.0
//@     for a in v:
//@         sum += a
//@     return sum
//@ ```
//@
//@ A range is the exception and is not a value: it is a thing to count
//@ through, and what it binds is what it counts in.
