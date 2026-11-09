//@ toys/dialect 50
//@ ### DadoScript as a JavaScript library: `export`
//@
//@ A package of scripts can be built as a **JavaScript library** — an ES-module
//@ package that node, a browser page or a bundler imports with no Dado in it:
//@
//@     dadoc --emit=js-package --package -o dist/ game/Goblin.dados
//@     ./dado build game --target=js
//@
//@ **`export` says what the package publishes.** `export class Goblin` — a
//@ script's header, or a class block — publishes the class: its constructor
//@ (`init`'s parameters), and every method and field that is not `private`,
//@ inherited ones included. An `export` method that reads no `self` — no
//@ field, no other method — is published as a function of the package too,
//@ whether or not its class is:
//@
//@     export class Dice
//@
//@     int sides = 6
//@
//@     void init(int n):
//@         sides = n
//@
//@     int roll(int seed):
//@         return seed % sides + 1
//@
//@     export int clamp(int x, int lo, int hi):
//@         if x < lo:
//@             return lo
//@         if x > hi:
//@             return hi
//@         return x
//@
//@ A JavaScript caller writes `new Dice(20).roll(7)` and `clamp(9, 0, 5)`.
//@
//@ **What crosses is what a TypeScript declaration can say.** The package
//@ carries `index.d.ts`, written from the checked program: `int` is
//@ `number | bigint`, `float` `number`, `bool` `boolean`, `string`
//@ `string`, `[]T` `T[]`, `{K: V}` `Map<K, V>`, a tuple `readonly [A, B]`, an
//@ enum the union of its members' names, `any` `unknown`, a function type
//@ `(a: A) => R`, an answered `async T` `Promise<T>`, and an exported class
//@ its own declaration. A published signature naming anything else — a trait
//@ value, a class that is not exported — is refused at the type (`ERR1340`),
//@ naming the repair.
//@
//@ **The boundary is checked.** Every argument a JavaScript caller passes is
//@ checked as an `any` leaving into the parameter's type: a wrong one throws.
//@ An `int` takes a `number` holding an integer or a `bigint`, and an `int`
//@ answered past 2⁵³ − 1 arrives as a `bigint`. A failure — a wrong argument,
//@ a `fail`, a fault — reaches JavaScript as a thrown `DadosError`, an `Error`
//@ carrying `code`, `message` and `trace`. Calls between the scripts are not
//@ checked again.
//@
//@ **A library has no Dado host**, so its scripts call no Dado function and load
//@ no script by its path (`ERR1341`); `print` writes to the JavaScript host's
//@ standard output, or its console.
