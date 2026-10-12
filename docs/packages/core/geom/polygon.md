<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
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
    defer clip.destroy(&grown)   // one ring, (-1, -1) to (3, 3)

The outline is by winding numbers (Chen and McMains): each edge is pushed
out by its own distance, every corner gets a join or a V through the
original vertex, and `core:geom/tess` keeps the region the winding rule
says is inside. The result may split, merge or drop rings, so it is a set.
It is robust where `offset` is not, and far slower: on a 2000-vertex ring,
about 10,000 times.

**Errors.** `offset_by` and the outline offsets fail with `core:geom/tess`'s
codes, `tess.Error`. The outline offsets write their answer to an `out`
set rather than return it, as `core:geom/clip`'s set operations do; its
package comment says why that is the 1.0 shape.

**Tier N W H.** Pure Dado over `core:math`, `core:geom/clip` and `core:geom/tess`: it builds
and runs freestanding (`#NONE`), on the web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** Nothing yet. `offset`, `offset_by` and `normal` take square
roots through `core:math.root`, which is the C library's `sqrt` wherever there is one; they
are promised with it. The outline offsets also take angles, sines and cosines from the C
library, and run on `core:geom/tess`, whose arena allocator carries a `rawptr`.

## Declarations

22 declarations, 6 public.

* `void offset(ref [][2]$T polygon, T dt) where T is float` — Moves each vertex of a closed ring, in place, to where its two edges meet…
* `tess.Error!ref [][2]$T offset_by([][2]T polygon, T dt) where T is float` — A copy of `polygon` on `#default`, the caller's to `#delete`, moved as…
* `enum i32 Join: (Miter, Bevel, Round)` — How the offset edges meet at a corner they turn away from.
* `tess.Error!void offset_outline_edges([][2]$T polygon, []T deltas, Join join, ^clip.Contours(T) out, T arc_resolution = 0.1, T miter_limit = 0.0) where T is float` — The outline of a closed ring whose edges are each offset by their own…
* `tess.Error!void offset_outline([][2]$T polygon, T delta, Join join, ^clip.Contours(T) out, T arc_resolution = 0.1, T miter_limit = 0.0) where T is float` — The outline of a closed ring offset by `delta` on every edge, written to…
* `[3]$T normal([][3]T profile) where T is float` — The unit normal of a 3D ring by Newell's method, on the side from which the…
