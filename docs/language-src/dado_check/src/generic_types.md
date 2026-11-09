//@ generics/types 10
//@ ### A generic `type` is instantiated where it is written
//@
//@ A `type` head may bind whole types after its name, and its body writes
//@ them bare. `Pool(i32)` is then a type: the declaration instantiated at
//@ `i32`, made the first time it is written, from the declaration's own
//@ file, and the same type wherever it is written again. A generic
//@ declaration's name alone is not a type, and neither is one written
//@ with the wrong number of arguments; both are refused quoting the head.
//@
//@ ```dado
//@ package doc_sample
//@
//@ type Pool($T): ([4]T items, i64 used)
//@
//@ // A recursive container is a pointer away from itself, as any is.
//@ type List($T): (T head, ^List(T) tail)
//@
//@ // `Pool($T)` in a signature reads `T` back out of the argument.
//@ private void put(^Pool($T) p, T v):
//@     p^.items[p^.used] = v
//@     p^.used += 1
//@     return
//@
//@ public i64 filled():
//@     Pool(i32) ints = ([0, 0, 0, 0], 0)
//@     put(&ints, 7)
//@     Pool(f32) reals = ([0.0, 0.0, 0.0, 0.0], 0)
//@     put(&reals, 0.5)
//@     List(i32) last = (2, nil)
//@     List(i32) first = (1, &last)
//@     return ints.used + reals.used + i64(first.tail^.head)
//@ ```
//@
//@ **An instantiation is a declared name, so everything a declared name
//@ does it does.** Its slots are named by the declaration's body, it is
//@ interchangeable with its shape, and its C struct is its shape's, so
//@ `Pool(Vector2)` and `Pool(Size)` over one shape are one struct in the
//@ C. They are still two instantiations — one per argument, each
//@ carrying its argument's declaration exactly as `[]Vector2` and
//@ `[]Size` do — and `--emit=types` lists each as it was spelled,
//@ beside a `generic Pool($T)` row for the declaration itself. A
//@ `distinct` generic is a tag per instantiation.
//@
//@ A `where` on the head is the declaration's **domain**: it is asked of
//@ each instantiation's arguments where the type is written, and an
//@ instantiation outside it is refused there, naming the clause. A body
//@ nobody instantiates is never lowered, which is the bargain a template
//@ has: what is checked is what a program asks for.
//@
//@ A generic body names every slot it has. Embedding splices another
//@ shape's names into this one, which would give each instantiation a
//@ different view, so a member written without a name is refused with
//@ the name to give it; a binder is a whole type and never a count, and a
//@ channel, being one mailbox named after its declaration, belongs in a
//@ declaration that binds nothing.

//@ generics/types 12
//@ ### Methods and traits on a generic `type`
//@
//@ A method written in a generic body is a template over the
//@ declaration's binders. Each instantiation has its own copy of it,
//@ declared and checked the first time a call or a trait it takes on
//@ reaches it: `Pool(i32)` and `Pool(Vector2)` never share one, and a
//@ method nothing reaches is never checked — so its signature may write
//@ the generic at a bigger argument, made only when it is called. As
//@ with `Vec2.len`, `Pool(i32).push` names one as a function. Inside it,
//@ `T` is the instantiation's argument and `Self` the instantiation;
//@ `self` and `^self` take the value as for any method.
//@
//@ ```dado
//@ package doc_sample
//@
//@ trait Sized:
//@     i64 count(self)
//@
//@ type Stack($T) using Sized:
//@     [4]T items,
//@     i64  used,
//@     void push(^self, T v):
//@         self.items[self.used] = v
//@         self.used += 1
//@     i64 count(self):
//@         return self.used
//@
//@ public i64 both():
//@     Stack(i32) ints = ([0, 0, 0, 0], 0)
//@     ints.push(7)
//@     Stack(f32) reals = ([0.0, 0.0, 0.0, 0.0], 0)
//@     reals.push(0.5)
//@     reals.push(1.5)
//@     return ints.count() + reals.count()
//@ ```
//@
//@ An instantiation is a declaration of its own, named for the generic
//@ and its arguments: its methods, the traits it takes on and its
//@ number as a trait value's implementer are `Stack(i32)`'s, never its
//@ shape's. A `using` on a generic head takes the trait on for **every**
//@ instantiation, and conformance is checked for each at its own
//@ arguments: a method whose signature holds for `Stack(i32)` may not
//@ for `Stack(f32)`, and that one is refused, naming it. There is no
//@ conditional conformance — a trait taken on only where `T` has some
//@ other — so a `using` that needs something of `T` is written with a
//@ `where` that makes it the domain.
