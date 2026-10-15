<!-- dadoc 1.0.0-rc.4 -->
<!-- commit 9fddd51732b8 -->
# core:slices

core:slices — ordering, searching and rearranging a run of anything.

**Tier N W H. Script class S.** Pure Dado with no import: it builds and
runs freestanding, on the web and hosted, and answers the same on each.

**What raises into a script.** Every function in this file, at every
element type a script has — promised in `raises_test.dado`, not above each
function, because a promise written on a template binds every
instantiation a program makes, and a run of `rawptr` is one this package
serves and a script cannot. `message` is promised where it stands. The gap
buffer (`gap.dado`) keeps an allocator, which is two addresses, so it is
not promised.

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

The emitter writes a function only when something a program runs reaches
it, and a template only at the instantiations asked for. Nearly everything
here binds a `$`, so nearly everything is demand-driven, and the three
numbers below are what that buys. Measured against a 17-line empty-program
baseline:

    import and call nothing        17 lines      the package is pruned whole
    call `contains` only          111 lines      contains and index_of
    call `sort_ascending` only    539 lines      that sort and its helpers

The first line is the pruner rather than this package: *a package no root can
reach contributes nothing at all*. The interesting one is the third — sorting
brings the sort, and not the comparator sort beside it, not the equality
half, not the predicates, not the searches.

**`depth_budget` and `message` are the concrete functions here**, and a
program pays for neither unless it sorts or asks for a message: a concrete
function nothing reaches is dropped as an uncalled template is. Neither
calls a template, so nothing is dragged in behind them.

## No `foreign` block, no import, and allocation only when asked

This package emits no `#include`, so a program on a target whose OS is
`#NONE` may use every line of it. Everything sorts, searches and
rearranges **in place**, or writes into a slice the caller already owns —
the stable sort included, which merges by rotation rather than through a
buffer. Only `clone` allocates, because a copy is what it is for, and it
takes the ambient allocator: a caller who wants an arena writes
`using a:` around the call. The gap buffer in `gap.dado` grows on the
allocator it was opened with.

It imports nothing, `core:strings` included, and that is worth one sentence
because ordering text is the obvious thing to want. `<` on a `string8` is
deliberately undefined — the language declines to have an opinion about
collation — and `core:strings` already owns one. So the answer is the
comparator you were going to pass anyway:

    slices.sort(names, strings.before)

which needs no dependency here and keeps the collation decision in the one
package that has made it.

## Three doors to an ordering, and why not one

A template asks whether `T` is ordered with `where T has <`, and the
comparator-free functions take exactly that domain: every type `<`
applies to — the numbers, a `char8`, an enum, a pointer. (They took only
`where T is scalar` once; the wider domain admits every call the narrow
one did.) A comparator stays the universal mechanism. What is sugar over
it:

  * `sort(xs, less)` — universal, one instantiation per element type;
  * `sort_ascending(xs)`, `min(xs)`, `max(xs)`, `find_sorted(xs, v)` —
    over `[]T where T has <`, so the commonest case needs no comparator
    at all;
  * `slices.sort(xs, strings.before)` — for text, above.

A `less` is a **strict weak ordering**: `less(a, b)` is true exactly when `a`
sorts before `b`, and `less(a, a)` must be false. A comparator that says an
element precedes itself makes the partition below run past the end of the
slice, and nothing here checks it — it is the one thing a caller owes.

## Declarations

74 declarations, 57 public.

* `type Gap($T)` — A gap buffer of `T`. The block is `[0, #len(block))`; the gap is…
* `Error!void init(^self, i64 cap)` — Keep `#default` as the gap's allocator and take a block of `cap`
* `void destroy(^self)` — Return the block to its allocator. The gap is left empty with its…
* `void clear(^self)` — Forget every element and keep the block. Nothing is copied and nothing…
* `i64 len(^self)` — How many elements there are.
* `i64 cap(^self)` — How many the block holds before the next insert has to grow it.
* `u64 bytes_moved(^self)` — Every byte copied since `init`: gap motion plus growth. The two halves…
* `Error!T get(^self, i64 i)` — The element at `i`. Fails `OUT_OF_RANGE` when `i` is not in…
* `Error!void set(^self, i64 i, T v)` — Overwrite the element at `i`. Fails `OUT_OF_RANGE`, writing nothing,…
* `^T at(^self, i64 i)` — A pointer to the element at `i`, for changing it in place, or `nil`
* `void move(^self, i64 to)` — Park the gap in front of logical index `to`, clamped into…
* `Error!void insert(^self, i64 at, i64 n)` — Open `n` zeroed elements at logical index `at`, so the first of them is…
* `Error!void remove(^self, i64 at, i64 n)` — Close `n` elements starting at logical index `at`.
* `Error!void push(^self, T v)` — Put `v` on the end. Fails as `insert` does.
* `distinct enum i32 Error` — Why a call failed. The numbers are `core:slices`' own range, 2300–2399.
* `string8 message(Error why)` — A sentence for `why`, for a person reading a tool's output.
* `void swap([]$T xs, i64 i, i64 j)` — Exchange two elements. The right-hand side of a parenthesised place list is…
* `void reverse([]$T xs)` — Reverse in place.
* `void rotate([]$T xs, i64 n)` — Rotate left by `n`, so the element at `n` becomes the first. A negative `n`
* `void fill([]$T xs, T v)` — Set every element to `v`.
* `i64 copy_into([]const $T from, []T into)` — Copy `from` into `into`, and answer how many elements moved — the shorter of…
* `ref []T clone([]const $T xs)` — A copy of `xs` in a new run on the ambient allocator (a `using` block…
* `i64 chunk_count([]const $T xs, i64 size)` — How many pieces `chunk` cuts `xs` into at `size` elements each, the last…
* `[]T chunk([]$T xs, i64 size, i64 i)` — The `i`th piece of `size` elements: `xs[i·size ..< (i+1)·size]`, cut…
* `i64 window_count([]const $T xs, i64 size)` — How many windows of `size` elements `window` slides over `xs`:
* `[]T window([]$T xs, i64 size, i64 i)` — The window of `size` elements starting at `i`: `xs[i ..< i + size]`.
* `i64 index_of([]const $T xs, T probe) where #has_eq(T)` — The first index holding `probe`, or -1.
* `i64 last_index_of([]const $T xs, T probe) where #has_eq(T)` — The last index holding `probe`, or -1.
* `bool contains([]const $T xs, T probe) where #has_eq(T)` — Whether `probe` is anywhere in `xs`.
* `i64 count_of([]const $T xs, T probe) where #has_eq(T)` — How many elements equal `probe`.
* `bool equal([]const $T a, []const T b) where #has_eq(T)` — Whether two runs hold the same elements in the same order. Different lengths…
* `bool starts_with([]const $T xs, []const T prefix) where #has_eq(T)` — Whether `xs` begins with `prefix`.
* `bool ends_with([]const $T xs, []const T suffix) where #has_eq(T)` — Whether `xs` ends with `suffix`.
* `i64 index_of_run([]const $T xs, []const T needle) where #has_eq(T)` — The first index at which `needle` occurs as a run inside `xs`, or -1. An…
* `i64 last_index_of_run([]const $T xs, []const T needle) where #has_eq(T)` — The last index at which `needle` occurs as a run inside `xs`, or -1. An…
* `bool contains_run([]const $T xs, []const T needle) where #has_eq(T)` — Whether `needle` occurs as a run anywhere inside `xs`.
* `i64 unique([]$T xs) where #has_eq(T)` — Collapse runs of adjacent equal elements to one, in place, and answer the new…
* `bool all([]const $T xs, bool(T) pred)` — Whether every element satisfies `pred`. True for an empty run, which is the…
* `bool any([]const $T xs, bool(T) pred)` — Whether any element satisfies `pred`. False for an empty run.
* `bool none([]const $T xs, bool(T) pred)` — Whether no element satisfies `pred`.
* `i64 count_if([]const $T xs, bool(T) pred)` — How many elements satisfy `pred`.
* `i64 find_if([]const $T xs, bool(T) pred)` — The first index satisfying `pred`, or -1.
* `i64 partition([]$T xs, bool(T) pred)` — Move every element satisfying `pred` to the front, in place, and answer how…
* `i64 min_index([]const $T xs, bool(T, T) less)` — The index of the smallest element under `less`, or -1 for an empty run. The…
* `i64 max_index([]const $T xs, bool(T, T) less)` — The index of the largest element under `less`, or -1. The first such index.
* `bool is_sorted([]const $T xs, bool(T, T) less)` — Whether `xs` is ordered under `less`. True for a run of fewer than two.
* `Error!T min([]const $T xs) where T has <` — The smallest element; the first of several equal smallest, under `<`.
* `Error!T max([]const $T xs) where T has <` — The largest element; the first of several equal largest, under `<`.
* `bool is_sorted_ascending([]const $T xs) where T has <` — Whether `xs` is in ascending order under `<`: the run `sort_ascending`
* `void sort([]$T xs, bool(T, T) less)` — Sort in place under `less`.
* `void sort_ascending([]$T xs) where T has <` — Ascending, with no comparator at all.
* `void sort_descending([]$T xs) where T has <` — Descending. A separate template rather than a `bool` on the one above,…
* `void sort_stable([]$T xs, bool(T, T) less)` — Sort in place under `less`, keeping elements that compare equal in the…
* `i64 lower_bound([]const $T xs, T probe, bool(T, T) less)` — The first index at which `probe` could be inserted with the order preserved —…
* `i64 upper_bound([]const $T xs, T probe, bool(T, T) less)` — The last index at which `probe` could be inserted — i.e. the first element…
* `(i64 at, bool found) binary_search([]const $T xs, T probe, bool(T, T) less)` — Find `probe` in a sorted run.
* `(i64 at, bool found) find_sorted([]const $T xs, T probe) where T has <` — The same, over a run sorted ascending, with no comparator.
