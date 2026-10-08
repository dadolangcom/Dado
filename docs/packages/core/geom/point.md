<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:geom/point

## Declarations

10 declarations, 8 public.

* `bool point_on_line([2]$T point, l0, l1, T eps) where T is float`
* `bool point_on_ray([2]$T point, r0, r1, T eps) where T is float`
* `bool point_on_segment([2]$T point, s0, s1, T eps) where T is float`
* `bool point_in_triangle([2]$T point, t0, t1, t2, T eps) where T is float`
* `bool point_in_rect([2]$T point, rect_position, rect_size, T eps) where T is scalar` — Width-agnostic: a pure comparison, no…
* `bool point_in_polygon([2]$T point, [][2]T polygon, T eps) where T is float`
* `i32 winding_number([2]$T point, [][2]T polygon) where T is float` — The winding number of `polygon` (implicitly closed, either orientation) about `point`:
* `([$N]$T closest, T t) closest_point_on_segment([N]T point, s0, s1) where T is float` — The point of segment s0-s1 nearest `point`, in any dimension, and its parameter `t` in [0, 1]…
