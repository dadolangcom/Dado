<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:geom/tess

Polygon tessellation with winding rules.

A port of libtess2 (SGI Free Software License B; the notice is in this
directory's LICENSE). Contours of any winding, overlapping or
self-intersecting, go in; the regions a winding rule keeps come out as
triangles, convex polygons of up to `poly_size` vertices, or boundary
contours (CCW outers, CW holes). `core:geom/clip` builds its set
operations on it, and `core:geom/polygon` its outline offset.

A `Tess` is made by `init`, fed by `add_contour`, run by `tesselate` and
released by `destroy`; it must not be copied once made. Its working mesh
lives in an arena on the `#default` that `init` saw; the output arrays are
minted on the `#default` that `tesselate` sees, and `destroy` frees both.

## Declarations

138 declarations, 12 public.

* `const i32 MALFORMED = 1` — `add_contour`, `add_points` and `tesselate` fail with MALFORMED for a…
* `const i32 NO_MEMORY = 2` — The code for an allocation that answered nil.
* `const i32 UNDEF = -1` — An unused polygon slot, a missing neighbour, or a vertex made by an intersection.
* `const f64 MAX_COORD = 2251799813685248.0` — The largest coordinate magnitude the sweep accepts.
* `enum i32 Winding: (Odd, Nonzero, Positive, Negative, Abs_Geq_Two)` — Which regions are inside, by winding number: odd, nonzero, positive,…
* `enum i32 Element: (Polygons, Connected_Polygons, Boundary_Contours)` — What `tesselate` outputs. Polygons: `poly_size` vertex indices each, padded…
* `type Tess` — A tesselator. Set the options before adding contours, and read the output…
* `void init(^Tess t)` — Makes `t` ready, with the default options and no contours. Its working mesh…
* `void destroy(^Tess t)` — Frees `t`'s output arrays and its working mesh.
* `!void add_contour(^Tess t, []f64 coords, i32 size = 2)` — Adds one closed contour of `size` (2 or 3) coordinates per vertex. Fails…
* `!void add_points(^Tess t, [][2]$T ring) where T is float` — Adds one closed contour of 2D points, of any float width; the tesselator…
* `!void tesselate(^Tess t, Winding rule, Element kind, i32 poly_size = 3, i32 vertex_size = 2)` — Tessellates every contour added since the last call, replacing the output…
