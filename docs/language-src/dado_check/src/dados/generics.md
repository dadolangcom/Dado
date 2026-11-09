//@ toys/dialect 40
//@ ### Generic methods
//@
//@ A method whose parameters write `$T` is generic, as a Dado function is:
//@ `T first([]$T xs)` takes an array of anything and answers its element
//@ type. Each call binds `T` from what it passes — `first(names)` binds
//@ `string` — and the method is checked for that binding, so a mistake in
//@ its body is reported for the call that made it. `where` limits what a
//@ call may bind: `where T has ==` admits only types with `==`, which is
//@ what `contains` and `index_of` over a `[]T` need, and `where T is int`
//@ or `T is scalar` (an `int` or a `float`) names the types themselves.
//@
//@ Inside a generic method `when T is int:` selects by what `T` is bound
//@ to, so each binding keeps its own arm and the others are never checked.
//@ A generic method whose bindings all come out as the same JavaScript is
//@ emitted once; one whose arms or printing differ by type is emitted once
//@ per binding. A script's numbers are `int` and `float` only, so its
//@ generics are over containers.
