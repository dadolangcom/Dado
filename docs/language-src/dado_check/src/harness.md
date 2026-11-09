//@ toys/dialect 30
//@ ### A Dado program runs a script
//@
//@ A Dado program runs scripts in a VM it makes: `try VM vm = #script_vm()`
//@ makes one on `#heap`, and `#script_vm(alloc)` on the allocator written.
//@ A VM is an engine of its own, with its own heap; it belongs to the thread
//@ that made it, and a thread may make several. `#delete(vm)` ends the
//@ engine and everything in it.
//@
//@ `try npc.Goblin g = #script_new(vm, npc.Goblin, 3)` makes an instance of
//@ a class in the VM — `#script_new(vm, Goblin)` for a class of this
//@ package — passing its init method the arguments after the class, and
//@ holds it by a handle of the class's type. The script is never named by a
//@ path: the compiler checked it and carries it in the program, and the VM
//@ loads the class the first time it makes one. Making one can fail, since
//@ init runs, so it is written under `try`. `#delete(g)` lets go of the
//@ instance.
//@
//@ `#script_run(g)` runs the instance's `ready` method, as a program's
//@ main function is run: it waits for it, and answers an `i64` — what `ready`
//@ returns when it returns an `int`, 0 when it returns nothing, and when it
//@ fails, the status a failing main function exits with, its text already on the
//@ error sink. It never fails itself, so it is not written under `try`. The
//@ arguments after the handle are `ready`'s, as `#script_new` passes `init`
//@ the ones after the class: `#script_run(g, 60, "Ann")` runs
//@ `int ready(int frames, string who)`, checked as a method's call is.
//@
//@ Every other method is called through the handle: `g.hit(3)`. The
//@ compiler checks the method against the class, with the count of its
//@ arguments and the type of each: a script's `int` parameter takes any
//@ integer (a `u64` by its bits), its `float` an `f32` or `f64`, its `bool`
//@ a `bool`, and its `string` a copy of a `string8` or a `ref([]char8)`. A
//@ script's array, `[]int`, takes a Dado slice, a `ref([]T)` or an array
//@ whose elements cross; its fixed array, `[2]float`, a Dado array or shape
//@ of as many slots, `[2]f32` or `Vec2`; and a tuple a Dado tuple of as many
//@ slots. A Dado enum, `distinct` type or union the method's signature names
//@ takes a value of it, by Dado's own rules, and a union the script declares
//@ takes a value of one of its members' types (see *Enums, `distinct` types
//@ and unions cross the seam*).
//@ One call from Dado carries at most 16 arguments, and a call passing more
//@ is refused (`ERR1221`). It answers what the method returns, as Dado holds it: an int as `i64`, a
//@ float as `f64`, a bool as `bool`, a string as a `ref([]char8)`, an array
//@ as a `ref([]T)` of those — each made on the ambient allocator, which the
//@ program ends with `#delete` — a fixed array as a Dado array of those,
//@ `[2]float` as `[2]f64`, a tuple as a Dado tuple of those, and a Dado
//@ enum or union as itself.
//@
//@ What a script prints and what `fmt.println` writes go to the same
//@ standard output, in the order they ran, unless the VM is given sinks of
//@ its own (below). A method that throws, called under `try`, fails that
//@ statement, with its text already on the error sink; one called without
//@ `try` ends the program with the failure's status after the same text. A
//@ build that cannot run a script is refused where it makes a VM: a `#NONE`
//@ target has no C library for the engine. On the web a script runs on the
//@ page's own JavaScript engine, and the few features that engine cannot give a
//@ wasm module (new code at run time, waiting on an `async` method, a pause
//@ with no second thread) are refused where they are written.
