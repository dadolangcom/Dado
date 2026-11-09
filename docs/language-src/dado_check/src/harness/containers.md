//@ toys/dialect 45
//@ ### Arrays and tuples cross the seam
//@
//@ A script's `[]T` is a JavaScript array, and so is its tuple. Both cross
//@ the seam **as a copy, for the call**, element by element — a slice's
//@ copy lent, its writes copied back (below) — and an element
//@ crosses as it would alone: into a script's `[]int` a Dado program passes a
//@ slice, a `ref([]T)` or an array of integers, and a Dado function a script
//@ calls with a `[]int` may take it as a `[]i32`, each element wrapping as a
//@ lone `int` would. A **fixed array crosses as the same type**: a Dado
//@ array, `[4]f32`, and a declared shape whose slots share one type, like
//@ `type Vec2: (f32 x, f32 y)`, which is `[2]f32`, are the script's fixed
//@ array of as many slots, `[4]float` and `[2]float`, each slot crossing as
//@ a lone value does — so a `Vec2` keeps its `+` in a script. So is a
//@ `foreign` restatement of a C struct whose fields share one type, such
//@ as `raylib.Vector2`, which is one type with its `[2]f32`. A script's
//@ `[2]float` passed to a Dado `[2]f32` adapts slot by slot, each slot
//@ rounding as a lone `float` passed to an `f32` does, and an `int` slot
//@ passed to an `i32` wrapping as a lone `int` does; a fixed array of
//@ another length is refused (`ERR1201`). A script's `[]float` passed into
//@ one must hold exactly as many elements — any other length raises when the
//@ call is made, naming the function, the parameter and both lengths. Dado
//@ passes its arrays and same-typed shapes to a script method taking a
//@ `[2]float`, and the method's `[2]float` answer arrives as a `[2]f64`. A
//@ tuple whose slots differ crosses as a script's tuple of as many slots. A
//@ map, a matrix and a `#soa` array cross too (see *Maps, strings of every
//@ width, matrices and functions cross the seam*).
//@
//@ The glue reads a script's array by each element's tag and never converts
//@ one kind into another: an element of the wrong kind raises, naming the
//@ function, the parameter and where the element is (`at [2][0]`). A `[]T`
//@ answered to Dado is a create verb's: a `ref([]T)` on the ambient allocator,
//@ its strings `ref([]char8)`, each Dado's to end with `#delete`.
//@
//@ A run or a construction that crosses a container is a run like any
//@ other: it enters its engine before it builds a single argument, so on an
//@ engine another thread is running, or one paused, it answers
//@ `script.BUSY`, after `#script_stop` `script.STOPPED`, and through a handle
//@ that holds no script `script.STALE`; its `out:` and `err:` are its own,
//@ `#script_error()` reads its failure's text, and without `try` it follows
//@ the script's `on_error:` policy.
//@
//@ **What is lent is one storage.** A slice crossing either way is lent for
//@ the call: the borrower works on a copy, and what it writes reaches the
//@ lender's storage each time control comes back to the lender — when the
//@ call returns, whether it succeeded or failed, and before any call back
//@ into the lender. A Dado slice a script's method writes, `xs[1] = 20`,
//@ holds 20 when the method returns, and a Dado function the method calls
//@ meanwhile reads the script's writes so far; a script's array a Dado
//@ function writes holds Dado's writes when the function returns, and a
//@ script's method the function calls meanwhile sees them. A slice inside
//@ an array, a tuple or another slice is lent the same way, its elements
//@ written in place. A lent array keeps its length, as a Dado `[]T`
//@ parameter does: a script that grows or shrinks one fails the run with
//@ `script.TYPE`, naming the argument, and nothing of it comes back — grow
//@ a copy. New text comes back where the lender owns it: a `ref([]char8)`
//@ element takes the script's new string on its own allocator, but a
//@ `string8` is a view and takes none, and a script writing one fails the
//@ run with `script.TYPE`. One storage passed for two parameters is one
//@ array on the other side: `held.f(xs, xs)` hands the script one array,
//@ and a script's `f(a, a)` hands Dado one slice, when both parameters are
//@ of one type. An array, a shape and a tuple are values: each side has its
//@ own, and only the slices inside one are lent. What the borrower keeps
//@ past the call is its own copy, which the lender's later writes do not
//@ reach and whose later writes do not reach the lender.
//@
//@ **A structure lent across is locked for its call.** While it is lent,
//@ its lender does not write it: a Dado function the script calls back into
//@ must not write a slice the run lent the script, and a script's method
//@ Dado calls back into must not write an array the script lent Dado. A
//@ write raises: in Dado, the run fails with the reserved code
//@ `script.LENT`; in the script, an error naming the function and the
//@ parameter. Nothing more is copied back once it has. Nothing is checked
//@ until the call reaches back across the seam, so a call that does not
//@ pays only for the copy back when it returns.
