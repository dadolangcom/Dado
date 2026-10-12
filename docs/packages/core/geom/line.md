<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
# core:geom/line

Where two lines, rays or segments in the plane cross, for any float.

    [2]f64 a0 = (-1.0, -1.0)
    [2]f64 a1 = (1.0, 1.0)
    ([2]f64 at, bool hit) = line.segment_segment_intersect(a0, a1, (-1.0, 1.0), (1.0, -1.0))
    // at is (0, 0), hit is true

Each of the three kinds is given by two points, p0 and p1. A line runs through both without
end; a ray starts at p0 and runs through p1 without end; a segment runs from p0 to p1, both
ends included. Every function answers the crossing and whether there is one, with (0, 0) and
`false` for a miss. Lines that are parallel never cross here, even when they lie on each
other; `segment_segment_parameters` is the one function that finds where collinear segments
overlap.

The parallel test is exact: two directions are parallel when their 2D cross product is 0.0,
and nothing else is. Directions a rounding error apart are not parallel, and their crossing
can lie very far away. A caller that wants a tolerance asks `nearly_parallel` first.

**Tier N W H.** Pure Dado over `core:linalg`: it builds and runs freestanding
(`#NONE`), on the web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** Every public function, each promised by the `#assert_raises`
on the line above it and checked at each instantiation a program makes: the arithmetic here
is a script's own.

## Declarations

10 declarations, 8 public.

* `bool nearly_parallel([2]$T a0, a1, b0, b1, T eps) where T is float` — Whether the direction of a0-a1 and the direction of b0-b1 are within `eps` of parallel,…
* `([2]$T at, bool hit) line_line_intersect([2]T a0, a1, b0, b1) where T is float` — Where the line through a0 and a1 crosses the line through b0 and b1. Parallel lines miss.
* `([2]$T at, bool hit) ray_ray_intersect([2]T a0, a1, b0, b1) where T is float` — Where the ray from a0 through a1 crosses the ray from b0 through b1. Each ray includes its…
* `([2]$T at, bool hit) segment_segment_intersect([2]T a0, a1, b0, b1) where T is float` — Where segment a0-a1 crosses segment b0-b1, ends included. Parallel segments miss, even…
* `([2]$T at, bool hit) line_ray_intersect([2]T l0, l1, r0, r1) where T is float` — Where the line through l0 and l1 crosses the ray from r0 through r1. Parallel ones miss.
* `([2]$T at, bool hit) line_segment_intersect([2]T l0, l1, s0, s1) where T is float` — Where the line through l0 and l1 crosses segment s0-s1, ends included. Parallel ones miss.
* `([2]$T at, bool hit) ray_segment_intersect([2]T r0, r1, s0, s1) where T is float` — Where the ray from r0 through r1 crosses segment s0-s1, ends included. Parallel ones miss.
* `($T t, T u, bool hit) segment_segment_parameters([2]T a0, a1, b0, b1) where T is float` — Where segment a0-a1 meets segment b0-b1, as parameters: a0 + t*(a1 - a0) == b0 + u*(b1 - b0),…
