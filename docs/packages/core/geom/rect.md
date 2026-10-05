<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:geom/rect

## Declarations

24 declarations, 24 public.

* `type Rect: [2][2]i32` — A rectangle as (position, size): rect[0] = (x, y), rect[1] = (w, h). A…
* `Rect rect(i32 x, i32 y, i32 w, i32 h)` — Build a Rect from x, y, w, h.
* `$T rx([2][2]T r) where T is scalar`
* `$T ry([2][2]T r) where T is scalar`
* `$T rw([2][2]T r) where T is scalar`
* `$T rh([2][2]T r) where T is scalar`
* `bool contains_rect([2]$T outer_pos, outer_size, inner_pos, inner_size) where T is scalar`
* `[2][2]$T inset([2][2]T r, T dx, dy) where T is scalar` — Shrink a rect by `dx` on each side and `dy` on top and bottom, clamped so it…
* `[2][2]$T margin([2][2]T r, T left, top, right, bottom) where T is scalar` — Shrink a rect by explicit per-side margins (left, top, right, bottom),…
* `[2][2]$T grow([2]T position, extent, T dt) where T is scalar`
* `[2][2]$T grow_sides([2]T position, extent, T left, top, right, bottom) where T is scalar` — Standard ordering: left, top, right, bottom…
* `[2][2]$T grow_to_point([2]T position, extent, point) where T is scalar`
* `[2][2]$T grow_to_rect([2]T position, extent, to_position, to_size) where T is scalar`
* `[2][2][2]$T vslice([2]T position, extent, T distance) where T is scalar`
* `[2][2][2]$T hslice([2]T position, extent, T distance) where T is scalar`
* `[2][2]$T positive([2]T position, extent) where T is scalar`
* `$T area([2]T position, extent) where T is scalar`
* `$T perimeter([2]T position, extent) where T is scalar`
* `[2][2]$T stackv([2][2]T area, i32 n, gap, i) where T is scalar` — The i-th of `n` equal rows stacked down `area`, with `gap` blank rows…
* `[2][2]$T stackh([2][2]T area, i32 n, gap, i) where T is scalar` — The i-th of `n` equal columns across `area`, with `gap` blank columns…
* `[2][2]$T grid([2][2]T area, i32 cols, rows, gx, gy, col, row) where T is scalar` — The cell at (col, row) of a `cols`×`rows` grid over `area`, with `gx`/`gy`
* `enum i32 Align: (Start, Center, End)` — Where a fixed-size box sits inside a larger allocation.
* `[2][2]$T align_in([2][2]T parent, T w, h, Align ax, ay) where T is scalar` — Place a `w`×`h` box inside `parent`, aligned on each axis.
* `[2][2]$T center_in([2][2]T parent, T w, h) where T is scalar` — A `w`×`h` box centred in `parent`.
