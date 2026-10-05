<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:mem/slab

core:mem/slab — size classes over large chunks, behind the allocator interface.

    import "core:mem"
    import "core:mem/libc"
    import "core:mem/slab"

    slab.Slab lines = slab.over(libc.allocator())
    defer slab.destroy(&lines)
    mem.Allocator a = slab.allocator(&lines)
    ref []char8 run = #make([]char8, n, a)      // `#resize`, `#delete` as ever

The allocator for **many small blocks that are freed one at a time**, which is
the shape neither of the two cursor allocators in this collection can hold:
`core:mem/bump` and `core:mem/arena` never reclaim one block, and C's heap
reclaims every one but pays a call, a lock and a header for each. An editor
holding a file as one owned run per line is the case it is written for: a
load is one allocation per line, and every edit frees one line's run and
allocates its replacement.

## How it is built

A request of at most `MAX_CLASS` bytes is rounded up to a **size class**, and
each class keeps an intrusive free list threaded through its freed blocks. A
block comes off that list if it can, and otherwise is carved off the newest
**chunk** — a large block requested from the backing allocator, whose first
`CHUNK_HEADER` bytes link it to the one before. Chunks are shared by every
class: a chunk is a bump cursor, not a per-class page, so a slab that only ever
holds lines of 0..200 bytes asks for no chunk on behalf of a class it does not
use. A request above `MAX_CLASS` is a **large block**, passed straight to the
backing allocator behind a `LARGE_HEADER` that links it into a list, so
`reset` and `destroy` can find it.

## What it does with each mode

    Alloc     pop the class's free list, else carve from the newest chunk,
              else request a chunk; above `MAX_CLASS`, a large block
    Resize    **in place** when the new size rounds to the same class; a
              large block is handed to the backing allocator's own `Resize`,
              which may grow it in place; otherwise allocate, copy, free
    Free      push the block onto its class's free list — O(1), no call
    FreeAll   `reset`: release every large block and every chunk **but the
              oldest**, and empty every free list. `core:mem/arena`'s
              reasoning, for the same frame-loop reason

## The size is the caller's, and there is no per-block header

A small block carries **no header**: its class is recovered from `old_size`,
which the interface passes on every `Resize` and `Free`. The verbs pass the
size last requested — `#delete` and `#resize` send `cap * #size(T)`, and `cap`
is the element count the run was made or last resized to (read in the emitted
C for `#make`/`#resize`/`#delete` over this collection's `libc.allocator()`).
So the per-block overhead of a small block is its class rounding and nothing
else. **A caller that frees with a size it did not allocate with corrupts a
free list**, and nothing detects it — the same bargain `mem.resize_by_copy`
already makes when it copies `old_size` bytes on the caller's word.

The alternative was a per-chunk class table found from the address, which
survives a lying caller; it is refused because it costs a lookup on every
`Free` to defend against a caller the interface already trusts everywhere
else.

## Zero bytes

`Alloc` of 0 answers a **distinct, non-nil** block of the smallest class — the
compiler's own root heap answers `calloc(1, size ? size : 1)`, and
`core:mem/libc` clamps a zero `Resize` to one byte, so a zero-length run holds a
real handle that `#delete` hands back. Freeing it with `old_size` 0 finds the
same class, so the pair round-trips.

## Refused

  * `align` above `mem.MAX_ALIGN`, answered `nil` before anything is
    requested — chunks and large blocks are asked for at `MAX_ALIGN` and every
    class is a multiple of 16, so that is exactly the alignment this can
    vouch for, and no more;
  * a backing allocator that hands back a block not aligned to `MAX_ALIGN` —
    the block is given back and the request fails, `core:mem/arena`'s rule;
  * a large request whose size plus its header would wrap a `u64`.

## Trusted

  * `old_size`, as above;
  * the backing allocator's zeroing. A chunk and a large block are
    allocations, which the interface zeroes, so a block carved fresh from a
    chunk is handed out without a second pass — see `take` for what that
    pass cost. A backing that does not zero hands out blocks that are not
    zero, and a block reused off a free list is zeroed here either way.

## What it costs

Measured with a throwaway program over `mem.Allocator` values: 100 000
allocations of 0..200 bytes each written through, freed in shuffled order,
then 100 000 edits (free one, allocate its replacement), 30 rounds, median
of five runs, `./dado build` -O2 on a two-core VM. Nanoseconds per operation,
gcc 13 / clang 18:

                            load       shuffled free   edit pair
    malloc, first round     65 / 60    69 / 56         120 / 65
    malloc, later rounds    28 / 28    59 / 75         122 / 68
    slab, first round       42 / 59    23 / 23          53 / 58
    slab kept, later rounds 33 / 34    28 / 23          60 / 54

A free is a push, and costs between a quarter and a half of `free`'s. An
edit is half of `malloc`'s under gcc and level with it under clang. A load
is no faster: the first is first touch of twelve megabytes either way, and a
slab kept across loads no longer pays those page faults but hands its blocks
out in the order they were freed, which here is shuffled — glibc coalesces a
shuffled free and hands the next load out in address order. What the slab
changes is the backing: 15 calls for the load where `malloc` is called
100 000 times, and **none** in 29 further rounds on the kept slab.

**Not thread-safe.** A `Slab` is a plain value with no lock; two threads
allocating through one race on its free lists. One slab per thread, or a lock
the caller owns.

## Declarations

50 declarations, 25 public.

* `const u64 MAX_CLASS = 4096` — The largest request served from a size class. Above it a request is a large…
* `const u64 CLASSES = 16` — How many size classes there are. The classes are 16, 32, and then every power…
* `const u64 CHUNK = 65536` — The first chunk's size, and where every doubling starts. Later chunks double…
* `const u64 CHUNK_MAX = 1048576` — The ceiling the doubling stops at, and so the most a slab holds uncarved.
* `const u64 CHUNK_HEADER = 16` — Bytes at the front of every chunk: the link to the older chunk and the…
* `const u64 LARGE_HEADER = 32` — Bytes in front of every large block: two links, the backing size and the…
* `type Chunk` — A chunk's own first bytes.
* `type Link` — A freed small block's own first bytes. Only `next` is read; the second word…
* `type Large` — A large block's header, in front of the payload the caller holds.
* `type Slab` — The slab. The backing allocator is captured **by value**, as…
* `Slab over((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) backing)` — A slab over `backing`. **It does not allocate**: the first chunk is requested…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator(^Slab s)` — The allocator for `s`. The slab is the allocator's `data`, so it is borrowed…
* `type SlabAllocator using mem.Allocator` — The allocator for a slab, as a declaration that takes `mem.Allocator` on:
* `(rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) data(self)`
* `void reset(^Slab s)` — Release every block, **keeping the oldest chunk** — the one `FreeAll` does.
* `void destroy(^Slab s)` — Hand every chunk and every large block back. The slab is exactly what `over`
* `u64 reserved(^Slab s)` — Bytes held from the backing allocator right now: every chunk, and every large…
* `u64 in_use(^Slab s)` — Bytes of live blocks as this slab counts them: a small block is its whole…
* `u64 requested(^Slab s)` — Bytes callers asked for across every live block. `in_use / requested` is the…
* `u64 chunks(^Slab s)` — How many chunks are held.
* `u64 large_blocks(^Slab s)` — How many large blocks are live.
* `u64 backing_calls(^Slab s)` — How many calls this slab has made of its backing allocator — `Alloc`,…
* `u64 live(^Slab s, u64 class)` — How many blocks of class `class` are live. 0 for a class out of range.
* `u64 class_size(u64 class)` — The block size of class `class`, 0 out of range. `class_size(CLASSES - 1)`
* `u64 class_of(u64 nbytes)` — The class a request of `nbytes` is served from, or `CLASSES` for a large…
