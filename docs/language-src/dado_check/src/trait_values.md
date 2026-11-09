//@ types/traits 25
//@ ### Trait values
//@
//@ A trait name written anywhere but a parameter's whole type — a slice
//@ or array element, a field, a local, a return, a map value, a function
//@ type's parameter — is a **trait value**: a view of an implementer
//@ stored somewhere else, the pair of its address and its declaration's
//@ number. `#size(Drawable)` is the pair's: two words, 16 bytes where a
//@ pointer is 8.
//@
//@ ```dado
//@ trait Drawable:
//@     void draw(self)
//@
//@ type Scene:
//@     []Drawable items
//@     Drawable focus
//@ ```
//@
//@ Zero bytes are the empty value, so `Drawable d` with no initialiser
//@ and `[4]Drawable` are empty until assigned. A trait value copies into
//@ another of the same trait, and is passed and returned like any view.
//@ It has no `==`, is no map key and has no rendering — compare or
//@ render the implementers.
//@
//@ **A trait value is made by writing its implementer where one is
//@ asked** — a declaration, an assignment, an argument, a `return`. The
//@ implementer is a place — a name, an element, a field, `p^` — or a `^T`
//@ or `ref T` pointing at one: the pair views it where it is stored, so a
//@ temporary or a constant is refused with *bind it first*. Its type is a
//@ declaration that takes the trait on; a shape written out implements
//@ nothing. A value of a trait that extends the one asked converts as it
//@ stands. `Drawable(b)` is no conversion: there is one spelling. A
//@ `const` implementer is refused, since a `^self` method reached
//@ through the pair would write it.
//@
//@ **A call through a trait value dispatches at run time**: it reaches
//@ the method of whichever implementer the value views, its own or its
//@ default. `self` gets a copy of the implementer and `^self` the
//@ implementer itself, so a write through a pair writes the original.
//@ A call through an empty value stops the program, naming the trait.
//@ A method whose signature mentions `Self` beyond its receiver, or that
//@ binds a type of its own, cannot be called through a value — the pair
//@ does not say which declaration `Self` is — and is called through a
//@ binder or on the implementer instead. A trait value passed to a
//@ binder parameter binds the pair, which dispatches inside.
//@
//@ **Every rule a view obeys, a trait value obeys.** A pair of an
//@ implementer the function's own frame stores does not outlive it —
//@ returned, stored where it outlives the frame, or handed to a function
//@ that keeps it — as a window over a local array does not; a pair made of an element
//@ of a `ref` run is stale after `#append`, `#resize`, `#reserve` or
//@ `#delete` on it; a pair of `#stack` storage stays in its block; and
//@ no trait value rides in a channel's message or crosses to C through an
//@ `export` or a `foreign` signature — C is handed the implementer's own
//@ type.
