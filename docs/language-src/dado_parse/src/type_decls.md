//@ types/traits 10
//@ ### What a declaration head and body may now say
//@
//@ A `type` head reads, after its name and in this order, each part
//@ optional: a binder list `($T, $U)`, the traits it takes on
//@ (`using Drawable, Movable`), a `where` over its binders, and then its
//@ `:`. A `trait` head has the same shape with `extends` where a `type`
//@ has `using`, because a trait builds on traits and a type takes them
//@ on — so each word written on the other head is refused naming the
//@ right one. Nothing else can stand between a declaration's name and
//@ its `:`, so neither word is reserved: `i32 using = 2` and
//@ `i32 extends = 2` stay ordinary declarations.
//@
//@ A method is a line of a `type` body whose name is followed by `(`. A
//@ slot holding a function writes its type first, `R(A) name`, so one
//@ token tells the two apart. Three bodies carry methods: a member list
//@ written one per line, slots and methods mixed; a parenthesized list
//@ on the head line with the methods indented under it; and an alias
//@ with the methods indented under it. Those last two wrote the shape on
//@ the head line, so the block under them holds methods only.
//@
//@     type Vec2 using Drawable:
//@         f32 x
//@         f32 y
//@         f32 len(self):
//@             return self.x
//@         void move(^self, f32 dx):
//@             self.x += dx
//@
//@     type Size: (f32 w, f32 h)
//@         f32 area(self): return self.w * self.h
//@
//@ A method's first parameter is its receiver, written with no type:
//@ `self` for a copy of the value it was called on, `^self` for a pointer
//@ to it. The word is a receiver only there — `self` is an ordinary name
//@ everywhere else, including a typed parameter called `self`.
//@
//@ A method is promised to raise into a script as a function is:
//@ `#assert_raises` alone on the line above it, in the body, and checked
//@ as the function `Vec2.len` it is checked and emitted as.
//@
//@ A `trait` body is methods only, because a trait prescribes methods
//@ and never a shape: a signature ending its line is one every
//@ implementer writes, and one with a `:` and a suite is a default. A
//@ slot line in a trait is refused naming the `type` it belongs in.
//@
//@     trait Drawable:
//@         void draw(self)
//@         void draw_twice(self):
//@             self.draw()
//@             self.draw()
//@
//@ **Methods, traits, a head's `using`, a generic declaration's binder
//@ list and `where`, and a trait written as a value's type are built** —
//@ the last is a trait value, below. A generic trait's binders are read
//@ and refused `ERR0001` where they are written, so a program using one
//@ hears that rather than getting half of it.
