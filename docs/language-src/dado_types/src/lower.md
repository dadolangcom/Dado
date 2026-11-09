//@ procs/returns 10
//@ ### A signature is a tuple in and a tuple out
//@
//@ A parameter list is a tuple and so is a return, which is why multiple
//@ returns needed nothing built for them. `(i32 a, f64 b) g()` and a local
//@ written `(i32, f64)` are **one arena entry** — not two shapes that happen
//@ to agree, one interned shape reached by two spellings. A dump under
//@ `--emit=types` shows the single tuple where a reader might expect two.
//@
//@ So *how many values may a function return* is the same question as *how
//@ many slots may a shape have*, and it is answered once.
//@
//@ ```dado
//@ package doc_sample
//@
//@ type Pair: (i32, i32)
//@
//@ private (i32 lo, i32 hi) split():
//@     return (3, 4)
//@
//@ private i32 sum(Pair p):
//@     return p[0] + p[1]
//@
//@ // `split`'s names are a view over the shape, so its value fills a `Pair`
//@ // with no conversion written anywhere.
//@ public i32 total():
//@     Pair p = split()
//@     return sum(p)
//@ ```
//@
//@ The names travelling with the view rather than the type is the
//@ consequence people meet first: a caller that does not restate them
//@ reaches the slots by the axes instead, and a name the caller never
//@ declared is not found.
