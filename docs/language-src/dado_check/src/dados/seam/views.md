//@ toys/dialect 46
//@ ### A script names Dado's declarations
//@
//@ A script may write a Dado declaration of its package as a type, by its
//@ bare name, and one of a package it imports through the import's name:
//@ `entities.Vec2 p = entities.pos()`. The declaration is a view over the
//@ type it crosses the seam as — a shape whose slots share a type, like
//@ `type Vec2: (f32 x, f32 y)`, is the script's fixed array `[2]float`,
//@ with its operators, and a shape of mixed slots its tuple — so the two
//@ are interchangeable: `[2]float q = p` checks, as does passing the value
//@ where a `[2]float` is taken, and `p + q` is a `[2]float`. What the
//@ name adds is the slots' names, as Dado's do: `p.x` reads slot 0 and `p.y`
//@ slot 1, exactly as `p[0]` and `p[1]` read them. `entities.Vec2(1.0,
//@ 2.0)` makes one, a fixed array or a tuple of its arguments, one per slot
//@ in order or by the slots' names, as Dado's `Vec2(1, 2)` does. A slot
//@ is written as `p[0]` is, `p.x = 3.0`, whether the shape is held as a
//@ fixed array or, of mixed slots, as a tuple. The names travel with what
//@ was written: a local, a parameter or a field declared with the
//@ declaration's name, a Dado function answering one, a construction, a
//@ slot whose own type is a declaration (`r.pos.x`), and an operator over
//@ one, `(p + q).x`, as in Dado. A declaration whose type does not cross
//@ is not named (`ERR1208`).
