<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:mem/arena

core:mem/arena — a bump arena that **grows**, behind the allocator interface.

    import "core:mem"
    import "core:mem/libc"
    import "core:mem/arena"

    arena.Arena scratch = arena.over(libc.allocator())
    defer arena.destroy(&scratch)
    using arena.allocator(&scratch):
        ...                                  // `#default` is the arena here

`core:mem/bump` is a cursor over **one** block and answers `nil` when that
block is full. This is the same cursor over a **list** of them: when the head
block cannot fit an allocation, another is requested from a backing allocator
and becomes the head. Everything else is the same allocator, which is the
point — `Alloc` advances a cursor, `Free` is a no-op, and `FreeAll` is O(1)
in the number of allocations.

It is Odin's default temp allocator, which is the design worth copying: a
scratch allocator that is reached for constantly in a frame loop must not
have a size the caller has to have guessed right.

## What it does with each mode

    Alloc     carve from the head block; request another when it will not fit
    Resize    **in place** when the block is the head's last allocation, which
              is O(1) and invisible from outside; `mem.resize_by_copy`
              otherwise
    Free      a no-op — an arena does not reclaim one block
    FreeAll   release every block **but the oldest** and reset its cursor.
              Keeping one is what makes a frame loop steady-state
              allocation-free; releasing the rest is what stops a spike from
              being permanent. Every reference into the arena is stale from
              that moment, and nothing detects it

## Why this needed a window over a bare pointer

Every byte address below is `region[at..<at + n]` over a `[]u8` taken across
the block. There used to be no such form: reaching byte `at` meant
over-typing it as a `^[65536]u8`, which capped every allocator written in
Dado at a 64 KiB heap — and a *growing* arena whose blocks
cannot exceed 64 KiB is not one.

## Scoped release is `defer`, and is not automatic

    arena.Mark m = arena.begin(&scratch)
    defer arena.end(m)

Deliberately not emitted at scope exit. `FreeAll` at a scope boundary resets
the cursor to zero and therefore frees the **caller's** temp allocations,
which is a silent use-after-free; and the commonest thing a temp allocator is
used for is a helper that builds a temporary and **returns it**, which an
automatic release at block or function exit would break. Mark/release is the
correct primitive and where the marks go is the programmer's call.

## Declarations

29 declarations, 17 public.

* `type Block` — One block: the storage, the cursor into it, and the block allocated *before*
* `type Arena` — The arena: where new blocks come from, the head of the list, how large the…
* `type Mark` — A cursor position to rewind to. It carries the arena, so `end` takes one…
* `const u64 BLOCK = 65536` — The first block's size, and the one every doubling starts from. 64 KiB…
* `Arena over((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) backing)` — An arena that requests blocks from `backing`. **It does not allocate**: the…
* `Arena sized((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) backing, u64 first)` — The same, with the first block's size written. Later blocks double from it,…
* `bool prime(^Arena a)` — Request the first block **now** rather than on the first allocation, and say…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator(^Arena a)` — The allocator for `a`. The arena is the allocator's `data`, so it is borrowed…
* `type ArenaAllocator using mem.Allocator` — The allocator for an arena, as a declaration that takes `mem.Allocator` on:
* `(rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) data(self)`
* `void reset(^Arena a)` — Release every allocation, **keeping the oldest block**. One store per block…
* `void destroy(^Arena a)` — Hand every block back, including the oldest. The arena is reusable…
* `Mark begin(^Arena a)` — Where the arena is now. `bump.used` is already this for a single block; a…
* `void end(Mark m)` — Rewind to `m`: release every block requested since it, and put the cursor…
* `rawptr end_keeping(Mark m, rawptr p, u64 n)` — Rewind to `m`, as `end` does, but **keep** the `n` bytes at `p` — the arena's last…
* `u64 used(Arena a)` — How many bytes are in use, across every block. A test asserting an allocation…
* `u64 blocks(Arena a)` — How many blocks are live. The number a frame loop watches: it should stop…
