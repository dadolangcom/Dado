//@ toys/dialect 20
//@ ### A script is a class
//@
//@ A `.dados` file declares one class. Its first line may name it,
//@ `class Goblin`; without that line the class is named by the file's
//@ basename — a file `goblin.dados` declares a class named goblin — and a
//@ basename that is not an identifier is refused (`ERR1303`). A `class Name:` block with an
//@ indented body declares a class inside a `.dado` file. Either way the class
//@ is a member of its file's package, and two classes of one name in one
//@ package — or a class and a Dado declaration of that name — are refused
//@ (`ERR0402`).
//@
//@ A class holds **fields** and **methods**. A field is written like a
//@ variable, `int hp = 10`, and without an initializer starts at its type's
//@ zero. Fields are initialised in the order they are written, so an
//@ initializer reads only the fields above it. Every function written in a
//@ class is a method; writing `self` as its first parameter is allowed and
//@ changes nothing. Inside a method a field is reached by its bare name or as
//@ `self.hp`, and a method is called as `bump(1)` or `self.bump(1)`; a local
//@ or parameter of the same name hides the bare field, and `self.hp` still
//@ reaches it.
//@
//@ Two method names belong to the harness that runs a script: **`init`** runs
//@ once per instance and takes the constructor's arguments, so it returns
//@ nothing; **`ready`** is the entry a host runs with `#script_run`, as a
//@ program's main function is run, so it returns an `int` or nothing
//@ (`ERR1304`), and takes the arguments `#script_run` passes after the handle.
//@
//@ A script's types are `int` and `float` — Dado's own, `i64` and `f64`
//@ with the width left to the compiler (see *`int` and `float` adapt to the
//@ width they meet*) — `bool` and `string`, JavaScript's string, which has
//@ no width.
//@ An integer literal with nothing to adapt to is an `int`. Everything else
//@ is Dado's: the operators and what they accept, how a literal adapts and
//@ wraps, linted, where it does not fit, conditions that are `bool`, `for`
//@ loops over a range or on a condition, and every conversion written. So a `float` is never made an
//@ `int` by itself — write `int(x)`, which drops the fraction — nor an `int`
//@ a `float` — write `float(x)`. Strings join with `+` and compare with `==`
//@ and `!=`; a value is made a string only by `string(x)`, so
//@ `"total: " + 5` is refused naming `string(5)`. `print` and `println` take
//@ any number of those four types and write them with no separator. A fixed
//@ array, `[2]float`, is Dado's too: a value, with Dado's operators slot by
//@ slot (see *Fixed arrays*).
//@
//@ What DadoScript will have and the checker does not check yet is refused
//@ `ERR0001`, naming it. A script may import a Dado package, and calls its
//@ functions and reads its constants as a Dado file does (see *A script
//@ calls Dado* and *A script reads Dado's constants*).
//@
//@ Each class is emitted as a JavaScript module named `<package>/<Class>`,
//@ which `dadoc --emit=js` prints. There an `int` is still exactly `i64` —
//@ past 2⁵³ and wrapping at 2⁶³ — and an integer division or remainder by
//@ zero raises an error, `integer division by zero`, where Dado stops the
//@ program. A `float` is written as the fewest digits that read back as the
//@ same value, `1.0`, `0.1`, `1e+16`, `1e-05`; not-a-number prints nan, the
//@ infinities inf and -inf, and negative zero `-0.0`. A program holding DadoScript builds as its Dado, carrying the
//@ module of every class it loads and no other: a class nothing loads is
//@ checked and left out. A main package of scripts alone has no Dado to host
//@ them, and its build is refused `ERR0001`.
