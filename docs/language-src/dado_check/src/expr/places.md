//@ types/tuples 11
//@ **A name written `[1]T` still windows as a run of one.** By the
//@ collapse a `[1]u32` is a `u32`, so handed where a `[]u32` is wanted
//@ it is one `u32`, and refused (`ERR0504`). But the `[1]` a local or a
//@ parameter was declared with asks for a run of one, and that is all a
//@ window needs: `one[:]` over `[1]u32 one` is a `[]u32` one long over
//@ the binding's own storage, with no heap. It is so for every `T`, so
//@ `[1]string8 s` windows to a `[]string8` holding one string, never to
//@ the string's characters. The run being one long, its bounds are
//@ **constants**, written or named: `one[0..<1]`, or the empty
//@ `one[1..<1]`; a bound computed at run time is refused (`ERR0504`),
//@ its help naming `one[:]`. The window is a view of the frame's
//@ storage, so it does not outlive the frame (`ERR0826`).
//@
//@ ```dado
//@ package doc_sample
//@
//@ private i64 count([]const u32 xs):
//@     return #len(xs)
//@
//@ public i64 counted():
//@     [1]u32 one = [4]
//@     return count(one[:]) + count(one[1..<1])
//@ ```
