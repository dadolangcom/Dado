<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
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

## Declarations

54 declarations, 4 public.

* `const i32 MALFORMED = 1` — `triangulate` fails with MALFORMED when `dim` is below 2, the data ends…
* `const i32 NO_MEMORY = 2` — `triangulate` fails with NO_MEMORY when an allocation answers nil.
* `!ref []u32 triangulate([]$T data, []u32 holes, i32 dim = 2) where T is float` — Triangulates a polygon given as flat coordinates, `dim` per vertex, of…
* `f64 deviation([]$T data, []u32 holes, i32 dim, []u32 triangles) where T is float` — How far `triangles` is from covering the polygon: the difference between…
