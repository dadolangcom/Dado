//@ flow/when 40
//@ ### `when` selects, and the arms it does not select barely exist
//@
//@ `when` is the compile-time branch. Exactly one arm is selected; the
//@ others are lexed and parsed and **nothing else** — not resolved, not
//@ checked, not emitted, and the names they declare do not exist. That is
//@ what lets an arm name a header, a symbol or a type that is absent on this
//@ target: the arm for another platform is never asked to make sense here.
//@
//@ Selection happens before checking, and the condition is folded in an
//@ environment fixed before any arm is taken: compiler definitions,
//@ literals, and package-level constants declared outside every `when`. A
//@ condition may not name something a `when` declares, because selection
//@ decides which names exist and a condition that depended on that would
//@ have no answer to start from.
//@
//@ A bare name in a condition is its own package's constant when the
//@ package declares one, as it is on any other line, and otherwise the one
//@ package that does; a name two other packages declare is refused
//@ (`ERR0903`) until it is qualified with an import's binding,
//@ `lib.ROLE`. A build reaches a `@const` the same way: `-D NAME=value`
//@ names the one declaration of `NAME`, and where two packages each declare
//@ one, `-D package.NAME=value` names one of them and leaves the other at
//@ its default. So two libraries may each guard test-only code with a
//@ `@const bool FAULT_HOOKS`, and `-D libtess2.FAULT_HOOKS=true` compiles in
//@ one library's hooks and not the other's.
//@
//@ Only a name known before the program runs may be branched on — `#OS`,
//@ `#ARCH`, `#TRIPLE` and their members. The `#` sigil splits by case and
//@ the split is one-way: the half the compiler owns promises nothing at
//@ compile time, so there is no spelling of it a `when` can select on.
//@
//@ **The admitted cost is that an untaken arm rots.** It parses long after
//@ it has stopped meaning anything, and nothing reports that until somebody
//@ builds for the target it belongs to. Covering the combinations with real
//@ programs is the only thing that finds it.
//@
//@ A `switch` is not an alternative: every arm of a `switch` is checked and
//@ emitted, including the arms a given build can never take.
//@
//@ **A build invariant is written `#static_assert(COND, "message")`**, at
//@ package level. Its condition is folded exactly where and as a `when`
//@ condition is — so it may name `#` definitions and package-level constants
//@ declared outside every `when`, and a `#size` or a function call is
//@ refused with the `when` rule's own reason — and a build in which it folds
//@ to `false` is refused with the message. The refusal says where each
//@ operand's value came from, because the usual culprit is a `-D`:
//@
//@ ```dado
//@ package doc_sample
//@
//@ @const i32 RING_CAPACITY = 64
//@
//@ #static_assert(RING_CAPACITY & (RING_CAPACITY - 1) == 0, "RING_CAPACITY must be a power of two")
//@ ```
//@
//@ Built with `-D RING_CAPACITY=63`, that is refused, naming the flag. One
//@ inside an arm a `when` did not select is never folded, like everything
//@ else there.
