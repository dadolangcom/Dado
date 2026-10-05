<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:display/src

display/src — the private sokol substrate behind app:display.

Everything here is implementation the public `display` facade tucks out of
sight: the sokol frame loop and its callbacks, and the clear colour. The
public interface — what the module *provides* — is one directory up in
`display.dado`. Nothing here is meant to be imported except by that facade.

**This package owns no runtime.** It used to: it kept five hooks, a
per-frame scratch arena and a `draw` phase of its own, which made it a second
runtime beside `app:app`'s and the two did not reconcile. There is one
runtime now and it is `app:app`'s. What is left here is the part sokol
genuinely forces on somebody — `sapp_run` fuses window creation with the loop,
so the loop entry cannot live above the sokol layer — reduced to four
callbacks that carry no allocator and make no policy:

    ready()      after sg_setup, before the first frame
    frame(dt)    once per frame; the caller decides what a frame is
    event(ev)    one raw sapp_event address, whenever sokol has one
    finish()     before sg_shutdown

and the clear pass split into `begin_pass`/`end_pass` so the caller — not this
package — chooses what sits between them.

## Declarations

19 declarations, 5 public.

* `void run(i32 width, i32 height, string8 title, i32 clipboard_bytes, i32 max_dropped_files, i32 max_dropped_path_bytes, void() ready, void(f32) frame, void(rawptr) event, void() finish)` — Open the window, install the callbacks, run sokol's frame loop until it…
* `void set_background(f32 r, g, b, a)` — The colour the frame is cleared to each frame (0..1 per channel).
* `bool drawing()` — True only inside the render pass (between begin_pass and end_pass).
* `void begin_pass()` — Begin the default swapchain pass, clearing to the background colour.
* `void end_pass()` — End the pass and present the frame.
