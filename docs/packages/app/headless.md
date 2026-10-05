<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:headless

app:headless — the same runtime with nothing on the other end of it.

    import app "app:headless"
    import "app:draw"

    !void process(f32 dt): ...

    !i32 main():
        try app.init("my batch job")
        app.hook(process: process)
        try app.run() else code:
            fmt.println("app.run failed: ", code)
            return 1
        return 0

`main` is `!i32` because both `init` and `run` are failable — `init` refuses
a frame budget that is not one, and `run` carries out a failure raised inside
the frame loop, which is where a hook's own `fail` is raised.

## The facade is the front door, and the body is the other facade's

A program used to bind this package under a name of its own, reach into it
for one line, and write everything else against `app:app`. It does not any
more: this package carries the surface a program writes — `init`, `hook`,
`run`, `quit`, `canvas`, the two allocators, the four `on_*` setters — as
one-line forwards to `app:app`, and a program binds **it** as `app`. The
argument for the alias, and why `app:app` could not have grown the call
instead (it would be `ERR0302`, a cycle), is written out in `app:graphical`'s
package comment and is the same argument here.

**`hook`, `run`, `quit`, `canvas` and `allocator` match `app:graphical`'s
exactly**, which is the point of doing this in both places: the *body* of a
program is identical under either facade, and swapping
`import app "app:headless"` for `import app "app:graphical"` moves nothing
but the `init` line.

**`init` is where the two honestly differ**, and it differs rather than being
bent into a match. `app:graphical.init(w, h, title)` takes a window size
because it opens a window; this one takes `init(name, max_frames = 0)`
because a headless program has no window to size and does have a frame budget
to set. A facade whose `init` pretended to take a width would be a facade
lying about what it is, which is the exact failure `app:graphical` spent a
milestone committing with its capability set.

No window, no terminal, no input source, no GPU. It installs a `Backend` on
the app singleton whose `run` owns a plain frame loop: `app.startup()`, then
`app.tick(dt)` with a real monotonic `dt` until `app.quitting()`, then
`app.teardown()`. That is the whole of the loop.

## It brings `app:draw` up too, in the mode that paints nothing

This is the decision in this package most worth arguing, because it is not
free.

**What it does.** `surface_up` calls `draw.init_headless()` and makes a root
canvas, exactly as `app:graphical` does with `draw.init()`. `render` calls
`draw.canvas_render` on it every frame. `app:draw`'s headless server walks
the tree, orders it, resolves clips and tints and counts every command and
primitive a GPU render would have issued — and calls not one sokol function.
So `app.canvas()` answers a real canvas under this facade and a program's
recording code runs unchanged.

**Why.** The two facades exist so that a program's body is portable between
them, and a canvas that exists under one and not the other breaks exactly
that promise at the one moment it is most likely to be used: moving a sketch
from a batch check to a window, or the reverse — running a graphical
program's logic under a gate with no display. `app:draw`'s counting server
was built for that second case; leaving it out of the facade whose whole job
is *no display* would be leaving it where nobody can reach it.

**What it costs, in numbers.** `app:draw` imports `vendor:sokol/sokol_gp` and
`vendor:sokol/sokol_gfx`, so a headless program links them whether or not it
ever calls one. Measured on `example/log`, the same program built before and
after this change: **42 104 → 304 488 bytes** of binary, **222 209 → 953 166
bytes** of emitted C, and **3.0 s → 6.3 s** of build. That is a sevenfold
binary and a doubled build for a program that prints numbers, and it is the
honest price of the portability above.

**The escape is real and it is one line.** A program that must not link sokol
writes `import "app:app"` and installs a `Backend` of its own with the three
surface slots left nil; it gets the loop, both allocators, the job pool and
the whole frame order, and `app:app` imports no drawing at all. That escape
is *why* the services are vtable slots rather than calls inside the core —
see `app:app`'s vtable section.

**It does not bring `app:font` up, and neither facade does.** A font is a
face at a size and the size is the program's; the argument, and what a
runtime-owned default face would have cost every program in bytes, is at
`app:graphical`'s `canvas()`.

**It still does not advertise `VectorDraw`.** A capability is a promise about
what reaches a screen, and nothing here does. `draw.headless()` is how a
program asks whether it is recording into the counting server, and that is a
question about `app:draw` rather than about this backend.

## Why it exists

The terminal backend used to be the only working one, so every headless
program in the tree — a job-pool demo, a scene-graph demo, a network server —
imported a *terminal* package to get a frame loop, and then spent a branch on
"am I actually attached to a tty" to decide whether to paint. That is a loop
driver wearing a terminal's clothes. This package is the loop driver with the
clothes taken off: a program that never draws now says so in its imports, and
the graphical backend is free to be the only one that owns a screen.

**It advertises `Threads` and nothing else, and that is the honest set.**
`app:app`'s scheduler is real here — `core:jobs` starts a worker pool when
the backend says `Threads`, and it does not care that nobody is watching.
Everything else in `app.Capability` needs a substrate this package refuses to
have: there is no event source to fill `app:input`'s queue (`Input`), nothing
that reaches a screen (`VectorDraw`), no window (`Windowing`), no device
(`Audio`) and no selection to read (`Clipboard`). A program that reaches for
one of those gets the empty answer — `input.poll` returns 0 — and
`app.supports` is how it asks in advance.

**Termination is the caller's, with two backstops.** The loop runs until
`app.quit()`; a program that means to stop calls it from `ready` or
`process`. Because a headless program has nobody to notice it wedged, `init`
also takes an optional frame budget: `init("x", 300)` stops the loop after
300 ticks whatever the program does, and `hit_max_frames()` afterwards says
whether that is what happened — so a stuck example fails a gate run in
bounded time instead of hanging it. `0` (the default) means "no cap". The
second backstop is the runtime's: a hook that fails stops the loop too, and
`run` reports its code.

## Declarations

48 declarations, 30 public.

* `const i32 E_FRAME_BUDGET = 2` — ── failure codes ───────────────────────────────────────────────────────────…
* `const i32 E_DRAW_UP = 3`
* `const i32 E_ROOT_CANVAS = 4`
* `!void init(string8 name, i32 max_frames = 0)` — Install the headless backend on the app singleton. This is the first line of…
* `void new(string8 name, i32 max_frames = 0)` — Install the headless backend on the app singleton.
* `draw.Canvas canvas()` — The runtime's root canvas. Valid from the first line of `ready` until…
* `void set_surface_size(i32 width, i32 height)` — The size the counting render measures against, in pixels. There is no window…
* `void hook(app.ReadyFn ready = nothing, app.ProcessFn process = nothing_process, app.DrawFn draw = nothing, app.ShutdownFn shutdown = nothing)` — Assign any subset of the four lifecycle hooks in one call. An omitted…
* `void on_ready(app.ReadyFn f)` — The four hooks, one at a time. Unchanged in meaning; see `app:app`.
* `void on_process(app.ProcessFn f)`
* `void on_draw(app.DrawFn f)`
* `void on_shutdown(app.ShutdownFn f)`
* `!void run()` — Run until the app quits, a hook fails, or the frame budget runs out — then…
* `void quit()` — Ask the loop to stop after this frame.
* `bool quitting()` — Whether something has asked the loop to stop — the program with `quit()`, or…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator()` — The two allocators, and the temp arena's checkpoints.
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) temp()`
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) enter_temp(bool escaping = false)` — Open a temp frame and install it; close the innermost one.
* `bool close_temp()`
* `u64 temp_used()` — How many bytes the temp arena is holding (`0` at the top of every frame), and…
* `i32 temp_depth()`
* `void set_allocator((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a)` — Swap the persistent allocator. **Before `run`** — it refuses afterwards, and…
* `void request_redraw()` — The conditional-frame knobs (E7). Forwarded so a program's body stays…
* `void set_redraw_on_demand(bool on)`
* `bool redraw_pending()`
* `void set_user(rawptr p)` — The program's own state pointer, for a library that has to reach it through…
* `rawptr user()`
* `void set_frame_time(i32 milliseconds)` — Milliseconds this backend aims to spend on a frame. 0 runs flat out.
* `i32 frames()` — Frames ticked so far — readable from inside `process`, and after `run`
* `bool hit_max_frames()` — True if the loop stopped because it ran out of frame budget with the…
