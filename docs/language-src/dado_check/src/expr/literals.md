//@ types/distinct 50
//@ ### A number that does not fit its type
//@
//@ An integer literal takes the type it is written for, and a value that
//@ type does not hold **wraps**, exactly as the conversion does at run
//@ time: `u8 f = 300` holds 44, `i8 c = 200` holds -56, and `u32 x = -1`
//@ holds 4294967295. Because that is rarely what was meant, it is a
//@ lint — a warning while writing, an error under `--strict` — whose repair
//@ is to write the conversion, `u8(300)`, which says the wrap is on
//@ purpose and is accepted. The rule is the same wherever a number meets a
//@ type: a constant's initializer, a value folded from other constants, an
//@ untyped constant at its use, a default argument, an enum member, a
//@ `#LEN`, a slot of an array literal, and a literal meeting no type at
//@ all, which is an `i32`. A character literal that does not fit its
//@ character type keeps its low bits on the same terms, and `char8('é')`
//@ says so on purpose.
//@
//@ **The arithmetic over literals is done in the type the literals took.**
//@ A literal written for a float is a float, an integer one included, so
//@ an operator whose operands are literals computes in that float:
//@ `f64 c = 7 / 2` divides two `f64`s and is 3.5, `x + 1 / 2` for an
//@ `f64 x` adds a half, and `f32 h = 1 / 3` is a third computed in `f32`.
//@ The same holds in an argument, a `return`, a default, a package variable
//@ and a `const` (which folds to the value the program computes), and in
//@ both dialects: a script's `float f = 7 / 2` is 3.5 too. Where the
//@ literals are integers the division is the integer one, `i32 n = 7 / 2`
//@ is 3, and so is an operand with no type to adapt to, which is what a
//@ conversion's operand is: `f64(7 / 2)` is 3.0.
//@
//@ What has no value in any type is refused instead: an integer past 64
//@ bits, a constant whose folding overflows the compiler's arithmetic, and
//@ a float that would be infinite or not a number in its type.
