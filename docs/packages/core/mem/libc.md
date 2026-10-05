<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:mem/libc

core:mem/libc — C's heap behind the `Allocator` interface.

    import "core:mem"
    import "core:mem/libc"

    mem.Allocator a = libc.allocator()

This is the escape hatch shipped as the **reference implementation** of
the interface, and that is deliberate: a systems language that quietly
disapproves of its own escape hatch is a systems language people work
around. The back door is the front door.

It is what `#default` holds at depth 0 on a hosted target.

## What it does with each mode

    Alloc     malloc, then zero — allocation always zeroes, and `calloc` does
              it in one call but is a fourth C function for one mode
    Resize    realloc, which is the one thing an allocator can do that a
              caller cannot synthesize: it grows a block **in place** when
              the heap has adjacent space
    Free      free
    FreeAll   **unimplemented**, returns `nil`

`FreeAll` returning `nil` is not a stub to be filled in later. C's heap has
no notion of "everything this allocator handed out", so there is nothing to
release; the interface drops `Query_Features` because trying and getting
`nil` answers the question a query would have asked. The contrast with
`core:mem/bump`, whose `FreeAll` is the whole point of the allocator, is
what makes the pair of them exercise the interface rather than agree with
each other.

Using `realloc` is an optimization, **not a dependency**: this could have
been malloc-copy-free, and nothing in the interface requires C's heap to
exist.

## Declarations

12 declarations, 9 public.

* `rawptr malloc(#c.size_t nbytes)`
* `rawptr realloc(rawptr ptr, #c.size_t nbytes)`
* `void free(rawptr ptr)`
* `rawptr malloc(#c.size_t nbytes)`
* `rawptr realloc(rawptr ptr, #c.size_t nbytes)`
* `void free(rawptr ptr)`
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator()` — The allocator. A value, not a singleton: it is two words and copying it…
* `type Heap using mem.Allocator` — C's heap as a declaration that takes `mem.Allocator` on, for code that…
* `(rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) data(self)`
