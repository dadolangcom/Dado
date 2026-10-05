<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:mem/gc

core:mem/gc — a conservative mark-sweep collector behind `mem.Allocator`.

    gc.Heap heap
    gc.init(&heap)
    using gc.allocator(&heap):
        run()                                 // every verb allocates here

**PROTOTYPE.** It collects, and it is correct for what it scans. What it does
not scan is below under *Roots*, and that list is a correctness hole, not a
performance note. Do not ship a program on this.

Conservative means: any stack word that looks like an address into a live
block keeps that block alive. False positives retain garbage; there are no
false negatives for anything the root set covers.

## Declarations

44 declarations, 25 public.

* `rawptr malloc(#c.size_t nbytes)`
* `void free(rawptr p)`
* `void __builtin_unwind_init()`
* `rawptr __builtin_frame_address(i32 level)`
* `type Visit: void(rawptr, rawptr)` — What the walk hands back: the heap, and one candidate address.
* `type pthread_t`
* `pthread_t pthread_self()`
* `rawptr pthread_get_stackaddr_np(pthread_t thread)`
* `type pthread_attr_t`
* `type pthread_t`
* `pthread_t pthread_self()`
* `i32 pthread_getattr_np(pthread_t thread, ^pthread_attr_t attr)`
* `i32 pthread_attr_getstack(^pthread_attr_t attr, ^rawptr addr, ^#c.size_t size)`
* `i32 pthread_attr_destroy(^pthread_attr_t attr)`
* `const i32 MAX_BLOCKS = 4096`
* `type Block: (rawptr at, u64 size, bool marked)`
* `type Heap`
* `void init(^Heap h)` — Everything between the current frame and the top of this thread's stack is…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator(^Heap h)`
* `type HeapAllocator using mem.Allocator` — The allocator for a collected heap, as a declaration that takes `mem.Allocator` on:
* `(rawptr(rawptr, mem.AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) data(self)`
* `i32 live(^const Heap h)`
* `i64 collections(^const Heap h)`
* `i64 freed(^const Heap h)`
* `i64 collect(^Heap h)` — Mark from the roots, free what is unreachable, answer how many blocks went.
