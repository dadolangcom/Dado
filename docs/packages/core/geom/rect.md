<!-- dadoc 1.0.0-rc.4 -->
<!-- commit cc341d1a7513 -->
# core:geom/rect

Axis-aligned rectangles for layout and hit-testing: padding, growing, slicing, equal
division into rows, columns and grids, alignment, and overlap. For any number, so the same
body lays out a terminal's integer cells and a window's float pixels.

    rect.Rect screen = rect.rect(0, 0, 80, 24)
    rect.Rect body = rect.inset(screen, 1, 1)            // (1, 1) 78x22
    rect.Rect row = rect.stackv(body, 2, 0, 1)            // the lower of two rows
    bool hot = rect.contains(row, [40, 15])               // true

A rectangle is a position and a size: its top-left corner (x, y) and its extent (w, h), with
y growing downwards, as layout has it (`core:geom/clip`'s rings are y-up). Every function
takes it as one `[2][2]T`, `[[x, y], [w, h]]`; `Rect` is that shape at `i32`, with a
constructor. Every function works for both kinds of number: a width that would go negative is
clamped to zero where a function says so, and an integer division rounds towards zero.

**Tier N W H.** Pure Dado over `core:math`: it builds and runs freestanding
(`#NONE`), on the web, and hosted (Linux, macOS and Windows) alike.

**What raises into a script.** Every public function, each promised by the `#assert_raises`
on the line above it and checked at each instantiation a program makes.

## Declarations

27 declarations, 27 public.

* `type Rect: [2][2]i32` — A rectangle of integer cells as (position, size): rect[0] = (x, y), rect[1] = (w, h). A plain…
* `Rect rect(i32 x, i32 y, i32 w, i32 h)` — Build a Rect from x, y, w, h.
* `$T x([2][2]T r) where T is scalar` — The x of `r`'s top-left corner.
* `$T y([2][2]T r) where T is scalar` — The y of `r`'s top-left corner.
* `$T w([2][2]T r) where T is scalar` — The width of `r`.
* `$T h([2][2]T r) where T is scalar` — The height of `r`.
* `bool contains_rect([2][2]$T outer, [2][2]T inner) where T is scalar` — Whether `inner` lies wholly inside `outer`, edges included: a rectangle contains itself.
* `bool contains([2][2]$T r, [2]T point) where T is scalar` — Whether `point` lies in `r`, its top and left edges included and its bottom and right…
* `bool overlaps([2][2]$T a, [2][2]T b) where T is scalar` — Whether `a` and `b` share some area. Rectangles that only touch along an edge or at a corner…
* `([2][2]$T both, bool hit) intersection([2][2]T a, [2][2]T b) where T is scalar` — The rectangle `a` and `b` share, and whether they share any area. When they do not (they…
* `[2][2]$T inset([2][2]T r, T dx, dy) where T is scalar` — Shrink a rect by `dx` on each side and `dy` on top and bottom, clamped so it…
* `[2][2]$T margin([2][2]T r, T left, top, right, bottom) where T is scalar` — Shrink a rect by explicit per-side margins (left, top, right, bottom),…
* `[2][2]$T grow([2][2]T r, T dt) where T is scalar` — `r` grown by `dt` on every side. A negative `dt` shrinks it, without the clamp `inset`
* `[2][2]$T grow_sides([2][2]T r, T left, top, right, bottom) where T is scalar` — `r` grown by a margin on each side, given in the order left, top, right, bottom. Negative…
* `[2][2]$T grow_to_point([2][2]T r, [2]T point) where T is scalar` — The smallest rectangle holding both `r` and `point`.
* `[2][2]$T grow_to_rect([2][2]T a, [2][2]T b) where T is scalar` — The smallest rectangle holding both `a` and `b`: their union's bounds.
* `[2][2][2]$T vslice([2][2]T r, T distance) where T is scalar` — `r` cut by a vertical line `distance` from its left edge into a left and a right part,…
* `[2][2][2]$T hslice([2][2]T r, T distance) where T is scalar` — `r` cut by a horizontal line `distance` from its top edge into a top and a bottom part,…
* `[2][2]$T positive([2][2]T r) where T is scalar` — `r` with a non-negative size: a negative width or height moves the corner by it and flips…
* `$T area([2][2]T r) where T is scalar` — The area of `r`, negative when one side is.
* `$T perimeter([2][2]T r) where T is scalar` — The length of the edge of `r`.
* `[2][2]$T stackv([2][2]T area, i32 n, gap, i) where T is scalar` — The i-th of `n` equal rows stacked down `area`, with `gap` blank rows…
* `[2][2]$T stackh([2][2]T area, i32 n, gap, i) where T is scalar` — The i-th of `n` equal columns across `area`, with `gap` blank columns…
* `[2][2]$T grid([2][2]T area, i32 cols, rows, gx, gy, col, row) where T is scalar` — The cell at (col, row) of a `cols`×`rows` grid over `area`, with `gx`/`gy`
* `enum i32 Align: (Start, Center, End)` — Where a fixed-size box sits inside a larger allocation.
* `[2][2]$T align_in([2][2]T parent, T w, h, Align ax, ay) where T is scalar` — Place a `w`×`h` box inside `parent`, aligned on each axis.
* `[2][2]$T center_in([2][2]T parent, T w, h) where T is scalar` — A `w`×`h` box centred in `parent`.
