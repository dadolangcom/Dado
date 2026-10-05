# The text backend — survey, and a plan

**Opened 2026-09-22. Companion to the framework's work order, which is kept in
the repository and not shipped.**
**Where this file and `dadoc` disagree, `dadoc` is right.**

## What is measured here and what is only read

Everything in §1–§2 is **read from the source**, not run: it is a description of
what the packages say and do, gathered by reading `collections/app/font`,
`collections/app/draw`, `collections/app/text`, `collections/app/graphical`,
`collections/core/chars` and `collections/vendor/sokol`. Where a *number*
appears in §1 it is a number **this repository already recorded** in its own
headers, and it is attributed as such — none of it was re-run for this document.

Everything in §3–§8 is a **proposal**. None of it is in the tree. No design
below has been built, compiled or timed, and the acceptance numbers in §8 are
targets to be argued with, not measurements.

---

## 1. The stack as it stands

### 1.1 `app:font` — one atlas, one face, one size

`font.dado` rasterises with vendored `stb_truetype` only
(`collections/vendor/stb/stb_truetype`). There is no FreeType, no HarfBuzz, no
fontconfig, no CoreText and no DirectWrite anywhere in the tree. The C is
compiled into Dado's own translation unit by a `foreign` block, so there is no
separate build step for it.

What the atlas is: **one RGBA8 image**, `(255,255,255,coverage)` per texel, shelf
packed, width ladder 128→4096 doubling, height doubled on growth and capped.
Two counters are kept deliberately apart — `atlas_generation` (pixels moved, so
re-upload) and `atlas_resizes` (shape moved, so destroy and remake).

The five properties that matter to a grid, each a fact about the current code:

1. **Keyed by codepoint, and one size per `Font`.** ASCII `' '..'~'` is a dense
   array of 96 slots rasterised eagerly at load. Everything else goes into a
   parallel `extra_cp[]`/`extra[]` pair and is found by **linear scan** —
   `glyph` walks `0..<f.extra_used`. That is O(n) per non-ASCII glyph per draw.
2. **No fallback chain.** A `Font` is one face. A missing codepoint yields
   `.notdef`, never a second face.
3. **No subpixel positioning, no hinting control, no gamma.** Good news for a
   grid: it is one less thing to undo.
4. **`measure` is the hot path and it is expensive.** `measure_tabbed` walks
   the string, decodes UTF-8, looks up each glyph (dense for ASCII, linear scan
   beyond it), then decodes a *second* time and makes one
   `stbtt_GetCodepointKernAdvance` **FFI call per adjacent pair**. No cache of
   any kind. `ui.dado`'s own header records `Painter.measure` at 249 calls a
   frame.
5. **`cell_at` hardwires `columns = 1`** for every non-tab character. Double
   width does not work today, and the wcwidth table that would fix it exists
   already and is never consulted (§1.4).

### 1.2 `app:draw` — a retained tape, one quad per glyph

A `Canvas` owns a tree of `Item`s and an `Item` **is** a command tape, fifteen
opcodes wide. `canvas_render` collects, z-sorts and replays into `sokol_gp`.

**A text draw is one `CMD_TEXTURE_RECT_REGION` per inked glyph**, which becomes
one `sgp_draw_textured_rect` on replay. There is no glyph batching, no
instancing, no per-run vertex buffer. Rectangles that want to ride in the same
batch (`text.fill`, `text.underline`, `text.caret`) are drawn as textured quads
against a reserved opaque texel with a zero-extent source rect.

The batch optimiser is **sokol_gp's, not ours**: `_sgp_merge_batch_command`
folds a draw into the previous one when pipeline and bound textures match.
`app:draw` does not optimise, it *accounts*: `commands`, `primitives`,
`state_changes` (an explicit upper bound), `transform_changes`, and
`gpu_draw_calls` read off `sg_query_stats().prev_frame.num_draw` and therefore
one frame late.

Two details that bite a grid directly: the pen is f32 and quads land at
`pen + bearing_x` with **no rounding**, while the sampler is
`SG_FILTER_LINEAR`. Fractional glyph origins therefore bilinear-blur. And the
command budget is `DEFAULT_MAX_COMMANDS = 16384` — a 200×60 grid is 12 000
commands a frame, which fits only just, and only if the grid is a leaf item.

### 1.3 Shaders — there are none

Searching the tree for `sg_make_shader`, `sokol-shdc`, `*_shd.h`, `.glsl` and
`.hlsl` finds nothing outside the vendored sokol headers. `app:draw` used to
carry hand-written GLSL410/GLES300/MSL strings and they were **deliberately
deleted** when sokol_gp landed; the RGBA8-white-with-coverage atlas format
exists precisely so that sokol_gp's stock `texture(tex,uv) * color` suffices
with no shader of our own.

sokol_gp's own shaders are precompiled source strings inside the vendored
header, chosen at runtime by `sg_query_backend()`. The `.glsl` + `.glsl.h`
pairs under `collections/vendor/sokol/sokol_gp/shaders/` are **upstream
artifacts**: nothing in this repository's build regenerates them, and
**sokol-shdc is not vendored and is not referenced by any script.**

Custom pipelines are nonetheless reachable — `sgp_make_pipeline`,
`sgp_set_pipeline`, `sgp_set_uniform` and `sgp_pipeline_desc` are all bound in
`collections/vendor/sokol/sokol_gp/sokol_gp.dado`, and `app:draw` never calls
any of them. **A grid shader would be the first shader in this tree.**

### 1.4 Unicode — what exists, and what does not

`collections/core/chars/utf8.dado` is the whole of it, and it is better than
expected:

* `decode` is correct UTF-8 with over-long, surrogate and out-of-range
  rejection to U+FFFD; it never fails and always advances.
* **`width(cp)` is already a wcwidth** — `ZERO_RANGES` (combining marks, ZWSP,
  bidi marks, variation selectors) and `WIDE_RANGES` (CJK, Hangul, fullwidth,
  common emoji planes), scanned linearly by `in_ranges`.
* `display_width` and `fit` are built on it.

**Nothing in `app:font` or `app:text` ever calls `chars.width`.** The table and
the pen loop have never been connected. That is the single cheapest correctness
win available.

Absent entirely, anywhere in the tree: grapheme cluster segmentation,
normalisation, bidi (only the *marks* appear, as zero-width entries), complex
shaping, line breaking, and case folding beyond ASCII. Both text layers say so
in their own headers.

### 1.5 Performance — there is no text benchmark

`app:draw`'s header states plainly that **nothing in it has been measured on a
GPU** (the session that wrote it had none). `ui.dado` records a *call count* for
`measure`, not a time. `font.dado` records memory only. `example/edit`'s driver
records a load rate and a flat frame time against file size.

There is no glyph/s figure, no ms/frame for a text-heavy scene, and no
benchmark harness for text anywhere under `example/`. **Building one is part of
this work, because it is the only way the bar in §8 can be settled.**

---

## 2. The seven obstructions, ranked

1. **No shader infrastructure at all.** A grid renderer wants one instanced
   draw for the whole screen. That needs the first `sg_make_shader` in the tree
   *and* a cross-backend source story across six slangs. This is the biggest
   single gap and it is a tooling problem before it is a rendering one.
2. **Per-glyph draw commands.** Throughput is bounded by tape append plus one
   `sgp_draw_textured_rect` per glyph, both CPU-side, with the MVP multiplied
   per-vertex on the CPU by sokol_gp.
3. **Linear scan for non-ASCII glyphs.** Must be a hash, or better, an index
   (§4.2), before any CJK grid.
4. **`columns` hardwired to 1**, with a correct wcwidth table sitting unused
   one package away.
5. **`measure`'s per-pair FFI kerning call.** In a cell grid kerning is
   sub-cell and therefore *illegal*; deleting it removes the cost entirely.
6. **Whole-atlas re-upload on any glyph add.** `sg_update_image` replaces the
   whole image — sokol has no partial-image update — so a new codepoint
   re-uploads the lot. At the 2 MB ceiling that is a visible stall.
7. **No grapheme clustering**, so combining marks cannot compose into one cell.

---

## 2a. One ruling that reaches all of this

`app:` is a runtime framework — an engine — and is allowed to be opinionated
and to do things behind the developer's back (Sam, 2026-09-22). For the text
stack that is not a licence for sloppiness, it is a licence for **three specific things a library would not do**:

1. **The face chain is the framework's.** No program names a font, a size, a
   fallback or a page. The framework picks, measures, derives the cell (§6.1),
   grows pages and re-uploads them.
2. **There is no backend seam.** The grid imports `sokol_gfx` directly rather
   than through a function table. Headlessness is a build-time `@const`, not a
   runtime indirection, and the tree already spells it `-D BACKEND=dummy`.
3. **Atlas residency is automatic.** A cluster that appears in a cell is
   rasterised, packed and uploaded without anyone asking. `example/edit`'s
   current `sync_atlas` — a visible-line codepoint scan the *program* runs — is
   exactly the kind of thing that stops being the program's business.

The cost is stated once and it is this document's only real constraint on the
design: **every hidden thing needs a reader**, because §8 is numeric and a
number nothing can query is a number that quietly stops being true. Page count,
resident clusters, upload bytes, re-recorded cells, evictions — all readable,
whether or not a program would ever read them.

## 3. The plan, in one paragraph

Stop drawing glyphs and start drawing **cells**. The screen is a single array
of fixed-size cells; each cell is a small record; the whole array is uploaded
once and drawn as **one instanced draw call** by one shader of our own. The
glyph atlas becomes **itself a grid** of uniform slots, so the shader computes
its texture coordinates arithmetically and needs no lookup table. Free-pixel
regions and panel chrome draw around the grid in a small, fixed number of layer
passes. `measure` collapses to a column count with no FFI in it.

---

## 4. Architecture

### 4.1 The cell record

Proposed, 12 bytes:

```
slot   u16    index into the atlas grid; 0 = blank
fg     u8[4]
bg     u8[4]   alpha 0 means "the layer under me shows through"
flags  u16     wide-left / wide-right / underline / strike / region-punch
```

A 240×70 grid is 16 800 cells and **202 KB**. Uploading the entire buffer every
frame at 60 Hz is 12 MB/s, which is nothing. **This is a load-bearing
consequence and it should be stated plainly: with one instanced draw, the
per-frame GPU upload stops being worth optimising.**

That changes what the dirty-cell machinery is *for*. `uismoke`'s header already
makes the parallel argument about retention — that it saves the **recording**
and not the **replay**. Here it is the same shape: dirty tracking saves the
**CPU work of deciding what to write into the cell buffer**, not the upload and
not the draw. Anyone who expects dirty cells to save GPU time will be wrong by
exactly the upload, and the perf harness should print both numbers side by side
so that reading cannot happen.

### 4.2 The atlas is itself a grid

Because every cell is the same size, **every glyph slot can be the same size**,
and then the shader computes its UV arithmetically:

```
uv = (slot % cols, slot / cols) * slot_size + frac * slot_size
```

**No glyph-metrics texture, no uniform array, no lookup.** This is only
available because the UI is a cell grid, and it deletes obstruction 3 rather
than fixing it.

* `slot_size` is `(cell_w, slot_h)` with `slot_h >= cell_h`, the slack being
  overshoot room for descenders, accents and box drawing.
* A **double-width** cluster takes two horizontally adjacent slots, and the two
  cells that show it carry `wide-left` / `wide-right` flags.
* Pages are fixed (512×512 proposed) so that adding a glyph re-uploads **one
  page**, not the whole atlas. This is the answer to obstruction 6 and it is
  forced by sokol having no partial-image update.

**The atlas is keyed by cluster, not by codepoint.** A base plus its combining
marks is rasterised *composited* into one slot, which is how Vietnamese `ế` and
Hebrew nikud become one cell. Key: `(face_chain_id, px_size, cluster_bytes)`.

### 4.3 Layer groups

Grid cells cannot express a gradient or a shadow, because both are sub-cell.
So chrome is drawn *around* the grid, not in it:

```
for each layer group, in z order:
    1. chrome pass   — panel fills, gradients, shadows, borders   (sokol_gp)
    2. grid pass     — one instanced draw over that layer's cells (our shader)
    3. region pass   — images, viewports, custom ink, clipped to
                       cell-aligned bounds                        (anything)
```

A layer group is a set of panels at one z: the base UI, each floating window,
the tooltip, a drag ghost. **Four or five groups is the expected steady state**,
so this is four or five grid draws a frame, not twelve thousand.

This is what makes a floating window's chrome sit above the text beneath it
without interleaving per-widget, and it is the reason the design does not
collapse the moment two panels overlap.

### 4.4 Where this sits relative to `app:draw`

The grid pass is **raw `sokol_gfx`**, not a tape opcode: its own `sg_pipeline`,
its own vertex and instance buffers, its own `sg_draw`. `app:draw` keeps the
chrome and region passes. The two interleave by `sgp_flush()` between them,
which is the supported way to mix sokol_gp with hand-written sokol_gfx.

Chose against: adding a `CMD_GRID` opcode to `app:draw`. Cost: two rendering
paths to reason about instead of one. Reversal condition: if the flush between
passes turns out to cost more than the per-glyph tape it replaces — measure it
before believing either way.

---

## 5. The shader story

This is the first shader in the tree and therefore the first decision.

**sokol_gp's precedent is precompiled-and-committed**: `sokol_gp.glsl.h` is
sokol-shdc output, committed beside its `.glsl` source, embedded by the header,
and regenerated by nobody. Sam's ruling of 2026-09-22 points the same way —
*vendor shaders, sokol's shader system, precompiled to header form as sokol_gp
does.*

So the proposal is:

* one `.glsl` source, `--slang glsl410:glsl300es:hlsl4:metal_macos:metal_ios:wgsl`,
  matching sokol_gp's own invocation;
* the generated `.h` **committed**;
* the regeneration command written into the work order and **not** into any
  gate step, because a gate step that names a document is refused outright by
  audit H, and a gate step that shells out to a binary nobody has installed is
  a step that is permanently blocked.

**Q2 is ruled: sokol-shdc is vendored**, all five upstream prebuilds, at
`collections/vendor/sokol/sokol_shdc` (Sam, 2026-09-22). Its README carries the
provenance, the hashes and the invocation. Two findings from landing it belong
here rather than there:

* **The vendored compiler and the vendored `sokol_gfx.h` agree.** `sokol_gfx`
  went through an image→view refactor upstream; today's shdc emits `VIEW_*`
  bind slots, and the header vendored here holds
  `sg_view views[SG_MAX_VIEW_BINDSLOTS]`. A header generated by this binary is
  one this `sokol_gfx` can consume. That was measured, not assumed.
* **`../sokol_gp/shaders/` is stale and nothing builds it.** Regenerating
  `sokol_gp.glsl.h` with this binary produces a 368-line diff, because
  `sokol_gp.h` carries its shader as inline source strings selected by
  `sg_query_backend()` and never includes that header. Do not read that diff as
  a version mismatch and do not "fix" it.

**The shader itself is small.** Vertex: expand an instance index into a cell
quad, emit `fg`, `bg` and the arithmetic UV of §4.2. Fragment: sample the
atlas's alpha, `mix(bg, fg, coverage)`. That is the entire thing, and it is
deliberately trivial so that the first shader in the tree is not also the
hardest.

Two settings that are part of the design and not incidental: the grid sampler
is **NEAREST**, and cell origins are **integers**. Together they undo the
bilinear blur noted in §1.2.

---

## 6. The cell, the cluster, and the column

### 6.1 Deriving the cell

**The cell width is measured off the live primary face, never assumed.** IBM
Plex Mono's advance is 0.60205em, not the 0.6 that every spec sheet implies; at
14 px that is 8.42875 px rather than 8.4, and the 0.34 % error accumulates to
**2.3 px over an eighty-character line**. That figure was measured in a browser
against the real face while prototyping, not in Dado, and it should be re-derived
here at load time rather than believed from this sentence.

Consequence: the cell is only knowable *after* the face resolves, so layout
cannot be computed before that point.

### 6.2 The pipeline, end to end

```
bytes → UTF-8 decode → cluster → column width → slot → cell
```

* **decode** is `chars.decode`, which already exists and is already correct.
* **cluster** is a base codepoint plus any following zero-width codepoints.
  Full UAX #29 is **not** needed and is not proposed: the script exclusions
  (no cursive, no Indic) mean there is no reordering, so a cluster is a base
  and its trailing marks and nothing more.
* **column width** is `chars.width`, which already exists — replace the linear
  `in_ranges` scan with a binary search or a two-level table, and **connect
  it**, which nothing currently does.
* **slot** is the atlas-grid index from §4.2.

### 6.3 What `measure` becomes

A column count. No FFI, no kerning, no per-glyph atlas lookup, no second
decode. **Kerning is sub-cell and therefore illegal in this design**, which
means obstruction 5 is deleted rather than optimised.

### 6.4 The bidi seam — built now, filled later

Sam has ruled RTL in scope with implementation deferred. Bidi is a
*permutation* and not a transformation — it changes no cluster's width, no
column count and no line width, only which column each already-determined cell
sits in — so it defers cleanly **provided the seam exists from the first line
of code**:

* `LogicalOffset` and `VisualCol` are **distinct types**, with `visual_of` and
  `logical_of` that are the identity function today;
* a line is a **sequence of runs with a direction**, today always one LTR run;
* selection is a **list of rects**, today always of length one;
* caret is `(offset, affinity)`, affinity ignored today;
* caret motion goes through `next_visual`, never `offset + 1`;
* paragraph direction is a field on a text region, defaulting LTR.

The risk is not the renderer, it is the **public widget API**. If a widget can
only move a caret by calling `next_visual`, the eventual retrofit is one
function body; if it can do arithmetic on an offset, it is everywhere. Note
that today's `LineEdit` does `st.caret = st.caret + 1` in three places and
`example/edit` does byte arithmetic throughout — both are being replaced, which
is exactly why the seam costs nothing to install now.

---

## 7. What gets deleted

* **`measure`'s kerning loop** and its per-pair FFI call (§6.3).
* **The `extra_cp[]` linear scan** — replaced by the atlas grid index (§4.2).
* **Per-glyph `CMD_TEXTURE_RECT_REGION`** for grid text. `app:text`'s pen loop
  stays for anything drawing text *inside a region*, where there is no grid.
* **`cell_at`'s `columns = 1`.**
* **Whole-atlas re-upload** — replaced by pages.
* **`LineEdit` entirely.** `TEXTCAP = 256`, an inline `[256]char8`, an
  ASCII-only input gate (`cp >= 32 && cp < 127`), a click that slams the caret
  to end of text, and no selection at all. Nothing in it survives contact with
  a text editor; the new widget shares no code with it.
* **`ItemState`'s storage** — `[1000]ItemSpan` is ~324 KB of fixed array per
  list. **Keep its pixel-offset scroll model**, which derives the first visible
  row from a pixel offset and breaks out of the paint loop at the bottom edge;
  that is genuinely O(viewport) and is the right model for a text viewport too.
  `example/edit`'s integer `g_top` is the weaker one and cannot smooth-scroll.

---

## 8. The bar, and how it is proven

Sam's bar: **open a 100 000-line text file easily.** That is a scaling claim
and it needs to be turned into commands.

What the repository already records about `example/edit` (read, not re-run):
load is roughly 0.5 µs a line — about 23 ms for 50 000 lines — and frame time
is already flat in file size because only visible rows are walked. **So drawing
is not the wall.** The wall is two things the buffer's own header names:

1. **one `#make` per line**, so 100 000 lines is 100 000 allocations; and
2. **a line-count change memmoves the whole line array** — at 100 000 lines an
   Enter near the top moves about 1.6 MB, and it is the one operation that is
   O(file) rather than O(line).

Both fixes are named in that header and neither moves a caller, because all
mutation funnels through `raw_insert`/`raw_delete` and all reads through
`nlines`/`llen`/`lbyte`: a **slab** for line storage, and a **gap over the line
array**.

### 8.1 The ratchet

**There are no target numbers here, and their absence is the design.** An
earlier draft of this document carried a table of proposed milliseconds. They were invented, this repository's rule is *write the
derivation, not the number*, and inventing a bar for a renderer nobody has built
is how a design gets optimised toward a guess.

So: the instrument is built at CP-S, it prints from the first frame there is
one, **the first run's measurement becomes the ceiling**, and every checkpoint
that claims it will move a number moves it and records the new ceiling with the
date, the machine and the compiler. It fails over the ceiling (the regression)
and under it with nobody lowering it (the slack). This is the shape audit G used
to take 7 005 citation lines to zero, and the shape of the fall is the argument.

What is measured, all of it printed every run:

| what | how |
|---|---|
| load a 100k-line file | `#! load_ms` |
| steady idle frame, CPU | `#! frame_cpu_us` |
| scroll one page | `#! scroll_cpu_us` |
| type one character at line 1 of 100k | `#! edit_cpu_us` |
| draw calls a frame | `gpu_draw_calls` — expect layer groups, not cells |
| cell-buffer upload a frame | `#! upload_bytes` |
| re-recorded cells a frame, idle | `#! cells_recorded` |
| resident clusters, atlas pages, page uploads | `#! atlas_*` |

**The last two rows of the first block are the honest pair** from §4.1, and they
are printed side by side deliberately: with one instanced draw the upload does
not fall when the re-record count does. A reader who takes "dirty cells" to mean
"an idle frame is free" will be wrong by exactly the upload, and two numbers
beside each other is what stops that reading — the same argument `uismoke`'s
header makes about retention saving the recording and not the replay.

**Sam's bar — *open a 100 000-line file easily* — is read off this at CP-6.**
It is an exit condition judged with the harness running, not a number guessed at
before the first line of code.

### 8.2 Where the harness lives

**Under `ide/`**, not under `example/` — the editor is the only consumer of
this framework and `example/` is the author's scratch, deletable at any time.
It has its own `run.sh` + `check.py`, Xvfb and `xdotool`,
following the pattern `example/uismoke` and `example/edit/verify` established
before either was retired: the program prints machine-readable `#!` lines and
the checker asserts them **against the pixels**. It is **not** a gate step.

Putting it inside the editor rather than beside it has a second effect worth
having: the thing being measured is the **program**, not a fixture written to
flatter the framework.

A 100 000-line fixture must be **generated by the harness**, not committed.

---

## 9. Open questions for the work order's register

1. **sokol-shdc**: vendor the binary, or commit generated headers and
   regenerate by hand as the vendored sokol_gp copy already does?
2. **Raw `sokol_gfx` for the grid pass** (§4.4) — confirm, or keep everything
   inside `app:draw`?
3. **Emoji**: a text cell with a colour atlas page, or an image in a two-cell
   region? The region answer is free; the cell answer needs a second atlas
   format.
4. **East Asian Width `A` (ambiguous)** — recommend **always 1**. It needs
   ruling rather than discovering.
5. **Atlas page size and page count ceiling**, which together set the memory
   ceiling for a multi-face, multi-size grid.
6. **The numbers in §8.1.**
