<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:mem/heap

core:mem/heap — a growable `List`, on the heap, by hand.

    import "core:mem/heap"
    heap.List v = heap.make(8)
    heap.push(&v, 42)
    println(heap.get(v, 0))
    heap.delete(&v)

## DEPRECATED — use `ref []T` and the verbs

**This package is retired by first-class references.** Everything
it does, the five allocation verbs now do in the language:

    heap.List v = heap.make(8)          ->   ref []T v = #make([]T, 8)
    heap.resize(&v, 32)                 ->   #resize(v, 32)
    heap.get(v, i) / heap.set(&v, i, x) ->   v[i]
    heap.delete(&v)                     ->   #delete(v)
    heap.count(v)                       ->   #len(v)

and the verbs are better on three counts rather than one: they are **not
monomorphic** — `T` below is one line you edit, `ref []T` is any `T`; they
allocate from `#default`, so the same code works over an arena or C's
heap; and a reference **carries its allocator**, so nothing has to remember
which one frees what.

It was kept and not deleted because `example/snake`, `blackjack` and `editor`
were built on it and were the **heap regression corpus**: whatever happened
here, they had to keep compiling and keep behaving identically. **That reason
expired**, when `corpus/` replaced `example/` and took importing a
collection with it — the three programs now write the list inline, and
`heap_test.dado` is the only caller left in the tree. Its header states the
position, and whether this package survives the v1.0 library freeze is now an
open question rather than a settled one. It also still
declares three names that are builtins now — `make`, `resize`, `delete` — and
a *qualified* call reaches a public package function perfectly well,
which is exactly the property that keeps them green.

**Do not build anything new on it.** A reader looking for a growable sequence
wants `#make([]T, n)`.

## What it was for

The language it was written for had no allocator and no slices, so this builds the one
container a program reaches for most — a resizable sequence — out of the only
materials the language ships: three functions bound from C's `<stdlib.h>`, the
`size` builtin, and the rule that `p^[i]` is "dereference, then index". There
is **no pointer arithmetic** anywhere — Dado has none at all — and there is no
helper `.c`. `example/snake` discovered the mechanism inline; this is that
mechanism, named and reused — and `corpus/snake` writes it inline again, which
is where to read it as a program rather than as a package.

**It is monomorphic, and honestly so.** A `List` holds one element type, `T`,
declared on one line below — because a shape cannot yet be passed as a type
parameter (generics bind an *arity* or a *width* off a value, not an arbitrary
element type; `shapeof` needs a place and cannot nest under `^` or `[]`). To
hold a different element, copy this file and change `T`. **First-class
references landed**, and `ref []T` is the supplanting the paragraph
above predicted — the API shape did move over, which is why the mapping at the
top of this file is a rename rather than a rewrite.

**It is unmanaged**, and this is the paragraph the verbs answer. `make`
allocates, `delete` frees, and nothing does it for
you: a `List` you `make` and never `delete` leaks, and a `List` used after
`delete` reads freed memory. That is the raw-pointer bargain stated
as an API. In return, `get`/`set`/`push` are ordinary indexed access with no
bounds cost beyond the assertions below, and the whole thing erases to a
struct of three machine words.

## Declarations

24 declarations, 22 public.

* `rawptr malloc(#c.size_t nbytes)`
* `rawptr realloc(rawptr ptr, #c.size_t nbytes)`
* `void free(rawptr ptr)`
* `type T: i32` — ── The element, and the one place it is named ───────────────────────────────…
* `const i32 CAP_MAX = 65536` — `CAP_MAX` is the ceiling the backing pointer is *typed* over — `^[CAP_MAX]T` —…
* `type List: (Ptr data, i32 len, i32 cap)` — A `List` is three words: where the block is, how many elements are live, and…
* `List make(i32 cap0)` — Allocate a `List` with room for `cap0` elements (at least one). The block is…
* `bool ok(List v)` — Whether the last allocation succeeded — `malloc`/`realloc` returned non-null.
* `void delete(^List v)` — Return the block to the allocator and leave the `List` empty. Idempotent only…
* `i32 count(List v)` — How many elements are live. Not spelled `len`, because `len` is a global…
* `i32 capacity(List v)`
* `bool is_empty(List v)`
* `u64 bytes(List v)` — Bytes the backing block occupies right now — `cap` elements of `T`. Handy for…
* `void clear(^List v)` — Drop every element without freeing the block, so the capacity is kept for…
* `bool resize(^List v, i32 ncap)` — Grow or shrink the backing block to hold exactly `ncap` elements, never below…
* `T get(List v, i32 i)` — The element at `i`. Asserted in range — an out-of-bounds read is a bug, and an…
* `void set(^List v, i32 i, T x)` — Overwrite the element at `i`. Same bounds contract as `get`.
* `T peek(List v)` — The last element, without removing it. Precondition: not empty.
* `bool push(^List v, T x)` — Append `x`, doubling the block with `realloc` when it is full. Returns whether…
* `T pop(^List v)` — Remove and return the last element. Precondition: not empty. The block is not…
* `bool insert(^List v, i32 i, T x)` — Insert `x` at `i`, shifting the tail one to the right; `i` may equal `count`,…
* `T remove(^List v, i32 i)` — Remove and return the element at `i`, shifting the tail one to the left.
