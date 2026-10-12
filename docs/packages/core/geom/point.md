<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
# core:geom/point

Whether a point in the plane lies on a line, ray or segment, or inside a triangle, rectangle
or polygon, within a tolerance; the winding number of a polygon about a point; and the nearest
point of a segment, in any dimension. For any float, and `in_rect` for any number.

    [4][2]f64 square = [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)]
    [2]f64 p = (0.5, 0.5)
    bool inside = point.in_polygon(p, square[:], 0.000001)   // true

A line, ray or segment is given by two points, as `core:geom/line` gives them: a line runs
through both, a ray starts at the first, a segment runs between them. `eps` is a distance
everywhere but `in_triangle`, which says what it bounds there:
a point within `eps` of the shape counts as on it, and an `eps` below zero accepts nothing a
distance test would. Distances are compared squared, so no function here takes a square root.

**Tier N W H.** Pure Dado over `core:linalg` and `core:math`: it builds and runs freestanding
(`#NONE`), on the web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** Every public function, each promised by the `#assert_raises`
on the line above it and checked at each instantiation a program makes.

## Declarations

10 declarations, 8 public.

* `bool on_line([2]$T point, l0, l1, T eps) where T is float` — Whether `point` lies within `eps` of the line through l0 and l1. When l0 and l1 are the same…
* `bool on_ray([2]$T point, r0, r1, T eps) where T is float` — Whether `point` lies within `eps` of the ray from r0 through r1: within `eps` of its line,…
* `bool on_segment([2]$T point, s0, s1, T eps) where T is float` — Whether `point` lies within `eps` of segment s0-s1: within `eps` of its line, with its…
* `bool in_triangle([2]$T point, t0, t1, t2, T eps) where T is float` — Whether `point` lies inside triangle t0 t1 t2, of either orientation, edges included. `eps`
* `bool in_rect([2]$T point, rect_position, rect_size, T eps) where T is scalar` — Whether `point` lies in the rectangle at `rect_position` with size `rect_size`, edges…
* `bool in_polygon([2]$T point, [][2]T polygon, T eps) where T is float` — Whether `point` lies inside `polygon`, implicitly closed and of either orientation, by the…
* `i32 winding_number([2]$T point, [][2]T polygon) where T is float` — The winding number of `polygon` (implicitly closed, either orientation) about `point`:
* `([$N]$T closest, T t) closest_point_on_segment([N]T point, s0, s1) where T is float` — The point of segment s0-s1 nearest `point`, in any dimension, and its parameter `t` in [0, 1]…
