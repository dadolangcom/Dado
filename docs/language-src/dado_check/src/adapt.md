//@ types/distinct 55
//@ ### `int` and `float` adapt to the width they meet
//@
//@ `int` is `i64` and `float` is `f64`, on every target and in both
//@ dialects. They are one type with those two for everything that asks
//@ about data: identity, `==` between types, layout, `#size`, the emitted
//@ C (an int64_t and a double) and generic binding, so a `[]int` is a
//@ `[]i64`, `int(int)` is `i64(i64)`, and `typeof` of an `int` value is
//@ `i64`. What the name adds is a **behaviour**: a value whose type is
//@ written `int` or `float` **adapts** to whatever width of its own kind
//@ it meets — any integer width for `int`, `f32` or `f64` for `float` —
//@ widening or narrowing, the way a literal does. Spelling the width is
//@ the opt-in to strictness: `i64` keeps today's rule, under which only a
//@ value-preserving widening is supplied.
//@
//@ ```dado
//@ package doc_sample
//@
//@ i32 clamp_to_byte(int n):
//@     u8 b = n        // adapts: narrows, and wraps as u8(n) would
//@     return b        // u8 widens to i32, as it always has
//@ ```
//@
//@ Two words, kept apart in this document: a change of **width** within
//@ one kind is *adapting*, and a change of **kind** is *coercing*. Nothing
//@ coerces implicitly, `int` and `float` included: an integer becoming a
//@ float is written `float(n)` or `f32(n)`, and so are a number and a
//@ string, a `bool` and a number, an enum and its backing integer.
//@
//@ **The operand of a conversion is its own expression**: it is checked
//@ and computed on its own, its literals taking their own type, and only
//@ then converted. So `f32(7 / 2)` is the integer division, 3, made `3.0`,
//@ and `f32(1 << 23)` the integer shift made an `f32`, never a shift on
//@ floats. A number written alone is the target's literal — `f32(1)` is
//@ `1.0` in single precision, `u8(300)` the 44 the wrap asks for — and an
//@ integer target lets a literal in a longer operand take its width
//@ (`i64(1 << 40)` is a 64-bit shift), since that never changes its kind.
//@ Either way the conversion's result is a value of the target type, and
//@ the operator around it is that type's: `f64(7) / f64(2)` is 3.5.
//@
//@ **Known at compile time**, an adapted value is a literal: a constant
//@ `int` that does not fit the width it meets is linted as `u8 b = 300`
//@ is, and folded to the value the width holds. **Not known**, it narrows
//@ at run time as the written conversion does — an integer keeps its low
//@ bits, a `float` meeting `f32` rounds — and the compiler keeps a list of
//@ every such place: `dadoc --emit=narrowings` prints it, one line a
//@ place, for code that wants to see each place it relies on the
//@ leniency. Nothing lints them.
//@
//@ **A dev build warns when a narrowing changes the value**, and does
//@ nothing else: the value wraps or rounds exactly as it would without
//@ the warning. The first time a place changes a value, a line names the
//@ place, the value and the width (`warning: main.dado:12:12: implicit
//@ narrowing changed the value: the int 300 does not fit in u8, and wraps
//@ to 44`); later ones there are counted, and at exit each place that
//@ changed a value more than once is written again with its count, so a
//@ narrowing in a loop or every frame says so once. *Changes the value*
//@ means an integer the width cannot hold, or a finite `float` too large
//@ for `f32`, which becomes infinity. Rounding to the nearest `f32` is
//@ what an `f32` is, and is not reported; a NaN stays a NaN.
//@
//@ The warning goes to standard error; on the web to the page's
//@ `console.warn`; for a script's value crossing into Dado (see *A script
//@ calls Dado*), to the script engine's error sink. A target with no C
//@ library (`#NONE`) has no stream: there the first warning at a
//@ place is handed to `narrowing_warning` if the program defines one —
//@ `export void narrowing_warning(cstring where, cstring width, i64
//@ value, f64 fvalue)`, `value` an integer's and `fvalue` a float's —
//@ and nothing is written when it does not. Because the check never
//@ changes a result, **`--release` drops it**: a release build narrows
//@ silently. `--narrowing-warnings` keeps it in a release build, and
//@ `--no-narrowing-warnings` drops it from a dev one; neither changes a
//@ value or a diagnostic.
//@
//@ **Where an `int` meets an explicit width in an operator, the explicit
//@ width decides, from whichever side it stands on**: `i32 + int` and
//@ `int + i32` are both `i32` arithmetic, so reordering the operands never
//@ changes what is computed. Two explicit widths keep today's rule (the
//@ wider wins; nothing narrows), and `int` with `int` is `int`. An
//@ assignment, compound ones included, takes the width of its target
//@ (`count += n` narrows `n` into an `i32 count`); a shift takes the
//@ width of the value shifted, and its count adapts; a comparison compares
//@ the two **values**, widened to the larger type and never narrowed, so
//@ `i32 x < int n` cannot wrap `n`. A conditional value's two arms
//@ meet the same way: an explicit width on either arm is the value's type.
//@
//@ **Shapes adapt slot by slot.** A `[2]float`, or a tuple or `type` whose
//@ slots are `float`, adapts to a shape of the same length whose slots are
//@ `f32`, each slot narrowing as a scalar would. A window or a reference
//@ never does: `[]int` is `[]i64`, and a `[]i32` is other storage.
//@
//@ A generic binding binds the data: `$T` bound from an `int` argument is
//@ `i64`. After `is` and in a `where`, `int` and `float` still name their
//@ families, so `where T is int` holds for every integer width and
//@ `where T is float` for both floats; everywhere a type is written they
//@ are the type.
