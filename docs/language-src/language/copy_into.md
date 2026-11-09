//@ types/views 32
//@ ### Copying into a run that already exists
//@
//@ Two forms copy elements into storage that is already there; neither
//@ allocates. **A slice assignment copies an exact window:**
//@ `dst[i..<j] = src` writes every element of the window from `src`, and
//@ `dst[:] = src` the whole run. The lengths are the same number: when
//@ both are known before the program runs a mismatch is `ERR0617`, and
//@ otherwise it is a trap that names both lengths, on every target.
//@ **`#copy_into(dst, src)` copies as many as both hold** — the shorter
//@ of the two runs, from the start of each — and answers the count as an
//@ `i64`, as Go's `copy` does; a count both lengths fix is a constant.
//@ `core:slices` has no `copy_into`: it took `(from, into)`, the
//@ reverse of this order, so `slices.copy_into(a, b)` is now
//@ `#copy_into(b, a)`.
//@
//@ Either side is a fixed array, a window or a `ref []T`; the destination
//@ is written through, so it is a place and not `[]const T` (`ERR0606`).
//@ The elements are moved as they are, by memory, so the source's element
//@ type is the destination's (`ERR0618`): a copy converts nothing, and a
//@ conversion is a loop you write. Text and maps are refused (`ERR0619`):
//@ a string is immutable and shares its bytes, and a map's entries have
//@ no positions. **The two runs may overlap**, as `memmove`'s may, so
//@ `xs[1..<n] = xs[0..<n - 1]` shifts a run up by one.
//@
//@ A copy writes into a block and never moves it, so a view of the
//@ destination taken before the copy still points where it did. A view
//@ that was stale before the copy is as stale as a source or a
//@ destination as anywhere else (`WRN0803`). When the source is a call
//@ that may grow the destination's block, the window is taken after the
//@ call, so the copy lands where the block then is.
//@
//@ ```dado
//@ package doc_sample
//@
//@ // Insert `v` at `at`, shifting the tail up by one; the last element falls off.
//@ public void insert_at([]i32 xs, i64 at, i32 v):
//@     xs[at + 1..<#len(xs)] = xs[at..<#len(xs) - 1]
//@     xs[at] = v
//@
//@ public i64 head([]i32 into, []const i32 from):
//@     return #copy_into(into, from)
//@ ```
//@
//@ DadoScript has neither form: a script's `xs[i..<j]` answers a new
//@ array rather than a window into storage. The Rust `dadoc` predates both
//@ forms and refuses them (`ERR0606`, `ERR0405`).
