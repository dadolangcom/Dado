//@ toys/dialect 30
//@ ### Dado calls a script's function
//@
//@ An `export` method of a script that reaches no `self` — no field, no
//@ other method, no `self` — is a **function of its package**, and Dado
//@ calls it by the package, with no VM and no handle:
//@
//@     // game/Rules.dados
//@     class Rules
//@
//@     export int on_hit(int hp, int damage):
//@         if damage > hp:
//@             return 0
//@         return hp - damage
//@
//@     // main.dado
//@     import "./game"
//@
//@     i64 left = game.on_hit(10, 3)
//@
//@ Its arguments and its answer are a script's scalars — an `int`, a
//@ `float`, a `bool`, a `string` — crossing as a method's do through a
//@ handle; anything else is refused (`ERR1226`). Under `try` a failure is the
//@ script's `Error`'s code, its text already on the error sink; with no
//@ `try`, a failure ends the program as an `abort` does, status 134. The
//@ call runs on an engine of the calling thread's own for package
//@ functions, made the first time, so it sees no instance a VM holds. A
//@ function that reads only its parameters, constants and the JavaScript it
//@ imports is called directly — on the web its JavaScript is the wasm
//@ module's import, with nothing in between; one that does more, such as
//@ `print`, goes through the seam, and `dadoc --emit=strategies` says which,
//@ and why.
