<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
# core:geom/xform

A 2D affine transform: translation, rotation and a per-axis scale, as `app:graph` composes a
scene node into its parent's space.

    xform.Transform2D parent = xform.from_trs((10.0, 0.0), 0.0, (1.0, 1.0))
    xform.Transform2D child = xform.from_trs((0.0, 0.0), 0.0, (2.0, 2.0))
    [2]f32 p = xform.transform_point(parent * child, (1.0, 1.0))   // (12, 2)

A `Transform2D` is a `distinct` homogeneous 3x3 matrix, `[3,3]f32`, stored column-major. A
distinct type over a matrix keeps the matrix's `*`, so `parent * child` is the child's
transform in the parent's space and composition is the language's own matrix product. The
type hides no slot: convert to `Mat3` to read or build the numbers by hand.

It holds translation, rotation and scale and no skew. `from_trs` builds one from the three,
and `position_of`, `rotation_of` and `scale_of` read them back. They read back exactly what
`from_trs` built, but composing a rotation onto an earlier non-uniform scale makes shear, which
three numbers cannot hold: there `rotation_of` and `scale_of` answer the angle and the lengths
of the transformed axes, and the shear is lost. A reflection reads back as a positive scale
and a turned rotation.

It is `f32` only, the precision `app:draw` and `app:graph` draw at. A `distinct` type needs a
concrete type under it (ERR0401 refuses a category such as `float` there), so a generic
version would be a second type with a second copy of every function, and no caller needs one.

**Tier N W H.** Pure Dado over `core:linalg`: it builds and runs freestanding (`#NONE`), on the
web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** The functions that are arithmetic on the matrix:
`transform_point`, `transform_vector`, `position_of` and `inverse`, each promised by the
`#assert_raises` on the line above it. `from_trs`, `rotation_of` and `scale_of` are not: they
reach the C library's `cos`, `sin`, `atan2` and `sqrt` through `core:linalg` wherever there
is one.

## Declarations

10 declarations, 10 public.

* `type Mat3: [3,3]f32` — The matrix under `Transform2D`, column-major: convert a transform to it to read or build the…
* `distinct type Transform2D: [3,3]f32` — A 2D affine transform: scale, then rotate, then translate, as a homogeneous 3x3 matrix.
* `const Transform2D IDENTITY = Transform2D([ [1.0, 0.0, 0.0], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0], ])` — The identity transform: no rotation, unit scale, zero translation.
* `Transform2D from_trs([2]f32 position, f32 rotation, [2]f32 scale)` — The transform that scales by `scale` along each axis, then rotates by `rotation` (radians,…
* `[2]f32 transform_point(Transform2D t, [2]f32 p)` — `p` moved by `t`: scaled, rotated and translated.
* `[2]f32 transform_vector(Transform2D t, [2]f32 v)` — `v` moved by `t`'s scale and rotation only, without its translation: for a direction or an…
* `[2]f32 position_of(Transform2D t)` — The translation of `t`: where it moves the origin, the matrix's third column.
* `f32 rotation_of(Transform2D t)` — The rotation of `t` in radians, in (-pi, pi]: the angle of the transformed +x axis. The angle…
* `[2]f32 scale_of(Transform2D t)` — The scale of `t`: the length of each transformed axis, never negative. A reflection reads as a…
* `Transform2D inverse(Transform2D t)` — The transform that undoes `t`: `transform_point(inverse(t), transform_point(t, p))` is `p`, up…
