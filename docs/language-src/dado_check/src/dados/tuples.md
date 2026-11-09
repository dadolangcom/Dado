//@ toys/dialect 26
//@ ### Fixed arrays
//@
//@ A fixed array is written as Dado writes it, `[2]float`, `[4]int`,
//@ `[3][3]float`, its length a literal or a constant the script sees — one
//@ of its package's Dado files declares, `[STEPS]float`. It is Dado's fixed
//@ array: a **value** of that many slots, so assigning one, passing it and
//@ returning it copy it, where a `[]T` hands over the same array. An array
//@ literal is one where a fixed array is wanted — `[2]float v = [1.0, 2.0]`
//@ — and a `[]T` everywhere else. `v[i]` reads a slot and `v[i] = n` stores
//@ one: a written position past the end is refused, `ERR0519`, and a
//@ computed one raises as an array's index does. `for n in v` walks it,
//@ `for n, i in v` with the position, `n in v` asks whether a slot is `==`
//@ to `n`, `v.len()` is its length, which its type holds, and `v[a..<b]`
//@ cuts a new `[]T` out of it.
//@
//@ Its operators are Dado's, slot by slot, with Dado's rules for which apply:
//@ `+ - * /` over two fixed arrays of one type, or over one and a scalar of
//@ its element type, `v * 2.0`; `%` and the bit operators over an `int` one;
//@ unary `-`; and `==` and `!=`, true when every slot is equal. Two lengths or two
//@ element types do not meet, and `<` is refused on a shape. A slot of
//@ `[4]int` wraps as an `int` does, and a fixed array prints as Dado's does,
//@ `[1.0, 2.0]`. A fixed array crosses between Dado and a script as the
//@ same type, each way (see *Arrays and tuples cross the seam*): a Dado
//@ `[2]f32` or `Vec2` is a script's `[2]float`. A method Dado calls whose
//@ fixed array's length is a constant (`[STEPS]float`) is not reached yet
//@ (`ERR1201`), nor is a field holding one (`ERR1219`).

//@ toys/dialect 29
//@ ### Tuples
//@
//@ A tuple holds a fixed number of values of fixed types: `(int, string)` is
//@ its type and `(3, "ann")` a value of it, so a method may answer two
//@ values at once. A tuple is a value, like Dado's: assigning one copies it.
//@ Its slots are read by destructuring it into locals,
//@ `int n, string name = pair()`, an underscore naming a slot not wanted, or
//@ by a written position, `t[1]`, which also stores one. A tuple of one type
//@ throughout is a fixed array, `(float, float)` being `[2]float`. Its
//@ operators are Dado's, slot by slot: `==` compares every slot, a `string`
//@ by its contents, and an operator applies when it applies to every slot's
//@ type — so a tuple holding an array or a map has no `==`. A tuple prints
//@ as its slots in parentheses, `(3, ann)`.
