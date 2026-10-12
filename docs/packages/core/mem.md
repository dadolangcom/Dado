<!-- dadoc 1.0.0-rc.4 -->
<!-- commit f5f6761926d6 (dirty) -->
# core:mem

core:mem — the allocator interface, the rules every allocator keeps, and the
byte helpers an allocator written in Dado is made of.

    import "core:mem"
    import "core:mem/libc"

    (mem.Proc proc, rawptr data) a = libc.allocator()
    rawptr p = a.proc(a.data, mem.AllocatorMode.Alloc, 64, 16, nil, 0)
    a.proc(a.data, mem.AllocatorMode.Free, 0, 0, p, 64)

An allocator is expressible with what the language already
has and **needs no compiler support to define**. This package and its
siblings (`mem/libc`, `mem/bump`, `mem/arena`, `mem/slab`, `mem/gc`) are that
claim, cashed: nothing below is privileged, nothing below asks the compiler
for anything, and a user writes an allocator exactly the way these do.

What it uses, and all it uses: function values in slots,
`rawptr`, enums, and ordinary functions. `#new`, `#make`,
`#resize`, `#delete` and `#copy` are the *verbs* the compiler lowers
through this interface; they are not what the interface is made of.

## Allocation is `using`, not a parameter

**A library function does not take an allocator.** It allocates on
`#default`, and its doc comment says who ends what it answers ("the caller
`#delete`s it", "end it with `destroy`"). A caller who wants the memory from
an arena, a slab or a scratch frame says so once, around the call:

    using arena.allocator(&scratch):
        ref []char8 line = strings.concat(a, b)

An allocator parameter is the exception, and its doc comment says why: a
value that captures an allocator for its whole life and outlives the `using`
it was made in, a process-lifetime answer deliberately on `#heap`, or an
allocator package itself (these, whose subject *is* the allocator). Where an
allocator must be spelled, a signature takes `Allocator` (the trait, as a
binder) where only implementers are passed, else the two words
`(mem.Proc proc, rawptr data)`, which a rung such as `#default` also is.

## What every allocator answers

* **`nil` is failure**, and the only failure: the interface has no codes.
* **Allocation is zeroed**, `Alloc` whole and `Resize` from the old size
  up, because `i32 a` is zero and a `T` from an allocator that was not
  would make that rule depend on where the storage came from.
* **A zero-size allocation answers a distinct, non-nil pointer that may not
  be dereferenced**: distinct from every other live block, so a zero-length
  run is a real handle, and non-nil, so it does not read as failure. Freeing
  it with `old_size` 0 is valid. C's `malloc(0)` may answer NULL, so an
  allocator over C asks for one byte, as the compiler's own root heap does
  (`calloc(1, size ? size : 1)`).
* **The alignment asked for, up to `MAX_ALIGN`.** Above it an allocator may
  refuse (`nil`) rather than under-align.

**Every target**: freestanding (`#NONE`), the web, and hosted (Linux, macOS
and Windows) alike. Nothing here reaches a C library.

**What raises into a script.** `align_up`, promised by the `#assert_raises`
above it: it is arithmetic. The interface, `zero`, `copy` and
`resize_by_copy` are made of `rawptr`s, and a script has no addresses.

**Thread-safety.** No package state. Whether one allocator may be called
from two threads is that allocator's to say.

## Declarations

9 declarations, 9 public.

* `enum AllocatorMode: (Alloc, Resize, Free, FreeAll)` — What an allocator is asked to do: the second argument of every call.
* `type Proc: rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64)` — The allocator procedure: the first of the two words an allocator is. Its…
* `trait Allocator` — **An allocator, as a trait whose one required method answers its two…
* `const u64 MAX_ALIGN = 16` — The alignment every shipped allocator hands out: worst case, always correct,…
* `u64 align_up(u64 n, u64 align)` — `n` rounded up to a multiple of `align`. `align` must be a power of two,…
* `void zero(rawptr p, u64 n)` — Zero the `n` bytes at `p`: how an allocator keeps "allocation is zeroed"…
* `void copy(rawptr dst, rawptr src, u64 n)` — Copy `n` bytes from `src` to `dst`, front to back: correct when the two do…
* `type Bytes: ^u8` — The block, as the run of bytes it is: a `^u8` is what a `rawptr` converts to…
* `rawptr resize_by_copy((Proc proc, rawptr data) a, rawptr old, u64 old_size, u64 nbytes, u64 align)` — `Resize` by allocating, copying and freeing, through `a`: what an allocator…
