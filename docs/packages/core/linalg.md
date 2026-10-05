<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:linalg

std/linalg — vector and matrix maths, written once, over generics.

Every function here is a **template**: `[$N]T` binds the arity from the
argument and `T` binds the element type, `where T is scalar`, so `dot` is one page of source
and as many C functions as a program actually calls. There is no `dot2`, no
`dot3` and no `dot4`, and there is no macro — an instantiation is ordinary
code with a literal arity, which is what makes `for i in 0..<N` emit
`for (i = 0; i < 3; i++)` and unroll at `-O2`.

    import "core:linalg"
    linalg.length(v)
    linalg.normalize(v)
    linalg.inverse4(model)

**The authority is GLSL, and glm for what GLSL leaves out.**
So `cross` is three-dimensional and nothing else, `length` is the Euclidean
norm, `reflect`/`refract`/`faceforward` are GLSL's, `outer_product` is
GLSL's `outerProduct`, and the camera matrices — `look_at`, `perspective`,
`orthographic` — carry glm's names and glm's conventions. There is no
ordering on vectors — `<` on a shape is refused for GLSL's own reason,
which is that it has three defensible meanings and no obvious one.

**Handedness and layout, stated once for the whole file.** Everything is
**right-handed** and every matrix is **column-major**, because that is how
`[R,C]T` erases: the outer index is the column, so `m[c][r]` is
row `r` of column `c`, each `[ … ]` in a matrix literal is a *column*, and the
literals read transposed from the way a textbook writes the rows. `cross`
obeys the right-hand rule (`x cross y == z`); `look_at` builds a view looking
down **−z**; `perspective`/`orthographic` map to OpenGL/GLSL clip space, with
`z` in **[−1, 1]** (glm's `_NO` / negative-one-to-one convention). Composition
is right-to-left as usual: `rot_y(h) * rot_x(p)` pitches first, then yaws.

**`#transpose(m)` is a language builtin** — `[R,C]T` to `[C,R]T` —
so it is not restated here; call it directly. `*` on a matrix is the matrix
product, `m * v` is matrix-times-column-vector, and `m * s` broadcasts a
scalar, all builtin. What this file adds is everything those operators do not.

**It generalizes over `scalar` wherever a square root is not in the way.**
`dot`, `cross`, the componentwise `min`/`max`/`clamp`/`abs`/`sign`, the matrix
`determinant`s and `trace`s are one body over `int` and `float` alike — an
integer `dot` is exact and means what it says. Only the functions that reach
libm (`length`, `normalize`, the trig, every `inverse`) narrow to `float`,
because a square root or a reciprocal has no integer answer.

**It works against a `foreign` type with nothing written at the call.**
A transparent foreign type is interchangeable with the shape it
tags, so `linalg.length(v)` on raylib's `Vector2` is this file's `[$N]T where T is float`
with N = 2 and no conversion anywhere. The same holds for a user
`distinct`: `Meters` and `[3]f32` are two instantiations and **one** emitted
function, because a tag is not in the C.

## Declarations

49 declarations, 49 public.

* `f64 sqrt(f64 x)`
* `f64 sin(f64 x)`
* `f64 cos(f64 x)`
* `f64 tan(f64 x)`
* `f64 acos(f64 x)`
* `f64 atan2(f64 y, f64 x)`
* `$T dot([$N]T a, [N]T b) where T is scalar` — The dot product: the sum of the products of corresponding slots.
* `[3]$T cross([3]T a, [3]T b) where T is scalar` — The cross product, which is **three-dimensional and nothing else** — GLSL's…
* `[N,N]$T outer_product([$N]T a, [N]T b) where T is scalar` — The outer product `a bᵀ` — GLSL's `outerProduct`. An N-vector against an…
* `$T sum([$N]T v) where T is scalar` — The sum of the slots.
* `$T min_component([$N]T v) where T is scalar` — The smallest slot — glm's `compMin`.
* `$T max_component([$N]T v) where T is scalar` — The largest slot — glm's `compMax`.
* `$T length_squared([$N]T v) where T is scalar` — The squared length. Free of a square root, which is what makes it the right…
* `$T length([$N]T v) where T is float` — The Euclidean norm.
* `$T distance_squared([$N]T a, [N]T b) where T is scalar` — The squared distance between two points — the norm-free companion to…
* `$T distance([$N]T a, [N]T b) where T is float` — The distance between two points.
* `[$N]$T min([N]T a, [N]T b) where T is scalar` — The componentwise minimum of two vectors — GLSL `min`.
* `[$N]$T max([N]T a, [N]T b) where T is scalar` — The componentwise maximum of two vectors — GLSL `max`.
* `[$N]$T clamp([N]T v, [N]T lo, [N]T hi) where T is scalar` — Each slot of `v` clamped into `[lo, hi]` slot-wise — GLSL `clamp`. The bounds…
* `[$N]$T abs([N]T v) where T is scalar` — The componentwise absolute value — GLSL `abs`.
* `[$N]$T sign([N]T v) where T is scalar` — The componentwise sign, −1 / 0 / +1 — GLSL `sign`.
* `[$N]$T normalize([N]T v) where T is float` — The unit vector in `v`'s direction, and the zero vector for a zero `v`.
* `[$N]$T clamp_length([N]T v, T limit) where T is float` — `v` scaled to a length of at most `limit`.
* `[$N]$T lerp([N]T a, [N]T b, T t) where T is float` — The linear interpolation from `a` to `b` at `t` — GLSL calls it `mix`.
* `[$N]$T reflect([N]T v, [N]T n) where T is float` — `v` reflected in the plane whose unit normal is `n` — GLSL's `reflect`.
* `[$N]$T refract([N]T v, [N]T n, T eta) where T is float` — `v` refracted through a surface of unit normal `n` at ratio of indices `eta`
* `[$N]$T faceforward([N]T n, [N]T i, [N]T nref) where T is float` — `n` flipped, if need be, to face against `i` — GLSL's `faceforward`. Returns…
* `[$N]$T project([N]T a, [N]T b) where T is float` — The vector projection of `a` onto `b` — the component of `a` that lies along…
* `[$N]$T reject([N]T a, [N]T b) where T is float` — The vector rejection of `a` from `b` — the component of `a` perpendicular to…
* `$T angle_between([$N]T a, [N]T b) where T is float` — The unqualified angle between `a` and `b`, in radians, in `[0, π]` — glm's…
* `[2]$T from_angle(T radians) where T is float` — The unit vector at `radians`, measured counter-clockwise from +x. The inverse…
* `[2]$T rotate([2]T v, T radians) where T is float` — `v` rotated by `radians` counter-clockwise. The standard 2×2 rotation, spelled…
* `$T angle_of([2]T v) where T is float` — The angle of `v`, in radians, quadrant included — `atan2`, which is the whole…
* `[N,N]$T diagonal([$N]T v) where T is scalar` — The diagonal matrix whose diagonal is `v` — and so the pure scaling matrix…
* `$T trace([$N,N]T m) where T is scalar` — The trace — the sum of the diagonal — at any square size.
* `$T determinant2([2,2]T m) where T is scalar` — The determinant at each square size. Column-major, so `m[c][r]` is row `r` of…
* `$T determinant3([3,3]T m) where T is scalar`
* `$T determinant4([4,4]T m) where T is scalar`
* `[2,2]$T inverse2([2,2]T m) where T is float`
* `[3,3]$T inverse3([3,3]T m) where T is float`
* `[4,4]$T inverse4([4,4]T m) where T is float`
* `[3,3]$T rot_x(T radians) where T is float`
* `[3,3]$T rot_y(T radians) where T is float`
* `[3,3]$T rot_z(T radians) where T is float`
* `[4,4]$T translation([3]T t) where T is float` — The translation matrix for offset `t` — glm's `translate` of the identity.
* `[4,4]$T scaling([3]T s) where T is float` — The 4×4 scaling matrix for per-axis factors `s` — glm's `scale`. The…
* `[4,4]$T look_at([3]T eye, [3]T center, [3]T up) where T is float` — The right-handed view matrix for a camera at `eye` looking at `center`, with…
* `[4,4]$T perspective(T fovy, T aspect, T near, T far) where T is float` — The right-handed perspective projection — glm's `perspective`, OpenGL clip…
* `[4,4]$T orthographic(T left, T right, T bottom, T top, T near, T far) where T is float` — The right-handed orthographic projection — glm's `ortho`, OpenGL clip space…
