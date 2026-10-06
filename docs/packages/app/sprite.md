<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 2b5ebeeb9435 (dirty) -->
# app:sprite

`app:sprite` — a sheet cut into frames, a resource holding **named**
animations over them, and the player that walks one, recorded into a canvas
item.

    import "app:draw"
    import "app:sprite"

    sprite.Sheet sheet = sprite.grid(tex, 16, 16)

    sprite.SpriteFrames frames
    sprite.add_animation(&frames, "walk", 12.0, true)
    sprite.add_frames(&frames, "walk", 0, 4)

    sprite.AnimatedSprite hero = sprite.animated_sprite()
    hero.flip_h = facing_left
    sprite.play(&hero, &frames, "walk")

    // once per tick, with the frame's own dt
    sprite.advance(&hero, &frames, dt)

    // once per tick that re-records, which is not the same thing
    draw.canvas_item_clear(item)
    sprite.draw_animated(item, sheet, &frames, &hero, x, y, 2.0, WHITE)

The two loops above are deliberately not one loop, and that is the whole of
what retention changed here. `advance` is a clock and belongs to the
simulation; `draw_animated` is a recording and belongs to whatever decides an
item is dirty. A client that advanced inside the draw call would have tied
its animation speed to how often it happened to re-record — which under an
immediate surface is *every frame* and so looks correct, and under this one
is *whenever something changed*, which for a sprite standing still is never.
So `draw_animated` does not advance, does not take a `dt`, and says so in its
own comment as well as here.

## Why this package exists at all

`app:draw` knows how to put a textured rectangle on the screen and
`app:font` knows how to cut a glyph's box out of an atlas, and **between
them there is no notion of a sprite.** A frame of a sheet, an animation over
frames, and a clock that advances it are three things every game client
needs and none of them is a drawing primitive or a font fact, so every client
writes them — the same integer division into columns and rows, the same
`i32(clock * fps)`, the same forgotten wrap.

That is the argument `app:text` makes for the pen loop, one level over, and
this package is built the same way: it imports `app:draw`, takes a
`draw.Item` as its **first** parameter, records into it, and adds nothing but
arithmetic and a small table. Nothing else in `app:` imports it, so a program
that wants this model takes it and a program that wants its own still writes
one.

**The target comes first**, for the reason `app:text` gives: a client reading
a paint function sees one target named once per line and in the same column
every time, and `draw.rect(item, …)`, `text.run(item, …)` and
`sprite.draw_frame(item, …)` all read the same way.

## The names here are Godot's, and here is the map

`app:draw` is `RenderingServer` with canvas items and `draw_*` calls, and
this package is the two classes that sit on top of it. **Godot splits this
job in two and the split is better than the one this package had**, because
it separates the *data* — which animations exist, what frames they hold, how
fast they run — from the *thing playing it*, which is a clock and a handful
of per-instance flags. Ten sprites playing one walk cycle share one
`SpriteFrames` and hold ten `AnimatedSprite`s, and nothing in the second is a
copy of anything in the first.

A reader arriving from GDScript should not have to infer the mapping, so it
is written out:

| Godot | here | note |
|---|---|---|
| `SpriteFrames` | `SpriteFrames` | the resource; a value, held by `^` |
| `SpriteFrames.add_animation(name)` | `add_animation(f, name, speed, loop)` | the last two are Godot's own defaults folded in |
| `SpriteFrames.has_animation(name)` | `has_animation(f, name)` | |
| `SpriteFrames.add_frame(anim, tex, …)` | `add_frame(f, anim, index)` | a frame is a **sheet index**, not a texture |
| — | `add_frames(f, anim, first, count)` | a contiguous run, which is what a sheet row is |
| `SpriteFrames.get_frame_count(anim)` | `get_frame_count(f, anim)` | |
| `SpriteFrames.get_frame_texture(anim, i)` | `get_frame(f, anim, i)` | answers the sheet index |
| `SpriteFrames.set_animation_speed(anim, fps)` | `set_animation_speed(f, anim, fps)` | |
| `SpriteFrames.get_animation_speed(anim)` | `get_animation_speed(f, anim)` | |
| `SpriteFrames.set_animation_loop(anim, on)` | `set_animation_loop(f, anim, on)` | |
| `SpriteFrames.get_animation_loop(anim)` | `get_animation_loop(f, anim)` | |
| `SpriteFrames.get_animation_names().size()` | `animation_count(f)` | Godot hands back an array; this hands back the count |
| `AnimatedSprite2D` | `AnimatedSprite` | the player; the `2D` is what `app:draw` already is |
| `AnimatedSprite2D.play(name)` | `play(s, f, name)` | **continues** an animation already current — see below |
| `play(name)` + `set_frame_and_progress(0, 0)` | `play_from_start(s, f, name)` | |
| `AnimatedSprite2D.stop()` | `stop(s)` | stops and rewinds |
| `AnimatedSprite2D.pause()` | `pause(s)` | stops and keeps the position |
| `AnimatedSprite2D.is_playing()` | `is_playing(s)` | |
| `AnimatedSprite2D.animation` | `animation_name(s, f)` | the slot `s.animation` is the resolved index — see "names" |
| `AnimatedSprite2D.frame` | `s.frame` | **an index into the animation**, not into the sheet |
| — | `sheet_frame(f, s)` | the sheet index `s.frame` resolves to |
| `AnimatedSprite2D.speed_scale` | `s.speed_scale` | |
| `AnimatedSprite2D.flip_h` / `flip_v` | `s.flip_h` / `s.flip_v` | |
| `AnimatedSprite2D.centered` | `s.centered` | defaults **true**, as Godot's does |
| `AnimatedSprite2D.offset` | `s.offset` | |
| `animation_finished` (a signal) | `s.finished` (a bool) | there are no signals here — a **level**, not an edge |
| `Sprite2D.hframes` / `vframes` | `Sheet.cols` / `Sheet.rows` | a `Sheet` is `grid`'s answer, not a node property |
| `Sprite2D.region_rect` | `frame_src(sheet, index)` | in sheet pixels |

Two names in that table are worth stopping on, because both are places a
GDScript reader's assumption is *right* and a reader of this package's
previous shape would get it wrong.

**`play` continues rather than restarts.** `AnimatedSprite2D::play` calls
`set_animation`, which returns early when the name is the one already set, so
calling `play("walk")` every tick of a walk does not restart the walk. This
package previously spelled that `resume` and gave `play` the restarting
meaning; the names have swapped, and `play_from_start` is the restart. The
argument for having both is unchanged and is still the best reason to read
this pair: a hit reaction retriggers on every hit and must snap back to frame
zero, and a walk cycle asked for again on the next tick must not.

**What "already playing" means is the animation, and not its speed or its
loop flag.** `play` compares the resolved slot, so a caller that changes an
animation's speed with `set_animation_speed` and then calls `play` with the
same name gets the new speed applied from here on and does not get the
animation restarted. There is also nothing here that compares two `f32`s for
exact equality to decide it, which is the other reason the comparison is of
the slot.

**`frame` is the animation's index and not the sheet's.** Godot's
`AnimatedSprite2D.frame` counts within the current animation — `0` is that
animation's first frame whatever texture it holds — where `Sprite2D.frame` is
a cell of the sheet. Both facts exist here and they are two different
numbers, so they have two names: `s.frame` is Godot's animated one, and
`sheet_frame(f, s)` is the cell it resolves to. The previous shape had only
the second and called it `frame`, which is the spelling a GDScript reader
would have read as the first.

## Names live in the resource, copied, capped, and never stored as `string`

This is the one thing the package gained that costs storage, and the decision
has two halves.

**A `string` is a window and is never held here.** `add_animation` copies the
name's bytes into a fixed array inside the `SpriteFrames` and every lookup
compares against the copy. The alternative — keeping the caller's `string` in
the table — stores a pointer and a length into memory this package does not
own, and the trap is not hypothetical: `tools/doc` once printed a completely
correct document while ASan reported `heap-use-after-free` in the sort that
ordered its records, because the records held windows into a file buffer that
had been freed. A name read out of a level file, a `#format`
buffer or a scratch arena would do exactly that here, and it would do it only
once the arena was reused, which is the shape of bug that ships.

**This is `app:input`'s convention and not a second one.** That package is
solving the identical problem for action names — `InputMap.add_action(name)`,
`is_action_pressed(name)` — and it got there first; this package read what it
settled on and agreed with it rather than inventing a second spelling of the
same table. Point for point:

  * a **fixed table** sized by two `const`s that have to justify themselves,
    no allocator anywhere, because neither package has ever allocated;
  * the name's **bytes copied in** at registration, with the caller's
    `string` never stored;
  * a name cap of **24 bytes**, arrived at independently on both sides from
    counting the names that actually exist;
  * **a longer name is a refusal and not a truncation** — `add_action` and
    `add_animation` both answer `false` and store nothing — because a name is
    a key, and truncating a key silently merges two of them;
  * the cap is **a `bool` a caller can see** rather than a silent drop, so a
    startup path that checks its return values outgrows a cap visibly;
  * and a registered name is held afterwards **as a slot index**, so nothing
    downstream carries a window.

`app:input` also has the older, looser case beside it — the drop table, a
fixed `[MAX_DROPPED][DROP_PATH_MAX]char8` that **truncates** an over-long
path. That is not an inconsistency and the line between the two is worth
naming, because it is the line this package sits on the same side of as the
action map: a path is **content**, so truncating it loses something the
client can see it lost, and a name is a **key**, so truncating it silently
makes two different things one.

The one thing that is this package's own is where the table lives. `app:input`
keeps its actions in package storage, because there is one keyboard; a
`SpriteFrames` is per character and a program holds several, so it is a value
the client declares and this package holds nothing at all.

**What a name borrows, and for how long.** `animation_name` answers a
`string` windowing the `SpriteFrames` it was given. That borrow is bounded by
the resource, which a client owns and usually holds for the life of a level —
a far longer and far more obvious lifetime than `dropped_path`'s ring slot.
It is still a borrow, and a client that keeps a name past the resource copies
it with `#format`.

**The sprite stores the resolved slot and not the name**, which is the same
decision one level on: an `AnimatedSprite` that held a `string` would hold a
window into a `SpriteFrames` that a client may copy, move or drop, and there
would be no way to check it. An `i32` slot cannot dangle; the worst it can do
is name an animation a *different* resource does not have, and every reader
here range-checks it and answers a value.

## A frame is pixels, not uv

`frame_src` answers a rectangle **in sheet pixels** — the unit
`draw.texture_rect_region` takes, which `sgp_draw_textured_rect` divides by
the bound image's size itself. There is no normalised form of it here and
there will not be one.

This is said out loud because a sprite sheet is the second place somebody
reaches for uv, and the first place was `app:font`, which carried
`u0`/`v0`/`u1`/`v1` on every `Glyph` and a whole `rewindow` pass to repair
them. The division has the atlas height underneath it, so growing the atlas
staled every glyph that had ever been handed out. S2 deleted both.

A sheet is not grown the way an atlas is, so the staleness hazard is not the
same hazard — but the invariance is the same invariance and it is worth
having for a plainer reason: a `Sheet` is five integers that a client may
hold in a table, write to a level file and compare against what an artist
typed into a tool, and `(32, 0, 16, 16)` is all four of those things where
`(0.5, 0.0, 0.25, 0.333333)` is none of them.

## `flip_h` and `flip_v` cost nothing, and here is why

A mirrored frame is **the same command with a negative source extent**. No
second row in the sheet, no transform, no second command, no pipeline change,
and nothing for the batch optimiser to break a run over.

`sokol_gp` builds a textured rect's texture coordinates as

    float tl = rects[i].src.x*iw;
    float tt = rects[i].src.y*ih;
    float tr = (rects[i].src.x + rects[i].src.w)*iw;
    float tb = (rects[i].src.y + rects[i].src.h)*ih;

so a negative `src.w` puts `tr` to the **left** of `tl` and the quad samples
the frame backwards, and a negative `src.h` puts `tb` **above** `tt` and it
samples the frame upside down. **This was read rather than assumed**, in
`collections/vendor/sokol/sokol_gp/sokol_gp.h`: `sgp_draw_textured_rects` at
line 3059, the texcoords at lines 3116–3119, and `sgp_draw_textured_rect` —
the singular form `app:draw` calls — at line 3142, which forwards to it with
a count of one.

**The vertical case was checked and not assumed from the horizontal one**,
because the two axes are not automatically symmetric in a library that flips
`y` somewhere: `w` reaches the vertex through `tr` at line 3118 and `h`
reaches it through `tb` at line 3119, each appearing exactly once and in the
same form, and the texcoord quad at lines 3120–3125 pairs `{tl,tb}`,
`{tr,tb}`, `{tr,tt}`, `{tl,tt}` against a position quad built in the same
bottom-left/bottom-right/top-right/top-left order. So the mechanism really is
one mechanism run on two axes, and `flip_h && flip_v` is the 180° rotation
rather than two half-applied ones. Two further facts were read at the same
time and both matter:

  * **The destination quad is built from `dst` alone**, at lines 3088–3093,
    before the texcoords are computed at all. So a negative source extent
    cannot move, resize or rewind the destination: the sprite mirrors in
    place, on either axis. `sprite_test.dado` asserts exactly that — the
    flipped and unflipped commands carry the *same* `a` and differ only in
    `b`.
  * **Nothing on the path clamps the rectangle.** `app:draw` records `src`
    verbatim into the command (`texture_rect_region`), and `device.dado`'s
    `gp_rect` hands the same four floats to `sokol_gp` unchanged. So the
    negative survives from this call site to the vertex.

The previous shape had `flip_x` only and pointed a caller wanting the other
axis at `frame_src` and `draw.texture_rect_region`. Godot has both, the
mechanism is the same mechanism, and the knob costs one `bool` and two lines
— so the seam is no longer the answer and `flip_v` is a parameter.

## `centered` defaults to **true**, which is a departure and is the right one

Everything else in this surface anchors top-left: `draw.texture_rect`'s
destination is a box, `app:text`'s `run` takes the top of the line box, and
`app:ui`'s `Painter` is in box space throughout. The previous shape followed
them, and `draw_frame` put the frame's top-left corner at `(x, y)`.

**Godot's `Sprite2D.centered` and `AnimatedSprite2D.centered` default to
true, and this package follows Godot.** Four reasons, in the order they
mattered:

  * **A character's position is not its top-left.** A platformer's actor is
    at its feet or its centre, and its collision box is centred on the same
    point; a top-left anchor makes every caller subtract a half-extent it had
    no other reason to compute, on every draw, for every actor. `app:ui`'s
    callers are laying out boxes and genuinely want the corner. This
    package's callers are not laying out boxes.
  * **The corner is not a stable point and the centre is.** A sheet cell's
    top-left moves when an artist re-trims or re-pads the sheet; the cell's
    centre does not move relative to what is drawn in it. Anchoring on the
    thing that moves is how a re-export shifts every sprite in a level by two
    pixels.
  * **This is the one property a GDScript reader is most likely to assume**,
    and the whole point of the reshape is that such assumptions hold. A
    silent half-frame offset is not a crash, not a diagnostic and not visible
    in a unit test — it is a sprite that looks slightly wrong, which is the
    most expensive kind of wrong to find.
  * **It is a default and not a convention.** `centered: false` is one named
    argument on `draw_frame`, and one assignment on an `AnimatedSprite`, and
    it gives back exactly the previous behaviour — anchor at the top-left
    corner, byte for byte the same destination rectangle. The departure costs
    a client that wants box space one word; the alternative cost every client
    that wants a character an arithmetic expression.

So the rule, stated once: **`(x, y)` is where the sprite's centre goes**, and
with `centered` false it is where its top-left corner goes.

## `offset` is in sheet pixels and `scale` multiplies it

Godot's `offset` is a per-sprite pixel offset in the node's own local space,
applied with the centering and before the node's transform — so the node's
scale scales it. This does the same: the destination's origin is

    x + offset.x * scale          (minus half the scaled width when centered)

and that is the property a caller wants. A 16×16 character drawn at scale 2
whose feet should sit on a tile boundary sets `offset = (0.0, -8.0)` once, in
the units the artist drew in, and the answer is right at every scale — where
an unscaled offset would be right at scale 1 and half a sprite out at scale 2.
It is tested at a scale that is not 1 for exactly that reason.

## `speed_scale` is per sprite and composes with the animation's own fps

`SpriteFrames` holds how fast an animation runs and `AnimatedSprite` holds a
multiplier on it, and the effective rate is the product. That is Godot's
split and it is the right one: *how fast a walk cycle is* is a property of
the walk cycle, and *how fast this actor is moving right now* is a property
of the actor. A game that slows an enemy to half speed, or runs a hit
reaction at 1.5×, does it without editing data ten other actors share.

A `speed_scale` of **zero freezes** the sprite — it is the same statement a
zero `fps` makes and it takes the same path — and a **negative** one is
floored to zero by `set_speed_scale`, because reverse playback would have to
decide what `finished` means walking backwards and is not in this package.
Note that the freeze is a *rate* of zero and not a pause: `is_playing` stays
true, because the animation has not stopped, it is running at no speed.

## The clock does not drift, and that is a decision about where `clock` lives

`advance` must be frame-rate independent — an animation must look the same on
a machine running at 144 Hz and one running at 30 — and the obvious way to
write that is wrong in a way that takes minutes of play to show up:

    s.clock = s.clock + dt            // seconds since the animation started
    s.frame = i32(s.clock * fps) % count

That is correct arithmetic over the reals and it loses resolution as a level
runs. `f32` has 24 bits of significand, so at 600 seconds the representable
steps are about 61 µs and at 6 000 seconds about 488 µs; a `dt` smaller than
the step **is added and disappears**, and the animation slows against the
wall clock by an amount that depends on how long the process has been up. A
sprite is the most visible possible place for that, because two of them
started at different times drift apart on screen.

**So `clock` is not the time since the animation started. It is the time
accumulated toward the next frame, and it never leaves `[0, 1/fps)`.** Each
`advance` adds `dt`, takes out as many whole frames as are in there, and
leaves the remainder behind for the next call. The remainder is the same size
every time however long the animation has been playing, so there is no
accumulating loss of resolution — the only rounding is in one `dt`-sized
addition, and it is carried forward rather than dropped.

The `fps` in that window is the **effective** one, the animation's speed
times the sprite's `speed_scale`. A client that changes either mid-play
leaves a remainder behind that was accumulated toward a frame at the old
rate, which is at most one frame's worth of phase and is the same thing that
happens in Godot for the same reason.

That is also what makes the equivalence testable, and it is tested twice:

  * **Exactly.** At 16 fps stepped by 1/128 s, every quantity in the loop is
    a dyadic rational that `f32` holds exactly, so 1 280 small steps and one
    10-second step agree on the frame *and on the residual clock*, bit for
    bit. That is an assertion about the algorithm with the floating point
    taken out of it.
  * **Realistically.** At 12.05 fps stepped by 10 ms — neither exact in
    binary — a thousand steps and one big one agree on the frame. The fps is
    chosen so that ten seconds is 120.5 frames rather than a whole number,
    because a test sitting exactly on a frame boundary measures `f32`'s
    rounding and not this function.

**The number of whole frames is taken with a multiply and not a loop**, so a
ten-second step costs what a ten-millisecond one costs; a subtract-in-a-loop
would be a hundred iterations for the first and is unbounded for a client
that stalls. The conversion is capped at `STEP_CAP`, for the reason given
there.

## What the edges answer, and why each is a value

Every answer here is a **value**, never a trap, which is the rule
`app:draw`'s accessors set: an overrun reads as a zero rather than aborting,
because a program that computed a bad index has something to do about it and
a test cannot watch an abort.

**An index outside the sheet.** `frame_src` answers `(0, 0, 0, 0)` and
`draw_frame` records **nothing at all**. Those are two different answers on
purpose. A zero-extent source rectangle is not "nothing" to `app:draw` — it
is a degenerate point that samples the sheet's top-left texel and stretches
it over the destination, which is precisely the mechanism `font.solid_src`
relies on — so handing a zero rect onward would paint a solid block of
whatever colour happens to be at the sheet's corner. `frame_src` answers a
zero because it is an accessor and that is what an accessor answers; the
recording call checks the index itself. **The index is not wrapped**: an
animation wraps, because walking off the end of a run is what a loop means,
and a sheet index does not, because an off-by-one there is a bug and silently
showing frame 0 hides it.

**A zero-speed animation is a still frame, not a refusal.** Speed is *how
fast to walk the run*, and zero is a speed. `advance` returns without
touching anything, so the animation shows its first frame for as long as it
is played and never finishes — nothing has ended, it simply is not moving,
which is what a caller writing `add_animation(&f, "idle", 0.0, true)` means.
The alternatives were worse in both directions: a refusal has nowhere to be
reported, since `advance` answers `void`; and clamping to some floor invents a
speed nobody asked for. A **negative** speed is floored to zero by
`add_animation` and `set_animation_speed`, so it means the same thing —
reverse playback would have to decide what `finished` means walking backwards
and is not in this package.

**An animation with no frames is finished the moment it is played**, looping
or not, and `draw_animated` records nothing for one. The tempting rule is
that a looping animation never finishes, and it is wrong here: `finished`
means *there is nothing more to show*, and a looping animation normally never
reaches that because there is always a next frame. An animation of no frames
has none, so "never finishes" would be a promise about frames that do not
exist, and a client waiting on `finished` to sequence a death animation would
wait forever on the one input that most plainly says stop.

**A negative or zero `scale` records nothing.** `scale` is a magnitude.
Mirroring already has two parameters, both are free, and both keep the sprite
where the caller put it; a negative scale would be a *third* way to mirror
that also moves the sprite to the other side of its anchor, so `scale = -1.0`
with `flip_h` true would cancel the mirror and keep the move, which is a
result nobody wants and nobody could have predicted from the signature.
Refusing it makes `draw_frame` total: for every input it either records one
command with a positive destination extent or records none.

**A dead `draw.Item`** drops the command, as everywhere in this surface. A
dead `draw.Texture` is subtler and is `app:draw`'s behaviour rather than
this package's: the command *is* recorded and the replay skips it, so the
item's `canvas_item_command_count` counts it and `draw.primitives()` does
not.

**A name that is not there.** Every `SpriteFrames` reader answers the value a
zeroed animation would: `get_frame_count` and `get_frame` answer `0` and
`-1`, `get_animation_speed` answers `0.0`, `get_animation_loop` answers
`false`, and `has_animation` is the question to ask when the difference
matters. `play` on an unknown name leaves the sprite exactly as it was rather
than stopping it, because the likely cause is a typo in one call site and
silently stopping an actor is harder to see than an actor whose new animation
did not take.

## What it does not do

**No loading.** A `Sheet` is made from a `draw.Texture` a client already has;
this package opens no file, decodes no PNG and knows no format. A
`SpriteFrames` is built by calls, not read from a `.tres`. **No packing** — a
sheet is a grid somebody else laid out, and the packer in this tree is
`app:font`'s, for glyphs.

**No per-frame durations.** Godot 4's `add_frame` takes a per-frame
`duration` multiplier and this does not, because it would put a second clock
inside the one that is carefully not drifting: the remainder would have to be
measured against *this* frame's length rather than a constant, and the exact
dyadic-rational equivalence that makes `advance` testable at all would go
with it. An animation that needs one frame held longer repeats the index.

**No per-frame origins, hitboxes, pivots or trim**, deliberately. Those are
the four things a sheet format carries and they are a *data* problem: the
moment this package holds a per-frame table of them it needs a way to fill
it, which is a file format, which is the loading it just refused. A client
that needs them holds them beside the `Sheet` in its own table indexed by the
same sheet index, which costs it one array and costs this package nothing.

**No scene graph, no ordering, no z, no transforms, no rotation.** That is
`app:graph`'s and `app:draw`'s, all of it, and a sprite that needs to rotate
is an item with a rotation on it rather than an argument here. **No state
machine** — which animation plays next is a game's decision, and `finished`
is the one fact this package owes it.

**No signals.** `animation_finished` is a `bool` a client reads, and it is a
**level rather than an edge**: it stays true until something plays an
animation. A client that needs the edge compares it against what it saw last
tick, which is one bool beside the sprite and is the same thing a signal
connection would have cost it.

**No atlas.** Two sheets in one texture would want a sub-rectangle offset on
every `Sheet`, which is a fifteen-minute change and is not made until
something wants it; a `Sheet` names a whole texture and a client with two
sheets makes two textures.

## Declarations

38 declarations, 34 public.

* `type Sheet: (draw.Texture tex, i32 frame_w, i32 frame_h, i32 cols, i32 rows)` — A texture cut into a uniform grid of frames, numbered from zero, **row-major…
* `Sheet grid(draw.Texture t, i32 frame_w, i32 frame_h)` — Cut `t` into `frame_w` by `frame_h` frames.
* `i32 frame_count(Sheet s)` — How many frames the grid holds. `0 ..< frame_count(s)` is exactly the set of…
* `[4]f32 frame_src(Sheet s, i32 index)` — The pixel source rectangle of frame `index`, as `draw.texture_rect_region`
* `const i32 MAX_ANIMATIONS = 16` — How many animations one `SpriteFrames` holds. An `add_animation` past it…
* `const i32 ANIMATION_NAME_MAX = 24` — The longest animation name, in bytes. A **name is a key**, so a longer one is…
* `const i32 MAX_FRAMES = 32` — How many frames one animation holds. Beyond it `add_frame` answers `false`
* `type Animation` — One animation of a `SpriteFrames`: its name, the sheet indices it walks, and…
* `type SpriteFrames: ([MAX_ANIMATIONS]Animation anims, i32 used)` — A table of named animations over one sheet — Godot's `SpriteFrames`.
* `const i32 FRAMES_BYTES = MAX_ANIMATIONS * (ANIMATION_NAME_MAX + 4 + MAX_FRAMES * 4 + 4 + 4 + 1) + 4` — What one `SpriteFrames` costs, in bytes of the client's storage, as a number…
* `bool has_animation(^SpriteFrames f, string8 name)` — Whether `name` is an animation of `f` — Godot's `has_animation`.
* `i32 animation_count(^SpriteFrames f)` — How many animations `f` holds. Godot answers this by handing back the whole…
* `bool add_animation(^SpriteFrames f, string8 name, f32 speed = 5.0, bool loops = true)` — Add an empty animation called `name` — Godot's `add_animation`, with that…
* `bool add_frame(^SpriteFrames f, string8 anim, i32 index)` — Append sheet frame `index` to `anim` — Godot's `add_frame`, with a sheet…
* `i32 add_frames(^SpriteFrames f, string8 anim, i32 first, i32 count)` — Append the contiguous run `first ..< first + count` — the shape a sheet is…
* `i32 get_frame_count(^SpriteFrames f, string8 anim)` — How many frames `anim` holds — Godot's `get_frame_count`. `0` for a name that…
* `i32 get_frame(^SpriteFrames f, string8 anim, i32 at)` — The **sheet index** frame `at` of `anim` names — Godot's…
* `f32 get_animation_speed(^SpriteFrames f, string8 anim)` — How fast `anim` runs, in frames a second — Godot's `get_animation_speed`.
* `void set_animation_speed(^SpriteFrames f, string8 anim, f32 fps)` — Set how fast `anim` runs — Godot's `set_animation_speed`. A negative rate is…
* `bool get_animation_loop(^SpriteFrames f, string8 anim)` — Whether `anim` wraps at its end — Godot's `get_animation_loop`. `false` for a…
* `void set_animation_loop(^SpriteFrames f, string8 anim, bool loops)` — Set whether `anim` wraps — Godot's `set_animation_loop`. An unknown name does…
* `type AnimatedSprite` — Where a sprite is in an animation, and the handful of per-instance properties…
* `AnimatedSprite animated_sprite()` — A sprite with Godot's defaults: no animation, stopped, `speed_scale` 1,…
* `void play(^AnimatedSprite s, ^SpriteFrames f, string8 name)` — Play `name` — Godot's `play`, including the part people are surprised by:
* `void play_from_start(^AnimatedSprite s, ^SpriteFrames f, string8 name)` — Play `name` from its first frame, **even if it is already the animation…
* `void stop(^AnimatedSprite s)` — Stop, and rewind to the first frame — Godot's `stop`, which resets the…
* `void pause(^AnimatedSprite s)` — Stop, keeping the frame and the clock — Godot's `pause`. `play` resumes from…
* `bool is_playing(^AnimatedSprite s)` — Whether an animation is running — Godot's `is_playing`.
* `string8 animation_name(^AnimatedSprite s, ^SpriteFrames f)` — The name of the animation `s` is on, as a `string` **windowing `f`** — Godot's…
* `void set_speed_scale(^AnimatedSprite s, f32 scale)` — Set the playback multiplier — Godot's `speed_scale`, floored at zero.
* `i32 sheet_frame(^SpriteFrames f, ^AnimatedSprite s)` — The **sheet index** `s` is currently showing, or `-1` when there is nothing…
* `void advance(^AnimatedSprite s, ^SpriteFrames f, f32 dt)` — Walk the clock forward by `dt` seconds and move `frame` by however many whole…
* `void draw_frame(draw.Item into, Sheet s, i32 index, f32 x, f32 y, f32 scale, [4]u8 tint, bool flip_h = false, bool flip_v = false, bool centered = true, [2]f32 offset = (0.0, 0.0))` — Record sheet frame `index` of `s` at `(x, y)`, `scale` times its size in the…
* `void draw_animated(draw.Item into, Sheet sh, ^SpriteFrames f, ^AnimatedSprite s, f32 x, f32 y, f32 scale, [4]u8 tint)` — Record whatever frame `s` is on, with its own `flip_h`, `flip_v`, `centered`
