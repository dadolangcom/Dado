//@ types/views 35
//@ ### What a frame takes back
//@
//@ A block minted from an allocator the compiler can **name** lies in a
//@ region: `core:runtime`'s scratch (`runtime.temp()`, `runtime.enter_temp()`,
//@ or a `using` of either), or a `core:mem/arena` arena
//@ (`arena.allocator(&a)`). It lies above the innermost frame or mark the
//@ function holds open on that region when it is minted — a `runtime.enter_temp`, a
//@ `runtime.temp_open`, an `arena.begin` — and growing it moves it above the innermost
//@ one open when it grows. Only what can point lies in a region: the `i64`
//@ that `(tmp, n) = work()` writes beside an arena's `ref` is a number.
//@ **What ends the region ends the block**: rolling
//@ the frame back (`runtime.close_temp()`, `runtime.temp_close(cp)`,
//@ `runtime.close_temp_hoisting`), ending the mark (`arena.end`, `arena.end_keeping`),
//@ destroying or resetting the arena or the scratch. After that the handle, its
//@ copies and its views are not read, written, walked, grown or handed on
//@ (WRN0803, a warning); `#len`, `#cap`, `#delete` and rebinding stay legal. A frame
//@ closed with `runtime.close_temp_keep()`, or opened escaping
//@ (`runtime.enter_temp(true)`), is not rolled back: what it held belongs to the
//@ enclosing frame.
//@
//@ The same holds through calls. A helper that closes the frame its caller
//@ opened (`close_frame()`) ends that frame's blocks at the call. A function
//@ that grows a handle it was handed inside a scratch frame it rolls back takes
//@ the growth back with the frame, so a caller that hands it a handle **it
//@ knows is in the scratch** — or in an arena over it — may not use that
//@ handle after the call. And a function that mints its result on its
//@ caller's allocator inside a frame it rolls back hands a scratch caller
//@ memory that is already gone.
//@
//@ ```
//@ void build(^ref []i64 out):
//@     using runtime.enter_temp():
//@         out^ = #append(out^, 1)   // grown above build's frame
//@     _ = runtime.close_temp()      // ... and rolled back with it
//@
//@ using runtime.enter_temp():
//@     ref []i64 small = #make([]i64, 0)
//@     build(&small)
//@     i64 v = small[0]              // WRN0803: the call took it back
//@ ```
//@
//@ What is not followed is said out loud. A handle whose allocator the
//@ compiler cannot name — `#default` or `#return` a caller chose, a parameter,
//@ a package variable, `core:app`'s own frames — is not checked, and neither is
//@ its growth inside a callee's frame: the callee cannot see its caller's
//@ allocator, and it hands the result out by reserving before the frame,
//@ hoisting it, or keeping the frame when `runtime.temp_allocs()` moved. A
//@ frame some path closes with `runtime.close_temp_keep()` is taken to decide at its
//@ close. A handle kept in a struct's field, a `rawptr` (the bytes
//@ `runtime.close_temp_hoisting` and `arena.end_keeping` hand back among them), and a frame
//@ opened or closed on only some paths are not followed either, and neither is
//@ a run handed by value and answered back: `small = build(small)`, for a
//@ `ref []i64 build(ref []i64 out)` that grows its parameter inside a frame it rolls
//@ back, is not warned of.
