# `app:ui` — the outline

**Opened 2026-09-22. Companion to `TEXT_BACKEND.md` and to the framework's work
order, which is kept in the repository and not shipped.**
**Where this file and `dadoc` disagree, `dadoc` is right.**

This is the shape a new agent builds into. It is a **plan, not a record**:
nothing below exists. Names are proposals and the first row that finds a better
one should take it and edit this file.

## Three packages, and why the lines are there

*(Revised 2026-09-22 at CP-1's landing, `0ca6e549`: the outline proposed two
packages with the vocabulary inside `app:ui`. That is a cycle — the renderer
needs `Cell` and the framework calls the renderer — so the vocabulary is its
own package, imported by both.)*

```
collections/app/ui/cell/     package  app:ui/cell   the vocabulary, pure arithmetic
collections/app/ui/grid/     package  app:ui/grid   the renderer
collections/app/ui/          package  app:ui        the framework
```

`app:ui/grid` is the only thing that imports `sokol_gfx`. That is organisation,
**not a seam** — R13 killed the `Painter` vtable and nothing replaces it; the
framework calls the renderer by name. `app:ui/cell` imports nothing but
`core:chars`, so every piece of text arithmetic is testable with no window.
`app:ui` imports `app:graphical` (its `frame.dado` installs the runtime hooks),
so its tests link the windowing libraries even though none opens a window.

## `app:ui` — file by file

| file | holds | CP |
|---|---|---|
| `frame.dado` | `open`/`hook`/`run`: the UI installs its own hooks on `app:graphical`; dispatch, layout, paint, atlas upload and the draw happen without the program. `frame_stats` is the reader. | 1 |
| `ui.dado` | The tree. `Control` handle (slot + generation), the node pool, parent/child/sibling links, lifecycle hooks, signals, the two dirty flags and their two walks. | 1 |
| `layout.dado` | Two-phase layout **in integer cells**: minimums bottom-up, rects top-down, anchors, size flags, sort hooks. Containers live here. | 1 |
| `input.dado` | Cell-indexed hit test (an array read, not a tree walk), focus/hover/press cursors, key routing, bubbling, focus traversal. | 2 |
| `layers.dado` | Layer groups in the tree: a node's layer, what a floating subtree clears when it goes. | 5 |
| `modal.dado` | The modal stack: focus confined, keys swallowed, the mouse outside swallowed and reported, focus restored on close. | 9 |
| `theme.dado` | `StyleBox` (Flat, Texture, Line, Empty), `Theme`, and the **compile** step that resolves a theme once into flat per-type tables (R15). | 5 |
| `region.dado` | Free-pixel regions with cell-aligned bounds, `ImageBox`, `TextureBox`, subgrids and their rational scale. | 7, 10 |
| `export.dado` | The plain-text round trip. Grid-bound content exports; region interiors export as a labelled placeholder; order is **logical, never visual**. | 8 |
| `widgets_*.dado` | One file per family — see below. A widget is a constructor plus a state struct, never a subclass; Dado has no methods and no vtables. | 1+ |

### The widget files

| file | widgets | CP |
|---|---|---|
| `widgets_text.dado` | `Label`, `TextView` (read-only, scrolling, the thing CP-1 needs) | 1 |
| `widgets_edit.dado` | `TextEdit` (editable, multi-line), its style and mark sources | 2, 8, 9 |
| `widgets_line.dado` | `LineEdit`: a `TextEdit` in single-line mode over its own text (the file says why it is not a second widget) | 9 |
| `widgets_button.dado` | `Button`, `IconButton`, `Toggle`, `CheckBox`, `RadioButton`, `OptionButton` | 5 |
| `widgets_box.dado` | `HBox`, `VBox`, `GridBox`, `MarginBox`, `CenterBox`, `HSplitBox`, `VSplitBox`, `ScrollBox`, `Stack` (anchored overlays) | 1, 6, 9 |
| `widgets_panel.dado` | `Panel`, `Popup` (a panel on its own layer), `Dialog` (a modal popup); `SubPanel`, `Window` to come | 5, 9 |
| `widgets_list.dado` | `ItemList`, `Tree`, `Table` | 7 |
| `widgets_meter.dado` | `ProgressBar`, `Spinner` | 5 |

**No fixed-size text caps anywhere.** The prototype's `Span [256]char8`,
`TEXTCAP 256`, `ItemSpan [320]char8`, `MAXITEMS 1000` and `MAXOPTS 16` are all
gone; a widget that holds text holds a `ref []char8`.

## `app:ui/cell` — file by file

| file | holds | CP |
|---|---|---|
| `cell.dado` | `Cell` (slot, flags, fg, bg — 12 bytes, the instance layout), the flag bits including the reserved `COLOR_SLOT`, `CellRect`/`CellPoint`, `LayerGroup`. | 1 |
| `column.dado` | Columns: `cluster_columns_at`, `columns`, `column_of`, `byte_at_column`, `TAB_COLUMNS`. Measurement is a count. | 1 |
| `seam.dado` | The bidi seam, all six items, identity today. | 1 |

## `app:ui/grid` — file by file

| file | holds | CP |
|---|---|---|
| `grid.dado` | The cell buffer, the instance upload, one `sg_draw` per layer group, and the counters R16 ratchets. | 1 |
| `atlas.dado` | One R8 page of uniform slots (more pages later), cluster keying `(face_chain, px, cluster_bytes)`, the face chain, composited combining marks, a pixel-format field reserved for colour. | 1 |
| `grid.glsl` | Vertex: expand an instance index into a cell quad, emit fg, bg and the arithmetic UV. Fragment: sample alpha, mix. | 1 |
| `grid.glsl.h` | Committed `sokol-shdc` output. Regenerated by hand; see the vendored tool's README for the invocation. | 1 |

## The public surface, sketched

Godot-esque outwardly (R4), hiding its machinery (R13). A program should read
like this and never mention a painter, a face, an atlas or a layer:

```dado
import "app:ui"

ui.Control root  = ui.vbox(ui.NIL)
ui.Control head  = ui.label(root, "Aquitaine")
ui.Control body  = ui.text_view(root)
ui.Control save  = ui.button(root, "Save")

ui.set_theme_variation(head, "Heading")
ui.on_pressed(save, on_save, &state)

bool armed = ui.is_pressed(save)
```

Three shapes, and each is a rule:

* **Constructors take a parent and return a handle.** `ui.button(parent, "Save")`.
  A handle is a slot index plus a generation, so one held past `remove` reads as
  invalid rather than reaching a recycled node.
* **Every entry point is total over a bad handle.** A read of a dead handle
  answers as the empty thing; it does not abort and does not corrupt. This is
  lifted verbatim from the archived IDE's document table, where it was a
  policy that held for 1 467 lines.
* **Signals hide the fn-ref and the context pointer.** `ui.on_pressed(node, fn,
  ctx)` is the surface; that Dado has no closures is the language's business.

## Three internal rules, each from something that has already gone wrong here

**Arithmetic is split from state.** Every public function that computes with
layout metrics is a shell over a private helper taking those metrics as
arguments. The archived IDE's viewport did this because **a windowless test has
no face** — `draw.init` aborts inside sokol with no GL context, even under
Xvfb — and it is the concrete technique behind R13's *addressable internals*.
Without it, the only way to test layout is to open a window.

**A cross-package re-exported record loses its slot names** (`ERR0410`). That is
why the archived project grew a `vocab` layer, and it is why `cell.dado` holds
the shared types rather than each package defining its own and re-exporting.

**Two packages declaring a same-named template over one shape collapse into one
emitted function, silently, exit 0** (`tests/cases/pkg_generic_collide`). The
defence is to prefer an alias to a forwarder and to keep generic helpers in one
package. This one does not announce itself; it is found by wondering why the
wrong code ran.

## What `app:ui` may assume about the rest of `app:`

`app:` is an engine (R13). `app:ui` registers itself with `app:graphical`'s
frame loop, allocates per-frame scratch from `core:runtime`'s arena and rolls
back with it, and reads input from `app:input` without a program forwarding
anything. A program's `main` opens a window and builds a tree; it does not wire
a pipeline.

One lifetime trap, paid for once already: **`app:input.dropped_path` lends a
slot in a 16-entry ring.** A client that borrows rather than copies reads a
different drop's path after sixteen pushes. Copy it.
