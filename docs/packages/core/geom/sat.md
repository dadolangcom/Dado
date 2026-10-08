<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:geom/sat

Separating-axis tests for convex polygons in 2D: whether two overlap, and the shortest push that
separates them.

    [4][2]f64 a = [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)]
    [4][2]f64 b = [(0.75, 0.0), (1.75, 0.0), (1.75, 1.0), (0.75, 1.0)]
    ([2]f64 dir, f64 depth, bool hit) = sat.mtv(a[:], b[:], 0.0)
    // hit: dir = (-1, 0), depth = 0.25

A polygon is a run of points in either winding, implicitly closed; one point is a point and two
are a segment. The axes tested are the edge normals of both shapes. `tol` is the overlap that
still counts as apart: 0 makes touching apart, a positive value ignores grazing contact, and a
negative one asks for clearance. A concave shape goes in as convex parts (`overlap_parts`,
`mtv_parts`). Every function is generic over f32 and f64.

## Declarations

11 declarations, 6 public.

* `bool overlap([][2]$T a, [][2]T b, T tol) where T is float` — Whether convex polygons `a` and `b` overlap by more than `tol`. O((n + m)^2) for n and m…
* `([2]$T dir, T depth, bool hit) mtv([][2]T a, [][2]T b, T tol) where T is float` — The minimum translation vector of convex polygons `a` and `b`: moving `a` by dir * depth…
* `bool overlap_box([][2]$T a, [2]T lo, [2]T hi, T tol) where T is float` — Whether convex polygon `a` overlaps the axis-aligned box [lo, hi] by more than `tol`: `overlap`
* `([2]$T dir, T depth, bool hit) mtv_box([][2]T a, [2]T lo, [2]T hi, T tol) where T is float` — `mtv` of convex polygon `a` against the axis-aligned box [lo, hi]: moving `a` by dir * depth…
* `bool overlap_parts([][2]$T a, []u32 a_starts, [][2]T b, []u32 b_starts, T tol) where T is float` — Whether any convex part of `a` overlaps any convex part of `b` by more than `tol`. A parts set…
* `([2]$T dir, T depth, bool hit) mtv_parts([][2]T a, []u32 a_starts, [][2]T b, []u32 b_starts, T tol) where T is float` — One push that separates every overlapping pair of parts, for parts sets as `overlap_parts`
