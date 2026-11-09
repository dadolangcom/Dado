//@ toys/dialect 25
//@ ### Classes are types
//@
//@ A class's name is its type and its constructor: `Goblin g = Goblin(3, 4.0)`
//@ declares a local of the class's type and makes an instance, passing the
//@ arguments to the class's init method, as a method call passes them. An
//@ instance is held by reference: a local, a field, an argument and a return
//@ all hand over the same one. `g.hit(2)` calls a method of it, and `g.hp`
//@ reads or writes a field of it. A field or method is public unless written
//@ `private`, which keeps it to the package, or `private "file"`, which keeps
//@ it to the file. A field or local whose type is a class is written with its
//@ value, since a script has no null to start it at. A class has no operators
//@ but its methods, and `==` asks whether two names hold the same instance; a
//@ value that is copied and compared by its slots is a `type` (see *Types
//@ and unions*).
//@
//@ A script may hold an inner class: a `class Loot:` block with an indented
//@ body, named `Goblin.Loot` in the package and `Loot` inside `Goblin`. Inner
//@ classes go one level deep, and one is never loaded on its own.
//@
//@ A class extends one other: `class Boss extends Goblin`. It inherits every
//@ field and method, and a method of the same name overrides the inherited
//@ one, taking the same parameters and answering the same type. In a
//@ subclass, `super.hit(n)` calls the parent's method, and `super(…)`,
//@ written once in the init method, runs the parent's — required when the
//@ parent's init takes arguments, and otherwise run first by itself. A class
//@ that extends nothing extends `Script`. A cycle of `extends` is refused
//@ (`ERR1310`), and so is extending a type Dado declares: a Dado base for scripts
//@ is not built yet.
//@
//@ A package of scripts is imported as any package is: `import "game/npc"`,
//@ then `npc.Goblin` is its class, as a type and as a constructor, and
//@ `import foes "game/npc"` binds the package under that other name. Scripts may import each
//@ other in a cycle. A directory holding only scripts is a package named by
//@ the directory, since a script names no package.
