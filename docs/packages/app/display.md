<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:display

app:display — the display server.

"Server" is Godot parlance. This is not Godot's server architecture; it
approximates the *surface* of Godot's DisplayServer (see
Godot's own, held only as reference material) over vendor:sokol.

This file is the whole public interface — the window/mouse/clipboard/timing
surface and the run entry. The private sokol machinery (the sokol callbacks
and the clear colour) lives in `src/` and is not imported from anywhere but
here; see `src/backend.dado`.

Two deliberate differences from Godot's DisplayServer, both forced by the
substrate:
  * It is single-window. sokol_app is a single-window library.
  * It also owns the frame loop entry, because sokol_app fuses window
    creation with the loop — sapp_run is the only way to open the window.

**`Config`, `Hooks`, `run`, `begin_pass` and `end_pass` are the seam
`app:graphical` drives, and a program should not call them.** Write a program
against `app:app` + `app:graphical`: `graphical.new(w, h, title)` installs a
real `app.Backend` on the singleton and `app.run()` opens the window. The
seam stays *here* rather than moving into `graphical` for one reason: sokol's
window handle, its `sapp_desc` and its `^const sapp_event` are the substrate
this package exists to be the only importer of, and exporting the loop entry
would export them with it. Everything below `run` is ordinary
public surface a program may call from `ready`/`process`.

## Declarations

39 declarations, 39 public.

* `enum WindowMode: (Windowed, Fullscreen)` — Window presentation mode. sokol_app exposes only windowed vs fullscreen;…
* `enum MouseMode: (Visible, Hidden, Captured)` — Pointer mode, after Godot's MouseMode. sokol supports these three; Godot's…
* `enum u32 CursorShape` — Cursor shape — the subset of Godot's CursorShape that sokol_app provides. The…
* `type Config` — How to open the window.
* `type Hooks` — Lifecycle callbacks `run` invokes. Any may be nil.
* `void run(Config cfg, Hooks hooks)` — Open the window described by `cfg`, install `hooks`, and run the frame loop…
* `void begin_pass()` — Begin the default swapchain pass, clearing to the background colour.
* `void end_pass()` — End the pass and present the frame.
* `void set_background(f32 r, g, b, a)` — The colour the frame is cleared to each frame (0..1 per channel).
* `i32 window_get_width()`
* `i32 window_get_height()`
* `(i32, i32) window_get_size()`
* `f32 window_get_dpi_scale()`
* `bool window_is_high_dpi()`
* `void window_set_title(string8 title)`
* `WindowMode window_get_mode()`
* `void window_set_mode(WindowMode mode)`
* `void mouse_set_mode(MouseMode mode)`
* `MouseMode mouse_get_mode()`
* `void cursor_set_shape(CursorShape shape)`
* `const i32 CURSOR_DEFAULT = 0` — The cursors a UI toolkit asks for by name, as plain `i32`s so a widget's…
* `const i32 CURSOR_IBEAM = 1`
* `const i32 CURSOR_POINTER = 2`
* `const i32 CURSOR_RESIZE_EW = 3`
* `const i32 CURSOR_RESIZE_NS = 4`
* `u32 cursor_sapp_value(i32 cursor)` — The `sapp_mouse_cursor` number a `CURSOR_*` names. `CURSOR_DEFAULT`, and…
* `void set_cursor(i32 cursor)` — Show `cursor` (a `CURSOR_*`) over the window. **A no-op with no window…
* `i32 clipboard_capacity()` — How many bytes `clipboard_set` will accept, not counting the terminator.
* `@const i32 CLIPBOARD_BYTES = 1048576` — The clipboard buffer sokol is asked for, in bytes.
* `bool clipboard_accepts(string8 text)` — Whether `clipboard_set` would take `text` whole, the window aside: it is…
* `bool clipboard_set(string8 text)` — Put `text` on the system clipboard. **Answers whether it landed.**
* `cstring clipboard_get()` — Returns a `cstring`, not a `string`, matching core:os.env: a C string cannot…
* `void request_close()` — Ask the window to close; the loop finishes the frame, runs cleanup, `run`
* `void cancel_close()` — Cancel a close that a quit-requested event began.
* `void quit()` — Close now, without the quit-requested handshake.
* `bool valid()` — Whether the display is initialised and the graphics context is valid.
* `bool drawing()` — True only inside the render pass (between `begin_pass` and `end_pass`) — so,…
* `u64 frame_count()`
* `f64 frame_duration()` — Seconds the last frame took (smoothed).
