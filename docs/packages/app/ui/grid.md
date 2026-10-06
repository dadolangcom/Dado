<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# app:ui/grid

app:ui/grid — the cell-grid renderer: a screen of `cell.Cell`s, the glyph
atlas behind them, and one instanced draw that paints the lot.

    grid.up(display.window_get_dpi_scale())     // once, after sokol_gfx is set up
    if grid.begin(fb_w, fb_h): repaint_all()    // per frame, before writing
    grid.text(0, 0, "hello", 80, fg, bg)
    grid.prepare()                              // per frame, outside the pass
    grid.render(fb_w, fb_h)                     // per frame, inside the pass
    grid.down()

    grid.set_layers(2)                          // a floating layer over the base
    grid.target(1)                              // writers now write layer 1
    _ = grid.chrome_add(1, box)                 // free-pixel chrome under it

**An engine: the mechanism is hidden and every piece of it can be read.**
No program names a face, a slot, a texture or a buffer. The face chain is
chosen here; printable ASCII is made resident by `up`, any other cluster
the first time a writer meets it; the page is uploaded when it changed. Each of those has a
reader — `stats`, `cell_w`, `cell_at`, `face_count` and the rest — so a test
or an instrument can see what the engine did without reaching into it.

## The cell is derived from the live face

The requested pixel height is `PIXEL_HEIGHT` times the display's DPI scale,
which the caller hands to `up`.
The primary face's advance of `'0'` at that height is `adv`; the cell is
`cw = round(adv)` pixels wide, and the face is then rasterised at the
pixel height that makes its advance exactly `cw` — `px = requested * cw /
adv`, since an advance scales linearly with the pixel height. Rounding the
face rather than the grid is the point: every glyph lands on whole pixels
with no drift across a line. The cell is `ceil(ascent - descent + line_gap)`
pixels tall at `px`, and the baseline `round(ascent)` pixels below its top.
`cell_size_for`, `cell_height_for` and `baseline_for` are that arithmetic,
pure, with the face's numbers as arguments.

## Cells are retained

`begin` sizes the screen to whole cells and clears it **only when the size
changed**, answering true so the caller repaints everything. Otherwise the
cells a caller wrote last frame are still there; nothing clears them. Each
drawn layer's whole buffer is still uploaded every frame — `cols * rows *
12` bytes a layer, which `stats().upload_bytes` sums — because one upload of
a contiguous run is what an instanced draw wants, and tracking dirty cells
would not save it.

The grid's origin is the framebuffer's (0, 0): cell (`col`, `row`) covers
pixels `col * cell_w()` to `(col + 1) * cell_w()` across and `row *
cell_h()` to `(row + 1) * cell_h()` down, which is how a caller turns a
cell rectangle into the pixel rectangle of chrome that sits under it.

## Layer groups

A layer is a whole screen of cells at one depth — the base UI, a floating
window, a tooltip — each with its own cell buffer and its own chrome
(`chrome.dado`), drawn in layer order, each as its chrome and then its cells
in one instanced draw. Layer 0 is `BASE`; `set_layers` says how many there
are, up to `MAX_LAYERS`. Every writer writes the layer `target` names, and a
target that names no layer writes nowhere. `begin` points the target back
at `BASE`, so a paint that targets a layer and forgets to point back loses
at most one frame of the base's writes rather than every one after it.

A layer above the base starts fully transparent — every cell blank, with
background alpha 0, which is what a zeroed `cell.Cell` is — so the layers
below show through everywhere it was not written. Its cell buffer is not
allocated until something writes to it, and a layer that has not been
written since it was last cleared (`clear_layer`, or `begin` on a resize) is
neither uploaded nor drawn; its chrome still is. The base always draws.
Measured under Xvfb with Mesa's software GL, 2026-09-22, at 1000 x 360
pixels of 8 x 16 cells (125 x 22 = 2750 cells): one layer uploaded 33000
bytes and made one instanced draw; a second, written layer made that 66000
bytes and two; the same second layer cleared went back to 33000 and one.

## The atlas upload is once a frame, outside the keystroke

A writer that meets a new cluster rasterises it into its slot at once —
the cell needs the slot number now — and only marks the page dirty. The
upload is `prepare`'s, once per frame at most, whatever number of clusters
arrived since the last one; it is the whole page, because `sg_update_image`
has no partial form. Since `up` makes printable ASCII resident before the
first upload, typing ordinary text uploads nothing; a new character of
another script costs one upload of the page's current size, which the atlas
keeps as small as what it holds (see `atlas.dado`). `stats` says what each
frame sent: `frame_page_upload_bytes`.

## Memory

Everything this package allocates comes from the allocator that was ambient
when `up` ran, captured there. The writers, `begin` and `chrome_add` run
later, inside a program's per-frame temporary arena, and a cell buffer, a
layer's cells made at its first write, a chrome list or an atlas entry
placed there would be rolled back under the renderer at the end of the
frame. Before `up` nothing is allocated: a write to a layer above the base
is dropped and `chrome_add` answers false.

## Scrolling regions

The window keeps its one cell grid for layout and hit-testing. A **region**
is a cell-aligned rectangle of that grid whose interior is drawn from a cell
buffer of its own, shifted up by a sub-cell number of pixels and clipped to
the rectangle, so a list or a document can scroll smoothly while every
other cell stays on the lattice.

    i32 id = grid.region_make(grid.BASE)       // once; -1 when refused
    if grid.region_set_rect(id, r): repaint(id) // true: remade blank
    grid.target_region(id)                     // writers now write it
    _ = grid.text(r.col, r.row + 3, "...", r.cols, fg, bg)
    grid.region_set_offset(id, dy)             // 0 <= dy < cell_h()
    grid.region_scroll_rows(id, 1)             // content up a whole row
    grid.target(grid.BASE)

**The buffer is `rect.cols` x `(rect.rows + 1)` cells.** Shifted up by `dy`
pixels, the rect's `rect.rows` rows show the bottom `cell_h() - dy` pixels
of buffer row 0, rows 1 .. `rect.rows - 1` whole, and the top `dy` pixels
of buffer row `rect.rows` — the spare row, one below the rect, which is
why it exists: a partially visible bottom row always has content. A
scroll by a pixel is therefore `region_set_offset` alone, which uploads
nothing; when the offset would reach a whole cell the caller instead sets
it back to 0, calls `region_scroll_rows(id, 1)` — a `memmove` of the
buffer, not a repaint — and paints the one row it exposed. Only vertical
pixel offsets exist: a horizontal scroll stays column-granular, done by
repainting from a different line column (`text_from`).

**Writers speak screen cells.** While a region is targeted, `put`, `fill`,
`text` and `text_from` take the same screen coordinates as ever, and a cell
is stored when it lies in the region's **extent** — the rect's columns,
rows `rect.row` .. `rect.row + rect.rows` inclusive, the last being the
spare row — and, while a clip is set, in the clip as `region_clip_for`
extends it: a clip whose bottom edge is the rect's own bottom edge is
grown by the spare row, since a widget clipped to the region's rect must be
able to paint the row that scrolls into it; any other clip is used as it
is. The screen's edge does not clip a region (its spare row may lie below
the screen). `target(layer)` points the writers back at a layer, and so
does `begin`.

**Drawing.** `render` draws each layer as its chrome, its cells, then its
regions in creation order, then its ink lists (`ink_make`). A region's
buffer is uploaded only when it was written, scrolled or remade since its
last upload — a frame that only moves the offset sends nothing — then the
same pipeline draws it with the uniforms' grid origin at the rect's pixel
origin less `(0, dy)` and its column count at the rect's, one instanced draw
of `cols * (rows + 1)` cells, under a scissor of the rect's pixels, and the
scissor goes back to the whole framebuffer. The shader is unchanged: its
origin uniform already places the grid, and `dy` is whole pixels, so every
corner still lands on a pixel edge and the NEAREST sampler still reads each
texel once.

**Bookkeeping.** A region's cells are not in its layer's buffer; the
layer's own cells under the rect should be left transparent by the caller
(a zeroed cell is), because the region draws over them and whatever they
hold shows through the region's own transparent cells. A rect that changes
remakes the buffer blank and `region_set_rect` answers true so the caller
repaints. Regions survive `begin`'s resize with their buffers, but the rect
is in cells and only the caller knows where it goes at the new size, so it
is the caller's to set again. A region on a layer that `set_layers` has
since dropped is kept and not drawn. An id that names no live region does
nothing anywhere; ids are reused after `region_free`. The region table
grows by doubling from the captured allocator, with no fixed cap.

## The ASCII fast path

`text` spends most of a plain line finding clusters and looking them up:
a UTF-8 decode and a cluster scan in `layout`, then a decode and a hashed
table probe in `atlas_slot`, per cell. A printable ASCII byte (0x20..0x7E)
whose next byte is also below 0x80 is a cluster of its own — no combining
mark, variation selector or joiner is ASCII, and the only ASCII pair a
cluster joins is CR LF, whose CR is not printable — and one column wide, so
it goes straight to its slot through the 95-entry table `atlas_prewarm`
fills. A tab, any other byte, and a printable byte followed by one at or
above 0x80 take the general path, one cluster at a time, and the fast path
resumes after it. The table is the atlas's, so a rebuilt atlas starts with
an empty one and every byte takes the general path until it is prewarmed.
A line written as several coloured runs (`text_span`, one call a run) takes
the fast path on exactly the cells one `text_from` over the whole line
would: the lookahead reads the line past the run's end, and a run resumes
at the byte and column the last one answered.

## Headless

Under `-D BACKEND=dummy` the package compiles and `up` answers false: the
shader compiler writes no shader for sokol's dummy backend, so there is
nothing to draw with, and `last_error` says so. The pure arithmetic and the
atlas work the same either way; the tests use nothing else. A test that
needs cells stored — layers, regions, a widget's paint — takes a cell size
and the allocator with `up_headless` instead, which touches no GPU.

That is why the DPI scale is an argument to `up` and not read here:
`app:display` imports `sokol_app`, which has no dummy backend and refuses
to compile under one, so importing it would take the headless build away.
For the same reason `up` makes the instance buffer one cell long rather
than sized to the window, and `render` grows it on the first frame.

## Declarations

292 declarations, 177 public.

* `@const i32 PAGE_PX = 1024` — The texel budget of a page, as the side of a square: a page is never wider…
* `@const i32 MIN_SLOTS = 384` — The slots the first page must hold, which picks its width: the 97 slots…
* `const i32 PREWARM_FIRST = 0x20` — The code points `atlas_prewarm` makes resident: printable ASCII.
* `const i32 PREWARM_LAST = 0x7E`
* `const i32 PREWARM_COUNT = PREWARM_LAST - PREWARM_FIRST + 1`
* `const u16 BLANK_SLOT = 0`
* `const u16 NOTDEF_SLOT = 1`
* `const i32 FIRST_FREE = 2` — The first slot a cluster can be given.
* `enum PixelFormat` — What a page's texels are. **Only `R8` is used**: one byte of coverage per…
* `i32 texel_bytes(PixelFormat f)` — Bytes per texel of `f`: 1 for `R8`, 4 for `Rgba8`.
* `type Page` — One page: its size, the height it may grow to, its pixels (row-major from…
* `i32 slots_across(i32 page_w, i32 cw)` — Slots across one row of a page `page_w` wide for a cell `cw` wide: whole…
* `i32 slots_down(i32 page_h, i32 ch)` — Rows of slots down a page `page_h` tall for a cell `ch` tall.
* `i32 page_capacity(i32 page_w, i32 page_h, i32 cw, i32 ch)` — How many slots a `page_w` x `page_h` page holds, counting the blank and…
* `i32 page_width_for(i32 cw, i32 ch, i32 min_slots, i32 max_w)` — The page width for a `cw` x `ch` cell: the smallest power of two whose…
* `i32 page_height_cap(i32 w, i32 budget_px, i32 limit)` — The tallest a page `w` wide may grow: the texel budget of one `budget_px`
* `i32 page_grown(i32 h, i32 max_h)` — The next rung of the ladder above a page `h` tall: twice as tall, held at…
* `(i32 x, i32 y, bool ok) slot_origin(i32 slot, i32 across, i32 capacity, i32 cw, i32 ch)` — The top-left pixel of `slot` on a page `across` slots wide, and whether…
* `type Slots: (i32 across, i32 capacity, i32 next, i32 spare, i32 wasted)` — Where the next slot comes from. `next` only ever grows; `spare` is a slot…
* `Slots slots_make(i32 across, i32 capacity)`
* `i32 slots_take(^Slots s, i32 cols)` — Take room for a cluster `cols` columns wide (1 or 2; anything else reads as…
* `i32 slots_used(^Slots s)` — Slots handed out so far, counting the blank and `.notdef`, and not counting…
* `i32 prewarm_count()` — How many code points `atlas_prewarm` makes resident, and the `i`th of them…
* `i32 prewarm_code_point(i32 i)`
* `type Entry: (u64 hash, u32 chain, f32 px, i32 off, i32 len, u16 slot, bool used)`
* `type Table`
* `u64 key_hash(u32 chain, f32 px, string8 bytes)` — FNV-1a, 64-bit, over the chain id's four bytes, the pixel height in 1/64…
* `i32 table_size_for(i32 n)` — The smallest power of two at or above `n`, and at least `MIN_ENTRIES`.
* `Table table_make(i32 initial)` — An empty table with room for `initial` entries before it first grows, from…
* `void table_free(^Table t)`
* `i32 table_count(^Table t)`
* `i32 table_capacity(^Table t)`
* `(u16 slot, bool found) table_get(^Table t, u32 chain, f32 px, string8 bytes)` — The slot recorded for the key, and whether there was one.
* `bool table_put(^Table t, u32 chain, f32 px, string8 bytes, u16 slot)` — Record `slot` for the key, replacing what was there. Answers false, having…
* `type Face: (font.Font f, f32 scale, i32 baseline)` — One face of the chain: the loaded face (only its `stbtt_fontinfo` is…
* `i32 face_baseline(^font.Font f, f32 scale, i32 ch)` — Where `f`'s baseline goes in a cell `ch` tall at scale `scale`: its own…
* `f32 squeeze_for(i32 span, f32 advance)` — The horizontal squeeze for a glyph `advance` pixels wide in a run of slots…
* `f32 stb_scale(^font.Font f, f32 px)` — The scale that sets `f`'s ascent-to-descent span to `px` pixels, which is…
* `(i32 face, i32 glyph) find_glyph([]Face faces, i32 prefer, i32 cp)` — Which face of `faces` draws `cp`, and its glyph index there: the first that…
* `type Atlas`
* `i32 pen_x(i32 span, f32 advance)` — The pen's x within a slot `span` pixels wide for a glyph whose advance is…
* `f32 round_half_up(f32 x)` — Round to nearest, halves up, without `core:math`'s f64 round trip.
* `Atlas atlas_make(i32 cw, i32 ch, f32 px, u32 chain, i32 max_side = 0)` — Make an empty atlas for a `cw` x `ch` cell at pixel height `px` in face…
* `bool atlas_grow(^Atlas a)` — Grow the page one rung: twice as tall, the old rows kept where they were…
* `void atlas_free(^Atlas a)`
* `i32 atlas_resident(^Atlas a)` — Clusters resident: keys in the table, which counts each wide cluster once.
* `void atlas_ink_notdef(^Atlas a, []Face faces)` — Ink `.notdef` into slot 1: the primary face's glyph 0, centred. Called once…
* `i32 atlas_prewarm(^Atlas a, []Face faces)` — Make every code point of the prewarmed set (printable ASCII) resident, each…
* `u16 atlas_ascii_slot(^Atlas a, u8 b)` — The slot `atlas_prewarm` recorded for the printable ASCII byte `b`, or 0…
* `u16 atlas_slot(^Atlas a, []Face faces, string8 bytes, i32 cols)` — The slot of the cluster `bytes`, `cols` columns wide, rasterising it into a…
* `i32 blend_clipped([]u8 dst, i32 page_w, i32 page_h, i32 rx, i32 ry, i32 span, i32 ch, []u8 src, i32 w, i32 h, i32 dx, i32 dy)` — Max-blend a `w` x `h` coverage box `src` into the `span` x `ch` rectangle…
* `type ChromeBox` — One box of chrome. The rectangle is the box's own area in framebuffer…
* `ChromeBox chrome_box(i32 x, i32 y, i32 w, i32 h)` — A box at (`x`, `y`), `w` x `h`, with no fill, no border, square corners and…
* `const i32 MAX_CHROME_PX = 16384` — The largest pixel quantity the tessellation takes: every coordinate,…
* `ChromeBox chrome_clamped(ChromeBox b)` — `b` with every pixel quantity held to the range above; colours untouched.
* `type ChromePt: (f32 x, f32 y)`
* `type ChromeRect: (i32 x, i32 y, i32 w, i32 h)`
* `type ChromeTri: (ChromePt a, ChromePt b, ChromePt c, [4]u8 col)` — One triangle of chrome in one straight-alpha colour.
* `const i32 MAX_CORNER_SEGMENTS = 16` — The most chords a quarter-circle is cut into, and the most copies a soft…
* `const i32 SHADOW_STEPS = 4`
* `const i32 MAX_OUTLINE_POINTS = 68` — The outline of a rounded box has four corners of `segs + 1` points each.
* `const i32 MAX_BOX_TRIS = 512`
* `i32 corner_segments(f32 r)` — The chords a quarter-circle of radius `r` is cut into: the fewest, from 1…
* `f32 clamp_radius(f32 r, f32 w, f32 h)` — The largest corner radius a `w` x `h` box can take: half its shorter side.
* `i32 rounded_outline(f32 x, f32 y, f32 w, f32 h, [4]f32 rx, [4]f32 ry, i32 segs, []ChromePt out)` — The outline of the box (`x`, `y`, `w`, `h`), clockwise on a y-down screen…
* `i32 border_rects(ChromeBox box, []ChromeRect out)` — The four border rectangles of a square-cornered box, each width drawn…
* `i32 shadow_steps(ChromeBox box)` — How many copies a box's shadow is drawn as: its size in pixels, held to…
* `u8 shadow_step_alpha(u8 a, i32 steps)` — The alpha each of `steps` stacked copies carries so that all of them…
* `(ChromeRect r, i32 radius, u8 alpha) shadow_ring(ChromeBox box, i32 k)` — Copy `k` of the box's shadow (0 innermost): the box moved by the shadow…
* `i32 box_triangles(ChromeBox box, []ChromeTri out)` — Tessellate one box into `out`: its shadow copies, then its fill, then its…
* `type ChromeList: ((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc, ref []ChromeBox boxes, i32 n)` — One layer's boxes, in draw order. The storage comes from the allocator…
* `bool chrome_list_add(^ChromeList l, ChromeBox b, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a)` — Append `b`, growing by doubling. Answers false, having changed nothing,…
* `void chrome_list_clear(^ChromeList l)` — Forget every box, keeping the storage.
* `void chrome_list_free(^ChromeList l)`
* `@const i32 CHROME_FRAME_PERCENT = 75` — The share of sokol_gp's per-frame vertex buffer the chrome may use, in…
* `enum ChunkStep` — What to do with the next box, of `m` triangles, when the open flush already…
* `ChunkStep chunk_step(i32 m, i32 in_flush, i32 flush_cap, i32 frame_left)` — The one rule both `chunk_plan` and `chrome_draw` follow. A box that does not…
* `(i32 drawn, i32 flushes) chunk_plan([]i32 box_tris, i32 flush_cap, i32 frame_left)` — How a list of boxes of `box_tris` triangles each is drawn: how many boxes…
* `i32 chrome_frame_tris()` — The chrome's triangle budget for a whole frame: `CHROME_FRAME_PERCENT` of…
* `type ChromeDrawn: (i32 tris, i32 flushes, i32 dropped, bool failed)` — What one layer's chrome pass did: triangles drawn, flushes made, boxes…
* `ChromeDrawn chrome_draw(^ChromeList l, i32 fb_w, i32 fb_h, i32 frame_left)` — Draw the boxes of `l` with sokol_gp, inside the open pass, into a…
* `^const sokol_gfx.sg_shader_desc ui_grid_grid_shader_desc(sokol_gfx.sg_backend backend)`
* `@const f32 PIXEL_HEIGHT = 16.0` — The requested text height in logical pixels, before the DPI scale.
* `const i32 BASE = 0` — The base layer, which always exists and always draws, and the most layers…
* `const i32 MAX_LAYERS = 8`
* `const u32 CHAIN_ID = 1` — The id of the one face chain there is. A key in the atlas carries it, so a…
* `(i32 cw, f32 px) cell_size_for(f32 requested, f32 adv)` — The cell width and the pixel height to rasterise at, from the requested…
* `i32 cell_height_for(f32 ascent, f32 descent, f32 line_gap)` — The cell height for a face whose ascent, descent (negative, below the…
* `i32 baseline_for(f32 ascent, i32 ch)` — The baseline's pixel row within a cell `ch` tall: `round(ascent)`, held…
* `(i32 underline, i32 strike, i32 thick) lines_for(i32 baseline, i32 ch, f32 ul_pos, f32 ul_thick, f32 x_height)` — Where the underline and the strike line go within a cell, and their shared…
* `(i32 cols, i32 rows) grid_size_for(i32 fb_w, i32 fb_h, i32 cw, i32 ch)` — The screen in whole cells for a framebuffer `fb_w` x `fb_h`: a partial cell…
* `enum SpanKind` — What one cluster of a laid-out string becomes. `Glyph` is drawn from the…
* `type Span: (i32 at, i32 end, i32 col, i32 cols, SpanKind kind)` — One cluster: bytes `at` up to `end`, starting at column `col` (counted from…
* `i32 layout(string8 s, i32 at, i32 col, i32 limit, []Span out)` — Lay out `s` from byte `at`, which sits at column `col`, up to column `limit`,…
* `const i32 MAX_REGION_SIDE = 4096` — The widest and tallest rect a region takes, in cells: a choice, past any…
* `type Stats`
* `Stats stats()`
* `bool is_up()`
* `string8 last_error()`
* `i32 cell_w()` — The cell in framebuffer pixels; 0 before `up`.
* `i32 cell_h()`
* `i32 baseline()` — The baseline's pixel row within a cell, and the pixel height the chain is…
* `f32 pixel_height()`
* `f32 requested_height()`
* `(i32 underline, i32 strike, i32 thick) lines()` — The underline row, the strike row and their thickness, within a cell.
* `i32 cols()` — The screen in cells, as the last `begin` sized it.
* `i32 rows()`
* `cell.Cell cell_at(i32 col, i32 row)` — The cell at (`col`, `row`) of the base layer; off-screen answers the blank…
* `cell.Cell cell_at_layer(i32 layer, i32 col, i32 row)` — The cell at (`col`, `row`) of `layer`; off-screen, a layer that does not…
* `void set_layers(i32 n)` — Have `n` layers, held to 1..`MAX_LAYERS`. Layers dropped by a smaller `n`
* `i32 layers()`
* `void target(i32 layer)` — Point every writer at `layer`. One that does not exist is accepted and…
* `i32 targeted()`
* `void clear_layer(i32 layer)` — Make `layer` fully transparent again — every cell blank with background…
* `bool layer_touched(i32 layer)` — Whether `layer` has been written since it was last cleared. The base…
* `bool layer_draws(i32 layer, bool touched)` — Whether a layer's cells are uploaded and drawn: the base always; any other…
* `void chrome_clear(i32 layer)` — Forget `layer`'s chrome.
* `bool chrome_add(i32 layer, ChromeBox b)` — Add `b` to `layer`'s chrome, after what is there. Answers false for a layer…
* `i32 chrome_count(i32 layer)`
* `ChromeBox chrome_at(i32 layer, i32 i)` — The `i`th box of `layer`'s chrome; a zero box off the end.
* `i32 face_count()` — The faces of the chain that loaded, primary first, and their paths.
* `string8 face_path(i32 i)`
* `i32 face_baseline_at(i32 i)` — The pixel row within a cell where face `i` stands; -1 for no such face.
* `i32 atlas_across()` — Slots across one page row and the page's capacity in slots; slots handed…
* `i32 atlas_capacity()`
* `i32 atlas_slots_used()`
* `bool atlas_dirty()`
* `i32 atlas_page_w()` — The atlas page's size in pixels now, and the height it may grow to. Zero…
* `i32 atlas_page_h()`
* `i32 atlas_page_max_h()`
* `bool up(f32 dpi_scale)` — Load the face chain, derive the cell, make the atlas and the GPU objects.
* `void down()` — Give back everything: GPU objects, the atlas, the faces, the cells. Safe…
* `bool up_headless(i32 cw, i32 ch)` — Take a cell of `cw` x `ch` pixels and the ambient allocator, as `up` does,…
* `bool begin(i32 fb_w, i32 fb_h)` — Size the screen for a framebuffer `fb_w` x `fb_h`, and point the writers at…
* `void clip(cell.CellRect r)` — Clip every writer to `r` (as well as to the screen) until `unclip`.
* `void unclip()`
* `void put(i32 col, i32 row, cell.Cell c)` — Store `c` at (`col`, `row`); off-screen is ignored.
* `void fill(cell.CellRect r, [4]u8 fg, [4]u8 bg)` — Blank every cell of `r` on the screen, in the given colours — or, while a…
* `u16 slot_of(string8 s, i32 at, i32 end)` — The atlas slot of the cluster at bytes `at` up to `end` of `s`, made…
* `i32 text(i32 col, i32 row, string8 s, i32 max_cols, [4]u8 fg, [4]u8 bg)` — Write `s` from (`col`, `row`) in at most `max_cols` columns, one cluster per…
* `i32 text_from(i32 col, i32 row, string8 s, i32 at, i32 at_col, i32 max_cols, [4]u8 fg, [4]u8 bg)` — Like `text`, but starting part-way into a line: the cluster at byte `at`
* `(i32 at, i32 col) text_span(i32 col, i32 row, string8 s, i32 at, i32 end, i32 at_col, i32 max_cols, [4]u8 fg, [4]u8 bg)` — `text_from` for the clusters that begin before byte `end` only — one styled…
* `void prepare()` — Upload the atlas page if it changed. Outside the render pass, once per…
* `void render(i32 fb_w, i32 fb_h)` — Draw the screen: inside the pass, layer by layer from `BASE` up, each as…
* `(i32 x, i32 y, i32 w, i32 h) region_scissor(cell.CellRect r, i32 cw, i32 ch, i32 fb_w, i32 fb_h)` — The pixel rectangle a region's rect covers in a framebuffer `fb_w` x…
* `cell.CellRect region_extent(cell.CellRect r)` — The cells a region's writers reach: the rect and the spare row below it.
* `cell.CellRect region_clip_for(cell.CellRect clip, cell.CellRect r)` — The clip a region's writers are held to while `clip` is set: `clip` grown…
* `bool region_valid(i32 id)` — Whether `id` names a live region.
* `i32 region_count()` — Live regions.
* `i32 region_make(i32 layer)` — A new region on `layer`, with an empty rect and offset 0; answers its id,…
* `void region_free(i32 id)` — Free region `id`; a bad id does nothing. A writer still aimed at it writes…
* `bool region_set_rect(i32 id, cell.CellRect r)` — Set the screen cells region `id` covers. **Answers true when the rect…
* `void region_set_offset(i32 id, i32 dy)` — Draw region `id`'s content shifted up by `dy` pixels, held to 0 ..
* `void region_scroll_rows(i32 id, i32 n)` — Shift region `id`'s buffer up by `n` rows (down for `n` < 0): one…
* `void target_region(i32 id)` — Point every writer at region `id`, in screen cells (see the file's head).
* `i32 targeted_region()` — The region the writers write, or -1 while they write a layer.
* `cell.CellRect region_rect(i32 id)` — A region's rect (empty for a bad id), offset, and layer (-1 for a bad id).
* `i32 region_offset(i32 id)`
* `i32 region_layer(i32 id)`
* `cell.Cell region_cell_at(i32 id, i32 col, i32 row)` — The cell of region `id` at screen cell (`col`, `row`), the spare row…
* `type RegionStats: (i32 uploads, i64 upload_bytes, i64 rows_shifted, i64 bytes_moved, bool dirty)` — What one region has done since it was made: buffer uploads and their…
* `RegionStats region_stats(i32 id)`
* `bool ink_valid(i32 id)` — Whether `id` names a live ink list.
* `i32 ink_lists()` — Live ink lists.
* `i32 ink_make(i32 layer)` — A new, empty ink list drawn with `layer`, with an empty scissor; answers its…
* `void ink_free(i32 id)` — Give back list `id`'s triangles; the id is then free for reuse.
* `void ink_clear(i32 id)` — Forget list `id`'s triangles, keeping their storage, and count a clear.
* `void ink_set_layer(i32 id, i32 layer)` — Draw list `id` with `layer` from now on. A layer that does not exist is…
* `i32 ink_layer(i32 id)` — The layer list `id` draws with; -1 for a bad id.
* `void ink_set_scissor(i32 id, ChromeRect r)` — Clip list `id` to `r`, in framebuffer pixels. A rect of no area draws…
* `ChromeRect ink_scissor(i32 id)`
* `bool ink_add(i32 id, ChromeTri t)` — Append one triangle to list `id`, growing its run by doubling. Answers…
* `i32 ink_count(i32 id)` — How many triangles list `id` holds; 0 for a bad id.
* `ChromeTri ink_at(i32 id, i32 k)` — Triangle `k` of list `id`; a zero triangle out of range.
* `i64 ink_clears(i32 id)` — How many times list `id` has been cleared since it was made: the count of…
* `(i32 lists, i32 tris, i32 dropped) ink_drawn()` — What the last `render` did with ink, all layers: lists drawn, their…
* `ChromeRect ink_scissor_on(ChromeRect r, i32 fb_w, i32 fb_h)` — `r` cut to a framebuffer `fb_w` x `fb_h`; `w` or `h` 0 when nothing of it…
