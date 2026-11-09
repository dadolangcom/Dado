//@ types/distinct 40
//@ ### A C type is not a third kind
//@
//@ A transparent `foreign` type restates a layout a header already
//@ named — `type Vector2: (f32 x, f32 y)` — and it **is** the shape it
//@ restates, as a `type` declaration is: `Vector2`, `(f32, f32)` and
//@ `[2]f32` are one type for checking, for generics, for operators and
//@ for `where`. `v.x + v.y`, `v[i]`, `for a in v` and
//@ `linalg.length(v)` all work on one, and there is no parallel native
//@ type to keep beside it and no conversion to call. A window is the one
//@ exception, for C's reason: `v[0..<2]` is one run of memory, and where
//@ the header named each slot as a member of its own the slots are two
//@ objects, not one run — so the window is refused there and taken where
//@ one member holds every slot (`type Big: ([40]f64 a)`).
//@
//@ What stays two is the **spelling**. The header calls the type
//@ `Vector2` and Dado calls its own `(f32, f32)` something else, and C
//@ never makes two separately declared structs compatible — so wherever a
//@ value moves between the two, the emitted C copies it across member by
//@ member. The layout assertions the emitted C carries for every
//@ restatement are what make that a copy and not a reinterpretation.
//@ One type in the checker, two names in the emitter.
//@
//@ **A value crosses; an address does not.** A `^Vector2` and a
//@ `^(f32, f32)` point at two C struct types, and there is nothing to
//@ copy — `ERR1025` refuses the pointer, the window and the reference
//@ alike. Nor is a restatement that has no Dado shape to be one type with:
//@ a struct laid out in **bits**, and one ending in a flexible array
//@ member, stay types of their own, as a `distinct` does. And a program
//@ that wants a C type kept apart from its shape writes `distinct` over
//@ it, which is the word for that everywhere else.
