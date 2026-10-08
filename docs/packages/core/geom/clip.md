<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:geom/clip

Polygon set operations, triangulation and cleanup, for any float.

Polygons are closed rings, CCW in a y-up frame (positive signed area); a
CW ring is a hole. A set of rings is `points` with `starts`: ring k is
points[starts[k] ..< starts[k + 1]], the last running to the end, and no
starts is one ring.

    [8][2]f64 ab = [(0.0, 0.0), (2.0, 0.0), (2.0, 2.0), (0.0, 2.0), (1.0, 1.0), (3.0, 1.0), (3.0, 3.0), (1.0, 3.0)]
    [2]u32 two = [0, 4]
    clip.Contours(f64) u
    try clip.union_polygon(ab[:], two[:], &u) else:
        return 1
    defer clip.release(&u)   // one ring of 8 points, area 7

The set operations run on `core:geom/tess`, which works in f64: points of
another width are widened as they go in, and answers are written at the
input's width. They fail with tess's codes.

## Declarations

28 declarations, 16 public.

* `type Contours($T)` — A set of rings answered by an operation, in the layout of its input: outer…
* `void release(^Contours($T) c)` — Frees `c`'s points and starts.
* `i64 count(^Contours($T) c)` — The number of rings in `c`.
* `[][2]$T contour(^Contours(T) c, i64 k)` — Ring k of `c`, a view into its points; k is below `count(c)`.
* `enum i32 Mode: (Robust, Fast)` — Which triangulator `triangulate` uses: the tesselator (any input, f64…
* `$T signed_area([][2]T polygon) where T is float` — The area a ring encloses, positive when it winds CCW (y up) and negative…
* `$T area([][2]T polygon) where T is float` — The area a ring encloses, whatever its winding.
* `!ref [][$N]$T simplify([][N]T polygon, T epsilon) where T is float` — Drops every vertex within `epsilon` of the line through its neighbours,…
* `!void boundary([][2]$T points, []u32 starts, tess.Winding rule, ^Contours(T) out) where T is float` — Writes to `out` the boundary of the region `rule` keeps over every ring of…
* `!void union_polygon([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the union of the set, everywhere any ring covers, as…
* `!void intersect([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the regions covered at least twice: for two rings, their…
* `!void difference([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the first ring minus every other: exact when the cutters do…
* `!void xor([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the regions covered an odd number of times. The answer and…
* `!void union_fuzzy([][2]$T points, []u32 starts, T fuzz, ^Contours(T) out) where T is float` — Writes to `out` the union of the set welded at `fuzz`: every vertex snaps to…
* `!ref [][3][2]$T triangulate([][2]T polygon, Mode mode = Mode.Robust) where T is float` — The triangles of one ring, on `#default`, the caller's to `#delete`.
* `!ref [][3][2]$T triangulate_polygons([][2]T points, []u32 starts) where T is float` — Triangles covering the odd-winding region of a set of rings, on `#default`,…
