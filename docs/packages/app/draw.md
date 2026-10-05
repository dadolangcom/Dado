<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:draw

app:draw — a retained 2D graphics server over `vendor:sokol/sokol_gp`.

This package is a **server**, in Godot's sense of the word: it owns
resources, it owns a tree of canvas items, and a client talks to it entirely
through handles. Nothing is drawn when a `draw_*` call is made. A call
*records*, and `canvas_render` is what turns a whole tree into draw calls.

    draw.init() using alloc                            // after sg_setup
    draw.Canvas c = draw.canvas_create()
    draw.Item bg = draw.canvas_item_create(c)
    draw.rect(bg, (0.0, 0.0, 800.0, 600.0), [26, 30, 38, 255])
    draw.Item label = draw.canvas_item_create(c)
    _ = draw.canvas_item_set_parent(label, bg)
    draw.canvas_item_set_position(label, (8.0, 8.0))
    ...
    sg_begin_pass(&pass)
    draw.canvas_render(c, fb_w, fb_h)                  // inside the pass
    sg_end_pass()

The two lines worth staring at are the ones that are *not* in the frame
loop. `rect` is written once and the rectangle is drawn every frame
thereafter; a frame that changes nothing re-records nothing. That is the
whole difference from what this package used to be, and it is why the
command tape below is the architecture rather than a testability device.

── Why `sokol_gp` ─────────────────────────────────────────────────────────

What it deletes: **every hand-written shader string.** This package used to
carry GLSL 4.10, GLSL ES 3.00 and MSL as source strings chosen off
`sg_query_backend()`, of which one was pixel-verified, one had never been
compiled by any machine in this tree, and three further backends had no
string at all, so `init` answered `false` on them. `sokol_gp` ships shaders
for every backend it supports. Nine string constants, two shaders, two
pipelines, one sampler, one vertex buffer, one index buffer and the whole
backend switch are gone, and what replaced them is `sgp_setup`.

The two models turned out to be one model, which is the argument for the
swap rather than a happy accident:

| | this package, before | `sokol_gp` |
|---|---|---|
| coordinates | f32 pixels, origin top-left, y down | `sgp_begin`'s default projection, identically |
| colour | `[4]u8` RGBA, straight alpha | `sgp_color_ub4`, straight |
| blend | `SRC_ALPHA / ONE_MINUS_SRC_ALPHA`, alpha `ONE / ONE_MINUS_SRC_ALPHA` | `SGP_BLENDMODE_BLEND`, byte for byte |

The alpha source factor being `ONE` rather than `SRC_ALPHA` is a choice this
file wrote a parenthetical to justify — so an offscreen target accumulates a
correct coverage — and `sokol_gp` had made the same one independently.

**Three things it costs**, each written down rather than papered over.

1. `sgp_draw_filled_rects` gives its vertices texcoords spanning the *whole*
   bound texture, so an untextured rectangle drawn while an atlas view is
   bound samples the atlas. Every untextured primitive here issues
   `sgp_reset_view(0)` first. That is not a defect; it is why `app:font`'s
   reserved opaque texel still earns its place.
2. `sokol_gp` exposes no command count of its own.
3. `sgp_setup` needs a live `sokol_gfx` context, so `init_headless` — which
   makes no sokol call at all — cannot sit on top of it.

(2) and (3) are answered by the tape.

── The tape is the canvas item ────────────────────────────────────────────

A canvas item **is** a command list. Recording is appending to it. Headless
is record-and-do-not-replay, and the recorded list is the same list the GPU
path replays, so a headless test proves the live path rather than a stand-in
for it — every number `commands()`, `primitives()` and `state_changes()`
answer is computed by the same walk in both modes, because the emitters in
`device.dado` count first and call sokol second, and headless returns between
the two lines. There is no second copy of the traversal to drift.

Counting is then honest in two numbers instead of one invented one.
`state_changes()` is an **upper bound** — the optimiser only ever merges,
never splits — and `gpu_draw_calls()` is `sg_query_stats().prev_frame.num_draw`,
the truth, one frame late. The gap between them is the diagnostic: it says
what the optimiser saved.

That only works while the bound is **tight**, which is why `state_changes()`
counts exactly the four things `sokol_gp`'s merge test reads and not one
thing more. It briefly counted a fifth — the item transform — on the false
belief that the model-view projection was a uniform; the bound stayed
correct and the gap stopped meaning anything, which is a worse failure for a
diagnostic than being wrong in a way somebody notices.

**The cost, stated.** A primitive is written twice: once into the tape, once
into `sokol_gp`'s vertices. The immediate path wrote it once. For a
re-recorded frame that is strictly more CPU work; for an unchanged frame it
is strictly less, because nothing is recorded at all. **Nothing here has
been measured on a GPU** — the container this was written in has none — so
the crossover is unknown and is a row, not a claim.

── The command record is one shape, and it is 64 bytes ────────────────────

One `Cmd` per recorded call, uniform across every kind, with a `kind` field
selecting how the rest is read. `#size(Cmd)` is **64** — the test asserts it
— which is a cache line, and no slot is padding under any of the fifteen
kinds:

    kind  4   the discriminant
    tex   8   (i32 index, u32 generation)
    a    16   dest rect / line endpoints / clip rect / the matrix's 2x2
    b    16   source rect / centre+radius / the matrix's translation
    col   4   the recorded colour, before the tree's tint
    width 4   Godot's filled/hairline sentinel, negative for either
    first 4   where this command's point run starts
    count 4   how many points it has
    flags 4   filled, or the blend mode under CMD_SET_BLEND

A **union would have been smaller and is not what this is.** A tagged union
over fifteen members costs fifteen `switch` arms in every function that
touches a command — the recorder, the replayer, the counter — where one flat
shape costs fifteen arms in exactly one of them. And a `Cmd` array is
walked linearly, so the win from a 40-byte record would be bandwidth on a
pass that is already dominated by what it hands to `sokol_gp`.

It is also the shape that makes the record **readable from outside**, which
is what `canvas_item_command` is: one answer of fixed slots, with a public
discriminant saying how to read them. A union would have had to be
destructured into exactly this on the way out anyway.

**A point run does not fit and gets a side pool.** `polyline`, `multiline`,
`colored_polygon` and `primitive` each take an arbitrary number of points,
and a fixed record cannot hold them; `first`/`count` name a run in the
item's own `Vtx` pool instead. The pool is **per item**, not global, which
is what makes `canvas_item_clear` O(1): a global pool would leak every
cleared item's points until something compacted it, and compaction wants a
second pass over every command in the system to fix up indices.

One `Vtx` is 20 bytes — position, texcoord and colour — and a `polyline`
uses only the first two of the three, so it pays 12 bytes a point it does
not need. **One pool rather than three** is the trade: three parallel pools
(positions, colours, texcoords) would save that on the two commands that are
all-position and cost three allocations, three growth policies and three
bounds checks on the one command — `primitive` — that needs all three.

`set_transform_matrix` has no pool at all, because a `Transform2D` is a 3x3
whose third column is `(0, 0, 1)` by construction, so the six numbers that
mean anything fit in `a` and `b` exactly.

── Where the tree lives ───────────────────────────────────────────────────

**In `app:graph`.** The dependency runs `app:draw` → `app:graph` and never
the other way. A `Canvas` owns a `graph.Graph`; an `Item` is a graph node
plus a command list, and the node's `payload` — the opaque `u32` that
package stores and never interprets — is what names the list.

`app:graph` already had generational handles over a growable arena with a
free list, a parent/child tree, composed 2D transforms and a per-node
payload, and S1 added `visible`, `modulate`, `self_modulate`, `z_index`,
`z_as_relative`, `clip`, `custom_rect` and `update_transforms`. What it must
never learn is what a command is. The moment it imports a vendor package it
stops being the backend-neutral thing that made it reusable — so the command
list, the texture handles the opcodes name, the scissor and the traversal
that replays all live here, and `app:graph` holds a `u32` it never
dereferences.

The `payload` this package writes is `item index + 1`, so zero keeps meaning
"no list". The item and its list share one index because the list lives *on*
the item slot; naming one names the other, and there is no second table to
keep in step.

── Handles ────────────────────────────────────────────────────────────────

Godot has one untyped `RID` and finds a mistake at run time. Three
`distinct` types find it at compile time at no cost, because a name is a
view and `distinct` is the one construct whose whole job is to add an
identity. Each is `(i32 index, u32 generation)` — generational, because
items are created and freed constantly and a stale handle that silently
addresses a reused slot is the bug this shape exists to not have. Every
function below re-derives its own slot from a handle immediately before use;
a stale handle reads as dead and the call is a no-op or a `false`, never a
write to somebody else's item.

── Paint order ────────────────────────────────────────────────────────────

`canvas_render` is four passes and they are all cheap:

  1. `graph.update_transforms` — every node's global transform, with **no
     `on_process` fired**. A renderer must not tick game logic to find out
     where things are.
  2. The walk. Depth-first from `first_root` in root order, children in
     sibling order (which S1 made insertion order for exactly this reason).
     An invisible node prunes its whole subtree. Each visited item comes out
     of the walk with an absolute `z`, a composed tint and a screen-space
     clip rectangle.
  3. A **stable** sort of the visible items by `z`, so equal `z` keeps the
     order the walk produced. Insertion sort: it is stable, it is in place,
     and on the overwhelmingly common input — every `z` equal — it is
     `n - 1` comparisons and zero moves. A comparison sort with a better
     worst case has a worse best case, and the best case is the one every
     frame of a UI hits.
  4. Replay, in that order.

So a frame is **O(N + V + I + C)**, where N is every node in the tree, V the
visible items, I the number of `z` inversions among them, and C the total
commands those items hold. Nothing here allocates: the paint list is a
package-level run that grows once and is reused every frame.

**Clipping is resolved in the walk and not in the replay**, because the sort
moves items away from their ancestors. An item that clips contributes the
axis-aligned bounding box of its `custom_rect` under its own global
transform, met with whatever its ancestors contributed; the result travels
with the paint entry. A rotated clip rectangle therefore clips to its
bounding box and not to itself — the scissor is an axis-aligned integer
rectangle in `sokol_gfx` and in every API underneath it, and a rotated
scissor is not a thing that exists.

`push_clip`/`pop_clip` are **commands inside a list** as well as a per-item
property, because `app:ui`'s `item_draw` pushes a second clip *inside* its
node's own clip to keep rows out of the scrollbar gutter, and a per-item
flag alone cannot express that. An in-list clip is given in the item's own
coordinates and is met with the clip the walk computed.

── Tints compose, and `self_modulate` does not ────────────────────────────

`modulate` multiplies down the tree; `self_modulate` multiplies the item
alone and is not handed to its children. That is Godot's split and it is
kept because "fade this one thing" and "fade this whole branch" are
different edits. The product is per channel, `(a * b + 127) / 255`, which is
the rounding form — `a * b / 255` loses a level on every multiply and two
nested 50% fades come out visibly darker than one 25% one.

A recorded colour is multiplied by the item's tint at replay, never at
record time. That is the point of retaining: `canvas_item_set_modulate`
changes what a frame looks like without touching a single command.

── Allocation ─────────────────────────────────────────────────────────────

`init` takes **no** `Allocator`, and every run this package owns comes from
the ambient one — `#default` — as it stood at the line that brought the
server up. `shutdown` gives it all back.

That is a sentence about the *caller*, not about this file. A server whose
storage should land somewhere particular is brought up there and says so
once:

    draw.init()                              // the caller's own default
    draw.init() using arena.allocator(&a)    // every run below comes from `a`

Writing the allocator into the parameter list, which is what this package
used to do, put that decision in the callee's signature and made every
client thread a value through its own call graph to reach it. One word at
one call site reaches the whole dynamic extent instead, however deep it
goes — `pools` and `tex_table` below mention no allocator at all, and
`canvas_create` hands the ambient on to `app:graph`, which mentions none
either.

**`g_alloc` survives that change, and it is not there so `shutdown` can
free.** Freeing never needed it: `#delete` and `#resize` use the allocator
the `ref` itself carries, so every line of `shutdown` would work with the
global gone. It is there because two of this package's runs are **lazy** and
are minted nowhere near `init` — an item's command tape in `push_cmd` and
its point pool in `push_pt`, both on the first `rect` or `polyline` recorded
into that item, and a `graph.Graph`'s node pool on the first
`canvas_item_create`. Without it those `#make`s would read whatever ambient
was live at the *recording* call, so a client that records inside a scratch
frame would put a retained tape in the scratch arena and have it rolled back
under the tree that still points at it. `pools` reads `#default` once and
every later mint answers to that, which is the same fact the parameter used
to carry.

## Declarations

205 declarations, 105 public.

* `Texture texture_white()` — The built-in white 1x1 texel. An untextured primitive does not go through…
* `Texture texture_create(i32 w, i32 h, []u8 rgba8)` — Upload an RGBA8 image, `w * h * 4` bytes, row-major from the top-left.
* `bool texture_update(Texture t, []u8 rgba8)` — Replace a texture's pixels. The run must be at least as long as the texture…
* `Texture texture_target_create(i32 w, i32 h)` — A texture the GPU renders **into** rather than one uploaded from memory: an…
* `bool texture_attach(Texture t, ^sokol_gfx.sg_pass pass)` — Hang a render target on a pass as its colour attachment. Answers false if…
* `void texture_free(Texture t)` — Release a texture. Slot 0 — the white texel — is not destroyable, so a…
* `i32 texture_width(Texture t)`
* `i32 texture_height(Texture t)`
* `bool texture_is_target(Texture t)` — Whether the handle is a `texture_target_create`.
* `bool texture_valid(Texture t)` — Whether the handle still names a live texture. A handle held past…
* `u32 texture_gl_name(Texture t)` — The GL texture object behind a handle, or 0 under a backend that is not GL,…
* `sokol_gfx.sg_resource_state texture_state(Texture t)` — The sokol resource state of a texture's image — `SG_RESOURCESTATE_VALID`
* `i32 gpu_draw_calls()` — `sg_query_stats().prev_frame.num_draw` — how many draw calls the **previous**
* `f32 @c("sqrtf") sqrt_f(f32 x)`
* `f32 @c("sinf") sin_f(f32 x)`
* `f32 @c("cosf") cos_f(f32 x)`
* `distinct type Texture: (i32 index, u32 generation)` — A texture. Slot 0 is the built-in white 1x1 texel, which is what…
* `distinct type Item: (i32 index, u32 generation)` — A canvas item: one node of a `Canvas`'s tree, plus the command list hanging…
* `distinct type Canvas: (i32 index, u32 generation)` — A canvas: one tree of items, rendered by one `canvas_render`.
* `const Texture NIL_TEXTURE = Texture((-1, 0))` — The handle that names nothing. `canvas_item_set_parent(item, NIL_ITEM)`
* `const Item NIL_ITEM = Item((-1, 0))`
* `const Canvas NIL_CANVAS = Canvas((-1, 0))`
* `const i32 BLEND_NONE = 0`
* `const i32 BLEND_MIX = 1`
* `const i32 BLEND_PREMULTIPLIED = 2`
* `const i32 BLEND_ADD = 3`
* `const i32 BLEND_ADD_PREMULTIPLIED = 4`
* `const i32 BLEND_MOD = 5`
* `const i32 BLEND_MUL = 6`
* `const i32 DEFAULT_MAX_VERTICES = 65536` — What `init` asks `sokol_gp` for. These are `sokol_gp`'s own defaults, said…
* `const i32 DEFAULT_MAX_COMMANDS = 16384`
* `const i32 CLIP_DEPTH = 32` — The most nested in-list `push_clip`s honoured inside one item. Beyond this a…
* `const i32 CMD_NONE = 0`
* `const i32 CMD_RECT = 1`
* `const i32 CMD_TEXTURE_RECT = 2`
* `const i32 CMD_TEXTURE_RECT_REGION = 3`
* `const i32 CMD_LINE = 4`
* `const i32 CMD_MULTILINE = 5`
* `const i32 CMD_POLYLINE = 6`
* `const i32 CMD_CIRCLE = 7`
* `const i32 CMD_POLYGON = 8`
* `const i32 CMD_PRIMITIVE = 9`
* `const i32 CMD_SET_TRANSFORM = 10`
* `const i32 CMD_SET_TRANSFORM_MATRIX = 11`
* `const i32 CMD_PUSH_CLIP = 12`
* `const i32 CMD_POP_CLIP = 13`
* `const i32 CMD_SET_BLEND = 14`
* `const i32 CMD_RESET_BLEND = 15`
* `const i32 FLAG_FILLED = 1` — `flags` is a bit set rather than a second field, because `Cmd` is at 64…
* `bool init()` — Bring the server up with `sokol_gp`'s default budgets. `sg_setup` must…
* `bool init_capacity(i32 max_vertices, i32 max_commands)` — As `init`, with explicit `sokol_gp` budgets. `max_vertices` is how many…
* `bool init_headless()` — Bring the server up with **no GPU at all**: every `draw_*` call records,…
* `void shutdown()` — Release everything `init` took, including every canvas, every item and every…
* `bool ready()` — Whether `init` succeeded and the server will record.
* `bool headless()` — Whether this server is the no-GPU one `init_headless` builds.
* `string8 last_error()` — Why the last `init` answered false. Empty when nothing has failed.
* `Canvas canvas_create()` — A fresh, empty canvas. Allocates nothing beyond its slot: `app:graph`'s own…
* `void canvas_free(Canvas c)` — Free a canvas, every item on it and every node of its tree. Every handle…
* `bool canvas_valid(Canvas c)` — Whether the handle still names a live canvas.
* `Item canvas_item_create(Canvas c)` — A new item on `c`, attached as its **last root**. Godot makes a canvas item…
* `void canvas_item_free(Item it)` — Free an item, its command list, and **every item below it in the tree**.
* `void canvas_item_clear(Item it)` — Empty an item's list and **keep every property it has** — its transform, its…
* `bool canvas_item_set_parent(Item it, Item parent)` — Move `it` under `parent`, or to the end of the canvas's root list when…
* `void canvas_item_set_transform(Item it, xform.Transform2D t)` — The item's own transform, which composes with its ancestors'.
* `void canvas_item_set_position(Item it, [2]f32 p)`
* `void canvas_item_set_rotation(Item it, f32 r)`
* `void canvas_item_set_scale(Item it, [2]f32 s)`
* `void canvas_item_set_visible(Item it, bool v)` — Hide or show. An invisible item contributes nothing **and neither does…
* `void canvas_item_set_modulate(Item it, [4]u8 c)` — The tint this item and everything below it is multiplied by.
* `void canvas_item_set_self_modulate(Item it, [4]u8 c)` — The tint this item alone is multiplied by, and which its children never see.
* `void canvas_item_set_z_index(Item it, i32 z)`
* `void canvas_item_set_z_as_relative(Item it, bool rel)` — Whether `z_index` is an offset from the parent's layer (the default, and…
* `void canvas_item_set_clip(Item it, bool on)` — Whether this item clips itself and its subtree to its custom rectangle.
* `void canvas_item_set_custom_rect(Item it, bool use, [4]f32 r)` — The rectangle `canvas_item_set_clip` clips to, in the item's own…
* `bool canvas_item_is_empty(Item it)` — Whether the item's list holds no commands. Properties and children are not…
* `i32 canvas_item_command_count(Item it)` — How many commands the item's list holds. Every `draw_*` call appends exactly…
* `(i32 kind, Texture tex, [4]f32 a, [4]f32 b, [4]u8 col, f32 width, i32 count, i32 flags) canvas_item_command(Item it, i32 i)` — One recorded command, read back.
* `(f32 x, f32 y, f32 u, f32 v, [4]u8 col) canvas_item_command_point(Item it, i32 i, i32 k)` — Point `k` of command `i`'s run, in the coordinates it was recorded in.
* `bool canvas_item_valid(Item it)` — Whether the handle still names a live item. A handle held past…
* `Canvas canvas_of(Item it)` — The canvas an item belongs to, or `NIL_CANVAS`.
* `void rect(Item it, [4]f32 r, [4]u8 col, bool filled = true, f32 width = -1.0)` — A rectangle, filled or outlined.
* `void texture_rect(Item it, Texture t, [4]f32 r, [4]u8 modulate)` — A texture, stretched over `r` and multiplied by `modulate`. The whole image…
* `void texture_rect_region(Item it, Texture t, [4]f32 r, [4]f32 src, [4]u8 modulate)` — A region of a texture, stretched over `r`. **`src` is in image pixels, not…
* `void line(Item it, [2]f32 from, [2]f32 to, [4]u8 col, f32 width = -1.0)` — A line. A negative width is a hairline; a positive one is a quad centred on…
* `void multiline(Item it, [][2]f32 points, [4]u8 col, f32 width = -1.0)` — Disconnected segments, taken in pairs. An odd trailing point is dropped,…
* `void polyline(Item it, [][2]f32 points, [4]u8 col, f32 width = -1.0)` — A connected run of segments.
* `void circle(Item it, [2]f32 centre, f32 radius, [4]u8 col, bool filled = true, f32 width = -1.0)` — A circle, filled or outlined. `sokol_gp` has no circle, so this is…
* `void colored_polygon(Item it, [][2]f32 points, [4]u8 col)` — A convex polygon in one colour, drawn as a fan. A concave outline is drawn…
* `void primitive(Item it, [][2]f32 points, [][4]u8 cols, [][2]f32 uvs, Texture t)` — The general form: a point run with its own colours and texture coordinates,…
* `void set_transform(Item it, [2]f32 position, f32 rotation, [2]f32 scale)` — An extra transform, in the item's own coordinates, applied to every command…
* `void set_transform_matrix(Item it, xform.Transform2D t)` — The same, from a matrix. A `Transform2D`'s third column is `(0, 0, 1)` by…
* `void push_clip(Item it, [4]f32 r)` — Narrow the clip region to the meet of the region in force and `r`, given in…
* `void pop_clip(Item it)` — Undo the matching `push_clip`. An unmatched one is dropped at render time…
* `void set_blend(Item it, i32 mode)` — Change the blend mode for the rest of this list. A mode outside `BLEND_NONE…
* `void reset_blend(Item it)` — Go back to `BLEND_MIX` for the rest of this list.
* `void canvas_render(Canvas c, i32 fb_w, i32 fb_h)` — Walk `c`, order it, and draw it. Must be called **inside an open…
* `i32 commands()` — How many commands the last `canvas_render` replayed. A command that was…
* `i32 primitives()` — How many primitives those commands handed to `sokol_gp`: rectangles,…
* `i32 state_changes()` — How many times the last render changed something `sokol_gp` keys a batch on…
* `i32 transform_changes()` — How many times the last render rebuilt `sokol_gp`'s transform — once per…
* `i32 painted()` — How many items the last `canvas_render` painted, in the order it painted…
* `(Item item, i32 z, [4]u8 tint, [4]f32 clip, bool clipped) paint(i32 i)` — The `i`-th item of the last render's paint order, with the `z` it resolved…
* `bool overflowed()` — Whether anything has been dropped for want of room — an allocation that…
* `i32 capacity()` — The vertex budget `sokol_gp` is running under, which is what a render can…
* `i32 command_capacity()` — The command budget `sokol_gp` is running under.
