<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
# core:geom/tess

Polygon tessellation with winding rules.

A port of libtess2 (SGI Free Software License B; the notice is in this
directory's LICENSE). Contours of any winding, overlapping or
self-intersecting, go in; the regions a winding rule keeps come out as
triangles, convex polygons of up to `poly_size` vertices, or boundary
contours (CCW outers, CW holes). `core:geom/clip` builds its set
operations on it, and `core:geom/polygon` its outline offset.

A `Tess` is made by `init`, fed by `add_contour`, run by `tessellate` and
ended by `destroy`; it must not be copied once made. Its working mesh
lives in an arena on the `#default` that `init` saw; the output arrays are
made on the `#default` that `tessellate` sees, and `destroy` frees both.

**Allocation.** `init` captures the `#default` it sees for the tesselator's
whole life, rather than each call taking the `#default` of its moment: the
working mesh is one arena that every add and every run carves from, and a
`using` between `init` and `destroy` must not split it across allocators.

**Errors.** `add_contour`, `add_points` and `tessellate` fail with a member
of `Error` (codes 1800 to 1899), and `message(why)` is a sentence for one.
`core:geom/clip` and `core:geom/polygon` fail with the same codes.

**Tier N W H.** Pure Dado over `core:math` and `core:mem/arena`: it builds and runs
freestanding (`#NONE`), on the web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** `message` alone, promised by the `#assert_raises` on the line
above it. Every other public function reaches the arena that holds the working mesh (`init`
makes it, `destroy` frees it, the others allocate from it or reset it), and an arena's
allocator is a procedure and a `rawptr`, an address a script has no value for. Making the
tesselator raise means allocating its mesh some other way (an index pool, after 1.0: the
working state is one private slot, `work`, so that rewrite changes no public name).

## Declarations

139 declarations, 12 public.

* `distinct enum i32 Error` — Why an add or a run failed. The numbers are this package's, 1800 to 1899,…
* `string8 message(Error why)` — A sentence for each of this package's codes, and one for any other value.
* `const i32 UNDEF = -1` — An unused polygon slot, a missing neighbour, or a vertex made by an intersection.
* `const f64 MAX_COORD = 2251799813685248.0` — The largest coordinate magnitude the sweep accepts.
* `enum i32 Winding: (Odd, Nonzero, Positive, Negative, AbsGeqTwo)` — Which regions are inside, by winding number: odd, nonzero, positive,…
* `enum i32 Element: (Polygons, ConnectedPolygons, BoundaryContours)` — What `tessellate` outputs. Polygons: `poly_size` vertex indices each, padded…
* `type Tess` — A tesselator. Set the options before adding contours, and read the output…
* `void init(^Tess t)` — Makes `t` ready, with the default options and no contours. Its working mesh…
* `void destroy(^Tess t)` — Frees `t`'s output arrays and its working mesh.
* `Error!void add_contour(^Tess t, []f64 coords, i32 size = 2)` — Adds one closed contour of `size` (2 or 3) coordinates per vertex. Fails…
* `Error!void add_points(^Tess t, [][2]$T ring) where T is float` — Adds one closed contour of 2D points, of any float width; the tesselator…
* `Error!void tessellate(^Tess t, Winding rule, Element kind, i32 poly_size = 3, i32 vertex_size = 2)` — Tessellates every contour added since the last call, replacing the output…
