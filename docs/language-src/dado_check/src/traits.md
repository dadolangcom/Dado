//@ types/traits 20
//@ ### Traits
//@
//@ A `trait` names a set of methods and nothing else — never a slot, since
//@ a trait prescribes methods and never a shape. A signature ending its
//@ line is **required**; one with a body is a **default**, which an
//@ implementer that writes no method of that name gets.
//@
//@ ```dado
//@ trait Drawable:
//@     void draw(self)
//@
//@ trait Shape extends Drawable:
//@     f32 area(self)
//@
//@ type Square using Shape:
//@     f32 side
//@     void draw(self):
//@         return
//@     f32 area(self):
//@         return self.side * self.side
//@ ```
//@
//@ A trait is in the type namespace, so a trait and a type of one name in
//@ a package are refused. `extends` names traits and never cycles; a
//@ trait's methods are its own and its ancestors'. A child may give a
//@ parent's *required* method a default, and may not replace a parent's
//@ *default* — only an implementer overrides a default, so each pair of
//@ implementer and method has one body. Two parents asking one name with
//@ one signature ask one method; two defaults or two signatures for one
//@ name are refused at the child, naming both.
//@
//@ **Conformance is declared, at the `using`.** A `type` takes a trait on
//@ by writing `using` it (or a trait that extends it) at its head, and
//@ every required method must then be in its body — or promoted into it
//@ by embedding — with the trait's signature, `Self` read as the
//@ implementer, and the same receiver. A missing or different one is
//@ refused there, naming the line to write. Embedding an implementer
//@ does not make the embedder one.
//@
//@ **A trait written as a parameter's whole type is a binder**: `void
//@ render(Drawable v)` takes any declaration that takes `Drawable` on,
//@ and is instantiated once per implementer, in which `v.draw()` is a
//@ direct call to that implementer's method. `^Drawable v` binds a
//@ pointer the same way. `where T has Drawable` asks the same question
//@ of a template's own binding. A literal or a shape written out
//@ implements nothing — `Drawable` is answered by a declaration's
//@ `using` — and a declaration that does not take the trait on is
//@ refused at the call, naming the `using` to add.
//@
//@ **Taking a trait on publishes the trait's methods through it.** A
//@ binder or a `where T has Drawable` template in the trait's package
//@ calls `v.draw()` on an implementer declared `private` in another
//@ package, as a `Drawable` trait value always could: a method a trait
//@ the declaration takes on prescribes, required or default, is reached
//@ from every file that can name the trait, whatever the declaration's
//@ visibility or the method's. `private` still hides the type's name,
//@ and every method no trait prescribes keeps the visibility it was
//@ declared with.
