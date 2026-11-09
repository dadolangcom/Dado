//@ procs/returns 11
//@ **A function's type is the same two tuples, written return first**:
//@ `i32(i32)`, and a return of several slots spelled whole in front of
//@ the parameters, in a `type` body as anywhere a type is written:
//@ `type Fn: (f32, bool)(i32, rawptr)`. A return that opens with a
//@ prefix (`[]`, `[N]`, `^`, a qualifier) is **bracketed**, because a
//@ prefix takes the whole type after it: `[]i32(i32)` is a run of
//@ `i32(i32)` functions, the same reading that makes `[]Pair(T)` a run of
//@ `Pair(T)`, and the function answering `[]i32` is `([]i32)(i32)`.
//@ Diagnostics, hovers and `#spell` print a function's type that way, so
//@ the refusal of one where the other is wanted (`ERR0504`) shows two
//@ spellings that differ, and its help names the bracketed one.
//@
//@ ```dado
//@ package doc_sample
//@
//@ type Fn: (f32, bool)(i32, rawptr)
//@
//@ private (f32, bool) probe(i32 n, rawptr p):
//@     return (f32(n), p == nil)
//@
//@ private []i32 none(i32 n):
//@     return nil
//@
//@ private i32 twice(i32 x):
//@     return x * 2
//@
//@ public i32 calls():
//@     Fn f = probe
//@     ([]i32)(i32) mk = none
//@     []i32(i32) fs = [twice, twice]
//@     (f32 _, bool at_nil) = f(1, nil)
//@     []i32 got = mk(3)
//@     return fs[1](i32(#len(got))) + (1 if at_nil else 0)
//@ ```
