<!-- dadoc 1.0.0-rc.8 -->
<!-- commit 015d59b2e2c4 (dirty) -->
# core:geom/bvh

A static bounding volume hierarchy over 3D boxes: built once, queried for the nearest ray hit and
for every box overlapping a query box, and refitted in place when the boxes move.

    type Tris using bvh.RayTest:
        [][3]f32 corners
        (f32 t, bool hit) hit_item(^self, [3]f32 origin, [3]f32 dir, i32 item, f32 t_max):
            i32 k = 3 * item
            (f32 t, _, _, bool h) = hit.ray_triangle(origin, dir, self.corners[k], self.corners[k + 1], self.corners[k + 2], true)
            return (t, h && t < t_max)

    try bvh.Bvh tree = bvh.build(boxes[:])
    defer bvh.destroy(&tree)
    Tris scene = (corners)
    (i32 item, f32 t, bool hit) = bvh.raycast(&tree, origin, dir, 1000.0, &scene)

An item is the index of its box in the slice `build` was given; what it stands for (a triangle,
a widget) is the caller's, and the caller's `RayTest` decides whether a ray hits it and which
items a ray leaves out. The tree is built by binned SAH, and by median splits past depth 64,
which bounds its depth. It is f32 throughout, the precision of the vertex data such a tree
indexes.

**Errors.** `build` and `overlapping` fail with a member of `Error` (codes 1500 to 1599), and
`message(why)` is a sentence for one.

**Tier N W H.** Pure Dado over `core:linalg` and `core:geom/hit`: it builds and runs
freestanding (`#NONE`), on the web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** Every public function, each promised by the `#assert_raises` on
the line above it: `raycast` at each `RayTest` a program hands it, so an implementer whose
methods reach C or hold an address breaks the promise at the call that passes it.

## Declarations

21 declarations, 13 public.

* `distinct enum i32 Error` — Why `build` or `overlapping` failed. The numbers are this package's, 1500 to 1599, apart from…
* `string8 message(Error why)` — A sentence for this package's code, and one for any other value.
* `const i64 MAX_ITEMS = 1 << 30` — The most boxes `build` takes: node and item indices are i32, and a tree has 2n - 1 nodes.
* `type Box: ([3]f32 lo, [3]f32 hi)` — An axis-aligned box: its lowest and its highest corner.
* `type Node` — One node of the flat tree, with the box around everything under it.
* `type Bvh` — A tree made by `build` and ended by `destroy`. `nodes[0]` is the root, and a child's index is…
* `trait RayTest` — What `raycast` asks about the items a ray reaches: the caller's, since only the caller knows…
* `Box triangle_box([3]f32 a, [3]f32 b, [3]f32 c)` — The box around triangle a, b, c.
* `Error!Bvh build([]Box boxes)` — Builds a tree over `boxes` on `#default`; the caller ends it with `destroy`. Item i is…
* `void destroy(^Bvh b)` — Frees the tree's runs and leaves it empty.
* `(i32 item, f32 t, bool hit) raycast(^const Bvh b, [3]f32 origin, [3]f32 dir, f32 t_max, ^RayTest test)` — The nearest item the ray origin + t * dir hits with t < t_max, as `test.hit_item` decides,…
* `Error!i32 overlapping(^const Bvh b, []Box boxes, [3]f32 lo, [3]f32 hi, ^ref []i32 out)` — Appends to `out` every item whose box in `boxes` (the slice the tree was built or last refitted…
* `void refit(^Bvh b, []Box boxes)` — Recomputes every node's box from `boxes`, the same items moved, children before parents. The…
