//@ types/views 40
//@ ### `#truncate` shortens a run where it lives
//@
//@ `#resize` is exact because you named a number: a `#resize` down hands
//@ the capacity back to the allocator, and the block may move. Where the
//@ point is to keep the room — a stack popped and pushed again — the verb
//@ is `#truncate(r, n)`. It writes the length and nothing else: no
//@ allocator is asked, the block does not move, the capacity is what it
//@ was, and a view taken before it still points into the same block.
//@
//@ It only ever shortens. A count the compiler can see is negative is
//@ refused (`ERR0828`); a count past the run's length is a trap at run
//@ time that names both numbers, on every target, and `--no-assert` does
//@ not remove it. `#truncate` of a nil run to `0` does nothing.
//@
//@ **The elements it drops are not destroyed**, because Dado has no
//@ destructors: a `ref` held in one is not freed, and is lost if nothing
//@ else holds it — `#delete` it first. Their storage is zeroed, so the
//@ room past the length reads as zero, as a fresh `#make` does, when a
//@ later `#resize` grows back into it.
//@
//@ ```dado
//@ package doc_sample
//@
//@ // Pop the top of a stack: the length drops by one, the capacity stays.
//@ public i32 pop(^ref []i32 stack):
//@     i64 top = #len(stack^) - 1
//@     i32 value = stack^[top]
//@     #truncate(stack^, top)
//@     return value
//@ ```

//@ types/views 41
//@ ### A run grows where it is, and answers its handle
//@
//@ A reference carries the allocator that made it, and the verbs that
//@ grow one — `#append`, `#resize`, `#reserve` — ask that allocator and
//@ **answer the grown handle**: `xs = #append(xs, v)`, as Go writes it.
//@ The block may move, so the run handed in is moved into the answer; a
//@ dropped answer is `ERR0829`. When the same place is written on both
//@ sides and holds no call, it is evaluated once, so
//@ `rows[i].cells = #append(rows[i].cells, v)` grows the run where it
//@ lives. A different place on the left moves the run:
//@ after `ys = #append(xs, v)`, the run handed in is stale until it is
//@ rebound.
//@ `#append(xs, ..ys)` appends every element of a run (a `[]T`, a
//@ `ref []T`, a fixed array), growing once by `#len(ys)`; into a
//@ `ref []char8`, the run appended may be text, as Go's `append(b, s...)`:
//@ `b = #append(b, ..name)`.
//@
//@ **Growth cannot fail and be answered.** An allocator that declines —
//@ out of memory, an exhausted arena, a `#stack` rung, which cannot grow —
//@ stops the program, naming the verb, the run and the count, and
//@ `--no-assert` does not remove it. **A run that was never made grows on
//@ its own**: a `nil` `ref []T` (a slot of a zero-valued record, a
//@ declared local, one `#delete` has just nilled) takes its block from the
//@ current allocator, the one `#make([]T, 0)` written there would use.
//@
//@ ```dado
//@ package doc_sample
//@
//@ type Bag:
//@     ref []i32 items
//@
//@ // A bag that was never made grows on its first `put`.
//@ public void put(^Bag b, i32 x):
//@     b.items = #append(b.items, x)
//@ ```
//@
//@ **A helper grows a run it is handed by answering it.** A `ref []T`
//@ parameter is the callee's own copy of the handle, so the callee
//@ answers the grown run and the caller writes it back, `ys = push2(ys,
//@ 1, 2)`; a caller that keeps its old copy, `zs = push2(ys, 1, 2)`,
//@ holds a stale one (`WRN0803`). A function that grows a by-value
//@ parameter and neither answers nor keeps it loses the block
//@ (`LNT0830`). Through `^ref []T` the caller's run grows where it lives,
//@ `out^ = #append(out^, v)`. Only `#truncate` through a by-value copy is
//@ refused (`ERR0813`), since it answers nothing.
//@
//@ ```dado
//@ package doc_sample
//@
//@ public ref []i32 push2(ref []i32 xs, i32 a, i32 b):
//@     xs = #append(xs, a)
//@     return #append(xs, b)
//@
//@ public void push_into(^ref []i32 out, i32 v):
//@     out^ = #append(out^, v)
//@
//@ public ref []char8 greet(string8 name):
//@     ref []char8 b = nil
//@     b = #append(b, .."hello, ")
//@     return #append(b, ..name)
//@ ```
