<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:graphical

app:graphical — the graphical runtime: a real sokol backend, a window, a
drawing surface and a root canvas, on the app singleton.

    import app "app:graphical"
    import "app:draw"

    !void ready():
        draw.Item bg = draw.canvas_item_create(app.canvas())
        draw.rect(bg, (0.0, 0.0, 800.0, 600.0), (26, 30, 38, 255))

    !void process(f32 dt): ...

    !i32 main():
        try app.init(800, 600, "my app")
        app.hook(ready: ready, process: process)
        try app.run() else code:
            fmt.println("app.run failed: ", code)
            return 1
        return 0

**There is no `draw.init` in that program, no `canvas_create`, no
`canvas_render` and no `input.clear_edges`.** The runtime does all four, and
the argument for each is at the declaration that does it. `main` is `!i32`
because both `init` and `run` are failable — `init` refuses a window size
that is not one, and `run` carries out a failure raised inside the frame
loop, which is where every other thing that can go wrong goes wrong.

## The front door is this package, and the door is an import alias

A graphical program used to open with two imports, and with one line that
reached past the facade it had just bound:

    import "app:app"
    import gx "app:graphical"
    ...
    gx.new(1024, 576, "Kettle Hollow")
    app.on_ready(ready)
    app.on_process(process)
    app.on_draw(draw_frame)
    app.run()

One package, imported under a name of its own, touched **once**, for one
line, and never again for the rest of the program. Sam's reading of that was
that it is weird to reach into `gx` to get your window going, and it is: the
thing a program names `gx` is not a thing it uses, it is a thing it opens
with.

So this package now carries the whole surface a program writes — `init`,
`hook`, `run`, `quit`, `canvas`, the two allocators, the redraw knobs, the
four individual `on_*` setters — and a program binds **it** under the name
`app`:

    import app "app:graphical"

**`app:app` could not have grown a window call instead**, and that is what
forced an alias rather than a re-export. This package imports `app:app` (see
the import below), so `app:app` importing this one is `ERR0302` — the import
graph is acyclic and that edge is the cycle. Handing the window call up to
the core would also mean the core knowing what a window is, which is the one
thing it is built not to know: it is the same core under `app:headless`.
And a `type X: app.Y` re-export is not a workaround either: a cross-package
type re-export compiles or does not according to the *root* package's
import order, which the re-exporting file does not mention.

So `app:app` is untouched by the window and stays the substrate-agnostic
runtime core. Every name in the forwarding section below is one line that
calls it, and a program that wants the core directly still writes
`import "app:app"` and gets exactly what it always got. What the alias buys
is that the **body** of a program is identical under either facade: swap
`import app "app:graphical"` for `import app "app:headless"`, change the one
`init` line — which differs because a headless program has no window size —
and nothing else moves.

**What an alias cannot carry is a type.** `app.Capability`, `app.JobArgs`,
`app.JobFn` and `app.Handle` are types and the `ROLE` seat is a `@const`, and
none of the five can be re-exported here: a re-exported type is the
import-order defect named above, and a
second `@const ROLE` in this package would make `-D ROLE=server` name two
declarations (`ERR1103`) and break the build seat for everybody. So a program
that asks `app.supports(...)`, submits jobs or reads `ROLE` imports `app:app`
beside this one, under its own name, and writes `core.supports(...)`.

**`canvas()` is the one place this is visible in the new surface.** It
answers a `draw.Canvas`, which is `app:draw`'s type and not this package's,
so a program that touches the canvas imports `app:draw` for the type — which
it was going to import anyway, because everything it does with the canvas is
an `app:draw` call.

## What this facade fills the five service slots with

`app:app` drives five services it may not import (the argument is in its own
package comment). This facade is what fills them:

  surface_up    `draw.init()` under the runtime's allocator, then one
                `draw.canvas_create()` — the root canvas, which `canvas()`
                answers and which the program never has to make.
  surface_down  `draw.shutdown()`.
  render        `draw.canvas_render(root, w, h)` at the window's current
                size, inside the pass, after the program's draw hook.
  end_input     `input.clear_edges()`.
  deliver       `input.deliver()` — the frame's one drain point, before
                `process`.

**That is the whole of what changed here, and it moved four calls out of
every graphical program in the tree.**

## One runtime, and which one won

There used to be two. `app:app` is the singleton runtime — four hooks, one
ambient temp arena rolled back at the barrier, `core:jobs` started and stopped
around it, and a `Backend` vtable whose `begin_frame`/`end_frame` bracket the
draw hook. `app:display` had grown a second one: its own frame loop, its own
five hooks carrying an `Allocator` each, a separate `draw` phase, and a
second per-frame arena of its own. Nothing imported both, so the two never had
to agree.

**`app:app`'s shape wins, whole.** It is what every other `app:` package and
every example is written against, it is the one with a job system attached to
its barrier, and its `Backend` vtable is the only seat a facade can install
into. `app:display`'s hook shape is now four callbacks that carry no
allocator and make no policy — the seam this package drives, and nothing a
program writes against. There is exactly one frame loop (sokol's, entered
through `sapp_run`), exactly one temp arena (`app.temp()`, rolled back in
`app.tick`), one root canvas and one hook shape.

## Where the pass goes, and why the existing bracket is the right seat

Something has to sit between `sg_begin_pass` and `sg_end_pass` or nothing is
ever drawn. `app.Backend.begin_frame`/`end_frame` are the one bracket in the
runtime, so the pass goes there: `begin_frame` is `display.begin_pass()`,
`end_frame` is `display.end_pass()`, and what `app.tick` puts between them is
the program's `draw` hook and then the runtime's own `render`.

It is the right seat and not merely an available one, on three counts. The
bracket is *already* nil-checked per frame, so a backend with no pass (see
`app:headless`) costs nothing. It is inside `app.tick`, so the temp rollback
lands after `end_frame` — a draw hook may allocate temporaries. And
it is the only bracket in the runtime, so there is no second place a drawing
call could be legal, which is what `display.drawing()` reports.

## Idling, and what sokol will not do (E7)

`_sapp_linux_run` is a busy loop: `XPending` (non-blocking), drain, frame
callback, `XFlush`, again. There is no wait-for-event mode on GLX, no
`sapp_desc` field that adds one, and the only thing in the loop that ever
blocks is the buffer swap under vsync — which stops blocking under software
Mesa or an unmapped window. **sokol cannot idle**, and this package does not
pretend it can.

What it does instead is skip the expensive part and then give the core back:
`app.set_redraw_on_demand(true)` makes `app.tick` skip the whole bracket on a
frame nothing asked for, and `set_idle_frame_time` makes an iteration that
drew nothing last at least a few milliseconds. Neither is on by default. See
`app:app`'s conditional-frames section for why the default is off.

**A skipped frame still swaps.** `_sapp_linux_frame` calls
`glXSwapBuffers` after every frame callback, drawn or not, so under vsync
an undrawn iteration still waits up to a refresh in the swap. A fixed nap
on top of that would stack the two waits — an 8 ms nap plus a refresh is
about 25 ms an iteration at 60 Hz, a 40 Hz poll. So the nap is a budget, not
a fixed sleep: `_frame` measures the time since the previous iteration's
callback returned (the swap, the event drain and this tick) and naps only
for what is left of `set_idle_frame_time` (`idle_nap_ms`). Where the swap
blocks, it paces the loop and the nap is zero or nearly; where it does not
(software Mesa, Xvfb, an unmapped window), the nap brings each iteration up
to the budget. An idle loop therefore polls at the slower of the display's
refresh and the budget, and an event waits at most that one period before
its frame begins.

## The capability set is what this backend can do today

  Input      — yes. `app:input/sokol` decodes sokol's event stream into
               `app:input`'s queue; the wiring is `_event` below. File drops
               are part of it and are the one input the window has to *ask*
               for — see `set_file_drop`.
  Windowing  — yes. sokol_app opens, sizes, titles and fullscreens a window,
               and `app:display` exposes all of it.
  Clipboard  — yes. `desc.enable_clipboard` is set in `display`'s substrate,
               which is what makes `sapp_get_clipboard_string` return
               anything at all, and `display.clipboard_get`/`_set` reach it.
  Threads    — yes. `core:jobs` gets a real worker pool, same as `app:headless`.
  VectorDraw — **yes, and this is new.** The old entry said no, and the
               reason it gave was exact: *"this backend neither imports nor
               installs `app:draw`: a program calls `draw.init` itself and
               `draw.init` can fail and say so. A capability is a promise the
               backend keeps; this one would be a promise about a package it
               does not own and a call it cannot make."* Every clause of that
               is now false. This package imports `app:draw`, installs it in
               `surface_up`, owns the root canvas, renders it every frame,
               and carries `draw.init`'s refusal out to `main` as
               `E_DRAW_UP`. The promise is one it keeps.
  Audio      — **no.** `sokol_audio` is neither bound nor imported.

This package used to advertise all six and then `#panic` in `run_loop` having
honoured none of them. The set is still the honest one: five, because five is
what it does.

## Declarations

54 declarations, 31 public.

* `const i32 E_WINDOW_SIZE = 2` — ── failure codes ───────────────────────────────────────────────────────────…
* `const i32 E_DRAW_UP = 3`
* `const i32 E_ROOT_CANVAS = 4`
* `!void init(i32 width, i32 height, string8 title)` — Open a window `width` x `height` titled `title`: remember the window and…
* `void new(i32 width, i32 height, string8 title)` — Install the sokol backend on the app singleton. Call before `app.run()`.
* `draw.Canvas canvas()` — The runtime's root canvas. Valid from the first line of `ready` until…
* `void hook(app.ReadyFn ready = nothing, app.ProcessFn process = nothing_process, app.DrawFn draw = nothing, app.ShutdownFn shutdown = nothing)` — Assign any subset of the four lifecycle hooks in one call. An omitted…
* `void on_ready(app.ReadyFn f)` — The four hooks, one at a time. Unchanged in meaning; see `app:app`.
* `void on_process(app.ProcessFn f)`
* `void on_draw(app.DrawFn f)`
* `void on_shutdown(app.ShutdownFn f)`
* `!void run()` — Open the window and run the frame loop until it closes, then report whatever…
* `void quit()` — Ask the loop to stop after this frame.
* `bool quitting()` — Whether something has asked the loop to stop — the program with `quit()`, or…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator()` — The two allocators, and the temp arena's checkpoints.
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) temp()`
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) enter_temp(bool escaping = false)` — Open a temp frame and install it; close the innermost one.
* `bool close_temp()`
* `u64 temp_used()` — How many bytes the temp arena is holding (`0` at the top of every frame), and…
* `i32 temp_depth()`
* `void set_allocator((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a)` — Swap the persistent allocator. **Before `run`** — it refuses afterwards, and…
* `void request_redraw()` — The conditional-frame knobs (E7). See `app:app`'s own section for what sokol…
* `void set_redraw_on_demand(bool on)`
* `bool redraw_pending()`
* `void set_user(rawptr p)` — The program's own state pointer, for a library that has to reach it through…
* `rawptr user()`
* `void set_background(f32 r, g, b, a)` — The colour the frame is cleared to, before anything a program draws. Settable…
* `void set_idle_frame_time(i32 milliseconds)` — The least time, in milliseconds, one loop iteration takes when its frame…
* `void set_close_confirm(bool on)` — Make `input.EventKind.Quit` a decision the program takes, rather than an…
* `void set_file_drop(bool on)` — Let files be dropped on the window. Call before `app.run()`; sokol reads it…
* `i32 idle_nap_ms(i32 budget_ms, f64 spent_s)` — Milliseconds left to nap of an idle budget of `budget_ms` when `spent_s`
