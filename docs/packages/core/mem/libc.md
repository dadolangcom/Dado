<!-- dadoc 1.0.0-rc.4 -->
<!-- commit f5f6761926d6 (dirty) -->
# core:mem/libc

core:mem/libc — C's heap behind the `Allocator` interface.

    import "core:mem"
    import "core:mem/libc"

    (mem.Proc proc, rawptr data) a = libc.allocator()
    using a:
        ref []i32 xs = #make([]i32, 100)
        #delete(xs)

`allocator()` answers the two words an allocator is, as every allocator in
`core:mem` does; `mem.Allocator` is the trait a declaration takes on, not a
type a value is held in, and `libc.Heap` below is its implementer here.

**Tier W H.** Hosted (Linux, macOS and Windows alike), C's `<stdlib.h>`; on the web, the runtime's heap
over the module's linear memory, declared in `dado/web.h`. **Not `#NONE`**:
a freestanding target has no heap, `#default` there is the nil allocator,
and the `#static_assert` below refuses a `#NONE` build that imports this
package, naming the reason. Use `core:mem/bump` or `core:mem/arena` over
a block the program owns instead.

**What raises into a script: nothing.** Every function here hands out or
takes a `rawptr`, and `dispatch` calls C's heap, so none can be promised.

This is the escape hatch shipped as the **reference implementation** of
the interface, and that is deliberate: a systems language that quietly
disapproves of its own escape hatch is a systems language people work
around. The back door is the front door.

It is what `#default` holds at depth 0 on a hosted target, so a program
that wants C's heap rather than whatever it was called under writes `#heap`,
and imports this package only to hold the allocator as a value.

## What it does with each mode

    Alloc     malloc, then zero — allocation always zeroes, and `calloc` does
              it in one call but is a fourth C function for one mode. A
              zero size asks for one byte (`core:mem`'s zero-size rule)
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

13 declarations, 3 public.

* `(mem.Proc proc, rawptr data) allocator()` — The allocator. A value, not a singleton: it is two words and copying it…
* `type Heap using mem.Allocator` — C's heap as a declaration that takes `mem.Allocator` on, for code that…
* `(mem.Proc proc, rawptr data) data(self)` — The two words `allocator()` answers.
