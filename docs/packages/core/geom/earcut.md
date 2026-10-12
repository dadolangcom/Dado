<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
# core:geom/earcut

Polygon triangulation by ear clipping, holes included.

A port of mapbox/earcut 3.2.4 (ISC licence, Copyright (c) 2026 Mapbox; the
notice is in this directory's LICENSE). It answers the reference's indices
for f64 input:

    [8]f64 quad = [10.0, 0.0, 0.0, 50.0, 60.0, 60.0, 70.0, 10.0]
    []u32 no_holes
    try ref []u32 tris = earcut.triangulate(quad[:], no_holes) else:
        return 1
    defer #delete(tris)   // [1, 0, 3, 1, 3, 2]

Self-intersecting input still answers triangles; `deviation` measures how
far they are from the polygon's area.

**Errors.** `triangulate` fails with a member of `Error` (codes 1600 to 1699), and
`message(why)` is a sentence for one.

**Tier N W H.** Pure Dado over `core:math`: it builds and runs freestanding (`#NONE`), on the
web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** Every public function, `triangulate`, `deviation` and
`message`, each promised by the `#assert_raises` on the line above it, and with them every helper they reach.

## Declarations

51 declarations, 4 public.

* `distinct enum i32 Error` — Why `triangulate` failed. The numbers are this package's, 1600 to 1699,…
* `string8 message(Error why)` — A sentence for each of this package's codes, and one for any other value.
* `Error!ref []u32 triangulate([]$T data, []u32 holes, i32 dim = 2) where T is float` — Triangulates a polygon given as flat coordinates, `dim` per vertex, of…
* `f64 deviation([]$T data, []u32 holes, i32 dim, []u32 triangles) where T is float` — How far `triangles` is from covering the polygon: the difference between…
