//@ toys/dialect 32
//@ ### A script reaches JavaScript
//@
//@ A script imports a JavaScript module as it imports a package, by its
//@ path, and the extension says it is JavaScript: `import "lib/three.js"`
//@ (or `.mjs`). The path is the file's, relative to the script's directory
//@ or under a collection, `core:` and its path. The module's namespace is
//@ bound under the file's name without its extension, three, or under an
//@ alias, `import THREE "lib/three.js"`; a file whose name is no identifier,
//@ `three.module.js`, needs the alias (`ERR1320`). The namespace is an
//@ `any`: its members are read and called as JavaScript reads and calls
//@ them, and each answer is an `any`.
//@
//@ A `foreign "lib.js":` block, written in a class, declares the module's
//@ functions with a script's types, and the class calls them by name. Each
//@ argument is checked as a method's is, and the answer is checked when it
//@ arrives: a declared `int` that comes back `2.5` raises an `Error` of code
//@ 70, as an `any` leaving into an `int` does. An exception thrown in
//@ JavaScript is caught as an `Error` by `try … else e:`. A block declares
//@ functions; their parameters and answers are `int`, `float`, `bool`,
//@ `string`, `any`, arrays and maps of those, and `void` for no answer, and
//@ a last parameter `..any xs` takes the rest. If the file is also imported,
//@ the declared functions are typed through its namespace too, and every
//@ other member stays an `any`.
//@
//@ The JavaScript is part of the program: each file, and every file it
//@ imports by a relative path, is embedded in the binary under the name
//@ `js:` and its path, and the script's module imports it by that name.
