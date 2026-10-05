# `app:` for dummies

**A friendly, ground-up guide to building a small 2D game with the `app:` runtime.**

This is the gentle one. It explains things slowly, with pictures in words, and it
assumes you have never written a game before. If you want the arguments — *why*
the drawing surface is shaped this way, what was measured, what was rejected —
those live in the package headers themselves, which are long and opinionated on
purpose. Start here, go there when you want to know why.

> ### One honest warning, read it once
>
> **This document is hand-written and nothing checks it.** Everywhere else in
> this tree, documentation is either generated from the compiler or gated by a
> step that compiles every sample. This file is neither — it exists because Sam
> asked for a beginner's guide and granted it an exception.
>
> So: when this file and the code disagree, **the code is right**. The package
> headers under `collections/app/*/` are maintained beside the thing they
> describe; this one is maintained by somebody remembering to.

---

## Table of contents

1. [What `app:` is — and is not](#1-what-app-is)
2. [Your first window](#2-your-first-window)
3. [The big idea: you are not painting, you are arranging](#3-the-big-idea)
4. [Where your memory goes](#4-memory)
5. [Drawing shapes](#5-drawing-shapes)
6. [Moving things without redrawing them](#6-moving-things)
7. [The tree, and the one-line camera](#7-the-tree)
8. [Pictures: textures and sprites](#8-pictures)
9. [Words: fonts and text](#9-words)
10. [Input: knowing what the player pressed](#10-input)
11. [Physics: walking on the ground](#11-physics)
12. [When things go wrong](#12-failure)
13. [A tiny complete game](#13-a-tiny-complete-game)
14. [How to tell if you are doing it right](#14-counters)
15. [The gotcha list](#15-gotchas)
16. [Where to look next](#16-where-to-look-next)

---

## 1. What `app:` is

**`app:` is a runtime.** That word is doing real work, so here is what it means.

A *library* is something you call. A *runtime* is something that calls **you**.
You hand it four functions and it runs the whole show: it opens the window,
brings the graphics up, owns a canvas, renders it every frame, reads the
keyboard, hands you a clean input state, gives you an allocator that empties
itself, and shuts it all down in the right order at the end.

**It is opinionated, and that is the point.** It makes a pile of decisions for
you that you would otherwise have to make correctly, in the right order, every
time. You give up fine control and you get to start writing your game on line
one.

### What `app:` takes over

- Opening the window and running the frame loop
- Bringing `app:draw` up, and owning a **root canvas** you draw into
- **Rendering that canvas** every frame
- Clearing the keyboard's per-frame edges so `just_pressed` means this frame
- Installing an **allocator** so you never write one
- Emptying a per-frame scratch arena
- Tearing all of it down in reverse

### What you give up

If you need two render targets, or a frame graph, or a different allocator per
subsystem, or your own loop — **`app:` is the wrong package**, and it says so in
its own header. You would import `app:display`, `app:draw` and `app:input`
directly and drive them yourself. That road is open and `app:graphical`'s own
`run_loop` is the worked example of walking it.

For everything else — a prototype, a game jam, a demo, a tool — this is the
package.

### The pieces

| package | what it does | you need it when |
|---|---|---|
| `app:graphical` | **the runtime, with a window** | always, for a game |
| `app:headless` | the same runtime, no window | tests, servers, tools |
| `app:draw` | shapes, pictures, the screen | always |
| `app:font` + `app:text` | letters | if you show text |
| `app:input` | keyboard and mouse | if the player does anything |
| `app:sprite` | animated pictures | if something has frames |
| `app:body2d` | gravity-and-walls collision | if something walks |
| `app:ui` | buttons, checkboxes, lists | menus and tools, not gameplay |

Most of this is borrowed from **Godot**, deliberately. If you know GDScript you
already know the names: `canvas_item_create`, `draw_rect`, `AnimatedSprite`,
`move_and_slide`, `is_action_pressed`. If you do not, that is fine — nothing
here assumes it.

---

## 2. Your first window

Here is a whole program. It opens a window and paints it dark blue.

```dado
package main

import app "app:graphical"
import "core:fmt"

!void ready():
    app.set_background(0.05, 0.06, 0.10, 1.0)
    return

!void process(f32 dt):
    _ = dt
    return

!i32 main():
    try app.init(800, 600, "my first window")
    app.hook(ready: ready, process: process)
    try app.run() else code:
        fmt.println("could not start: ", code)
        return 1
    return 0
```

That is the whole thing. There is no graphics setup, no canvas, no render call,
no allocator and no shutdown — the runtime does all of it.

### Reading it line by line

**`import app "app:graphical"`** — you can *name* an import. The runtime with a
window is `app:graphical`, and calling it `app` is what makes the rest read
nicely. Swap that one line for `import app "app:headless"` and the same program
runs with no window at all.

**`try app.init(800, 600, "…")`** — set the window up. The `try` is because it
can fail (a zero width, say), and `try` means *"if this fails, stop and pass the
failure up."*

**`app.hook(ready: ready, process: process)`** — hand over your functions. Name
them, always; see the warning below. Any you leave out simply do not exist.

**`try app.run() else code:`** — go. This blocks until the window closes. The
`else` arm catches a failure, and §12 explains why you want it.

**The `!` marks.** A `!` in front of the return type means "this might fail."
`!void ready()` is a `ready` that can fail. `!i32 main()` says the same about
`main` — and there the failure code becomes the program's **exit status**, which
is free and genuinely useful.

The `!` sits in front of the *whole* return type, never inside it: `!(i32, i32)
f()`. Read it as "this can blow up." A `?` would sound like *maybe there is a
value*, which is a different idea entirely.

### ⚠ Always name your hook arguments

```dado
app.hook(ready: ready, process: process)      // RIGHT
app.hook(ready, process)                      // compiles. do not do this.
```

`ready`, `draw` and `shutdown` are all `!void()` — *the same type*. The compiler
cannot tell them apart, so a positional call where you swap two of them is
accepted silently and your game does something baffling. Naming them costs six
characters and removes the entire category.

### The four hooks

| hook | when | what it is for |
|---|---|---|
| `ready()` | once, after the window exists | load things, build your scene |
| `process(dt)` | every frame | think: read input, move things |
| `draw()` | every frame | **optional.** record extra drawing |
| `shutdown()` | once, at the end | save, release |

**You usually do not need `draw`.** The runtime renders its canvas for you. A
draw hook is for a program that wants to record something fresh each frame, and
the runtime renders *after* it, so anything you record there lands on the same
frame.

### About `dt`

`dt` is **seconds** — how long the previous frame took, about `0.0166` at 60fps.
Multiply by it so your game runs the same speed everywhere:

```dado
x = x + speed * dt          // `speed` pixels per SECOND, whatever the frame rate
```

**`dt` is not clamped, and you should clamp it.** Drag the window, or let the
machine hiccup, and one frame takes half a second — your character moves 300
pixels in one step and straight through a wall:

```dado
const f32 MAX_STEP = 0.05          // never simulate more than 1/20 s at once

!void process(f32 elapsed):
    f32 dt = elapsed
    if dt > MAX_STEP:
        dt = MAX_STEP
```

---

## 3. The big idea

This is the one concept that makes everything else click, and it is different
from how most drawing libraries work. Read this twice.

### The way you probably expect it to work

Most 2D libraries are like painting a wall: every frame you wipe it clean and
repaint the whole picture. That is **immediate mode**, and it means your program
says "draw the ground" sixty times a second, forever, even though the ground
never changes.

### The way `app:draw` actually works

`app:draw` is a **felt board**. You know the ones — a fuzzy board, and you stick
cut-out shapes on it, and they stay stuck.

You cut out the ground **once** and stick it on. Next frame you do not cut out a
new ground. It is already there. You touch the board only when something
actually changes. That is **retained mode**:

- **`app.canvas()`** is the felt board. The runtime owns it and renders it.
- An **`Item`** is one cut-out shape you stick on. It has a position, and it can
  be hidden, tinted or moved.
- Each `Item` holds a **command list** — "a blue rectangle here, a red circle
  there."

So the flow is:

```
in ready:        make an Item and record shapes into it   ← the cutting-out
every frame:     (the runtime renders)                    ← the looking
```

### The mistake everybody makes once

```dado
// WRONG — a new ground stuck on the board sixty times a second
!void draw_frame():
    draw.rect(g_ground, (0.0, 400.0, 800.0, 200.0), BROWN)
    return
```

Recording **appends**. After ten seconds you have six hundred identical brown
rectangles stacked on each other, your memory is full and your frame rate is on
the floor.

The ground goes in once:

```dado
// RIGHT
!void ready():
    g_ground = draw.canvas_item_create(app.canvas())
    draw.rect(g_ground, (0.0, 400.0, 800.0, 200.0), BROWN)   // once, forever
    return
```

### When something *does* change

Clear it and record again:

```dado
draw.canvas_item_clear(g_score)              // empty the list
text.run(g_score, &g_font, g_atlas, 10.0, 10.0, "SCORE 5", WHITE)
```

**`canvas_item_clear` empties the drawing commands and keeps everything else** —
position, visibility, tint, children, its place in the tree — and keeps the
memory, so an item you re-record every frame still allocates only once in its
life.

### Why bother?

Because most of your screen does not change most of the time. In this tree's
platformer demo a level of **381 tiles is recorded 5 times for a whole
14-second playthrough** — once per chunk — while the player runs its entire
length. Over a thousand frames out of fifteen hundred record *nothing at all*.

---

## 4. Memory

This section is short and it will save you a real debugging session.

### You do not pass allocators around

In some languages you thread an allocator through every function. **Not here.**
Dado's memory model comes down to one rule:

> **Don't thread allocators as parameters. Change the ambient allocator with
> `using`.**

So `draw.init()`, `font.load_file(path, size)`, `graph.init()` take no allocator.
They allocate to whatever the **ambient** allocator is, and the runtime sets
that for you. Most of the time you write nothing at all.

### The one thing you must know

**The ambient allocator is different in different hooks**, on purpose:

| hook | allocator | lives |
|---|---|---|
| `ready`, `shutdown` | the **persistent** one | for the whole program |
| **`process`, `draw`** | the **temp arena** | **until the end of this frame** |

The rule behind it, in one sentence: *a hook that runs once allocates for the
life of the program; a hook that runs every frame allocates for the life of the
frame.*

That makes the common case free — a game loop allocates scratch constantly and
now none of it needs cleaning up. And it means:

### ⚠ Anything you make in `process` and keep is gone next frame

```dado
// WRONG
private ref []Enemy g_spawned

!void process(f32 dt):
    g_spawned = #make([]Enemy, 10)     // lands in the temp arena
    return                              // ...which is emptied at the end of this frame
```

Next frame `g_spawned` points at memory the arena has handed to something else.
**It will not crash cleanly.** The arena keeps its blocks, so this is not a
use-after-free a sanitizer can see — it reads as your data quietly turning into
somebody else's.

Take the ambient back when you mean to keep something:

```dado
// RIGHT — one line, and it says what it is doing
!void process(f32 dt):
    using app.allocator():
        g_spawned = #make([]Enemy, 10)
    return
```

Transient things — a string you format and print, a temporary array you walk and
throw away — need no change at all. That is what the arena is *for*.

### One reassurance

**Growing something keeps its original allocator.** `#resize`, `#append` and
`#reserve` use the allocator the reference already carries, so a list you made in
`ready` keeps growing in the persistent allocator even when you append to it from
`process`. It is only the *create* verbs — `#make`, `#new`, `#copy`, `#format` —
that land wherever the ambient currently is.

That is why a text editor works: the buffer is minted once in `ready`, and every
keystroke after that is an `#append`.

### Checkpoints

The temp arena's high-water mark is the biggest single frame your program ever
had, and it keeps those blocks for the rest of the run. If one frame builds
something enormous, take it back inside the frame:

```dado
using app.enter_temp():
    ... build the big transient thing ...
_ = app.close_temp()                    // rolls the arena back
```

---

## 5. Drawing shapes

Every drawing call takes **the item you are recording into** first.

```dado
draw.rect(item, (x, y, w, h), colour)
draw.rect(item, (x, y, w, h), colour, false, 2.0)       // outline, 2px
draw.line(item, (x0, y0), (x1, y1), colour)
draw.line(item, (x0, y0), (x1, y1), colour, 3.0)        // 3px thick
draw.circle(item, (cx, cy), radius, colour)
draw.polyline(item, points, colour, 2.0)                // a connected path
draw.colored_polygon(item, points, colour)              // a filled shape
```

### Colours

Four bytes — **red, green, blue, alpha**, each `0`–`255`.

```dado
const [4]u8 WHITE = [255, 255, 255, 255]
const [4]u8 GHOST = [255, 255, 255, 128]     // half see-through
```

### Coordinates

**Pixels. `(0, 0)` is the top-left. `y` grows downward.**

```
(0,0) ─────────────► x
  │
  │      (100, 50) is 100 right, 50 DOWN
  ▼ y
```

So "up" is negative `y`, and a jump is `velocity.y = -760.0`. This catches
everybody who did maths at school; it is the same convention as Godot, every web
browser and every UI toolkit ever written.

A rectangle is `(x, y, width, height)` — **not** two corners.

### That `-1.0` you keep seeing

Some calls take a `width` defaulting to `-1.0`. Negative means "the default
thing": **filled** for a fillable shape, **one pixel** for a line. Godot's
convention, kept so code copied from GDScript does what it looks like.

### One language wrinkle

You cannot mix named and positional arguments in a call, so this is refused:

```dado
draw.rect(item, r, col, width: 1.0)          // error ERR0604
```

Write every argument up to the one you want: `draw.rect(item, r, col, false, 1.0)`.
(The exception is `app.hook`, which you name *all* of — see §2.)

---

## 6. Moving things

Here is the payoff of the felt board. **You do not re-record something to move
it.** You slide the cut-out.

```dado
draw.canvas_item_set_position(g_player, (x, y))
```

One call, no recording, no matter how complicated the thing is. The other knobs:

```dado
draw.canvas_item_set_rotation(item, radians)
draw.canvas_item_set_scale(item, (sx, sy))
draw.canvas_item_set_visible(item, false)                 // hide it and its children
draw.canvas_item_set_modulate(item, [255, 0, 0, 255])     // tint it and its children
draw.canvas_item_set_z_index(item, 5)                     // draw order
```

**Hiding is cheaper than deleting.** When a coin is collected, do not free its
item — hide it. A hidden item is skipped entirely, children and all.

**Draw order** is creation order, later on top. `set_z_index` overrides it, and
items with equal z keep their creation order.

---

## 7. The tree

Items can have parents, and a child moves with its parent:

```dado
_ = draw.canvas_item_set_parent(g_player, g_world)
```

### The one-line camera

This is the nicest trick in the system and it is worth building your game around.

Make one item called `g_world`. Make **everything in your level** a child of it —
tiles, enemies, coins, the player. Record it all once. Then scrolling is:

```dado
draw.canvas_item_set_position(g_world, (0.0 - camera_x, 0.0 - camera_y))
```

**One call.** Nothing is re-recorded; the whole level slides. A background moved
by a *smaller* amount gives you parallax for free:

```dado
draw.canvas_item_set_position(g_hills, (0.0 - camera_x * 0.35, 0.0))
```

If you are re-recording anything when the camera moves, stop and rethink.

### A useful shape for a game

```
app.canvas()
├── sky          (never moves)
├── hills        (camera × 0.35 — parallax)
├── world        (camera × 1.0 — THE CAMERA)
│   ├── chunk0 … chunkN   (tiles, recorded once)
│   ├── coins, enemies
│   └── player
└── hud          (never moves — it is on the glass)
```

### Two surprises

**`canvas_item_free` frees the whole subtree.** Freeing `g_world` takes every
tile and enemy with it — usually what you want at the end of a level.

**`set_parent` does not preserve where a thing appears.** It keeps the item's
*local* position, so it jumps to wherever that lands under the new parent.

---

## 8. Pictures

### Textures

```dado
draw.Texture t = draw.texture_create(width, height, pixels)
```

`pixels` is `[]u8` — **RGBA8**, four bytes per pixel, row by row from the
top-left. A 16×16 picture is 1024 bytes.

There is no image loader. Generate the pixels in code (the platformer demo
writes every sprite as sixteen strings of characters and bakes them at startup —
no asset files, and the art is obviously yours), or decode a file yourself.

### Drawing one

```dado
draw.texture_rect(item, t, (x, y, w, h), WHITE)
draw.texture_rect_region(item, t, (x, y, w, h), (sx, sy, sw, sh), WHITE)
```

The last colour is a **tint**, multiplied with the picture — `WHITE` leaves it
alone, red is a cheap damage flash.

**The source rectangle is in pixels of the texture**, not 0-to-1. If frames are
16 wide, frame 3 starts at x = 48. Much easier to reason about, and it is why a
sprite does not break when a texture grows.

### Sprites and animation

Three things, in order.

**1. A `Sheet` — cut a texture into a grid.**

```dado
sprite.Sheet sheet = sprite.grid(texture, 16, 16)
```

**2. `SpriteFrames` — name some animations.**

```dado
sprite.SpriteFrames frames
_ = sprite.add_animation(&frames, "idle", 4.0, true)    // 4 fps, loops
_ = sprite.add_frames(&frames, "idle", 0, 2)            // sheet frames 0,1
_ = sprite.add_animation(&frames, "run", 12.0, true)
_ = sprite.add_frames(&frames, "run", 2, 4)             // frames 2,3,4,5
```

**3. An `AnimatedSprite` — the thing playing.**

```dado
sprite.AnimatedSprite pose = sprite.animated_sprite()
```

**Always call `animated_sprite()`.** A plain zeroed one has `speed_scale` 0
(frozen) and `centered` false, and looks broken in a way that is hard to
diagnose.

Then each frame:

```dado
sprite.play(&pose, &frames, "run")          // pick
sprite.advance(&pose, &frames, dt)          // tick the clock
```

and when you record:

```dado
draw.canvas_item_clear(g_player)
sprite.draw_animated(g_player, sheet, &frames, &pose, 0.0, 0.0, 3.0, WHITE)
```

### Four things worth knowing now

**`play` continues, it does not restart.** Calling it every frame is correct and
intended. Use `play_from_start` to genuinely rewind.

**`advance` and `draw_animated` are separate on purpose.** Recording happens when
something is dirty, so if drawing ticked the clock your animation speed would
depend on how often you re-recorded.

**`centered` defaults true** — `(x, y)` is the sprite's *middle*, following
Godot.

**Flipping is free.** `pose.flip_h = true` mirrors the sprite at no cost — no
second set of frames, no transform, no extra command. It hands the graphics layer
a negative source width so it samples backwards.

---

## 9. Words

```dado
// 1. load a face
(font.Font f, bool ok) = font.load_file(font.default_path(), 18.0)
g_font = f

// 2. put its letter-pictures on the graphics card
(i32 aw, i32 ah) = font.atlas_size(&g_font)
g_atlas = draw.texture_create(aw, ah, font.atlas_pixels(&g_font))

// 3. draw
_ = text.run(g_hud, &g_font, g_atlas, 10.0, 10.0, "SCORE 0", WHITE)
```

Do this in `ready`, where the ambient allocator is the persistent one — a font
built in `process` would be gone next frame.

`load_file` answers a `bool` saying whether it found the file. If it did not you
still get a usable built-in face, so your game shows *something*.

`text.run` takes the **top of the line** and returns the pen x, so runs chain:

```dado
f32 x = text.run(g_hud, &g_font, g_atlas, 10.0, 10.0, "SCORE ", DIM)
_ = text.run(g_hud, &g_font, g_atlas, x, 10.0, score_string, BRIGHT)
```

### The rectangle trick

For a bar behind your text, use **`text.fill`** rather than `draw.rect`:

```dado
text.fill(g_hud, &g_font, g_atlas, 0.0, 0.0, 300.0, 60.0, PANEL)
```

Same rectangle, but drawn *using the font's own texture*, so it joins the same
batch as the letters instead of splitting it. In the platformer this took a frame
from 4 graphics calls to 3.

---

## 10. Input

Three ways to ask, and picking right is most of the battle.

| | answers | use for |
|---|---|---|
| the **queue** (`input.poll`) | *what happened, in order* | typing, mouse, quit |
| the **key table** (`input.key_down`) | *is this key held now* | debug keys |
| the **action map** (`input.is_action_pressed`) | *is the player trying to do this* | **your game** |

**Use the action map.** Same question as the key table, asked about a *name*, so
the player can rebind and your game code never changes.

### Setting it up, once, in `ready`

```dado
_ = input.add_action("move_left")
_ = input.action_add_key("move_left", i32(input.Keycode.Left))
_ = input.action_add_key("move_left", i32('A'))
_ = input.add_action("move_right")
_ = input.action_add_key("move_right", i32(input.Keycode.Right))
_ = input.action_add_key("move_right", i32('D'))
_ = input.add_action("jump")
_ = input.action_add_key("jump", i32(' '))
```

Up to 4 keys per action, 32 actions, 24-character names. Letters and digits are
their uppercase ASCII — `i32('A')`, `i32(' ')`; everything else is in
`input.Keycode`.

### Asking

```dado
if input.is_action_pressed("jump"):            // held right now
if input.is_action_just_pressed("jump"):       // went down THIS frame
f32 dir = input.get_axis("move_left", "move_right")      // -1.0, 0.0 or +1.0
```

`get_axis` is the one to reach for — it answers `0.0` when both are held, which
is what you want and what a hand-written `if`/`else` chain usually gets wrong.

### You do not call `clear_edges`

The `just_pressed` machinery needs wiping once a frame. **The runtime does it**,
at the very end of the frame. Do not call `input.clear_edges()` yourself — you
would wipe the edges twice and silently lose inputs.

### Quitting

Escape-to-quit is a one-off event, so it wants the queue:

```dado
[32]input.Event evs
i32 n = input.poll(evs[:])
for i in 0..<n:
    if input.is_key_press(evs[i]) && evs[i].code == i32(input.Keycode.Escape):
        app.quit()
```

---

## 11. Physics

`app:body2d` is one thing: **a rectangle that walks around and bumps into
walls**. It does *not* do gravity (one line of yours), slopes, rotation,
bouncing, one-way platforms, or bodies hitting each other.

### Telling it where the walls are

You write one function: *is the tile at this grid square solid?*

```dado
private bool tile_solid(rawptr user, i32 tx, i32 ty):
    _ = user
    if tx < 0 || tx >= LEVEL_W:
        return true                      // off the sides = wall
    if ty < 0 || ty >= LEVEL_H:
        return false                     // off top and bottom = open sky
    return g_tile[ty][tx] != 0
```

**It gets asked about squares outside your level all the time**, including
negative ones. Range-check first, always.

```dado
g_solids = body2d.solids(tile_solid, nil, 16.0, 16.0)      // 16x16 tiles
```

### The body

```dado
g_player = body2d.body((100.0, 50.0), (12.0, 16.0))        // position, size
```

**`position` is the box's top-left corner** — the opposite of `app:sprite`'s
`centered` default, so you will need a small offset between picture and box.

### Moving it

```dado
g_player.velocity.y = g_player.velocity.y + GRAVITY * dt   // gravity is yours
body2d.move_and_slide(&g_player, g_solids, dt)
if body2d.is_on_floor(&g_player):
    ...
```

`move_and_slide` moves by the velocity, stops cleanly against anything solid,
and **zeroes the velocity on whichever axis hit** — so a resting body does not
accumulate a thousand frames of downward speed and punch through.

**`is_on_floor` reports the last `move_and_slide`. It does not look around now.**
Same as Godot.

### A character controller, in full

This is the piece to copy.

```dado
private void player_physics(f32 dt):
    bool held = input.is_action_pressed("jump")

    // gravity, with a variable-height jump
    f32 pull = GRAVITY
    if g_player.velocity.y < 0.0 && !held:
        pull = GRAVITY * RELEASE_PULL      // let go early => fall faster => short hop
    g_player.velocity.y = g_player.velocity.y + pull * dt
    if g_player.velocity.y > MAX_FALL:
        g_player.velocity.y = MAX_FALL

    // running, with acceleration and friction
    f32 want = input.get_axis("move_left", "move_right") * RUN_SPEED
    f32 rate = ACCEL
    if want == 0.0:
        rate = FRICTION
    g_player.velocity.x = toward(g_player.velocity.x, want, rate * dt)

    // coyote time: you may still jump for a moment after walking off
    if body2d.is_on_floor(&g_player):
        g_coyote = COYOTE
    else:
        g_coyote = g_coyote - dt

    // jump buffering: a press just before landing still counts
    if input.is_action_just_pressed("jump"):
        g_buffer = BUFFER
    else:
        g_buffer = g_buffer - dt

    if g_buffer > 0.0 && g_coyote > 0.0:
        g_player.velocity.y = 0.0 - JUMP_SPEED
        g_buffer = 0.0                     // spend both, so one press is one jump
        g_coyote = 0.0

    body2d.move_and_slide(&g_player, g_solids, dt)
```

with numbers that feel good to start from:

```dado
const f32 GRAVITY = 2000.0      const f32 MAX_FALL     = 1500.0
const f32 RUN_SPEED = 420.0     const f32 ACCEL        = 2800.0
const f32 FRICTION = 2400.0     const f32 JUMP_SPEED   =  760.0
const f32 COYOTE   =    0.10    const f32 RELEASE_PULL =    2.6
const f32 BUFFER   =    0.12
```

**Coyote time and jump buffering are why good platformers feel good**, and
neither is physics — they are about forgiving the player. Coyote time lets you
jump for a tenth of a second after walking off a ledge. Jump buffering makes a
slightly-early press still jump when you land. Remove them and the controls feel
stiff and unfair and you will not be able to say why.

---

## 12. Failure

Dado has a failure mechanism and the runtime uses it, so it is worth five
minutes.

A function with a `!` in front of its return type may fail:

```dado
!void risky():
    if bad:
        fail 7          // any nonzero i32
    return
```

A caller either **handles** it:

```dado
try risky() else code:
    fmt.println("that did not work: ", code)
```

or **passes it up**, by being failable itself:

```dado
!void caller():
    try risky()         // if it fails, so do we, with the same code
    return
```

### Your hooks can fail

`ready`, `process`, `draw` and `shutdown` are all `!void`. If one fails, the
runtime stops the loop and **`app.run()` fails with that code** — so a problem
deep inside frame 900 comes back out to `main`.

```dado
!void ready():
    g_atlas = draw.texture_create(aw, ah, font.atlas_pixels(&g_font))
    if !draw.texture_valid(g_atlas):
        fail E_ATLAS
    return
```

### Catch it at the root

This is worth doing properly, because otherwise your program exits with a bare
number and an empty terminal:

```dado
!i32 main():
    try app.init(1024, 576, "my game")
    app.hook(ready: ready, process: process)
    try app.run() else code:
        fmt.println("stopped: ", code, " — ", why(code))
        return code
    return 0
```

where `why(code)` turns a number into a sentence. The facade's codes are
`app.E_WINDOW_SIZE`, `app.E_DRAW_UP` and `app.E_ROOT_CANVAS`; yours are whatever
you `fail` with. **Pick your own codes from 5 upward.** `1` is left free by
convention, and `2`, `3` and `4` are the facade's three — `app:graphical` and
`app:headless` use the same numbers deliberately, so one script can read either.
Reuse one and your failure is indistinguishable from a refused window size at
the exit status, which is the whole thing these codes are for.

And if you let a failure out of `main` instead, **the code becomes the exit
status** — `fail 7` exits 7. Useful for a program run by a script.

---

## 13. A tiny complete game

Everything above, in one file. A square you can run and jump around a room.

```dado
package main

import app "app:graphical"
import "app:draw"
import "app:input"
import "app:body2d"
import "core:fmt"

const i32 LEVEL_W = 25
const i32 LEVEL_H = 15
const f32 CELL    = 32.0

const f32 GRAVITY    = 2000.0
const f32 RUN_SPEED  =  420.0
const f32 ACCEL      = 2800.0
const f32 FRICTION   = 2400.0
const f32 JUMP_SPEED =  760.0
const f32 COYOTE     =    0.10
const f32 MAX_STEP   =    0.05

const [4]u8 WALL   = [ 90,  78,  66, 255]
const [4]u8 PLAYER = [120, 220, 200, 255]

// 1 is solid, 0 is air.
private [LEVEL_H][LEVEL_W]i32 g_tile

private draw.Item g_world
private draw.Item g_hero

private body2d.Body   g_body
private body2d.Solids g_solids
private f32 g_coyote

private bool tile_solid(rawptr user, i32 tx, i32 ty):
    _ = user
    if tx < 0 || tx >= LEVEL_W || ty < 0 || ty >= LEVEL_H:
        return true
    return g_tile[ty][tx] != 0

private void build_level():
    for y in 0..<LEVEL_H:
        for x in 0..<LEVEL_W:
            bool edge = y == 0 || y == LEVEL_H - 1 || x == 0 || x == LEVEL_W - 1
            bool ledge = y == 10 && x > 6 && x < 13
            if edge || ledge:
                g_tile[y][x] = 1
            else:
                g_tile[y][x] = 0

!void ready():
    app.set_background(0.07, 0.08, 0.12, 1.0)

    _ = input.add_action("move_left")
    _ = input.action_add_key("move_left", i32(input.Keycode.Left))
    _ = input.action_add_key("move_left", i32('A'))
    _ = input.add_action("move_right")
    _ = input.action_add_key("move_right", i32(input.Keycode.Right))
    _ = input.action_add_key("move_right", i32('D'))
    _ = input.add_action("jump")
    _ = input.action_add_key("jump", i32(' '))

    build_level()
    g_solids = body2d.solids(tile_solid, nil, CELL, CELL)
    g_body = body2d.body((64.0, 64.0), (24.0, 30.0))

    // The runtime already made the canvas. These are children of it.
    g_world = draw.canvas_item_create(app.canvas())
    g_hero  = draw.canvas_item_create(app.canvas())
    _ = draw.canvas_item_set_parent(g_hero, g_world)

    // Recorded ONCE, here in `ready`, where the allocator is the persistent one.
    for y in 0..<LEVEL_H:
        for x in 0..<LEVEL_W:
            if g_tile[y][x] != 0:
                draw.rect(g_world, (f32(x) * CELL, f32(y) * CELL, CELL, CELL), WALL)

    // The hero is recorded once too — at the ORIGIN. It is moved, not redrawn.
    draw.rect(g_hero, (0.0, 0.0, 24.0, 30.0), PLAYER)
    return

!void process(f32 elapsed):
    f32 dt = elapsed
    if dt > MAX_STEP:
        dt = MAX_STEP

    g_body.velocity.y = g_body.velocity.y + GRAVITY * dt

    f32 want = input.get_axis("move_left", "move_right") * RUN_SPEED
    f32 rate = ACCEL
    if want == 0.0:
        rate = FRICTION
    f32 dv = rate * dt
    f32 vx = g_body.velocity.x
    if vx < want:
        vx = vx + dv
        if vx > want:
            vx = want
    else if vx > want:
        vx = vx - dv
        if vx < want:
            vx = want
    g_body.velocity.x = vx

    if body2d.is_on_floor(&g_body):
        g_coyote = COYOTE
    else:
        g_coyote = g_coyote - dt

    if input.is_action_just_pressed("jump") && g_coyote > 0.0:
        g_body.velocity.y = 0.0 - JUMP_SPEED
        g_coyote = 0.0

    body2d.move_and_slide(&g_body, g_solids, dt)

    // The whole of "drawing the player": one call, no recording.
    draw.canvas_item_set_position(g_hero, (g_body.position.x, g_body.position.y))
    return

!i32 main():
    try app.init(800, 480, "tiny")
    app.hook(ready: ready, process: process)
    try app.run() else code:
        fmt.println("tiny: could not run, code ", code)
        return code
    return 0
```

**Look at what is not there.** No `draw.init`. No canvas creation. No render
call. No `clear_edges`. No allocator. No shutdown. No draw hook — the runtime
renders its canvas for you.

And look at what `process` does not do: it never draws the level, never draws the
player. It moves one item. The 82 wall rectangles were recorded once in `ready`
and are never thought about again.

---

## 14. Counters

`app:draw` will tell you what a frame cost. Print these while you are learning —
they teach more than any explanation.

```dado
draw.commands()          // drawing commands replayed
draw.primitives()        // shapes that came to
draw.state_changes()     // an UPPER BOUND on graphics calls
draw.gpu_draw_calls()    // the real number — for the PREVIOUS frame
draw.painted()           // how many items were visible
```

Two need care: **`state_changes()` is an upper bound**, because the graphics
layer merges work behind you; **`gpu_draw_calls()` is one frame late**, so read
it at the top of a frame and know what it is about.

### What good looks like

A whole screen of a tile game should be a handful of graphics calls — the
platformer's entire frame is **3**. If yours is 40, something is splitting your
batches; the usual culprit is alternating textured and untextured drawing.

And the number to watch while learning retained mode: **how many items you
re-record per frame.** Running at full speed it should be near zero.

---

## 15. Gotchas

**Lifecycle**

- Name your `app.hook` arguments. `ready`, `draw` and `shutdown` are one type and
  a positional swap compiles silently.
- `dt` is not clamped. Clamp it, or one slow frame teleports you through a wall.
- The runtime renders **after** your draw hook, so recording there lands on the
  same frame.

**Memory**

- `ready`/`shutdown` allocate persistently; **`process`/`draw` allocate into a
  per-frame arena that is emptied at the end of the frame.**
- Keeping something made in `process` needs `using app.allocator():`.
- `#resize`/`#append` keep the allocator the reference already had. Only the
  create verbs move.
- Do not pass allocators as parameters. That is what `using` is for.

**Drawing**

- **Recording appends.** `canvas_item_clear` first, every time you re-record.
- Invalid handles do nothing, silently — no crash, no message, just a thing that
  never appears.
- `canvas_item_free` takes the whole subtree.
- `set_parent` keeps the *local* position, so the item jumps.
- `y` grows downward.
- Don't put a `draw.rect` in the middle of a run of text — use `text.fill`.
- Clipping is axis-aligned; a rotated clip clips to its bounding box.
- There is no nearest-neighbour filtering. For crisp pixel art, bake your sheet
  at the size you show it and round the camera to whole pixels.

**Input**

- **Do not call `input.clear_edges()`** — the runtime owns it.
- An unknown action name answers `false` for everything and never complains. A
  typo in `"jmup"` is a jump that silently never happens; `input.has_action`
  checks.
- Two taps in one frame count as one. Use the queue if you need the count.

**Sprites**

- Always `sprite.animated_sprite()`. A zeroed one is frozen and mis-anchored.
- `sprite.advance` moves the animation; `draw_animated` does not.
- `centered` defaults true — `(x, y)` is the middle.

**Physics**

- `body.position` is the box's **top-left**; a sprite's is its **centre**.
- Your `SolidFn` gets asked about squares outside the level. Range-check first.
- `is_on_floor()` reports the last `move_and_slide`.
- Gravity is your line, not the package's.

**Language**

- You cannot mix named and positional arguments (`ERR0604`).
- A bare float literal has no width: bind `f32 a = x + 2.5` first.
- A declared name never read is a warning; `_ = thing` silences it.

---

## 16. Where to look next

**Read the programs.** They are the real documentation, they are all compiled,
and they run:

| program | why |
|---|---|
| `example/hellotext` | the smallest complete graphical program |
| `example/platformer` | a whole game: tiles, sprites, physics, camera, HUD. Its `README.md` lists what `app:` could not do |
| `example/drawsmoke` | every drawing feature checked against real pixels |
| `example/uismoke` | `app:ui`, if you want menus |
| `example/window` | the runtime's own knobs, exercised one at a time |

**Read `docs/language.md`** on allocators — *The `Allocator` trait* and *What a
frame takes back*. It is the authority on everything in §4.

**Read the package headers.** Every file under `collections/app/*/` opens with a
long comment arguing for its design, stating costs, and naming what was
considered and rejected. That is where the real answers live.

**When this file and the code disagree, the code is right.**
