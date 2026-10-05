<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:font

`app:font` — a glyph atlas and the metrics a layout engine needs.

    import "app/font"

    (font.Font f, bool from_disk) = font.load_file(font.default_path(), 16.0)
    defer font.destroy(&f)

    font.Glyph g = font.glyph(&f, 'A')
    f32 wide = font.measure(&f, "hello, world")

Three things a text view asks for that a `measure` cannot: where the tab
stops are, which byte the mouse landed on, and where an underline goes.

    font.Tabs t = font.tabs(4.0 * font.cell_width(&f), 4)
    f32 caret = font.advance_to(&f, line, cursor_byte, t)
    (i64 at, i32 col) = font.index_at_x(&f, line, mouse_x, t)
    (f32 pos, f32 thick, bool real) = font.underline(&f)

**What this package answers is *where the ink is*, and nothing about how it
reaches a screen.** It rasterises a TrueType face into one RGBA8 image and
reports, per codepoint, the rectangle of that image the glyph occupies, the
offset from the pen to that rectangle, and how far the pen then moves. A
drawing surface turns those into two triangles; this package never sees one,
which is why it has no dependency on `app:draw` and can be tested with no
GPU and no window.

## Every texel is white and the coverage is the alpha

`(255, 255, 255, coverage)`, and the three colour bytes are not padding.
`sokol_gp`'s stock shader is `texture(tex, uv) * color`: a white texel hands
the tint's rgb back unchanged and an alpha texel scales the tint's alpha, so
the product is `vec4(tint.rgb, tint.a * coverage)` — which is exactly what
`app:draw`'s A8 fragment computed, and it now computes it with no shader at
all. That fragment was carried as three source strings chosen off
`sg_query_backend()` — GLSL 4.10, GLSL ES 3.00 and MSL — of which one is
pixel-verified, one has never been compiled by any machine in this tree, and
three further backends had no string at all. This format deletes the reason
every one of them existed.

**It costs four times the memory and that is the whole of the cost.** The
fallback face at 16 px packs a 256x256 atlas, so 64 kB becomes 256 kB; the
ceiling — width 128, height `MAX_ATLAS_H` — was 512 kB and is 2 MB. Nothing
else moves: the boxes, the packer, the metrics and the two counters are what
they were.

**The unwritten texel is `(255, 255, 255, 0)` and not four zeroes.** A
transparent *black* texel fringes dark wherever a bilinear sample straddles
the edge of a box, because what the sample interpolates towards is black; a
transparent *white* one cannot, because what it interpolates towards is the
colour the tint already is. That is the single easiest thing to get wrong
here, so `pack` fills the whole image that way before anything is inked and
`grow_atlas` fills the rows it adds the same way.

### Why the rasteriser writes somewhere else first

`stbtt_MakeCodepointBitmap` and `stbtt_MakeGlyphBitmap` write **one coverage
byte per pixel at a byte stride**. No argument makes them skip three bytes
and there is no second entry point that writes four, so nothing stb offers
can write into an RGBA image. A glyph is therefore rasterised into a
**per-font scratch buffer** — one, allocated once at load, sized to the
largest box this face can produce at this scale from
`stbtt_GetFontBoundingBox` — and expanded from there into the atlas.

The alternative considered was a full-size A8 shadow of the atlas, expanded
wholesale at upload time. That holds `atlas_w * atlas_h` more bytes for the
life of the `Font`, and it re-expands the whole image where the scratch
expands only the box that changed. Measured on DejaVu Sans Mono at 32 px: the
atlas is 256x512, so the shadow would be **128 kB** and the scratch — the
face's own bounding box at this scale — is 37 by 40, **1480 bytes**. The
fallback face allocates neither, because its blitter writes texels directly
and never reaches stb at all.

## The two sources, behind one type

A `Font` comes from one of two places and a caller cannot tell which without
asking `source`:

  * **`Source.TrueType`** — `stb_truetype` rasterised the outlines of a real
    `.ttf` or `.ttc`. Kerning is real, the face may be proportional, and the
    metrics are the face's own.
  * **`Source.Fallback`** — the 8x8 monochrome ASCII bitmap compiled into
    this file as `FALLBACK_BITS`, scaled up by an integer factor. Always
    available, needs no file, no allocator luck and no font on the machine.

Both fill the same `Font`, so **every function below works on either** and a
program that only ever draws text does not branch. That is the whole point
of the fallback: a text editor on a machine with no fonts still shows its
buffer.

## Ownership

`load` **copies** the bytes it is handed. `stbtt_fontinfo` holds a pointer
into the font file and this package keeps it alive for kerning, so a `Font`
that borrowed the caller's buffer would answer `measure` out of freed memory
the moment that buffer went away. One copy at load, freed by `destroy`.

## Where a `Font`'s memory comes from

**No factory here takes an allocator.** `load`, `load_file` and `fallback`
allocate to the ambient one, so where a face lands is a sentence the caller
writes once, at the call, and never threads through its own signatures:

    Font f = fallback(16.0)                              // the caller's default
    Font f = fallback(16.0) using arena.allocator(&a)    // the face lives in `a`

This used to be an `Allocator` parameter on all three, which put the choice
in the callee's parameter list and made every intermediate function carry a
value it had no use for. `using` reaches the whole dynamic extent, so one
word at the outermost call places the copied `.ttf` bytes, the
`stbtt_fontinfo`, the dense glyph table, the scratch box and the atlas
image alike.

**`Font.alloc` survives, and it is not there so `destroy` can free.**
Freeing never needed it — `#delete` uses the allocator the `ref` already
carries, so every line of `destroy` works with the slot gone. It is there
because a `Font` keeps allocating *after* it is loaded, and nowhere near the
load: `add_codepoint` mints the sparse `extra_cp`/`extra` pair on the first
non-ASCII codepoint anybody measures, and `grow_atlas` remakes `pixels` when
a shelf runs out. Both happen at a draw, under whatever ambient the drawing
code is standing in. Reading the ambient once, at load, is what keeps a
face's later growth in the same place as the face — and a glyph added from
inside a scratch frame out of an arena that is about to roll back.

## Coordinates and signs, stated once

Pixels, y **down**, the pen on the baseline — `stb_truetype`'s convention,
not inverted, because inverting it here would put the one place the two
disagree inside a package that cannot test the difference.

  * `ascent` is **positive**: pixels from the baseline up to the top of the
    face.
  * `descent` is **negative**: pixels from the baseline down to the bottom.
    That is `stbtt_GetFontVMetrics`' own sign and this package does not
    "fix" it, because a caller that adds `ascent - descent` and a caller
    that adds `ascent + descent` must not both look right.
  * `Glyph.bearing_y` is the offset from the baseline to the **top** of the
    glyph's box, so it is negative for anything that rises above the
    baseline — which is nearly everything.
  * `Glyph.bearing_x` is the offset from the pen to the **left** of the box.

## Codepoints outside ASCII, and how one gets added

`load` packs **32..126** densely. `add_codepoint` packs anything else, one
at a time, from the shelf packer's retained cursor — so an atlas grows to fit
the text a program actually meets rather than to fit Unicode.

**The hard part is not the packing, it is telling the drawing surface.** A
rasterised glyph is bytes in `pixels`; the GPU holds a copy made at upload
time, and nothing in this package may know that. The answer is a
**monotonic generation counter** — `atlas_generation` — bumped by anything
that writes a pixel. A consumer keeps its own last-seen number and re-uploads
when the two differ.

    if font.atlas_generation(&f) != g_uploaded:
        _ = draw.texture_update(g_atlas, font.atlas_pixels(&f))
        g_uploaded = font.atlas_generation(&f)

The three candidates and why this one:

  * **A callback** puts an arbitrary consumer inside this package's call
    stack at rasterise time. That is a layering inversion — `app:font`
    compiles against no drawing surface, which is the claim the package
    exists to make — and it fires at the worst moment: sokol allows one
    update per image per frame, and `add_codepoint` is naturally called from
    the middle of a paint pass.
  * **A `flush` that reports and clears** is a *one-shot*. Two consumers of
    one face — a UI backend and a client's own overlay — cannot both learn,
    because whichever asks first consumes the fact. That is a defect that
    appears only in the program that has two consumers, which is the worst
    time to find it.
  * **A counter** is a value with no owner. It can be read any number of
    times by any number of consumers, each keeping its own watermark; it
    costs one `i32`; and each consumer chooses *when* to compare, which is
    what makes it fit sokol's once-per-frame rule instead of fighting it.

## The atlas never moves, and grows only when it is asked to

**`add_codepoint` never grows.** When the shelf runs out of room it answers
`false` and the codepoint keeps answering `.notdef`, exactly as before — a
grow is a texture the drawing surface has to *remake*, and no consumer
expects one in the middle of a paint pass. Who asks is the caller, and when
is between frames:

    if !font.add_codepoint(&f, cp):
        if font.grow_atlas(&f):
            _ = font.add_codepoint(&f, cp)

`grow_atlas` doubles the height and keeps the width. Every glyph therefore
stays on the pixel it was already on and the image is *copied*, not
re-rasterised — and because a `Glyph` carries a **pixel** rectangle and
nothing normalised, **no `Glyph` this package has handed out is stale after a
grow.** A box at `(px, py)` of `pw` by `ph` is at `(px, py)` of `pw` by `ph`,
and a pixel source rectangle is what `sgp_draw_textured_rect` takes, so
nothing divides by a denominator that moved.

**That was not always so, and the repair it needed is worth recording rather
than quietly deleting.** While a `Glyph` carried `u0`/`v0`/`u1`/`v1`, `v` was
`py / atlas_h`, so a grow staled every glyph ever handed out in `v` alone —
invisible to a caller, and drawn as the right ink at half the right height. A
`rewindow` pass walked all 96 dense slots and every sparse one to repair it,
and the invariant "a `Glyph` is valid for the life of the `Font`" had to be
narrowed to "until the next `grow_atlas`". Pixel rectangles gave the
invariant back whole, and `rewindow` went with the hazard it existed for.

**Two things a grow does still cost, and neither is a `Glyph`:**

  * `atlas_pixels` is a **borrow**, and the grow is a `#resize` that may move
    the block. A `[]u8` taken before a grow points at freed memory after one.
    Ask again — every consumer already does, because the run it wants is the
    one it is about to upload.
  * the texture's **shape** changes, so a consumer owes its drawing surface a
    destroy and a remake rather than an update. That is `atlas_resizes`, and
    the next section is the whole argument for why it is a second counter.

### Why a grow does not bump `atlas_generation`

This is the whole of what keeps a grow from silently corrupting a texture, so
it is stated here rather than left for a consumer to work out.

`atlas_generation` means *the pixels changed — update the texture in place*.
An in-place update uploads the texture's **own** byte count; a grown atlas is
longer than that. `app:draw`'s `texture_update` guards the run it is handed
with `<` rather than `!=` (`collections/app/draw/device.dado`), so an oversized
run **passes the guard, is truncated to the old size, and answers `true` — on
the GPU and on the headless surface alike**. A grow that announced itself
through `atlas_generation` would hand every consumer already written a silent
half-upload and a screenful of garbage.

So it does not. **`grow_atlas` leaves `atlas_generation` exactly where it
was** and bumps `atlas_resizes` instead — a second watermark meaning *the
atlas has a different shape; destroy the texture and make a new one*. A
consumer written before growth existed watches `atlas_generation`, never
calls `grow_atlas`, and cannot reach any of this. A consumer that grows
watches both, and the resize is tested first:

    if font.atlas_resizes(&f) != g_resized:
        draw.texture_free(g_atlas)
        (i32 w, i32 h) = font.atlas_size(&f)
        g_atlas = draw.texture_create(w, h, font.atlas_pixels(&f))
        g_resized = font.atlas_resizes(&f)
        g_uploaded = font.atlas_generation(&f)
    else if font.atlas_generation(&f) != g_uploaded:
        _ = draw.texture_update(g_atlas, font.atlas_pixels(&f))
        g_uploaded = font.atlas_generation(&f)

`draw.texture_create` and not `draw.texture_a8`: there is no A8 texture any
more, because there is no A8 shader any more. Every texture `app:draw` holds
is RGBA8 and this atlas is handed over as the RGBA8 run it is.

### The ceiling

The height doubles and stops at `MAX_ATLAS_H`. Past it `grow_atlas` answers
`false` and changes nothing at all — `add_codepoint` keeps answering `false`
and `glyph` keeps answering `.notdef`, which is what a program had before
growth existed, some doublings later. There is no unbounded case.

## Declarations

81 declarations, 43 public.

* `enum Source: (Fallback, TrueType)` — Where a `Font`'s pixels came from. A caller that must know — to warn, to…
* `const char32 FIRST = ' '` — The first and last codepoints `load` packs densely. Everything between them…
* `const char32 LAST = '~'`
* `type Glyph` — A codepoint's place in the atlas, and how to set it down.
* `type Font` — A rasterised face: one RGBA8 image, the boxes cut out of it, and the…
* `Font load([]u8 ttf_bytes, f32 pixel_height)` — Rasterise `ttf_bytes` at `pixel_height` and answer the atlas.
* `(Font f, bool ok) load_file(string8 path, f32 pixel_height)` — The same, reading the file itself.
* `Font fallback(f32 pixel_height)` — The built-in 8x8 bitmap, at the nearest whole multiple of 8 that does not…
* `void destroy(^Font f)` — Give back everything a `Font` owns. Safe on a fallback (there is no `.ttf`
* `Glyph glyph(^Font f, char32 cp)` — Where `cp` lives.
* `Glyph notdef(^Font f)` — The glyph every unknown codepoint answers. `present` is false and everything…
* `bool has(^Font f, char32 cp)` — Whether this atlas has `cp` itself, as opposed to answering `.notdef` for it.
* `f32 measure(^Font f, string8 s)` — The advance width of `s`, with kerning where the face has any.
* `type Tabs: (f32 width, i32 columns)` — Where the tab stops are: `width` in pixels and `columns` in characters. Both…
* `Tabs tabs(f32 width, i32 columns)` — The tab stops a text view usually wants: `width` pixels and `columns`
* `Tabs no_tabs()` — No tab stops at all.
* `type Cell: (char32 cp, i32 nbytes, f32 advance, i32 columns, bool tab)` — One character of a run, and everything a walk needs to take a step.
* `Cell cell_at(^Font f, string8 s, i64 at, f32 pen, i32 column, Tabs t)` — The character of `s` beginning at byte `at`, as it lands when `pen` pixels…
* `f32 advance_to(^Font f, string8 s, i64 upto, Tabs t)` — The pen x of byte offset `upto`, measured from the run's origin. An offset…
* `i32 column_to(^Font f, string8 s, i64 upto, Tabs t)` — The character column of byte offset `upto` — what a status line calls *Col*.
* `(i64 at, i32 column) index_at_x(^Font f, string8 s, f32 want, Tabs t)` — The character `want` pixels along the run lands on, rounded to the nearer…
* `f32 measure_tabbed(^Font f, string8 s, Tabs t)` — The advance width of the whole run with tab stops applied. `measure` is this…
* `f32 kern(^Font f, char32 a, char32 b)` — The kerning adjustment between two codepoints, in pixels. Zero for a…
* `f32 line_height(^Font f)` — Baseline to baseline: `ascent - descent + line_gap`, with `descent`
* `f32 ascent(^Font f)` — Pixels above the baseline. Positive.
* `f32 descent(^Font f)` — Pixels below the baseline. **Negative** — see the package comment.
* `f32 pixel_height(^Font f)` — The size this face was rasterised at.
* `bool monospace(^Font f)` — Whether every glyph in the dense block advances the same distance. A layout…
* `f32 cell_width(^Font f)` — The nominal width of one character cell: the advance of `'0'`, or of a space…
* `f32 cap_height(^Font f)` — Pixels from the baseline to the top of a capital letter, measured off this…
* `f32 x_height(^Font f)` — Pixels from the baseline to the top of a lowercase `x`, measured off this…
* `(f32 position, f32 thickness, bool from_face) underline(^Font f)` — Where to draw an underline, and how thick.
* `[]u8 atlas_pixels(^Font f)` — The image: RGBA8, row-major, four bytes per texel, stride `atlas_w * 4`.
* `(i32 w, i32 h) atlas_size(^Font f)`
* `i32 atlas_generation(^Font f)` — How many times this atlas's pixels have changed. Zero for a `Font` that…
* `i32 atlas_resizes(^Font f)` — How many times this atlas has changed **shape**. Zero for a `Font` that has…
* `bool grow_atlas(^Font f)` — Double the atlas's height, keeping its width. Answers whether it grew.
* `(f32 x, f32 y, f32 w, f32 h) solid_src(^Font f)` — A fully-opaque point inside the atlas, as a **zero-extent source rectangle**
* `(i32 x, i32 y, i32 w, i32 h) solid_box(^Font f)` — The reserved block's pixel rectangle. A test looks at this; **a drawing…
* `bool add_codepoint(^Font f, char32 cp)` — Rasterise `cp` into the first free space on the atlas's retained shelf and…
* `i32 added(^Font f)` — How many codepoints have been added past the dense block.
* `string8 default_path()` — A monospace face this platform is likely to have, or `""` where there is…
* `` const [95]u64 FALLBACK_BITS = [ 0x0000000000000000, 0x00180018183C3C18, 0x0000000000003636, 0x0036367F367F3636, // ' !"#' 0x000C1F301E033E0C, 0x0063660C18336300, 0x006E333B6E1C361C, 0x0000000000030606, // "$%&'" 0x00180C0606060C18, 0x00060C1818180C06, 0x0000663CFF3C6600, 0x00000C0C3F0C0C00, // '()*+' 0x060C0C0000000000, 0x000000003F000000, 0x000C0C0000000000, 0x000103060C183060, // ',-./' 0x003E676F7B73633E, 0x003F0C0C0C0C0E0C, 0x003F33061C30331E, 0x001E33301C30331E, // '0123' 0x0078307F33363C38, 0x001E3330301F033F, 0x001E33331F03061C, 0x000C0C0C1830333F, // '4567' 0x001E33331E33331E, 0x000E18303E33331E, 0x000C0C00000C0C00, 0x060C0C00000C0C00, // '89:;' 0x00180C0603060C18, 0x00003F00003F0000, 0x00060C1830180C06, 0x000C000C1830331E, // '<=>?' 0x001E037B7B7B633E, 0x0033333F33331E0C, 0x003F66663E66663F, 0x003C66030303663C, // '@ABC' 0x001F36666666361F, 0x007F46161E16467F, 0x000F06161E16467F, 0x007C66730303663C, // 'DEFG' 0x003333333F333333, 0x001E0C0C0C0C0C1E, 0x001E333330303078, 0x006766361E366667, // 'HIJK' 0x007F66460606060F, 0x0063636B7F7F7763, 0x006363737B6F6763, 0x001C36636363361C, // 'LMNO' 0x000F06063E66663F, 0x00381E3B3333331E, 0x006766363E66663F, 0x001E33380E07331E, // 'PQRS' 0x001E0C0C0C0C2D3F, 0x003F333333333333, 0x000C1E3333333333, 0x0063777F6B636363, // 'TUVW' 0x0063361C1C366363, 0x001E0C0C1E333333, 0x007F664C1831637F, 0x001E06060606061E, // 'XYZ[' 0x00406030180C0603, 0x001E18181818181E, 0x0000000063361C08, 0xFF00000000000000, // '\\]^_' 0x0000000000180C0C, 0x006E333E301E0000, 0x003B66663E060607, 0x001E3303331E0000, // '`abc' 0x006E33333E303038, 0x001E033F331E0000, 0x000F06060F06361C, 0x1F303E33336E0000, // 'defg' 0x006766666E360607, 0x001E0C0C0C0E000C, 0x1E33333030300030, 0x0067361E36660607, // 'hijk' 0x001E0C0C0C0C0C0E, 0x00636B7F7F330000, 0x00333333331F0000, 0x001E3333331E0000, // 'lmno' 0x0F063E66663B0000, 0x78303E33336E0000, 0x000F06666E3B0000, 0x001F301E033E0000, // 'pqrs' 0x00182C0C0C3E0C08, 0x006E333333330000, 0x000C1E3333330000, 0x00367F7F6B630000, // 'tuvw' 0x0063361C36630000, 0x1F303E3333330000, 0x003F260C193F0000, 0x00380C0C070C0C38, // 'xyz{' 0x0018181800181818, 0x00070C0C380C0C07, 0x0000000000003B6E // '|}~' ] `` — **`font8x8_basic` by Daniel Hepper — Public Domain.**
