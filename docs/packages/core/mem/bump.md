<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:mem/bump

core:mem/bump — a cursor over a block, behind the allocator interface.

    import "core:mem"
    import "core:mem/bump"

    [4096]u8 storage
    bump.Arena arena = bump.over(&storage, 4096)
    mem.Allocator a = bump.allocator(&arena)

**The foundation every other allocator is described in terms of**, and the
one whose `FreeAll` is the reason to reach for it: releasing every
allocation an arena ever made is one store, O(1), no matter how many there
were.

It is also the allocator a **freestanding** program uses. At depth 0 on a
freestanding target `#default` is `nil`, so allocating there
means supplying an arena — and an arena over a `[N]u8` in static storage
asks nothing of libc, which is the property `corpus/freestanding` tests by
inspecting the emitted `#include` lines rather than by asserting. (That program
writes its own bump arena rather than importing this package — a corpus program
imports nothing outside its own directory — so what it pins is the *language*
claim this package is one implementation of. `bump_test.dado` is what tests the
code here.)

## What it does with each mode

    Alloc     align the cursor, hand back the block, advance
    Resize    **in place** when the block is the last allocation, which is
              O(1) and invisible from outside; `mem.resize_by_copy`
              otherwise
    Free      a no-op — an arena does not reclaim one block
    FreeAll   `used = 0`. One store. Every reference into the arena is stale
              from that moment, and nothing detects it

`Free` being a no-op is not a gap: the shipped set of allocators already
contains an allocator that does not honour `delete`, which is what makes a
collector an ordinary member of the interface rather than a new idea.

## Declarations

15 declarations, 8 public.

* `type Arena` — The arena's state: where the block is, how large it is, and how much of it…
* `Arena over(rawptr block, u64 cap)` — An arena over storage that already exists — a `[N]u8` on the stack, a…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator(^Arena arena)` — The allocator for `arena`. The arena is the allocator's `data`, so it is…
* `type BumpAllocator using mem.Allocator` — The allocator for a bump arena, as a declaration that takes `mem.Allocator` on:
* `(rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) data(self)`
* `u64 used(Arena a)` — How many bytes are in use, and how many are left. A test asserting an…
* `u64 available(Arena a)`
* `void reset(^Arena a)` — Release every allocation in one store. **Lifetime-ending**: every…
