//@ types/traits 24
//@ ### Default bodies
//@
//@ A trait method written with a body is a **default**: a template over
//@ `Self`, the implementer. It is instantiated once per implementer that
//@ takes it, when a call reaches it — `v.draw_twice()` on the implementer,
//@ or on a binder's value, or `self.draw_twice()` in another default — and
//@ never by the `using` alone, so a default no call reaches is never
//@ checked. An instance is the implementer's own method as far as any
//@ caller can tell: it is called, and emitted, exactly as a method of that
//@ name written in the implementer's body would be: embedding promotes
//@ it, and `Box.draw_twice` names it as a function value, which
//@ instantiates it as a call would.
//@
//@ ```dado
//@ trait Drawable:
//@     void draw(self)
//@     void draw_twice(self):
//@         self.draw()
//@         self.draw()
//@ ```
//@
//@ Inside a default, `self` has **no view**: a trait prescribes methods
//@ and never a shape, so a default reaches `self` only through its
//@ trait's methods, required or default. `self.x` is refused whatever
//@ the implementer's slots are called, and so is a method only one
//@ implementer writes — make it a required method, and each implementer
//@ answers it. A `^self` default writes through the trait's `^self`
//@ methods the same way.
