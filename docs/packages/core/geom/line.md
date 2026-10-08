<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:geom/line

## Declarations

9 declarations, 7 public.

* `([2]$T, bool) line_line_intersect([2]T a0, a1, b0, b1) where T is float`
* `([2]$T, bool) ray_ray_intersect([2]T a0, a1, b0, b1) where T is float`
* `([2]$T, bool) segment_segment_intersect([2]T a0, a1, b0, b1) where T is float`
* `([2]$T, bool) line_ray_intersect([2]T l0, l1, r0, r1) where T is float`
* `([2]$T, bool) line_segment_intersect([2]T l0, l1, s0, s1) where T is float`
* `([2]$T, bool) ray_segment_intersect([2]T r0, r1, s0, s1) where T is float`
* `($T t, T u, bool hit) segment_segment_parameters([2]T a0, a1, b0, b1) where T is float` — Where segment a0-a1 meets segment b0-b1, as parameters: a0 + t*(a1 - a0) == b0 + u*(b1 - b0),…
