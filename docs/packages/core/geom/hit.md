<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:geom/hit

Ray and box tests for picking and collision, in 3D.

    [3]f64 a = (0.0, 0.0, 0.0)
    [3]f64 b = (1.0, 0.0, 0.0)
    [3]f64 c = (0.0, 1.0, 0.0)
    (f64 t, f64 u, f64 v, bool hit) = hit.ray_triangle((0.25, 0.25, 2.0), (0.0, 0.0, -1.0), a, b, c, false)
    // hit: t = 2, u = 0.25, v = 0.25

The ray tests are generic over f32 and f64. A ray is an origin and a direction that need not be
unit length, and every t answered is in units of that direction. Each test is exact at its
boundary, with no tolerance, and a boundary point counts as a hit. The 2D segment and point
tests are in core:geom/line and core:geom/point.

## Declarations

6 declarations, 6 public.

* `($T t_near, T t_far, bool hit) ray_aabb([3]T origin, [3]T inv_dir, [3]T lo, [3]T hi, T t_max) where T is float` — Where the ray origin + t * dir enters and leaves the box [lo, hi] (the slab test), clipped…
* `[3]$T inverse_dir([3]T dir) where T is float` — 1 / dir per component, for `ray_aabb`. A zero component answers an infinity of its sign.
* `($T t, T u, T v, bool hit) ray_triangle([3]T origin, [3]T dir, [3]T a, [3]T b, [3]T c, bool two_sided) where T is float` — Where the ray origin + t * dir meets triangle a, b, c (Moller-Trumbore): t >= 0, and the…
* `($T t, bool hit) ray_plane([3]T origin, [3]T dir, [3]T normal, T offset) where T is float` — Where the ray origin + t * dir meets the plane dot(normal, x) == offset, with t >= 0. A ray…
* `($T t, bool hit) ray_sphere([3]T origin, [3]T dir, [3]T center, T radius) where T is float` — The first t >= 0 where the ray origin + t * dir meets the sphere: the entry, or the exit when…
* `bool aabb_aabb([$N]$T alo, [N]T ahi, [N]T blo, [N]T bhi) where T is scalar` — Whether boxes [alo, ahi] and [blo, bhi] overlap, touching included, in any dimension.
