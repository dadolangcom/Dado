<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:input/sokol

app:input/sokol — the sokol decode: `sapp_event` in, `app:input`'s queue out.

`app:input`'s `Event`, `EventKind`, `Keycode` and `Modifier` were written
against sokol's own numbering before there was a sokol backend, so this is a
*filler* of that shape and not a translation into it. There is no lookup
table anywhere below: `sapp_event.key_code` is already an `input.Keycode`
value, `sapp_event.modifiers` is already an `input.Modifier` bitmask, and
`sapp_event.mouse_button` is already `input.Event.code` for a press. What is
left is the four places sokol's shape and `app:input`'s genuinely differ.
Three of them are one collapse — a direction belongs in `mods`, not in a
kind: sokol's four mouse event types become one `EventKind.Mouse` plus a
flag, its KEY_UP becomes `EventKind.Key` plus a flag, and its
FOCUSED/UNFOCUSED pair becomes `EventKind.Focus` plus a flag.

The fourth runs the other way, and it is the only place this file **reads**
sokol rather than reshaping what it was handed. A FILES_DROPPED carries no
payload in the `sapp_event` at all: the paths sit behind
`sapp_get_num_dropped_files` / `sapp_get_dropped_file_path`, in a buffer
sokol clears at the start of the next drop. So the decode fetches them and
hands them to `input.push_drop`, which copies their bytes out before the
event is queued — `input.dado`'s "Drop" section carries the argument for the
copy, and it is the reason this arm is more than two lines.

**Why this is a subpackage and not a function in `input.dado`.** A `foreign`
block's `#include` is emitted for every importer, reached or not. Putting the
decode in `app:input` would put `sokol_app.h` — and, with `define
"SOKOL_IMPL"` on its block, the whole of sokol_app's implementation — into
the translation unit of every program that imports `app:input`, including
every headless one. `app:input` stays backend-neutral, as its own doc comment
promises; the one package that already drags sokol in (`app:graphical`)
imports this.

## Declarations

5 declarations, 3 public.

* `type EventPtr: ^sokol_app.sapp_event` — The pointer type the raw callback payload is read through. `display`'s event…
* `bool decode(rawptr ev)` — Decode one raw `sapp_event` address and push whatever it means onto…
* `bool decode_event(EventPtr p)` — The decode proper, against a typed `sapp_event`. Separate from `decode` so a…
