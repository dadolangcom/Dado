<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:geom/xform

core:geom/xform — a 2D affine transform, the scene graph's mechanism half.

A scene graph composes its nodes out of `core:geom` transforms, and
this is that: translation + rotation + a per-axis scale, composed the way
`app:graph` needs to compose a child's transform into its parent's space.

**Stored as a homogeneous 3x3 matrix, deliberately, so composition is not
hand-written arithmetic — it is Dado's own matrix product.**
`distinct type Transform2D: [3,3]f32` keeps the `[R,C]T` matrix tag's `*`
operator — a tagged shape keeps its operators; verified live
before this file was written — a `distinct` over an already-tagged matrix
composes with `*` exactly like the bare matrix does, column-major storage
and all). So `parent * child` genuinely *is* "child's transform, in
parent's space," and `m * [x, y, 1]` genuinely *is* "apply this transform
to a point" — nothing here reimplements what the language already gives a
tagged matrix for free. `app:graph` never has to know this; it calls
`from_trs`/`transform_point`/`inverse` and multiplies two `Transform2D`s
with `*` when composing a node into its parent's space.

**A scene node's shape, not its whole surface** — Godot is "the
altitude of API to aim at… not a spec to copy": this
package gives `app:graph` translation, rotation and a per-axis scale — the
three fields Node2D's setters actually edit — and deliberately leaves out
skew. Godot's own skew is rare in practice and would turn `from_trs` from a
closed-form build into a small linear solve to keep decomposition
well-defined; nothing below needs it, and a caller who does can still reach
the raw `Mat3` and build one by hand (the type hides no field).

**Fixed at `f32`, not generic over `float`.** A `distinct type` needs a
concrete backing (a `float`/`int`/`scalar` category is never a type —
`distinct type X: [3,3]float` is refused with ERR0401, "`float` is a
numeric category and not a type").
`f32` is the graphics-precision default every downstream consumer (the
future sokol-backed renderer, `core:linalg`'s own vector ops) already
expects.

**`rotation_of`/`scale_of` decompose the matrix and can misread a
transform that was never built by `from_trs`/composition of the same** —
composing a rotation on top of a *prior* non-uniform scale bakes shear
into the combined matrix, and a TRS triple cannot represent shear. This is
not a bug to fix here; it is the same well-known limitation
`Transform2D::get_rotation`/`get_scale` have in Godot itself, from
decomposing a general affine matrix back into three numbers that cannot
always hold what the matrix actually contains.

## Declarations

10 declarations, 10 public.

* `type Mat3: [3,3]f32` — The bare matrix `Transform2D` tags — reach for this when you want to build…
* `distinct type Transform2D: [3,3]f32` — A 2D affine transform: rotate, scale, then translate, as a homogeneous 3x3…
* `const Transform2D IDENTITY = Transform2D([ [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0], ])` — The identity transform: no rotation, unit scale, zero translation.
* `Transform2D from_trs([2]f32 position, f32 rotation, [2]f32 scale)` — Build a transform from translation, rotation (radians, counter-clockwise)
* `[2]f32 transform_point(Transform2D t, [2]f32 p)` — Apply `t` to a point — the full affine map, translation included.
* `[2]f32 transform_vector(Transform2D t, [2]f32 v)` — Apply `t`'s linear part to a vector — rotation and scale, deliberately…
* `[2]f32 position_of(Transform2D t)` — The translation component — the matrix's own third column.
* `f32 rotation_of(Transform2D t)` — The rotation component, in radians — the angle of the transformed +x…
* `[2]f32 scale_of(Transform2D t)` — The scale component: the length of each transformed basis axis. Always…
* `Transform2D inverse(Transform2D t)` — The inverse transform: `transform_point(inverse(t), transform_point(t, p))`
