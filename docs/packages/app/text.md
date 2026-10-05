<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:text

`app:text` — the pen loop, written once, recorded into a canvas item.

    import "app:draw"
    import "app:text"

    draw.Item line = draw.canvas_item_create(canvas)
    f32 end = text.run(line, &font, atlas, x, top_of_line_box, "hello", WHITE)

The call that is **not** in the frame loop is the one worth staring at.
`run` appends one command per inked glyph to `line`'s list and returns; the
text is on the screen every frame thereafter, and a frame that does not
re-record `line` walks no string, looks up no glyph and appends nothing.
That is the whole of what changed here: the loop is the same loop, and where
it used to hand quads to an implicit frame it now hands commands to an item a
client holds.

## Why this package exists at all

`app:draw` does not know what a font is and `app:font` does not know what a
GPU is, and **that layering is right**: it is what lets a glyph atlas be
tested with no context and a drawing surface be tested with no font file.
But the seam between them is a loop — decode a character, look the glyph up,
place a quad at the pen, add the advance, add the kerning — and until this
package existed *every client wrote it*. `app:ui/draw`'s `Painter.text` and
`example/edit`'s line painter had each written the same twenty-five lines,
off-screen culling included, and neither could fix a bug in the other.

So this is the seam, as a package. It imports both and adds nothing but the
loop. Nothing else in `app:` imports it, so a program that wants the loop
takes it and a program that wants to write its own still can.

## The target comes first

Every function here takes a `draw.Item` as its **first** parameter, which is
the shape the whole retained surface uses: `draw.rect(item, …)`,
`draw.texture_rect_region(item, …)`, and now `text.run(item, …)`. A client
reading a paint function sees one target named once per line and in the same
column every time, which is what makes a block of recording calls skimmable.

A `run*` into a dead handle — a freed item, a stale generation, `NIL_ITEM`,
or any handle at all while the server is down — records nothing and **still
answers the pen**. `app:draw` drops a command written through a dead handle
rather than trapping, and the walk here is not the recording: what a
character measures does not depend on there being anywhere to put its quad.

## Where `y` is

**`run` takes the top of the line box** — the same anchor `app:ui`'s
`Painter.text` settled on, for the same reason: a widget's arithmetic is all
in box space, and a baseline anchor makes every caller add an ascent it has
no other reason to know.

A text view's arithmetic is in *baselines* instead, because that is what a
row of a scrolling buffer is. So `run_at_baseline` is the same function with
the other anchor, and every function here comes in both spellings rather than
one of them being right. Two names with one meaning each is cheaper than one
name a caller has to remember the convention of — and this is the seam where
getting it wrong is one ascent of silent misplacement, which looks like a
layout bug and is not.

## The pen a run answers is a record-time value

Every `run*` answers the pen it reached. Under the immediate surface that was
free in both directions: the loop walked the string to place the quads, so
the pen was a number already in hand, and handing it back cost a `return`.
Retention keeps half of that and takes half away, and being exact about which
half is the difference between a caller that is right and one that looks
right. **The pen is still free to compute, and it is computed only when
something records.** An item cached across a frame is not re-recorded, so
nothing walks, so there is no pen — not a stale pen, *no* pen. A client whose
layout depends on the width of a run it recorded three frames ago must have
kept the number. This is row **G2** of the work order and this package cannot
fix it for the client; what it can do is not pretend nothing happened.

**Two answers were available and the return value stays.**

  * *Drop it.* Make every `run*` a `void`, as every `draw.*` call is, so the
    surface cannot suggest a value that a cached item does not produce, and
    send anyone who wants a width to a measuring query. It costs every
    *recording* client a second walk of the same string for a number the
    first walk already had — and it buys the caching client nothing, because
    a client that does not record has to memoise either way.
  * *Keep it, and say what it is.* Which is this. The number is free where it
    is produced, and on the frame a client records — every dirty frame, and
    every frame at all for a client that has no dirty bit yet, which is all
    of them until row **G7** is done — it is the answer to the question the
    client was about to ask anyway.

**There is no new query here, because the query already exists and it is
`app:font`'s.** `font.measure`, `font.measure_tabbed` and `font.advance_to`
answer the same walk from the same tables, taking no `Item`, no `Canvas` and
no `Texture` — they are exactly the "`font.measure`-shaped query the client
uses when it is not recording", and they were there before this package was.
A `text.measure` forwarding to them would be a third spelling of one walk, in
the package whose entire reason for existing is that the walk is written
once.

So the rule, stated once: **the pen is the answer of the frame that
recorded.** Memoise it beside the item, next to the string that produced it
and next to the dirty bit that decides whether to re-record at all (row
**G7**), and invalidate all three together.

**Two edges, and both are why *memoise the pen* is not *memoise the width*.**
`run_clipped` and `run_tabbed` answer where drawing **stopped**, which is the
run's width only when nothing was culled off the tail: the walk breaks at the
first pen past `x1` rather than measuring a line it is not drawing, and that
is the whole saving. And their cull window is a record-time value too — an
item cached across a horizontal scroll was culled against the window that was
in force when it was recorded, which is row **G5**.

## The batch

**Everything this package draws goes through the atlas texture**, glyphs and
rectangles alike — `fill`, `outline`, `underline` and `caret` ride
`font.solid_src`, the fully-opaque point `app:font` reserves at the atlas
origin. What that is worth is smaller than it was and it is not nothing, and
the difference is worth writing down because the old claim is still true of
the wrong library.

**What this file used to say** is that `app:draw` breaks a batch on a texture
change, so a line of glyphs with a caret and a selection quad drawn the
obvious way costs three draw calls and a screen of them costs one per
primitive. That was a true statement about the hand-written batcher this
package used to emit into, which compared each quad's texture against the
previous quad's and split whenever they differed.

**`sokol_gp` is not that batcher.** Every draw it queues is offered to a
batch optimiser that looks back over the last `SGP_BATCH_OPTIMIZER_DEPTH`
draw commands — **8** by default — for one it can merge with, and merges when
the pipeline, the texture bindings and the shader uniforms all match *and* no
command between the two overlaps either one's region. So it will reorder a
textured draw back past an untextured one and merge it: one solid rectangle
dropped into a run of glyphs does not split the run, and row **E3**'s
simplest case is handled by the library rather than by this package.

**The bounds are what survives**, and they are read out of
`_sgp_merge_batch_command` rather than measured, because measuring them wants
`draw.gpu_draw_calls()` and there is no GPU in the container this was written
in. There are four, and a text renderer is the most likely thing in the
system to meet every one of them:

  * **The look-back is eight commands** and it is a look-back, not a sort.
    (Slightly more than eight in practice — a command already optimised away
    does not count against the depth, so the search steps over it.)
  * **A merge widens the surviving command's region to the union of the
    two.** That is the one that bites, and it bites sooner the better the
    optimiser is doing: merge two glyph runs either side of a box and the
    merged run's rectangle now *contains* that box, so the box overlaps every
    further merge and the next one is refused. A line of glyphs with a rule
    between every pair of them converges on exactly that state.
  * **An overlapping intermediary refuses the merge outright**, at any
    distance inside the window. A selection quad under its own text overlaps
    it by construction, and so does an underline under a descender — and both
    are things a text renderer draws constantly.
  * **Reordering moves vertices and the move is capped**, at
    `_SGP_MAX_MOVE_VERTICES`, which is **96** — sixteen rectangles' worth of
    intermediary content, after which the merge is refused rather than
    performed. And the search **stops dead at a scissor or viewport
    command**, so an `app:ui` row that pushes a clip has spent its window
    whatever was in it.

**And the number this package can count is an upper bound, not the draw
calls.** `draw.state_changes()` counts what the replay changed; the optimiser
only ever merges, never splits, so the truth is that number or lower and
`draw.gpu_draw_calls()` is where it is read — one frame late, and zero
headless. Measured in `text_test.dado`, in one item, and measured as the bound
it is: ten two-glyph runs with a `draw.rect` between each pair is **20**
counted state changes; the same content with `fill` is **1**.

So the claim is smaller and sharper than the one it replaces. It is not that
a texture change costs a draw call — it does not, usually. It is that riding
the atlas makes none of the four questions above arise: the upper bound is
one bind for a whole line however long it is, whatever it overlaps, however
many vertices sit between its ends and whatever is scissored around it — and
**a bound that holds is worth more than an optimiser that usually wins and
cannot tell you when it did not.**

A client that wants a rectangle in a *different colour* still gets one batch
either way: `sokol_gp` writes colour into the vertices, so a colour change
costs nothing and `app:draw` does not count one. A client that wants a
rectangle nowhere near any text should use `draw.rect`, which is a view
change either way, and this package is not the place to decide.

## What it does not do

No shaping, no bidi, no line breaking, no caching. One face per call. A
codepoint the atlas has not packed draws `app:font`'s `.notdef` box and
advances by it — which is a decision made there and not here.

## Declarations

10 declarations, 8 public.

* `f32 run(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 y, string8 s, [4]u8 col)` — Record `s` into `into` with the top of its line box at `y`. Answers the pen x…
* `f32 run_at_baseline(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 base, string8 s, [4]u8 col)` — Record `s` into `into` with its baseline at `base`. Answers the pen x it…
* `f32 run_clipped(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 base, string8 s, [4]u8 col, f32 x0, f32 x1)` — As `run_at_baseline`, but a glyph that lies entirely outside `[x0, x1)` is…
* `f32 run_tabbed(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 base, string8 s, [4]u8 col, font.Tabs t, f32 x0, f32 x1)` — The whole loop: tab stops, culling and all. The four forms above are this one…
* `void fill(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 y, f32 w, f32 h, [4]u8 col)` — A filled rectangle recorded over the atlas's reserved opaque point, so it…
* `void outline(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 y, f32 w, f32 h, f32 thickness, [4]u8 col)` — The four edges of a rectangle, inside it, in the glyph batch. Four `fill`s…
* `void underline(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 base, f32 w, [4]u8 col)` — An underline `w` pixels wide under a run whose baseline is `base`, at the…
* `void caret(draw.Item into, ^font.Font f, draw.Texture atlas, f32 x, f32 y, f32 thickness, [4]u8 col)` — A caret `thickness` wide at `x`, spanning the whole line box whose top is…
