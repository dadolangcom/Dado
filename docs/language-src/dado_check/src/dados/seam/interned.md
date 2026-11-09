//@ toys/dialect 40
//@ ### A script reads Dado's constants
//@
//@ A script reads a constant of its package's Dado files by its bare name,
//@ `KIND`, and one of a Dado package it imports through the import's name,
//@ `lib.KIND`, as it calls their functions — a field or a method of the
//@ class wins over a constant of the same name. The constant is
//@ **interned**: the value Dado folds is written into the script's code
//@ where it is read, so nothing crosses when the script runs, the script
//@ runs the same natively and on the web, and no constant is mirrored by
//@ hand. A `-D` definition reaches a script as it reaches Dado.
//@
//@ A constant is held as a Dado function's answer of its type is: every
//@ integer and a `char8` as an `int` (a `u64` by its bits), `f32` and `f64`
//@ as a `float` (an `f32` as the nearest `f32`), a `bool` as itself, a
//@ `char16`, a `char32` or a string of any width as a `string` (a
//@ `cstring` up to its first NUL, as C reads it), and a fixed array or a
//@ tuple of those as the script's own — a new one at each read, so a script
//@ changing the array it read changes no other read. An enum's constant is
//@ the script's enum member it is, or the number no member names, and a
//@ `distinct` type's is its value under the tag. One written without a
//@ type is its literal, as in Dado, and adapts to where it is read:
//@ `float f = MAX`. A constant of a declared shape reads its slots by name,
//@ `lib.ORIGIN.x`, and a method of the script's array or string is called
//@ on one, `SIZES.len()`. A constant has no storage, so a script assigning
//@ one, or a position or slot of one, is refused (`ERR0606`).
//@
//@ A constant whose type does not cross — a slice (`const []i32`, alone or
//@ inside a tuple: write its length, `const [2]i32`), an enum backed by no
//@ integer — one whose value C supplies (a `foreign` constant, whose value
//@ the compiler never learns), and one holding a function or a type are
//@ refused where the script reads them (`ERR1208`), naming why.
