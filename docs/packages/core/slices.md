<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:slices

core:slices — ordering, searching and rearranging a run of anything.

    import "core:slices"

    [6]i32 ns = [5, 3, 9, 1, 7, 2]
    slices.sort_ascending(ns[:])
    (i64 at, bool found) = slices.find_sorted(ns[:], 7)

**There is no array library to write beside this one, and that is the whole
shape of the package.** `[N]T` windows to a `[]T` with `v[:]`, and a
`ref []T` *lends* one — so a function taking `[]$T` already serves a fixed
array, a borrowed window and an owned run, with nothing written at the call
beyond the window that was always going to be there. One package, three
container kinds, no conversions.

## What an importer pays, measured

The emitter writes **every concrete function of every live package** whether
it is called or not; its one exemption is an uncalled template. Nearly
everything here binds a `$`, so nearly everything is demand-driven, and the
three numbers below are what that buys. Measured against a 12-line
empty-program baseline:

    import and call nothing        12 lines      the package is pruned whole
    call `contains` only           98 lines      contains, index_of, depth_budget
    call `sort_ascending` only    292 lines      that sort and its five helpers

The first line is the pruner rather than this package: *a package no root can
reach contributes nothing at all*. The interesting one is the third — sorting
brings the sort, and not the comparator sort beside it, not the equality
half, not the predicates, not the searches.

**`depth_budget` is the one concrete function here, and it is the whole floor
an importer pays** — eight lines, present in the second measurement above
even though `contains` has no use for it. It is left concrete rather than
contorted into a template to make the headline rounder: eight lines is a fair
price for a function that reads `i64 -> i64`, and stating the floor is worth
more than hiding it.

The sharpened half of the rule shapes everything else: **a template is free
only when nothing concrete in its own package calls it.** `depth_budget`
calls no template, so nothing is dragged in behind it.

## No `foreign` block, no import, no allocation

This package emits no `#include`, so a program on a target whose OS is
`#NONE` may use every line of it. Nothing here allocates: everything sorts,
searches and rearranges **in place**, or writes into a slice the caller
already owns. A stable sort needs O(n) scratch and is therefore not here —
when it lands it takes an allocator, written last and mandatory, the way
`#format` already settles that question.

It imports nothing, `core:strings` included, and that is worth one sentence
because ordering text is the obvious thing to want. `<` on a `string` is
deliberately undefined — the language declines to have an opinion about
collation — and `core:strings` already owns one. So the answer is the
comparator you were going to pass anyway:

    slices.sort(names, strings.before)

which needs no dependency here and keeps the collation decision in the one
package that has made it.

## Three doors to an ordering, and why not one

A template **can** ask whether `T` is ordered now — `where T has <` — but the
functions below predate it and keep their domains: the category-bound ones
take `[]T where T is scalar`, and widening them to every type `<` applies to (a `char8`,
an enum, a pointer, a `#c` scalar) is a change to what they accept, not a
respelling. A comparator stays the universal mechanism. What is sugar over
it:

  * `sort(xs, less)` — universal, one instantiation per element type;
  * `sort_ascending(xs)` — over `[]T where T is scalar`, whose category carries the six
    comparisons, so the commonest case needs no comparator at all;
  * `slices.sort(xs, strings.before)` — for text, above.

A `less` is a **strict weak ordering**: `less(a, b)` is true exactly when `a`
sorts before `b`, and `less(a, a)` must be false. A comparator that says an
element precedes itself makes the partition below run past the end of the
slice, and nothing here checks it — it is the one thing a caller owes.

## Declarations

57 declarations, 42 public.

* `type Gap: ((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc, i64 start, i64 end, u64 shifted, u64 grown)` — The bookkeeping half of the pair. The block is `[0, #len(block))`; the gap is…
* `bool gap_init(^(ref []$T, Gap) g, i64 cap, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a)` — Take a block of `cap` elements from `a` and set the gap to cover all of it.
* `void gap_free(^(ref []$T, Gap) g)` — Return the block to its allocator. The pair is left empty with its allocator…
* `void gap_clear(^(ref []$T, Gap) g)` — Forget every element and keep the block. Nothing is copied and nothing is…
* `i64 gap_len(^(ref []$T, Gap) g)` — How many elements there are.
* `i64 gap_cap(^(ref []$T, Gap) g)` — How many the block holds before the next insert has to grow it.
* `u64 gap_bytes_moved(^(ref []$T, Gap) g)` — Every byte copied since `gap_init`: gap motion plus growth. The two halves…
* `(T value, bool ok) gap_get(^(ref []$T, Gap) g, i64 i)` — The element at `i`, or a zero `T` and `false` when `i` is out of range.
* `bool gap_set(^(ref []$T, Gap) g, i64 i, T v)` — Overwrite the element at `i`, answering `false` and writing nothing when `i`
* `^T gap_at(^(ref []$T, Gap) g, i64 i)` — A pointer to the element at `i`, for changing it in place, or `nil` when `i`
* `void gap_move(^(ref []$T, Gap) g, i64 at)` — Park the gap in front of logical index `at`, clamped into `0 ..= gap_len(g)`.
* `bool gap_insert(^(ref []$T, Gap) g, i64 at, i64 n)` — Open `n` zeroed elements at logical index `at`, so the first of them is `at`
* `bool gap_remove(^(ref []$T, Gap) g, i64 at, i64 n)` — Close `n` elements starting at logical index `at`. Answers whether they are…
* `bool gap_push(^(ref []$T, Gap) g, T v)` — Put `v` on the end. Answers whether it is there.
* `void swap([]$T xs, i64 i, i64 j)` — Exchange two elements. The right-hand side of a parenthesised place list is…
* `void reverse([]$T xs)` — Reverse in place.
* `void rotate([]$T xs, i64 n)` — Rotate left by `n`, so the element at `n` becomes the first. A negative `n`
* `void fill([]$T xs, T v)` — Set every element to `v`.
* `i64 copy_into([]$T from, []T into)` — Copy `from` into `into`, and answer how many elements moved — the shorter of…
* `i64 index_of([]$T xs, T probe) where #has_eq(T)` — The first index holding `probe`, or -1.
* `i64 last_index_of([]$T xs, T probe) where #has_eq(T)` — The last index holding `probe`, or -1.
* `bool contains([]$T xs, T probe) where #has_eq(T)` — Whether `probe` is anywhere in `xs`.
* `i64 count_of([]$T xs, T probe) where #has_eq(T)` — How many elements equal `probe`.
* `bool equal([]$T a, []T b) where #has_eq(T)` — Whether two runs hold the same elements in the same order. Different lengths…
* `bool starts_with([]$T xs, []T prefix) where #has_eq(T)` — Whether `xs` begins with `prefix`.
* `i64 unique([]$T xs) where #has_eq(T)` — Collapse runs of adjacent equal elements to one, in place, and answer the new…
* `bool all([]$T xs, bool(T) pred)` — Whether every element satisfies `pred`. True for an empty run, which is the…
* `bool any([]$T xs, bool(T) pred)` — Whether any element satisfies `pred`. False for an empty run.
* `bool none([]$T xs, bool(T) pred)` — Whether no element satisfies `pred`.
* `i64 count_if([]$T xs, bool(T) pred)` — How many elements satisfy `pred`.
* `i64 find_if([]$T xs, bool(T) pred)` — The first index satisfying `pred`, or -1.
* `i64 partition([]$T xs, bool(T) pred)` — Move every element satisfying `pred` to the front, in place, and answer how…
* `i64 min_index([]$T xs, bool(T, T) less)` — The index of the smallest element under `less`, or -1 for an empty run. The…
* `i64 max_index([]$T xs, bool(T, T) less)` — The index of the largest element under `less`, or -1. The first such index.
* `bool is_sorted([]$T xs, bool(T, T) less)` — Whether `xs` is ordered under `less`. True for a run of fewer than two.
* `void sort([]$T xs, bool(T, T) less)` — Sort in place under `less`.
* `void sort_ascending([]$T xs) where T is scalar` — Ascending, with no comparator at all.
* `void sort_descending([]$T xs) where T is scalar` — Descending. A separate template rather than a `bool` on the one above,…
* `i64 lower_bound([]$T xs, T probe, bool(T, T) less)` — The first index at which `probe` could be inserted with the order preserved —…
* `i64 upper_bound([]$T xs, T probe, bool(T, T) less)` — The last index at which `probe` could be inserted — i.e. the first element…
* `(i64 at, bool found) binary_search([]$T xs, T probe, bool(T, T) less)` — Find `probe` in a sorted run.
* `(i64 at, bool found) find_sorted([]$T xs, T probe) where T is scalar` — The same, over a run sorted ascending, with no comparator.
