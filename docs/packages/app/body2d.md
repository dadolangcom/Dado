<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:body2d

`app:body2d` — Godot's `CharacterBody2D`: an axis-aligned box swept against
solid geometry the caller describes, moved as far as it can go on each axis
and reporting what it touched.

    import "app:body2d"

    body2d.Solids map  = body2d.solids(is_solid, nil, 16.0, 16.0)
    body2d.Body   hero = body2d.body((64.0, 0.0), (12.0, 16.0))

    void step(f32 dt):
        hero.velocity.y = hero.velocity.y + GRAVITY * dt
        hero.velocity.x = axis() * RUN
        if jump_pressed && body2d.is_on_floor(&hero):
            hero.velocity.y = -JUMP
        body2d.move_and_slide(&hero, map, dt)

That is the whole contract, and it is Godot's: the caller owns `velocity`,
`move_and_slide` resolves it against the world, and `is_on_floor` afterwards
says what the move found. A platformer character controller is about fifteen
lines on top of it, which is the point — the *sliding* is the part that is
hard to get right, and it is written once here rather than badly in every
game.

## Why this package exists at all

Everything above the seam is easy and everything below it is not. Reading a
jump button, choosing a run speed, deciding what a double jump costs: those
are a game, they are ten lines each, and no two games want the same ones.
*Moving a box through a tilemap without it snagging, jittering, sinking or
falling through the floor* is none of those things. It is a dozen boundary
conditions with one right answer each, every one of which looks like a
physics bug when it is wrong and is arithmetic when it is right:

  * a body walking along a floor made of two adjacent tiles catches on the
    seam between them;
  * a body resting on the floor accumulates a downward velocity and after
    four minutes is inside it;
  * a body that falls far enough passes through a one-tile floor and keeps
    going, and nobody notices until somebody builds a tall level;
  * a body pushed exactly onto a tile boundary oscillates between two cells
    forever, one pixel a frame.

Each of those is a line of arithmetic, and each of them is why this is a
package rather than a snippet. What is in here is the resolution and the
contact report. What is not in here is the game.

## What it collides against: one seam, and it is a callback

    type SolidFn: bool(rawptr, i32, i32)     // (user, tile_x, tile_y)

— *is the tile at this grid cell solid?* — plus a tile width and height. That
is the entire description of the world this package takes.

**It costs nothing.** No allocation, no ownership, no broadphase, no shape
system, no lifetime to get wrong, and no `init`/`shutdown` pair. A `Solids`
is four words the caller builds at the call site and throws away, and a
`Body` is forty bytes the caller keeps wherever its entities already live.
This package allocates not one byte and holds not one global.

**It covers the thing this is for.** A 2D platformer is a tilemap. Asking a
grid a question per cell is what a tilemap is *already* good at, and the
query count is small and bounded: one `solid` call per cell of the swept
box's cross-section, per cell it advances. A 12×16 body in a 16-pixel grid
crossing one tile asks at most two.

**A second seam was considered and refused, and the reason is that the
callback already contains it.** The obvious second one is an explicit list of
solid rectangles, and it earns nothing here, because a caller holding a list
of rectangles writes:

    bool solid(rawptr user, i32 tx, i32 ty):
        ^Level lv = ^Level(user)
        for r in lv.blocks:
            if overlaps_cell(r, tx, ty, lv.tile_w, lv.tile_h):
                return true
        return false

— and has a rectangle world, at whatever cost their own data structure
charges, with **no second resolution path in this package**. The sweeping,
the ordering, the substepping and the contact classification are the hard
part and there is one copy of them; a rectangle seam would be a second
broadphase, a second set of boundary cases, and a second place for the flush
seam to come back. The grid is the coordinate system the resolution is
written in, and letting the caller decide what a cell *means* is strictly
more general than letting them hand over a list.

The one thing the caller gives up is sub-cell geometry: a solid that is half
a tile tall has to be its own cell size, or be rounded. That is stated rather
than papered over, and it is the price of the seam being one function.

## The body is a value, and deliberately not a handle

`app:` is full of servers — `app:draw` hands out `Texture`, `Item` and
`Canvas`, and `app:graph` hands out `Node`, all of them `(i32 index, u32
generation)` over a pool with a free list. **This package hands out nothing**,
and that is a decision rather than an omission.

A handle exists to name something the client cannot hold: a GPU texture, a
node whose storage moves when a `ref` grows under it, a canvas item whose
list the server owns. A `Body` is none of those. It is plain data with no
allocation, no lifetime, no identity and nothing behind it — so a pool would
add a free list, a generation counter, an `init`/`shutdown` pair and a
brand-new failure mode (the dead handle) in exchange for nothing at all, and
it would take away the thing that makes this shape good: a `Body` sits
*inside* the caller's own entity struct, beside its sprite and its health,
and is copied, serialised and rewound with it.

So every function here takes `^Body` and the fields are public. Godot's
`velocity` is a property and it is a field here; `is_on_floor()` is a method
there and a function taking `^Body` here. What the surface does not have is a
server to bring up before any of it works.

## The axes are resolved separately, and the up axis goes first

A diagonal move is resolved as two axis-aligned moves, because an
axis-aligned move against an axis-aligned grid has an exact answer and a
diagonal one does not. Each axis is a **sweep**: the box's whole path along
that axis is a rectangle, the cells that rectangle covers are exactly the
cells the body passes through, and the first blocking one clamps the move.
There is no "move and then push out of whatever you landed in" anywhere in
this file, and most of the bugs in the list at the top of this comment are
symptoms of that pattern rather than of anything about ordering.

**The flush seam is worth being exact about, because it is the bug this
package is most likely to be blamed for.** A body walking along a floor of
two adjacent tiles catches on the join between them when the horizontal sweep
is run while the body is *inside* the floor by a fraction of a pixel: the
floor row then reads as a wall and the body stops against a seam that is not
there. The fix is not an ordering. The fix is that the body is **never inside
anything**: a blocked sweep assigns the resting coordinate out of the grid —
`f32(row) * tile_h - height`, one multiply and one subtract from integers —
rather than subtracting a penetration depth from a position that has already
drifted. With that invariant, the horizontal sweep's row range simply does
not contain the floor row, and it does not contain it at any speed or after
any number of frames. `body2d_test.dado` walks a body across the join of two
tiles and checks it arrives, because a future edit that reintroduces
penetrate-then-resolve will pass every other test in that file.

**What the order actually decides is the corner clip**, and it decides it
alone. When a diagonal step's destination overlaps a cell that neither
single-axis move would have touched — clipping the corner of a tile — one
axis survives and the other is refused, and which one is the order. So:

> **The axis parallel to `up_direction` is resolved first.**

Two reasons, and the second is the one that matters.

  * The up axis is the one carrying the acceleration the caller applies every
    single frame and the one whose contact zeroes it. Resolving it first
    means the horizontal sweep of a step runs against a vertical coordinate
    that was assigned out of the grid *this step*, not one left over from the
    last.
  * On a corner clip, this order keeps the **vertical** component and refuses
    the horizontal. A body falling past the corner of a ledge keeps falling
    and loses a frame of drift; under the other order it keeps drifting and
    loses a frame of fall, which reads on screen as the character catching on
    the corner and hanging there. Players have a word for that and it is not
    a kind one.

`up_direction` must be one of `UP`, `DOWN`, `LEFT` or `RIGHT`, and that is
asserted rather than documented-and-hoped. An AABB against an axis-aligned
grid has exactly four possible contact normals; an `up_direction` that is not
one of them would classify every contact in the world as a wall, which is not
a degraded answer but a useless one.

This is also why there is no `floor_max_angle`. Godot needs one because a
collision shape can rest on a slope at any angle and somebody has to say
where "floor" stops and "wall" begins. Here the angle between a contact
normal and `up_direction` is 0°, 90° or 180° and nothing else, so the
classification is the sign of a dot product and a maximum angle would be a
knob with two settings that both mean the same thing.

## Speed: there is no speed limit, and the substepping is not what buys that

A body moving faster than its own width in one step passes through a wall —
in an implementation that tests the *destination*. This one tests the
**path**: a single-axis move's swept box is a rectangle, the cell range it
covers is computed from its two ends, and every cell in it is asked. A body
falling ten thousand pixels in one step lands on the first solid row under
it, and a body falling ten million pixels lands on the same row. **There is
no vertical speed at which the floor is missed, and there is no cap on the
speed this package supports.**

So what is the substepping for? The *diagonal*. Two axis-aligned moves are a
right angle, and a right angle is a good approximation of a diagonal only
while it is short: over a long step the L-shaped path visits cells the true
path never does and misses cells it does. A body crossing a tilemap in one
step at 45° can therefore pass *diagonally* through a single tile that a
shorter step would have hit — which is a tunnelling bug, just not the one
people look for.

The motion is therefore cut into substeps of at most `min(tile, extent)` on
each axis:

  * **one tile** bounds the divergence between the L-path and the true
    diagonal to one cell, because between two substeps the body's cell
    footprint advances by at most one;
  * **the body's own extent**, where it is smaller, bounds it further — a 4×4
    projectile in a 16-pixel grid would otherwise cross most of a tile in a
    substep and clip corners a slower one would have struck.

The count is capped at `MAX_SUBSTEPS` (64) so that an absurd velocity, a
tiny body or a division by a small tile size cannot turn one frame into an
unbounded loop. **Above the cap the substeps get longer, and the only thing
that degrades is diagonal fidelity** — a very fast diagonal may clip a corner
it should have hit. The per-axis sweep stays exact, so the floor is still
never missed. `substeps(&b)` reports what the last move took, so a caller who
cares can see the cap being reached rather than guess.

The cost is stated: a sweep is O(cells crossed), and substepping walks the
same cells in chunks, so a long move costs what the distance costs and not
what the substep count costs. Sixty-three substeps of a thousand-pixel fall
is sixty-three sweeps over one or two cells each, not sixty-three sweeps of
the whole fall.

## `is_on_floor` is a report of the last move, not a question about now

This is Godot's semantics and it is kept deliberately, so it is worth a
paragraph rather than a footnote: **`is_on_floor`, `is_on_wall` and
`is_on_ceiling` answer what the last `move_and_slide` found. They do not look
at the geometry.** Move a body by assigning `position` and `is_on_floor` does
not change. Change the tilemap under a standing body and `is_on_floor` does
not change. Call `move_and_slide` and all three are recomputed from scratch.

The storage is spelled `last_on_floor`, `last_floor_normal` and so on for
exactly that reason: the field says what it is at every place it is read, and
a line that *writes* one reads as the mistake it is.

A caller that expects a live query writes a subtly wrong jump check — one
that works, because a platformer calls `move_and_slide` every frame anyway,
right up until the frame it does not. If a live question is genuinely wanted,
`overlaps` is one: it asks the geometry, now, whether the body is inside
anything. It is not the floor test and it is not spelled like one.

`move_and_collide` does **not** touch the contact state either, for the same
reason Godot's does not: it is a one-shot probe a caller uses to ask a
question, and having it overwrite the answer `move_and_slide` left would make
the two uninterleavable.

## Contact stability, which is the whole of the floating-point story

Three failures, one rule each, and the third was found by a test rather than
argued for in advance.

**A body at rest must not sink.** The caller adds gravity every frame, so
there is downward velocity every frame, so there is a downward sweep every
frame. Each one finds the floor at distance zero and does two things: it
**assigns** `position` the grid coordinate `f32(row) * tile_h - height`, and
it **zeroes** the velocity component on that axis — Godot's own rule. The
assignment is the important half. A resting body's coordinate is recomputed
from two integers every frame and is therefore bit-identical every frame;
nothing is subtracted from it, so nothing accumulates into it. A thousand
steps at rest leave a body on the same bit pattern it started on, and the
test asserts exactly that with `==` rather than a tolerance, because a
tolerance would hide the drift this is here to prevent.

**A body exactly on a boundary must not oscillate.** Cell ranges are computed
with `floor` on the near edge and `ceil(x) - 1` on the far one, which is a
half-open rule with no epsilon in it: a box whose edge lies exactly on a tile
boundary does not overlap the cell across that boundary, and a box pushed up
against a wall is therefore not inside it. Pushing again finds the same
blocking cell, computes the same coordinate, and moves the body by exactly
zero. There is no separation epsilon anywhere in this file, so there is
nothing to oscillate around.

**A body that hits nothing must arrive exactly where it was sent**, and the
substepping is what threatens that. Each sweep answers a *coordinate* and the
body is assigned it, never advanced by a distance, because `lo + (stop - lo)`
is not `stop` in binary; and each substep aims at `start + motion * (i+1)/n`
rather than adding `motion / n` to what is there, because the last such
fraction is exactly 1 and the sum of `n` roundings is not. Without both, an
unobstructed hundred-unit move over seven substeps lands a few units in the
last place short, every frame, for ever — a drift nothing downstream
corrects. `body2d_test.dado` checks a free move through an empty world with
`==` for exactly that reason, and that assertion is what found this: the
first draft of this package added `motion / n` up and was wrong by it.

The one place a body is moved *towards* geometry rather than away from it is
`floor_snap_length`, and it is bounded in the same way: the probe is a normal
sweep, so it finds the floor at distance zero when the body is already
resting and moves it by zero.

## Coyote time and jump buffering are not this package's job

They are both good, both nearly universal, both about four lines, and both
**game policy**. Coyote time is a rule about how long after leaving a ledge a
jump is still allowed; jump buffering is a rule about how long before landing
a press still counts. Their durations are tuning, their interaction with
double jumps and wall jumps is design, and a game that wants neither — a
physics puzzle, a top-down game, a moving platform — would have to work
around them.

A body that invented them would be unusable for anything but the one game it
was tuned for. `is_on_floor()` is the input they are both written in terms
of, this package answers it honestly, and the four lines belong in the
caller.

## What it does not do

The scope is "one axis-aligned box against static geometry", and naming what
that excludes is what stops the next reader expecting a physics engine.

**No gravity.** It is `velocity.y += g * dt` in the caller and it is one
line. A body that applied its own would need a direction, a magnitude, a
switch to turn it off for the swimming level, and an opinion about terminal
velocity — four knobs to replace one line.

**No rotation, ever.** It is an axis-aligned box. The name says so, the
`size` field is two numbers, and there is no angle anywhere in the file. A
rotating collider is a different shape system with a different sweep and a
different contact classification, and it would take slopes and normals and
an inertia tensor with it.

**No slopes.** Every surface is an axis of the grid. A slope wants a floor
angle, a slide-along-the-surface step, a stop-on-slope rule, a
constant-speed-on-slope rule and a max-angle — Godot has all five and they
are most of what `move_and_slide` is. A tile platformer built on square tiles
does not need them, and adding them would change every decision above.

**No one-way platforms.** They are a genuine tilemap feature and they are
deliberately out: a one-way tile is solid only to a body moving towards it
from one side, which makes solidity a function of the body's velocity and not
of the cell. That is a different `SolidFn` signature, and changing the
signature for a feature nobody has asked for yet is how a seam stops being
one.

**No other bodies.** Static geometry only. Two `Body` values do not collide,
because doing it properly needs an order of resolution, a rule for who pushes
whom, and a broadphase, and doing it improperly needs none of those and is
worse than not doing it.

**No mass, no restitution, no friction, no impulses, no joints, no areas or
triggers, no raycasts, no queries beyond `overlaps`, no layers or masks, and
no `max_slides`.** Several of those are one line in the caller
(`velocity.x *= 0.8` is friction); the rest are a physics engine, and this is
not one.

## Declarations

44 declarations, 22 public.

* `type SolidFn: bool(rawptr, i32, i32)` — *Is the tile at this grid cell solid?* Called with the `user` pointer the…
* `type Solids` — The description of solid geometry: a predicate, its context pointer, and the…
* `Solids solids(SolidFn solid, rawptr user, f32 tile_w, f32 tile_h)`
* `const [2]f32 UP = (0.0, -1.0)`
* `const [2]f32 DOWN = (0.0, 1.0)`
* `const [2]f32 LEFT = (-1.0, 0.0)`
* `const [2]f32 RIGHT = (1.0, 0.0)`
* `const i32 MAX_SUBSTEPS = 64` — The most substeps one `move_and_slide` or `move_and_collide` will take. See…
* `type Body` — An axis-aligned box with a velocity and a memory of its last move.
* `Body body([2]f32 position, [2]f32 size)` — A body at `position` of `size`, standing still, with `UP` for up and a one…
* `type Collision` — What one `move_and_collide` found. `travel` is what the body actually moved,…
* `void move_and_slide(^Body b, Solids w, f32 delta)` — Move by `velocity * delta`, resolving each axis against `w` and sliding along…
* `Collision move_and_collide(^Body b, Solids w, [2]f32 motion)` — Move by `motion` once, stopping at the first thing hit, and report it.
* `bool is_on_floor(^Body b)` — Whether the last `move_and_slide` ended against a surface whose normal points…
* `bool is_on_wall(^Body b)` — Whether the last `move_and_slide` ended against a surface whose normal is…
* `bool is_on_ceiling(^Body b)` — Whether the last `move_and_slide` ended against a surface whose normal points…
* `[2]f32 get_floor_normal(^Body b)` — The floor's normal as of the last `move_and_slide`, or `(0, 0)` where it…
* `[2]f32 get_wall_normal(^Body b)`
* `[2]f32 get_ceiling_normal(^Body b)`
* `i32 substeps(^Body b)` — How many substeps the last `move_and_slide` or `move_and_collide` took.
* `[4]f32 aabb(^Body b)` — The body's box as `(x, y, w, h)` — the shape `app:draw`'s `rect` takes, so a…
* `bool overlaps(^Body b, Solids w)` — **This one is a live question**, unlike the three above it: is the body, right…
