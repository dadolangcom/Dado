<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:geom/bvh

A static bounding volume hierarchy over 3D boxes: built once, queried for the nearest ray hit and
for every box overlapping a query box, and refitted in place when the boxes move.

    try bvh.Bvh tree = bvh.build(boxes[:])
    defer bvh.destroy(&tree)
    (i32 item, f32 t, bool hit) = bvh.raycast(&tree, origin, dir, 1000.0, (leaf_test, rawptr(&scene)), (nil, nil))

An item is the index of its box in the slice `build` was given; what it stands for (a triangle,
a widget) is the caller's, and the caller's leaf test decides whether a ray hits it. The tree is
built by binned SAH, and by median splits past depth 64, which bounds its depth. It is f32
throughout, the precision of the vertex data such a tree indexes.

## Declarations

24 declarations, 16 public.

* `const i32 NO_MEMORY = 1` — The code `build` and `overlapping` fail with: an allocation failed.
* `const i64 MAX_ITEMS = 1 << 30` — The most boxes `build` takes: node and item indices are i32, and a tree has 2n - 1 nodes.
* `type Box: ([3]f32 lo, [3]f32 hi)` — An axis-aligned box: its lowest and its highest corner.
* `type Node` — One node of the flat tree, with the box around everything under it.
* `type Bvh` — A tree made by `build` and ended by `destroy`. `nodes[0]` is the root, and a child's index is…
* `type RayHit: (f32 t, bool hit)` — What a leaf test answers: whether the ray hits the item before t_max, and at what t.
* `type RayFn: RayHit([3]f32, [3]f32, i32, f32, rawptr)` — A leaf test, called as fn(origin, dir, item, t_max, data) for each item a ray reaches. Named…
* `type RayTest: (RayFn fn, rawptr data)` — A leaf test and the data `raycast` hands back to it unchanged. A function and its data stand…
* `type SkipFn: bool(i32, rawptr)` — An exclusion filter, called as fn(item, data): true leaves the item out.
* `type Skip: (SkipFn fn, rawptr data)` — An exclusion filter and its data. A nil `fn` leaves nothing out.
* `Box triangle_box([3]f32 a, [3]f32 b, [3]f32 c)` — The box around triangle a, b, c.
* `!Bvh build([]Box boxes)` — Builds a tree over `boxes` on `#default`; the caller ends it with `destroy`. Item i is…
* `void destroy(^Bvh b)` — Frees the tree's runs and leaves it empty.
* `(i32 item, f32 t, bool hit) raycast(^const Bvh b, [3]f32 origin, [3]f32 dir, f32 t_max, RayTest test, Skip skip)` — The nearest item the ray origin + t * dir hits with t < t_max, as `test` decides, leaving out…
* `!i32 overlapping(^const Bvh b, []Box boxes, [3]f32 lo, [3]f32 hi, ^ref []i32 out)` — Appends to `out` every item whose box in `boxes` (the slice the tree was built or last refitted…
* `void refit(^Bvh b, []Box boxes)` — Recomputes every node's box from `boxes`, the same items moved, children before parents. The…
