<!-- dadoc 1.0.0-rc.4 -->
<!-- commit f5f6761926d6 (dirty) -->
# core:mem/gc

core:mem/gc — a conservative mark-sweep collector behind `mem.Allocator`.

    import "core:mem/gc"

    gc.Heap heap                              // about 115 KB: not a local

    i32 main():
        gc.init(&heap)
        using gc.allocator(&heap):
            ref []i32 xs = #make([]i32, 100)  // every verb allocates here
            xs[0] = 1
        _ = gc.collect(&heap)
        return 0

**Outside the 1.0 promise.** This package is a prototype and is versioned
with nothing: its API may change or the package go in any release, and the
standard library's version promise does not cover it.

**PROTOTYPE.** It collects, and it is correct for what it scans. What it does
not scan is below under *Roots*, and that list is a correctness hole, not a
performance note. Do not ship a program on this.

Conservative means: any stack word that looks like an address into a live
block keeps that block alive. False positives retain garbage; there are no
false negatives for anything the root set covers.

**Hosted only: Linux, macOS and Windows**, each answering where its thread's
stack ends in its own call (`stack_base` below). Not `#NONE`, which has no
`malloc`, and not the web, where a WebAssembly module cannot read its own
stack: the `#static_assert` below refuses both, naming the reason.

**What raises into a script: nothing.** A collector is made of addresses,
and every function here takes a `^Heap` or a `rawptr` or reaches `malloc`.

**Thread-safety.** One `Heap` belongs to one thread, and only that thread's
stack is scanned (see *Roots* below).

## Declarations

50 declarations, 12 public.

* `type Visit: void(rawptr, rawptr)` — What the walk hands back: the heap, and one candidate address.
* `const i32 MAX_BLOCKS = 4096` — How many blocks one `Heap` holds at once. A full table collects, and an…
* `type Block: (rawptr at, u64 size, bool marked)` — One block the collector handed out: where, how large, and whether the…
* `type Heap` — A collected heap: the block table, the marking worklist, the span every…
* `void init(^Heap h)` — Everything between the current frame and the top of this thread's stack is…
* `(mem.Proc proc, rawptr data) allocator(^Heap h)` — `h` as an allocator: the two words `using` and the verbs take.
* `type HeapAllocator using mem.Allocator` — The allocator for a collected heap, as a declaration that takes `mem.Allocator` on:
* `(mem.Proc proc, rawptr data) data(self)` — The two words `allocator(self.heap)` answers.
* `i32 live(^const Heap h)` — How many blocks `h` holds now, reachable or not yet collected.
* `i64 collections(^const Heap h)` — How many collections `h` has run, by hand or when its table filled.
* `i64 freed(^const Heap h)` — How many blocks `h`'s collections have freed, in all.
* `i64 collect(^Heap h)` — Mark from the roots, free what is unreachable, answer how many blocks went.
