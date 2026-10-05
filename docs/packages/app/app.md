<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:app

app:app — the runtime's engine room.

**`app` is a runtime, not a library.** That is the sentence the rest of this
file is written against, and it is the sentence this file used to get wrong.
It described a library a program drives: the program brought `app:draw` up,
the program made a canvas, the program rendered it, the program called
`input.clear_edges()` at the right moment, the program decided which
allocator everything landed in. Every one of those is now the runtime's, and
what is left for a program to write is the part that is actually the
program's.

## What a runtime is for, and who it is not for

It is **opinionated**, and that is the point rather than an apology. There is
one frame shape, one root canvas, one persistent allocator, one per-frame
temp arena, one input barrier and one place each of them happens. A program
that wants to prototype — to get a window, a canvas and a loop and then spend its
attention on the thing it is actually making — writes four lines and gets all
of it:

    import app "app:graphical"
    import "app:draw"

    !void ready():
        draw.Item bg = draw.canvas_item_create(app.canvas())
        draw.rect(bg, (0.0, 0.0, 800.0, 600.0), (26, 30, 38, 255))

    !i32 main():
        try app.init(800, 600, "sketch")
        app.hook(ready: ready)
        try app.run() else code:
            return code
        return 0

There is no `draw.init` in that program, no `canvas_create`, no
`canvas_render`, no `clear_edges` and no allocator written anywhere — and it
draws, every frame, for as long as the window is open. That is what "a
runtime is packaged for you" means here.

**What the runtime does not take over is a font.** That was the obvious next
thing to package and it is deliberately absent; the argument, with the bytes
it would have cost every program, is at `app:graphical`'s `canvas()`.

**What a program gives up is control over all six of those decisions.** The
root canvas is the runtime's and is rendered on the runtime's schedule. The
ambient allocator inside every hook is the runtime's, and which of its two
arenas it is, is the runtime's too. The input edges are
cleared on the runtime's clock. The pass is opened and closed by the runtime
and a program never sees the handle. If any of that is the thing you are
building — a custom engine, a second render target, a frame graph, an
allocator strategy per subsystem, a loop that is not one-frame-at-a-time —
**this is the wrong package**, and the repair is not to fight it: import
`app:display`, `app:draw`, `app:input` and `core:jobs` directly and write the
twenty lines this package is. They are all public, none of them requires this
one, and `app:graphical`'s `run_loop` is the worked example of driving them
by hand.

## What is still in this file, and what is not

The app is a singleton — one window, one loop, one app — so nothing is
threaded on a `^App`: the state lives here in package scope and is reached
through accessors, the way `#default` is ambient. The lifecycle hooks take no
app and no allocator, and **all four are failable**:

    !void ready()               // once, after the surface is up
    !void process(f32 dt)       // once per frame, outside the pass
    !void draw()                // once per drawn frame, inside the pass
    !void shutdown()            // once, on teardown

A program imports a *facade* (`app:headless`, `app:graphical`) to install a
backend on the singleton, assigns its hooks — one at a time with
`on_ready`/`on_process`/`on_draw`/`on_shutdown`, or all four in one call with
`hook` — and calls `run`. One backend at a time, one process-wide app, and the
hooks are the only entry points a program writes.

**The facade is the front door, and this package is the room behind it.** A
program binds its facade under the name `app` (`import app "app:graphical"`)
and reaches `init`, `hook`, `run`, `quit`, `canvas` and the two allocators
through it; every one of those but `init`, `run` and `canvas` is a forward to
a name declared here.

**This package imports no backend, no window and no drawing.** That is not
tidiness, it is what makes the escape hatch above real: a program that must
not link sokol installs a `Backend` of its own and gets the loop, the
allocators, the job pool and the frame order with nothing else attached. The
five *services* a runtime drives — bringing a drawing surface up, tearing it
down, rendering the root canvas, delivering what other threads queued for
the frame, and ending the input frame — are therefore **slots on the
`Backend` vtable** rather than calls written here.

One of the four is forced and the other three follow it. `app:input` imports
*this* package (it asks `supports(Capability.Input)` before it drains), so
this package calling `input.clear_edges()` is `ERR0302` — the import graph is
acyclic and that edge is the cycle. A vtable slot is the way out, and once
there is one mechanism for reaching a package this file may not import, using
a second mechanism for the three that it merely *should* not import would be
two answers to one question. So `surface_up`, `surface_down`, `render`,
`end_input` and `deliver` are all slots, all nil-checked, and a backend that
fills none of them still runs.

## Declarations

77 declarations, 46 public.

* `@const string8 ROLE = "standalone"` — ── role (an inert build seat) ─────────────────────────────────────────────…
* `type ReadyFn: !void()` — ── lifecycle hooks ─────────────────────────────────────────────────────────…
* `type ProcessFn: !void(f32)`
* `type DrawFn: !void()`
* `type ShutdownFn: !void()`
* `void on_ready(ReadyFn f)` — Assign the lifecycle hooks. Order does not matter; call before `run`.
* `void on_process(ProcessFn f)`
* `void on_shutdown(ShutdownFn f)`
* `void on_draw(DrawFn f)` — Assign the draw hook — see the section above. `process` is outside the render…
* `void hook(ReadyFn ready = nothing, ProcessFn process = nothing_process, DrawFn draw = nothing, ShutdownFn shutdown = nothing)` — ## And what "is a draw hook set" is for now…
* `void request_redraw()` — Ask for the next frame to be drawn. Cheap, idempotent, and safe to call from…
* `void set_redraw_on_demand(bool on)` — Draw every frame (`false`, the default) or only after `request_redraw`
* `bool redraw_pending()` — Whether the next `tick` will open the pass. Always true while on-demand…
* `bool drew()` — Whether the **last** `tick` opened the render-pass bracket — which is the…
* `enum u32 Capability` — ── capabilities ────────────────────────────────────────────────────────────…
* `distinct type Caps: u32`
* `Caps caps(..Capability cs)` — Build a capability set from a list of capabilities.
* `type Backend` — ── the backend vtable ──────────────────────────────────────────────────────…
* `void set_backend(Backend b)` — A facade installs its backend on the singleton before `run`.
* `rawptr backend_data()` — The backend's own opaque state, for a facade that needs it. Nil until set.
* `void quit()` — Ask the loop to stop after this frame.
* `bool quitting()` — A backend's loop polls this to know when to stop. True when the program asked…
* `bool failed()` — Whether the last `run` ended in a failure, and the code it ended with. `0`
* `i32 fail_code()`
* `bool supports(Capability cp)` — ── capability queries ──────────────────────────────────────────────────────…
* `void set_user(rawptr p)`
* `rawptr user()`
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) allocator()` — The persistent allocator — and the ambient `#default` inside `ready` and…
* `void set_allocator((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) a)` — Swap the persistent allocator. **Before `run`, and this now refuses…
* `const i32 TEMP_DEPTH = 32` — ── the app's temp arena ───────────────────────────────────────────────────…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) temp()` — The app's temp arena, as an allocator. Already the ambient `#default` inside…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) enter_temp(bool escaping = false)` — Open a temp frame and install it: a checkpoint is taken and the arena is…
* `bool close_temp()` — Close the innermost temp frame, rolling the arena back to where it was.
* `u64 temp_used()` — How many bytes the temp arena is holding. `0` at the top of every frame,…
* `i32 temp_depth()` — How many temp frames are open. `0` everywhere a correct program can observe…
* `type JobArgs` — ── the job system ─────────────────────────────────────────────────────────…
* `type JobFn: void(^JobArgs)`
* `distinct type Handle: i32`
* `Handle submit(JobFn fn, rawptr data)` — Submit one job. Returns a handle to wait on (`wait`); a job hands its result…
* `Handle submit_after(JobFn fn, rawptr data, Handle dep)` — Submit `fn` to run after `dep`.
* `void parallel_for(i32 n, i32 grain, JobFn fn, rawptr data)` — Fork-join over [0, n) in `grain`-sized chunks; joins before returning.
* `void wait(Handle h)` — Cooperatively wait for a handle.
* `void startup()` — Once, at the top of the loop: the job pool, the drawing surface, the `ready`
* `void tick(f32 dt)` — One frame. **One shape, for every program.**
* `void teardown()` — Once, at the bottom of the loop: the `shutdown` hook, the drawing surface,…
* `!void run()` — Run the app: hand the loop to the backend, which calls the frame steps above.
