<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:mem

core:mem — the allocator interface, and the one helper an allocator with
nothing clever to do reaches for.

    import "core:mem"
    import "core:mem/libc"

    (rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a = libc.allocator()
    rawptr p = a.proc(a.data, mem.AllocatorMode.Alloc, 64, 16, nil, 0)
    a.proc(a.data, mem.AllocatorMode.Free, 0, 0, p, 64)

An allocator is expressible with what the language already
has and **needs no compiler support to define**. This file and its two
siblings are that claim, cashed: nothing below is privileged, nothing below
asks the compiler for anything, and a user writes an allocator exactly the
way this package does.

What it uses, and all it uses: function values in slots,
`rawptr`, enums, and ordinary functions. `new`, `make`,
`resize`, `delete` and `copy` are the *verbs* the compiler lowers
through this interface; they are not what the interface is made of.

## Declarations

6 declarations, 6 public.

* `enum AllocatorMode: (Alloc, Resize, Free, FreeAll)` — ── The four modes ───────────────────────────────────────────────────────────…
* `trait Allocator` — ── The interface ────────────────────────────────────────────────────────────…
* `const u64 MAX_ALIGN = 16` — The alignment every shipped allocator hands out: worst case, always correct,…
* `u64 align_up(u64 n, u64 align)` — `n` rounded up to a multiple of `align`. `align` must be a power of two,…
* `rawptr resize_by_copy((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a, rawptr old, u64 old_size, u64 nbytes, u64 align)` — ── The fallback `Resize` ────────────────────────────────────────────────────…
* `type Bytes: ^u8` — The block, as the run of bytes it is. One name, used by every byte loop in…
