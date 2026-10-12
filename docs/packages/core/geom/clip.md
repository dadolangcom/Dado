<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
# core:geom/clip

Polygon set operations, triangulation and cleanup, for any float.

Polygons are closed rings, CCW in a y-up frame (positive signed area); a
CW ring is a hole. A set of rings is `points` with `starts`: ring k is
points[starts[k] ..< starts[k + 1]], the last running to the end, and no
starts is one ring.

    [8][2]f64 ab = [(0.0, 0.0), (2.0, 0.0), (2.0, 2.0), (0.0, 2.0), (1.0, 1.0), (3.0, 1.0), (3.0, 3.0), (1.0, 3.0)]
    [2]u32 two = [0, 4]
    clip.Contours(f64) u
    try clip.union(ab[:], two[:], &u) else:
        return 1
    defer clip.destroy(&u)   // one ring of 8 points, area 7

The set operations run on `core:geom/tess`, which works in f64: points of
another width are widened as they go in, and answers are written at the
input's width. They fail with tess's codes, `tess.Error`.

**The set operations write their answer to `out`**, a `Contours(T)` the
caller declares and ends with `destroy`, rather than return it. That is the
1.0 shape, adopted deliberately: a failable function answering a generic
type, `tess.Error!Contours(T) f(…)`, loses its failure in the compiler today
(ERR0714), and the shape that returns the set will come beside these as an
addition once it does not, without breaking a caller of these.

**Tier N W H.** Pure Dado over `core:geom/earcut` and `core:geom/tess`: it builds and runs
freestanding (`#NONE`), on the web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** The functions that read or free a ring set without making one:
`destroy`, `count`, `contour`, `signed_area`, `area` and `simplify`, each promised by the
`#assert_raises` on the line above it. The set operations (`boundary`, `union`,
`intersect`, `difference`, `xor`, `union_fuzzy`) and the triangulators (`triangulate`,
`triangulate_polygons`) are not: they run on `core:geom/tess`, whose working mesh lives in a
`core:mem/arena`, and an arena's allocator carries a `rawptr`, an address a script has no
value for (ERR1227 names that chain).

## Declarations

28 declarations, 16 public.

* `type Contours($T)` — A set of rings answered by an operation, in the layout of its input: outer…
* `void destroy(^Contours($T) c)` — Frees `c`'s points and starts.
* `i64 count(^Contours($T) c)` — The number of rings in `c`.
* `[][2]$T contour(^Contours(T) c, i64 k)` — Ring k of `c`, a view into its points; k is below `count(c)`.
* `enum i32 Mode: (Robust, Fast)` — Which triangulator `triangulate` uses: the tesselator (any input, f64…
* `$T signed_area([][2]T polygon) where T is float` — The area a ring encloses, positive when it winds CCW (y up) and negative…
* `$T area([][2]T polygon) where T is float` — The area a ring encloses, whatever its winding.
* `tess.Error!ref [][$N]$T simplify([][N]T polygon, T eps) where T is float` — Drops every vertex within `eps` of the line through its neighbours,…
* `tess.Error!void boundary([][2]$T points, []u32 starts, tess.Winding rule, ^Contours(T) out) where T is float` — Writes to `out` the boundary of the region `rule` keeps over every ring of…
* `tess.Error!void union([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the union of the set, everywhere any ring covers, as…
* `tess.Error!void intersect([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the regions covered at least twice: for two rings, their…
* `tess.Error!void difference([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the first ring minus every other: exact when the cutters do…
* `tess.Error!void xor([][2]$T points, []u32 starts, ^Contours(T) out) where T is float` — Writes to `out` the regions covered an odd number of times. The answer and…
* `tess.Error!void union_fuzzy([][2]$T points, []u32 starts, T fuzz, ^Contours(T) out) where T is float` — Writes to `out` the union of the set welded at `fuzz`: every vertex snaps to…
* `tess.Error!ref [][3][2]$T triangulate([][2]T polygon, Mode mode = Mode.Robust) where T is float` — The triangles of one ring, on `#default`, the caller's to `#delete`.
* `tess.Error!ref [][3][2]$T triangulate_polygons([][2]T points, []u32 starts) where T is float` — Triangles covering the odd-winding region of a set of rings, on `#default`,…
