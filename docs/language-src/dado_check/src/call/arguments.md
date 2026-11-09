//@ procs/arguments 30
//@ ### Named arguments are the parameter list being a view
//@
//@ A parameter list is a view over the argument tuple, so naming an argument
//@ is the same operation as naming a slot — there is no second mechanism and
//@ no extra machinery in the call. Names reorder: `sub(b: 4, a: 30)` is
//@ `sub(30, 4)`.
//@
//@ Three rules hold it together, and each is a refusal rather than a
//@ resolution.
//@
//@ A name must be one the callee declared. A slot may be filled once. And a
//@ call is **all positional or all named**, never a mixture — because in a
//@ mixed call which slot a positional argument fills depends on counting the
//@ names written to the left of it, and that is a rule a reader has to
//@ execute rather than read.
//@
//@ ```dado
//@ package doc_sample
//@
//@ private i32 sub(i32 a, i32 b):
//@     return a - b
//@
//@ public i32 backwards():
//@     return sub(b: 4, a: 30)
//@ ```
//@
//@ A splat fills positions, so it cannot be given a name.
