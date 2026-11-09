//@ toys/dialect 27
//@ ### Traits
//@
//@ A class takes a trait on with `using`, written on its first line after any
//@ `extends`: `class Boss extends Goblin using Drawable`. A trait is written
//@ in a class, as an inner class is — `trait Drawable:` in `Goblin` is
//@ `Goblin.Drawable` in the package and `Drawable` inside `Goblin` — and its
//@ body is methods only: a signature ending its line is required, one with a
//@ body is a default. A trait may extend other traits.
//@
//@ A class that takes a trait on has every method the trait prescribes, with
//@ the trait's signature, `Self` read as the class — its own or one it
//@ inherits — and a method missing or of another shape is refused at the
//@ `using`, naming the line to write (`ERR0536`). A default the class writes
//@ no method for becomes its method. Inside a default, `self` reaches only
//@ the trait's methods, since a trait prescribes methods and never fields.
//@
//@ A trait is a type: `Drawable d = g` holds any instance of a class that
//@ takes it on, and `[]Drawable` holds several of different classes. A call
//@ through one runs the method of whichever class the value holds. A method
//@ taking a `Self` is called on the class's own type, never through a trait
//@ value, which cannot say which class `Self` is. A trait Dado declares is not
//@ taken on by a script: that is the boundary between classes and Dado's
//@ types, not built yet.
//@
//@ A trait method may be `async` — `async int load()` — and the method a
//@ class writes for it is `async` too; a call through a trait value answers
//@ `async int`, awaited as any other. An `async` method answering `Self` is
//@ not built (`ERR1330`).
