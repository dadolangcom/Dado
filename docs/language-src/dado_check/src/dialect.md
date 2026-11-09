//@ toys/dialect 10
//@ ### One tree, two dialects: where each construct may be written
//@
//@ A `.dado` file is Dado and a `.dados` file is DadoScript; a `class` block is DadoScript
//@ wherever it stands, so a `.dado` file may hold one, and everything inside it
//@ is DadoScript. Nothing smaller than a declaration changes dialect: a function
//@ body, a statement and an expression are written in the dialect of the file
//@ or class around them.
//@
//@ Most of the grammar is shared, so most code means the same in both. What
//@ only Dado writes — memory management (`ref`, `^`, `&`, allocators), every
//@ `#` spelling, explicit widths (`u8`, `f32`), the C boundary (`@c`,
//@ `foreign "x.h":`, `cunion`, bitfields), the failable mark `!`, `quit`,
//@ `package` and `export` — is refused in DadoScript, `ERR1301`, naming what DadoScript
//@ writes instead: `#len(xs)` is `xs.len()`, a width such as `u8` is the one
//@ integer type DadoScript has, `!T` is `T`, and `ref []T` is `[]T`. What only DadoScript
//@ writes — lambdas, `async` and `await`, the `assert` statement, `super`, a
//@ script's header line, and a `foreign "x.js":` block — is refused in Dado,
//@ `ERR1302`, naming Dado's spelling where it has one: `assert c, "m"` is
//@ `#assert(c, "m")`.
//@
//@ DadoScript's eight bare built-in names — print, println, any, Error, Location,
//@ Script, Platform and globalThis — exist only in DadoScript. In Dado they are
//@ ordinary names a program may declare.
