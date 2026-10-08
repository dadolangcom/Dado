<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:geom/polygon

One closed ring: its offset, and the normal of a 3D ring.

`offset` moves each vertex to where its two edges, each moved by the same
distance, meet: the ring keeps its vertices, and a large or inward
distance can make it cross itself. `offset_outline` answers the region
such an offset bounds, as a set of rings:

    [4][2]f64 square = [(0.0, 0.0), (2.0, 0.0), (2.0, 2.0), (0.0, 2.0)]
    clip.Contours(f64) grown
    try polygon.offset_outline(square[:], 1.0, polygon.Join.Miter, &grown) else:
        return 1
    defer clip.release(&grown)   // one ring, (-1, -1) to (3, 3)

The outline is by winding numbers (Chen and McMains): each edge is pushed
out by its own distance, every corner gets a join or a V through the
original vertex, and `core:geom/tess` keeps the region the winding rule
says is inside. The result may split, merge or drop rings, so it is a set.

## Declarations

22 declarations, 6 public.

* `void offset(ref [][2]$T polygon, T dt) where T is float` — Moves each vertex of a closed ring, in place, to where its two edges meet…
* `ref [][2]$T offset_by([][2]T polygon, T dt, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc) where T is float` — A copy of `polygon` on `alloc`, the caller's to `#delete`, moved as `offset`
* `enum i32 Join: (Miter, Bevel, Round)` — How the offset edges meet at a corner they turn away from.
* `!void offset_outline_edges([][2]$T polygon, []T deltas, Join join, ^clip.Contours(T) out, T arc_resolution = 0.1, T miter_limit = 0.0) where T is float` — The outline of a closed ring whose edges are each offset by their own…
* `!void offset_outline([][2]$T polygon, T delta, Join join, ^clip.Contours(T) out, T arc_resolution = 0.1, T miter_limit = 0.0) where T is float` — The outline of a closed ring offset by `delta` on every edge, written the result to…
* `[3]$T normal([][3]T profile) where T is float` — The unit normal of a 3D ring by Newell's method, on the side from which the…
