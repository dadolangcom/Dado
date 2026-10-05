<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:runtime

core:runtime — the scratch allocator, its checkpoints, and `TEMP_GUARD`, one
of each per thread.

    import "core:runtime"

    _ = runtime.init_scratch(65536, true)
    defer runtime.destroy_scratch()

    ref Report out = #new(Report, #return)       // the caller's: minted before the frame
    defer runtime.close_temp()                   // pops on scope exit; no Checkpoint held
    using runtime.enter_temp():                  // opens the frame *and* installs it
        ref []i32 scratch = #make([]i32, n)       // the frame's own
        out.total = …                            // written into, never allocated here

The result is minted **before** the frame opens. Minted inside it on `#return`, it
would lie above the frame's mark whenever the caller is itself in the scratch, and
the rollback would take it back (`dadoc` refuses the caller's use, ERR0803).

`enter_temp`/`close_temp` is the recommended pattern; `temp_open`/`temp_close`
with a held `Checkpoint` remain for explicit, non-LIFO control.

This is what replaces `context.temp`, the second of `Context`'s two slots.
**It is a library and not a language feature**, and the reason it can be one
is the ladder: `#return` is a durable destination *inside* a temp scope, so
the arena resets without the compiler proving anything about what escaped —
provided the `#return` mint is made before the scope's frame opens (above).
Nothing below is privileged; a program that wanted its own scratch region
with its own policy writes this file.

## Why this is an import and not a global name

The global namespace is **freestanding-total** — every
name in it works on a target whose OS is `#NONE`. A scratch arena needs
storage, and `init_scratch` gets its first block from C's heap, so a name
like `temp()` in the global namespace would be a global name that does not
work on half the targets. An import a freestanding build simply omits is the
honest shape, and `init_scratch_over` is here so that a build with its own
backing allocator — a static block, a pool, an OS mapping — can have the same
scratch without `core:mem/libc` and without `<stdlib.h>`.

## What it is made of, and what it is not

The arena is `core:mem/arena`: a bump cursor over a growing **list** of
blocks, `Alloc` advancing a cursor, `Free` a no-op, `FreeAll` O(1) in the
number of allocations, and `begin`/`end` marks. That package is the allocator
and this one is the **policy** over it — one instance per thread, the
checkpoint discipline, and the guard. Writing a second arena here would be
two arenas to keep correct and only one of them tested.

## What each `AllocatorMode` does through `temp()`

    Alloc     carve from the arena; `nil` when a fixed scratch is full
    Resize    the arena's — **in place** when the block is the head's last
              allocation, `mem.resize_by_copy` otherwise. `resize` takes no
              allocator argument at all: it uses the one the
              `ref` already carries, so a `ref` minted inside a temp scope
              resizes through *this* allocator whoever asks
    Free      a no-op — an arena does not reclaim one block. `delete` on a
              temp `ref` is safe, does nothing, and is good practice: the
              allocator decides, and there is no second spelling to learn
    FreeAll   **refused while any temp scope is open**, and `reset_scratch`
              where none is. See `TEMP_GUARD` below — a `FreeAll` through the
              interface would rewind past every open checkpoint at once,
              which is the exact rollback this package exists to defer

So all four modes are answered and one of them is conditional. There is no
mode this arena cannot service: `Resize` is the one an arena is usually
written without, and `core:mem/arena` implements it, so nothing here has to
push a fallback onto a caller.

## `TEMP_GUARD`

**A rollback that would free storage a still-open temp scope is handing out
is deferred**, with a strippable warning, and nothing is freed. The failure
mode is bloat and never a use-after-free. It is one predicate — *is this the
innermost open scope* — asked in one place, `guard` below.

The warning is an `expect`, which is a strippable claim and the
stripping mechanism the language already has (`--no-expect`). It is
**not** a second one: stripping deletes the report and never the deferral,
because the predicate is computed before the claim is written and the
deferral is what the caller branches on.

## Where a deferral goes

It is discharged, and *never* is not the answer. Two places, and they are the
same place seen twice:

  * the outermost scope's own `temp_close` rewinds to a mark taken at depth
    zero, which frees everything above it — the deferred region included;
  * and if that outermost close was itself the deferred one, the bytes
    survive to depth zero, where the **next** `temp_open` resets the arena
    whole before taking its mark.

Both are safe for one reason: at depth zero no temp scope is open, so by this
package's own contract nothing allocated through `temp()` is still live. The
argument against *never* is the frame loop, which is what a scratch allocator
is for: a loop that defers once per frame and never discharges does not bloat,
it dies. `deferrals()` reports the running total so a program or a test can
see that it happened at all.

## Declarations

42 declarations, 21 public.

* `const u64 BLOCK = 65536` — The first block's size, and the one every doubling starts from.
* `type Checkpoint` — Where the arena was when a scope was entered, and which scope it was.
* `const u64 SCOPE_MAX = 64` — The recommended `enter_temp`/`close_temp` pattern keeps no `Checkpoint` at the…
* `bool init_scratch(u64 first_block = BLOCK, bool can_grow = true)`
* `bool init_scratch_over((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) backing, u64 first_block, bool can_grow)` — The same, over a backing allocator the caller names.
* `void destroy_scratch()` — Hand every block back. Every reference into the scratch is stale from here…
* `bool ready()` — Whether this thread's scratch is configured.
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) temp()` — The scratch, as an `Allocator`. This is what `using runtime.temp():` takes.
* `bool is_temp((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a)` — Whether `a` is this package's scratch, by the state it carries.
* `Checkpoint temp_open()` — Enter a temp scope: mark where the arena is, and say a scope is open.
* `bool temp_close(Checkpoint cp)` — Leave the temp scope `cp` opened, releasing everything allocated since.
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) enter_temp(bool escaping = false)` — Open a temp scope and return the arena as the ambient in one call — the…
* `bool close_temp()` — Close the innermost `enter_temp` frame. `false` when no frame is open or the…
* `bool close_temp_keep()` — Close the innermost `enter_temp` frame **without** rolling it back, whichever way it…
* `rawptr close_temp_hoisting(rawptr p, u64 n)` — Close the innermost `enter_temp` frame, rolling it back as `close_temp` does, but…
* `bool reset_scratch()` — Release the whole scratch, keeping the oldest block — the per-frame call.
* `u64 temp_used()` — How many bytes the scratch is holding, across every block.
* `u64 temp_blocks()` — How many blocks are live. The number a frame loop watches: it should stop…
* `u64 temp_allocs()` — How many `Alloc` and `Resize` requests the scratch has served on this thread, ever,…
* `u64 temp_depth()` — How many temp scopes are open on this thread.
* `u64 deferrals()` — How many rollbacks `TEMP_GUARD` has deferred on this thread, ever. Nonzero is not a failure…
