//@ types/views 42
//@ **A window is not a reference, and its `nil` is the empty one.**
//@ Where a `[]T` is wanted, `nil` is the value a `[]T` declared with
//@ nothing holds, the empty window: `[]i32 q = nil`, `q = nil`, an
//@ argument, a parameter's default. Compared, it is that value too, and
//@ `==` compares windows by their elements, so for a `[]T`, `q == nil`
//@ asks whether the window is empty, an empty window over real storage
//@ included. That is a different question from the one above: for a
//@ `ref []T`, `nil` is a run never made, and a made one is not `nil`
//@ however short.
//@
//@ ```dado
//@ package doc_sample
//@
//@ // The first element, or `fallback` when there is none.
//@ private i32 first_or([]const i32 xs, i32 fallback):
//@     if xs == nil:
//@         return fallback
//@     return xs[0]
//@
//@ public i32 firsts():
//@     [2]i32 store = [7, 8]
//@     return first_or(store[0..<0], 1) + first_or(nil, 2) + first_or(store[:], 0)
//@ ```
