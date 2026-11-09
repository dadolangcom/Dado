//@ types/views 32
//@ ### Arrays and slices: a read-only window is `[]const T`
//@
//@ A window nothing is written through is `[]const T`, the word on its
//@ elements. It reads, indexes, walks with `for`, measures with `#len`
//@ and cuts sub-windows as a `[]T` does, and an element write through
//@ it is `ERR0606`. A `[]T`, a fixed array's `a[:]`, a `ref []T` and a
//@ run written in place (`[a, 2, 3]`) all stand where one is wanted,
//@ so a function that only reads asks for `[]const T` and every caller
//@ reaches it; the other direction, handing one to a `[]T` that could
//@ write, is refused (`ERR0504`). A window over a `const` array is one.
//@
//@ **The word is a promise about the place, never part of a value read
//@ from it.** An element of a `[]const u32` indexes, `f64(x)` of a
//@ `const f32` widens, and a `$T` bound from one binds the type under
//@ the word, so `T` is `u32`. A template that only reads writes
//@ `[]const $T`, as `core:slices`' readers do; one written `[]$T` may
//@ write, so a read-only window does not reach it. `#copy` of one is a
//@ new block nobody else holds, a `ref []T` its caller writes, unless
//@ the position names `ref []const T`.
//@
//@ ```dado
//@ package doc_sample
//@
//@ private i64 total([]const i32 xs):
//@     i64 sum = 0
//@     for x in xs:
//@         sum += i64(x)
//@     return sum
//@
//@ public i64 counted():
//@     [3]i32 store = [1, 2, 3]
//@     []const i32 ro = store[:]
//@     return total(ro) + total(store[1..<3]) + total([4, 5])
//@ ```
//@
//@ **`const []T` is not a type** (`ERR0401`, naming `[]const T`). In
//@ front of the brackets the word would sit on the window's two-word
//@ header, which promises its elements nothing, so it would read as
//@ the promise it is not; `[]const T` is the one spelling.
