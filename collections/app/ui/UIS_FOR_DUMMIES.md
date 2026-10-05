# `app:ui` for dummies

**A friendly, ground-up guide to building a window full of widgets with `app:ui`,
by building one small program: a TODO list.**

This is the gentle one, the sibling of `../APP_FOR_DUMMIES.md`. It explains things
slowly, with pictures in words, and assumes you have never built a UI before. The
*why* of every decision — what was measured, what was rejected — lives in the
headers of the files beside this one (`ui.dado`, `frame.dado`, `input.dado`, …),
which are long and opinionated on purpose. Start here; go there when you want
the reasons.

> ### One honest warning, read it once
>
> **This document is hand-written and nothing checks it.** The TODO program in
> §11 was compiled, run under Xvfb, clicked and typed at, and its test in §12
> passed, on 2026-09-24 against `f05cc0f9` — and nothing will notice when that
> stops being true. When this file and the code disagree, **the code is right**,
> and the file headers are maintained beside the thing they describe.

---

## Table of contents

1. [What `app:ui` is](#1-what-app-ui-is)
2. [Five ideas, and then you know the whole thing](#2-five-ideas)
3. [Design first: draw it on graph paper](#3-design-first)
4. [Step 1 — an empty window](#4-step-1)
5. [Step 2 — stacking boxes](#5-step-2)
6. [Step 3 — doing something when Enter is pressed](#6-step-3)
7. [Step 4 — making a widget of your own](#7-step-4)
8. [Step 5 — changing what is on screen](#8-step-5)
9. [Step 6 — the keyboard and focus](#9-step-6)
10. [Step 7 — taking things away](#10-step-7)
11. [The whole program](#11-the-whole-program)
12. [Testing it with no window](#12-testing)
13. [What is in the box](#13-what-is-in-the-box)
14. [The gotcha list](#14-gotchas)
15. [Things to try next](#15-things-to-try)

---

<a id="1-what-app-ui-is"></a>
## 1. What `app:ui` is

**`app:ui` is a UI framework drawn on graph paper.**

Picture a sheet of squared paper. Every square is one **cell**, the size of one
letter in a monospace font. Everything `app:ui` draws — every label, every text
box, every border between panes — sits on those squares. A button is not
"137 pixels wide", it is "9 cells wide". A window 480 pixels across with 8-pixel
letters is 60 cells across.

That one decision makes most of the hard parts of UI go away. Text never needs
measuring in fractions of a pixel: `"Buy milk"` is 8 cells, always. Layout is
whole-number arithmetic. "What did the mouse click on?" is looking up one square
in a table. If you have ever used a terminal program like `htop` or `vim`, you
already know what this looks like — `app:ui` is that, in a real window, with real
borders and shadows drawn around the cells.

It is also a **runtime**, just like `app:graphical` (and built on top of it). You
do not write a frame loop. You hand it a `ready` function that builds your
window, and it does the rest every frame: reads the keyboard and mouse, works out
who they are for, lays everything out, redraws only what changed, and skips
drawing entirely when nothing did.

```
your program                    app:ui does, every frame
─────────────                   ────────────────────────────────────────────
ready():  build the tree   →    read input → route it to widgets → your
                                process() → lay out → paint what is dirty →
handlers: react to events       draw (only if the picture changed)
```

**What you give up:** pixel positioning. If you want a free-form canvas — a game,
a paint program — that is `app:draw`, and `APP_FOR_DUMMIES.md` is its guide. (You
*can* put a pixel area inside a UI — `pixel_box` — but that is a later chapter.)

---

<a id="2-five-ideas"></a>
## 2. Five ideas, and then you know the whole thing

Read these once now. Every step below is one of them in action.

### Idea 1: the window is a tree of `Control`s

Every widget is a **node** in one tree. A node has a parent and children, like a
family tree. A box's children are the things inside the box.

You never hold a node directly. You hold a **`ui.Control`**, which is a *ticket*
for one: a slot number and a generation number. When a node is removed its slot
gets a new generation, so an old ticket simply stops matching. **Using a stale
ticket is always safe** — every `ui.*` call answers "nothing" (an empty rect, an
empty string, `false`) and changes nothing. It never crashes.

`ui.NIL` is the ticket for nobody. Pass it as a parent and you get "the root",
the invisible vertical box that fills the window.

### Idea 2: boxes arrange, and they only speak cells

Containers (`vbox`, `hbox`, `panel`, `scroll_box`, …) do two things, in two passes:

1. **Measure, bottom-up.** Each node says "I need at least this many columns and
   rows." A label saying `Hello` needs 5×1. A vbox needs its widest child's width
   and the sum of its children's heights.
2. **Arrange, top-down.** Each box hands its children real rectangles. Spare room
   goes to children that said they **expand**.

You never compute a position yourself. You say *what goes in what*, and *who
stretches*.

### Idea 3: a widget is a kind plus some state — no classes

Dado has no methods, no subclasses, no virtual functions. So a widget type is a
**`Kind`**: a named bundle of up to seven plain functions ("hooks") that the tree
calls when it needs to:

| hook | the tree is asking… |
|---|---|
| `measure` | how big do you need to be? |
| `arrange` | (containers) where do your children go? |
| `paint` | draw yourself into these cells |
| `input` | here is a key or click — did you use it? |
| `free` | you are being removed — give back your memory |
| `style` | what box (border, fill, shadow) should be drawn around you? |
| `cursor` | what should the mouse pointer look like over you? |

Every hook gets the node's ticket and a pointer to its **state** — a little
record of your own. Any hook can be left out. That's the whole object model.

### Idea 4: two dirty flags, so idle frames cost nothing

`app:ui` does not repaint the window sixty times a second. Each node has two
flags:

- **layout-dirty** — "my size may have changed, measure and arrange me again";
- **paint-dirty** — "my cells are out of date, call my `paint` again".

The built-in setters (`set_label_text`, `line_edit_set_text`, …) set these for
you. In your own widget you set them yourself with `ui.mark_layout(c)` and
`ui.mark_paint(c)`. When nothing is dirty, the frame does no layout, no paint,
and **no draw at all** — the screen just stays as it was.

### Idea 5: events go to someone, and bubble up if they don't want them

- A **key** goes to the node with **focus** (at most one node has it).
- A **mouse** event goes to the node **under the pointer**.

That node's `input` hook answers `true` ("mine, handled") or `false`. On `false`
the event **bubbles** to its parent, then its grandparent, up to the root. If
nobody wants it, it lands in a list your program can read each frame:
`ui.unhandled_count()` / `ui.unhandled_at(i)`. That is where window-wide shortcuts
live.

Widgets that want to tell *you* something — "Enter was pressed in this text box" —
**emit a signal**, and you **connect** a function to it. Signals are how you hear
from built-in widgets; the `input` hook is how you listen in your own.

---

<a id="3-design-first"></a>
## 3. Design first: draw it on graph paper

Before any code, sketch the window in cells. This is the TODO list we'll build,
60 cells wide:

```
┌──────────────────────────────────────────────────────────┐
│ My TODOs                                                 │ ← a label
│ What needs doing?▁                                       │ ← a line edit (type here, Enter adds)
│╭────────────────────────────────────────────────────────╮│
││[x] Buy milk                                            ││ ← one "todo row" per item,
││[ ] Read UIS_FOR_DUMMIES.md                             ││   inside a scrolling list,
││[ ] Walk the dog                                        ││   inside a panel (the border)
││                                                        ││
││                                                        ││ ← this empty part stretches
│╰────────────────────────────────────────────────────────╯│
│ 2 of 3 left    Enter adds · Space ticks · Del removes    │ ← a status label
└──────────────────────────────────────────────────────────┘
```

Now turn the sketch into a tree. Ask of each piece: *what is it inside?* and
*should it stretch when the window grows?*

```
root (vbox, the whole window)
├── label        "My TODOs"              stays 1 row
├── line_edit    the text box            1 row, stretches sideways (by itself)
├── panel        the border              STRETCHES both ways  ← gets all spare rows
│   └── scroll_box                       stretches
│       └── page (vbox)                  stretches
│           ├── list (vbox)              only as tall as its rows
│           │   ├── todo_row "Buy milk"
│           │   └── todo_row …
│           └── filler (empty label)     stretches — soaks up the empty space (§10)
└── label        status line             stays 1 row
```

And decide the interactions up front — what each input does, and who handles it:

| the user… | who gets it | what happens |
|---|---|---|
| types and presses Enter | the line edit | it emits `submitted`; our handler adds a row |
| clicks a row | that row's `input` hook | tick/untick it |
| presses Space on a row | the focused row | tick/untick it |
| presses Delete on a row | the focused row | remove it, move focus to a neighbour |
| presses Up/Down on a row | the focused row | move focus to the row above/below |
| presses Tab | nobody wants it → our `process` | move focus to the next thing |
| turns the wheel | a row ignores it → bubbles → the scroll box | scroll |

There is no built-in "checkbox row" widget, so the rows will be **a widget of our
own** — which is great, because it shows every moving part.

---

<a id="4-step-1"></a>
## 4. Step 1 — an empty window

```dado
package main

import "app:ui"
import "core:fmt"

!void ready():
    return

!i32 main():
    try ui.open(480, 360, "todo")
    ui.hook(ready: ready)
    try ui.run() else code:
        fmt.println("todo: failed with code ", code)
        return 1
    return 0
```

Run it with `./dado run path/to/your/dir`. A blank, dark window.

- **`ui.open(w, h, title)`** makes the window (sizes in ordinary logical pixels;
  from here on, everything is cells).
- **`ui.hook(ready: …, process: …, draw: …, shutdown: …)`** hands over your
  functions. All optional. **Name them**, as `APP_FOR_DUMMIES.md` §2 explains —
  `ready` and `shutdown` have the same type, and a positional call that swaps them
  compiles.
- **`ui.run()`** blocks until the window closes.

Notice what is *not* here: no `app.init`, no grid setup, no render call, no input
polling. `ui.open` installs `app:ui`'s own hooks on the `app:graphical` runtime and
calls yours from inside them.

---

<a id="5-step-2"></a>
## 5. Step 2 — stacking boxes

Every constructor has the same shape: **`ui.thing(parent, …)` returns a
`ui.Control`**. The parent is where it goes; `ui.NIL` means "the root". Children
appear in the order you create them — in a vbox, top to bottom.

```dado
private ui.Control g_entry
private ui.Control g_list
private ui.Control g_status

!void ready():
    _ = ui.label(ui.NIL, " My TODOs")

    g_entry = ui.line_edit(ui.NIL)
    ui.line_edit_set_placeholder(g_entry, "What needs doing?")

    ui.Control frame = ui.panel(ui.NIL)
    ui.set_expand(frame, true, true)
    ui.Control scroll = ui.scroll_box(frame)
    ui.set_expand(scroll, true, true)
    ui.Control page = ui.vbox(scroll)
    ui.set_expand(page, true, true)
    g_list = ui.vbox(page)
    ui.set_expand(g_list, true, false)
    ui.Control filler = ui.label(page, "")
    ui.set_expand(filler, true, true)

    g_status = ui.label(ui.NIL, "")
    ui.label_set_status(g_status, true)
    return
```

That is the tree from §3, line for line.

### `set_expand(c, horizontally, vertically)` — who gets the spare room

The window is 360 pixels tall — say 22 rows. The two labels and the line edit
need one row each. Who gets the other 19? **Whoever expands vertically.** Only
the panel does, so the panel gets them all, and the status label is pushed to the
very bottom. Take that one `set_expand` away and the panel shrinks to its
minimum, and the status line floats up under it.

If several children expand, they share the spare room.

Horizontally, a vbox child that expands gets the full width; one that doesn't
gets its minimum width. (A `line_edit` already expands sideways by itself.)

### Why keep some tickets in globals?

`g_entry`, `g_list`, `g_status` are nodes we will need later — to read the text
box, add rows to the list, and update the status. Nodes you never touch again
(the title, the panel, the filler) can be forgotten; the tree owns them.

### Where did the border come from?

A `panel` draws a **style box** — fill, border, rounded corners, maybe a shadow —
from the **theme**. Style boxes are drawn in real pixels *around* the cells (the
"chrome"), which is how a cell-grid UI gets smooth borders. The panel lights its
border up while focus is inside it. You get all of that by writing `ui.panel`.

---

<a id="6-step-3"></a>
## 6. Step 3 — doing something when Enter is pressed

A line edit already does typing, the caret, selection, copy/paste and undo. What
it can't know is what *you* want Enter to mean. So it **emits** the `submitted`
signal, and we **connect** to it:

```dado
private void on_add(ui.Control entry, rawptr ctx):
    _ = ctx
    string text = ui.line_edit_text(entry)
    if #len(text) == 0:
        return
    _ = todo_row(g_list, text)            // §7 — make a new row in the list
    ui.line_edit_set_text(entry, "")      // empty the box for the next one
    refresh_status()

// in ready():
    ui.on_submitted(g_entry, on_add, nil)
```

A signal handler is always `void f(ui.Control who, rawptr ctx)`:

- **`who`** is the node that emitted — the line edit here.
- **`ctx`** is whatever pointer you passed when connecting (`nil` here). Dado has
  no closures, so this is how a handler finds "its" data: pass `rawptr(&my_state)`
  when connecting and cast it back inside. This program uses globals instead,
  which is fine for one window.

Other widgets have other signals: `on_cancelled` (Escape in a line edit),
`on_tab_selected`, `on_tree_activated`, `on_split_moved`, … They all look like
this.

### ⚠ Handlers run inside the frame — mind your memory

Signal handlers and `input` hooks run while `app:ui` is handling this frame's
input, which is inside the runtime's `process` step. There, as
`APP_FOR_DUMMIES.md` §4 explains, **the ambient allocator is the per-frame
arena, emptied at the end of the frame.**

- Built-in widgets **copy** what you give them into memory of their own.
  `set_label_text(c, s)` keeps its own copy of `s`, so a string you format
  just for the call is fine to throw away:

  ```dado
  ref []char8 msg = #format(" ", left, " of ", total, " left", #default)
  ui.set_label_text(g_status, string(msg[:]))       // label copies it; msg can die
  ```

- Anything **you** keep past this frame must be made in the persistent
  allocator — the row's text in §7 is the example:

  ```dado
  using app.allocator():
      s.text = #make([]char8, #len(text))
  ```

Forget this and the row's text quietly turns into garbage a frame later.

---

<a id="7-step-4"></a>
## 7. Step 4 — making a widget of your own

A todo row shows `[ ] Buy milk`, ticks when clicked, and highlights when focused.
Nothing built-in does that, so we make a **kind**. Four pieces.

### 7.1 The state: what one row remembers

```dado
private type RowState: (ref []char8 text, bool done)
private type RowPtr: ^RowState
```

Every hook receives the state as a bare `rawptr`; `RowPtr(st)` turns it back into
a pointer to our record.

### 7.2 `measure`: how big am I?

```dado
private cell.CellPoint row_measure(ui.Control c, rawptr st):
    _ = c
    cell.CellPoint p = (4 + cell.columns(row_text(RowPtr(st))), 1)
    return p
```

Four cells for `[ ] `, plus the text's width, by one row. `cell.columns` counts
**columns**, not bytes — a Chinese character is two columns wide, an accent
combined onto a letter is zero — so use it rather than `#len`. The answer is
`(cols, rows)`, stored in a `CellPoint`'s `(col, row)`.

This is a **minimum**. Because the row will be set to expand sideways, the list
will actually give it the full width.

### 7.3 `paint`: draw me

```dado
private void row_paint(ui.Control c, rawptr st, cell.CellRect clip):
    ^RowState s = RowPtr(st)
    [4]u8 fg = ui.TEXT_FG
    [4]u8 bg = ui.theme_label().bg
    if ui.focused() == c:
        bg = ui.TEXT_SEL_BG               // highlighted while focused
    if s.done:
        fg = ui.GUTTER_FG                 // greyed out when ticked
    cell.CellRect r = ui.rect(c)
    grid.clip(clip)
    grid.fill(clip, fg, bg)               // every cell, blank, in our colours
    string box = "[ ] "
    if s.done:
        box = "[x] "
    _ = grid.text(r.col, r.row, box, r.cols, fg, bg)
    _ = grid.text(r.col + 4, r.row, row_text(s), r.cols - 4, fg, bg)
    grid.unclip()
```

Painting is **writing letters into squares** with `app:ui/grid`:

- `grid.fill(rect, fg, bg)` — blank a rectangle of cells;
- `grid.text(col, row, s, max_cols, fg, bg)` — write a string starting at a cell,
  at most `max_cols` columns of it;
- `grid.clip(r)` / `grid.unclip()` — nothing outside `r` gets written.

Two rectangles are in play. `ui.rect(c)` is **where the node is**; `clip` is **the
part of it you may draw** (it's the node's rect cut down by every ancestor — a
row half-scrolled out of the scroll box is clipped). Lay text out from `rect`,
fence it with `clip`.

**⚠ The rule of paint: you own every cell of `clip`.** A cell you don't write
keeps whatever was there before — maybe a row that used to be here. That's why
`fill` comes first.

**Never change the tree from `paint` or `measure`.** While layout and paint run,
the tree is frozen: `create`, `remove`, `set_label_text` and friends *silently do
nothing*. Paint reads; it doesn't write.

### 7.4 `input`: something happened to me

```dado
private bool row_input(ui.Control c, rawptr st, input.Event e):
    _ = st
    if input.mouse_kind(e) == input.MouseKind.Press && input.mouse_button(e) == 0:
        row_toggle(c)                     // left click
        return true
    if !input.is_key_press(e):
        return false                      // releases, motion, the wheel: not mine
    if e.code == 32:                      // Space
        row_toggle(c)
        return true
    if e.code == i32(input.Keycode.Delete) || e.code == i32(input.Keycode.Backspace):
        row_delete(c)
        return true
    // Up and Down: §9
    return false
```

`true` means "handled, stop here". `false` means "not mine" and the event bubbles
up to the list, the scroll box, and so on. **Returning `false` for the wheel is
what makes scrolling work**: the row shrugs, the scroll box catches it.

Two things about the event:

- **`e.code` for a key is the key, not the letter typed.** Letters are their
  *uppercase* ASCII value whether or not Shift is held; Space is 32; the rest are
  `input.Keycode.*`. For "what character did they type", listen for
  `input.EventKind.Text` events instead — that's what a line edit does.
- **A press on a focusable node focuses it first**, then the hook sees it. So
  clicking a row also selects it. Free.

### 7.5 `free`, and gluing it together

```dado
private void row_free(ui.Control c, rawptr st):
    _ = c
    ^RowState s = RowPtr(st)
    if s.text != nil:
        #delete(s.text)
        s.text = nil

private ui.Kind g_row_kind = ui.NO_KIND

private ui.Kind row_kind():
    if g_row_kind == ui.NO_KIND:
        ui.Hooks h
        h.measure = row_measure
        h.paint = row_paint
        h.input = row_input
        h.free = row_free
        g_row_kind = ui.register_kind("todo_row", h)
    return g_row_kind
```

`register_kind` once, lazily, the first time a row is made. Hooks left out
(`arrange`, `style`, `cursor`) are nil and mean "the default".

### 7.6 The constructor

Make it look exactly like the built-in ones — parent in, ticket out:

```dado
private ui.Control todo_row(ui.Control parent, string text):
    ui.Control c = ui.create_owned(row_kind(), parent, i64(#size(RowState)))
    if !ui.valid(c):
        return ui.NIL
    ^RowState s = RowPtr(ui.state(c))
    using app.allocator():                          // we keep this: persistent
        s.text = #make([]char8, #len(text))
    for i in 0..<#len(text):
        s.text[i] = text[i]
    ui.set_focusable(c, true)                       // can hold focus → gets keys
    ui.set_focus_repaint(c, true)                   // repaint me when focus moves (§8)
    ui.set_expand(c, true, false)                   // full width, one row
    return c
```

**`create_owned`** makes the node *and* a zeroed state block of that many bytes,
which the tree frees for you after your `free` hook runs. (Plain `create` takes a
state pointer you manage yourself.) We copy the text because the caller's string
— the line edit's contents — is about to be cleared.

---

<a id="8-step-5"></a>
## 8. Step 5 — changing what is on screen

When a row is ticked, its state changes — but the tree has no idea. You tell it:

```dado
private void row_toggle(ui.Control c):
    ^RowState s = RowPtr(ui.state(c))
    s.done = !s.done
    ui.mark_paint(c)          // "my cells are stale"
    refresh_status()
```

Which mark?

| what changed | call | what happens next frame |
|---|---|---|
| how it **looks**, not how big it is | `ui.mark_paint(c)` | `paint` is called again for `c` (and its children) |
| its **size** might have changed | `ui.mark_layout(c)` | measure and arrange again, then paint |

Ticking changes colours, not size → `mark_paint`. (If the row's text could be
edited, that would change its width → `mark_layout`.) The built-in setters mark
for you: `set_label_text` marks layout, which is why `refresh_status` just calls
it.

Forget to mark, and nothing is wrong *yet* — the screen just doesn't update until
something else happens to repaint that spot. That "it only shows up when I move
the mouse" bug is always a missing mark.

### Focus changes mark nothing

Moving focus is common and usually changes no cells, so it doesn't mark anything
by default. Our rows *do* look different when focused, so each one asks:
`ui.set_focus_repaint(c, true)` — "repaint me whenever focus moves anywhere".
(Panels don't need this; their focus border is chrome, which is rebuilt
separately.)

### ⚠ Don't set things every frame

It is tempting to write, in `process`:

```dado
ui.set_label_text(g_status, …)        // WRONG in process: every frame
```

That marks the label dirty every frame, so every frame repaints and **draws**,
and the "do nothing when idle" machinery never gets to do nothing. Update things
**when they change** — from the handler that changed them. That's why
`refresh_status()` is called from `on_add`, `row_toggle` and `row_delete`, and
never from `process`.

---

<a id="9-step-6"></a>
## 9. Step 6 — the keyboard and focus

Keys go to the focused node. So "where the keyboard is" is the same question as
"which node has focus".

- `ui.set_focus(c)` — give focus to `c`.
- `ui.focused()` — who has it.
- `ui.focus_next()` / `ui.focus_prev()` — move to the next/previous **focusable,
  visible** node in tree order.
- A left click on a focusable node focuses it.

In `ready`, put the cursor in the text box so the user can type straight away:

```dado
    ui.set_focus(g_entry)
```

### Up and Down between rows

Each row moves focus to its neighbour. The list is just the rows' parent, so
"the row above" is `prev_sibling`:

```dado
    if e.code == i32(input.Keycode.Up):
        ui.Control prev = ui.prev_sibling(c)
        if !ui.valid(prev):
            prev = g_entry                 // above the first row: back to the text box
        ui.set_focus(prev)
        return true
    if e.code == i32(input.Keycode.Down):
        ui.Control next = ui.next_sibling(c)
        if ui.valid(next):
            ui.set_focus(next)
        return true
```

### Tab is yours to decide

Outside a dialog, **`app:ui` does not move focus on Tab** — Tab might mean
"indent" to a text editor. A line edit leaves Tab alone, a row returns `false`,
and it bubbles up to nobody. So it arrives in your `process` as **unhandled**:

```dado
!void process(f32 dt):
    _ = dt
    for i in 0..<ui.unhandled_count():
        input.Event e = ui.unhandled_at(i)
        if !input.is_key_press(e) || e.code != i32(input.Keycode.Tab):
            continue
        if (e.mods & u32(input.Modifier.Shift)) != 0:
            _ = ui.focus_prev()
        else:
            _ = ui.focus_next()
    return
```

This is the pattern for every window-wide shortcut — Ctrl+S, Ctrl+Q: let the
widgets have first go, and handle what they leave. (Inside a `dialog`, Tab and
Shift+Tab *are* handled for you, and focus can't escape the dialog.)

The order each frame is: events are delivered (handlers and `input` hooks run),
**then** your `process` runs, **then** layout and paint. So anything your
`process` changes is on screen the same frame.

---

<a id="10-step-7"></a>
## 10. Step 7 — taking things away

```dado
private void row_delete(ui.Control c):
    ui.Control next = ui.next_sibling(c)
    if !ui.valid(next):
        next = ui.prev_sibling(c)
    if !ui.valid(next):
        next = g_entry
    ui.set_focus(next)                     // hand focus on BEFORE removing
    ui.remove(c)
    refresh_status()
```

- **`ui.remove(c)`** removes `c` and everything under it. Each node's `free` hook
  runs (children first), its connections are dropped, and its slot is recycled.
- **Afterwards `c` is a dead ticket.** `ui.valid(c)` is `false`; every call on it
  quietly does nothing. You can't hurt anything with it, but you can confuse
  yourself — so drop tickets you've removed.
- **Pick the next focus yourself**, while the neighbours are easy to find.
  Otherwise focus just vanishes and the keyboard goes nowhere.
- Removing a node from inside its own `input` hook, as here, is allowed.

### ⚠ Who repaints the hole?

When a row goes, the rows below slide up, and the list is one row shorter. The
cells at the bottom that the last row used to cover belong to… the list's
parent. **A plain `vbox` has no `paint` of its own** — it only arranges — so if
nothing else covers those cells, they keep showing the row that used to be there.
A ghost.

That's why the tree in §3 puts the list inside a `page` vbox next to an expanding
empty label, the **filler**. The filler covers every row the list doesn't; when
the list shrinks, the filler grows, repaints itself, and wipes the ghost. The
general rule: **make sure every area of the window belongs to something that
paints** — a label, a panel, a widget of your own.

(And to hide something rather than delete it: `ui.set_visible(c, false)`. Cheaper,
and reversible.)

---

<a id="11-the-whole-program"></a>
## 11. The whole program

Put this in a directory of its own as `main.dado` and `./dado run` the directory.

```dado
// todo — the example program from collections/app/ui/UIS_FOR_DUMMIES.md.
package main

import app "app:graphical"
import "app:input"
import "app:ui"
import "app:ui/cell"
import "app:ui/grid"
import "core:fmt"

// ── The todo row: a widget of our own ──────────────────────────────────────

private type RowState: (ref []char8 text, bool done)
private type RowPtr: ^RowState

private ui.Kind g_row_kind = ui.NO_KIND

private string row_text(^RowState s):
    if s.text == nil:
        return ""
    return string(s.text[:])

// How big am I? "[x] " plus the text, one row high.
private cell.CellPoint row_measure(ui.Control c, rawptr st):
    _ = c
    cell.CellPoint p = (4 + cell.columns(row_text(RowPtr(st))), 1)
    return p

// Draw me. I own every cell of `clip`.
private void row_paint(ui.Control c, rawptr st, cell.CellRect clip):
    ^RowState s = RowPtr(st)
    [4]u8 fg = ui.TEXT_FG
    [4]u8 bg = ui.theme_label().bg
    if ui.focused() == c:
        bg = ui.TEXT_SEL_BG
    if s.done:
        fg = ui.GUTTER_FG
    cell.CellRect r = ui.rect(c)
    grid.clip(clip)
    grid.fill(clip, fg, bg)
    string box = "[ ] "
    if s.done:
        box = "[x] "
    _ = grid.text(r.col, r.row, box, r.cols, fg, bg)
    _ = grid.text(r.col + 4, r.row, row_text(s), r.cols - 4, fg, bg)
    grid.unclip()

private void row_toggle(ui.Control c):
    ^RowState s = RowPtr(ui.state(c))
    s.done = !s.done
    ui.mark_paint(c)
    refresh_status()

private void row_delete(ui.Control c):
    ui.Control next = ui.next_sibling(c)
    if !ui.valid(next):
        next = ui.prev_sibling(c)
    if !ui.valid(next):
        next = g_entry
    ui.set_focus(next)
    ui.remove(c)
    refresh_status()

// Something happened to me. Answer true if I dealt with it.
private bool row_input(ui.Control c, rawptr st, input.Event e):
    _ = st
    if input.mouse_kind(e) == input.MouseKind.Press && input.mouse_button(e) == 0:
        row_toggle(c)
        return true
    if !input.is_key_press(e):
        return false
    if e.code == 32:
        row_toggle(c)
        return true
    if e.code == i32(input.Keycode.Delete) || e.code == i32(input.Keycode.Backspace):
        row_delete(c)
        return true
    if e.code == i32(input.Keycode.Up):
        ui.Control prev = ui.prev_sibling(c)
        if !ui.valid(prev):
            prev = g_entry
        ui.set_focus(prev)
        return true
    if e.code == i32(input.Keycode.Down):
        ui.Control next = ui.next_sibling(c)
        if ui.valid(next):
            ui.set_focus(next)
        return true
    return false

// I am being removed: give back the memory I own.
private void row_free(ui.Control c, rawptr st):
    _ = c
    ^RowState s = RowPtr(st)
    if s.text != nil:
        #delete(s.text)
        s.text = nil

private ui.Kind row_kind():
    if g_row_kind == ui.NO_KIND:
        ui.Hooks h
        h.measure = row_measure
        h.paint = row_paint
        h.input = row_input
        h.free = row_free
        g_row_kind = ui.register_kind("todo_row", h)
    return g_row_kind

private ui.Control todo_row(ui.Control parent, string text):
    ui.Control c = ui.create_owned(row_kind(), parent, i64(#size(RowState)))
    if !ui.valid(c):
        return ui.NIL
    ^RowState s = RowPtr(ui.state(c))
    using app.allocator():
        s.text = #make([]char8, #len(text))
    for i in 0..<#len(text):
        s.text[i] = text[i]
    ui.set_focusable(c, true)
    ui.set_focus_repaint(c, true)
    ui.set_expand(c, true, false)
    return c

// ── The window ─────────────────────────────────────────────────────────────

private ui.Control g_entry
private ui.Control g_list
private ui.Control g_status

private void refresh_status():
    i32 left = 0
    i32 total = 0
    ui.Control c = ui.first_child(g_list)
    for ui.valid(c):
        total = total + 1
        if !RowPtr(ui.state(c)).done:
            left = left + 1
        c = ui.next_sibling(c)
    ref []char8 msg = #format(" ", left, " of ", total, " left    Enter adds · Space ticks · Del removes", #default)
    ui.set_label_text(g_status, string(msg[:]))

private void on_add(ui.Control entry, rawptr ctx):
    _ = ctx
    string text = ui.line_edit_text(entry)
    if #len(text) == 0:
        return
    _ = todo_row(g_list, text)
    ui.line_edit_set_text(entry, "")
    refresh_status()

!void ready():
    ui.Control title = ui.label(ui.NIL, " My TODOs")
    _ = title
    g_entry = ui.line_edit(ui.NIL)
    ui.line_edit_set_placeholder(g_entry, "What needs doing?")
    ui.on_submitted(g_entry, on_add, nil)

    ui.Control frame = ui.panel(ui.NIL)
    ui.set_expand(frame, true, true)
    ui.Control scroll = ui.scroll_box(frame)
    ui.set_expand(scroll, true, true)
    ui.Control page = ui.vbox(scroll)
    ui.set_expand(page, true, true)
    g_list = ui.vbox(page)
    ui.set_expand(g_list, true, false)
    ui.Control filler = ui.label(page, "")
    ui.set_expand(filler, true, true)

    g_status = ui.label(ui.NIL, "")
    ui.label_set_status(g_status, true)

    _ = todo_row(g_list, "Buy milk")
    _ = todo_row(g_list, "Read UIS_FOR_DUMMIES.md")
    refresh_status()
    ui.set_focus(g_entry)
    return

!void process(f32 dt):
    _ = dt
    for i in 0..<ui.unhandled_count():
        input.Event e = ui.unhandled_at(i)
        if !input.is_key_press(e) || e.code != i32(input.Keycode.Tab):
            continue
        if (e.mods & u32(input.Modifier.Shift)) != 0:
            _ = ui.focus_prev()
        else:
            _ = ui.focus_next()
    return

!i32 main():
    try ui.open(480, 360, "todo")
    ui.hook(ready: ready, process: process)
    try ui.run() else code:
        fmt.println("todo: failed with code ", code)
        return 1
    return 0
```

Try it: type, Enter, Tab onto the first row, Space, Down, Delete, click a row.

---

<a id="12-testing"></a>
## 12. Testing it with no window

The tree doesn't need a window. `ui.update(screen)` runs layout and paint on a
screen rectangle of your choosing; `ui.dispatch(event)` delivers one event
exactly as the frame would. With no grid up, **a pixel counts as a cell**, so a
mouse event at `(1.5, 0.5)` is a click in cell `(1, 0)`.

Next to `main.dado`, a `todo_test.dado`:

```dado
@test
// The todo list with no window: build the tree, lay it out, click a row.
package main

import "app:input"
import "app:ui"
import "app:ui/cell"

i32 test():
    bool ok = true
    g_list = ui.vbox(ui.NIL)
    g_status = ui.label(ui.NIL, "")
    ui.Control milk = todo_row(g_list, "Buy milk")
    ui.update(cell.rect(0, 0, 40, 10))
    ok = #expect(ui.minimum(milk).col == 12, "four for the box, eight for the text") && ok

    // No grid, so a pixel is a cell: this is a left press on cell (1, 0).
    input.Event click = (input.EventKind.Mouse, 0, f32(1.5), f32(0.5), u32(input.Modifier.Lmb))
    ok = #expect(ui.dispatch(click), "the row took the click") && ok
    ok = #expect(RowPtr(ui.state(milk)).done, "and ticked itself") && ok
    ok = #expect(ui.focused() == milk, "a click focuses a focusable node") && ok
    ok = #expect(ui.is_paint_dirty(milk), "waiting to be repainted") && ok
    ok = #expect(ui.label_text(g_status) == " 0 of 1 left    Enter adds · Space ticks · Del removes", "status: ", ui.label_text(g_status)) && ok

    ui.reset()
    if ok: return 0
    return 1
```

`./dado test path/to/your/dir`. Two things to notice:

- **`update` before `dispatch`.** The mouse is routed by the map of who painted
  which cell; until something has painted, every cell belongs to nobody.
- **`ui.reset()`** at the end empties the tree, so the next test starts clean.

The package's own `*_test.dado` files beside this one do this at scale — they are
the best examples of every widget there is.

---

<a id="13-what-is-in-the-box"></a>
## 13. What is in the box

Every constructor is `ui.name(parent, …) → ui.Control`.

**Containers**

| constructor | lays its children out… |
|---|---|
| `vbox` | top to bottom |
| `hbox` | left to right |
| `grid_box(parent, columns)` | in a table, row by row |
| `margin_box(parent, l, t, r, b)` | inset by some cells |
| `center_box` | centred |
| `scroll_box` | its first child, scrolled vertically (wheel, PageUp/Down) |
| `hsplit_box` / `vsplit_box` | two panes and a draggable handle |
| `stack` + `set_anchor` | on top of each other — pinned to a corner or edge |
| `panel` | inside a themed border |
| `popup` | a panel floating on its own layer, above its siblings |
| `dialog` | a popup that is modal while open (`dialog_open` / `dialog_close`) |

**Widgets**

| constructor | what it is |
|---|---|
| `label(parent, text)` | one line of text (`label_set_status` for a status bar look) |
| `line_edit` | a one-line text box: Enter → `submitted`, Escape → `cancelled` |
| `text_edit` | a multi-line editor over a document you provide (a "source") |
| `text_view` | a read-only scrolling view of lines you provide |
| `tree_view` | a file-tree-style list over a source; `tree_set_flat` for a plain list |
| `tab_bar` | a row of tabs |
| `pixel_box` | a rectangle of cells you draw pixels into yourself |

**Not there yet:** buttons, checkboxes, radio buttons, progress bars
(`OUTLINE.md` plans them). Until then, a button is a widget of your own, exactly
like the todo row: a label-ish `paint` and an `input` hook that calls
`ui.emit(c, ui.SIG_PRESSED)`, which anyone can then `ui.on_pressed` to.

**The look** comes from the theme: `ui.theme_default()` returns a description you
can change and install with `ui.theme_set(&d)`; the `ui.theme_*()` readers (as
`row_paint` uses `theme_label()`) answer the current one.

---

<a id="14-gotchas"></a>
## 14. The gotcha list

1. **Name your hook arguments.** `ui.hook(ready: ready, process: process)`, never
   positional.
2. **Handlers run in the per-frame arena.** What you keep, allocate under
   `using app.allocator():`. What you hand to a built-in setter, it copies.
3. **Paint owns every cell of its clip.** Fill first, then write.
4. **The tree is frozen during layout and paint.** Setters called from `measure`
   or `paint` silently do nothing.
5. **Changed your own widget's state? Mark it.** `mark_paint` for looks,
   `mark_layout` for size. Missing marks show up as "only updates when I move
   the mouse".
6. **Don't call setters every frame.** Update things when they change, or the
   window never gets to sit idle.
7. **Looks depend on focus? `set_focus_repaint(c, true)`.** Focus moves mark
   nothing by themselves.
8. **Tab isn't automatic** outside a dialog. Handle it from `unhandled_*`.
9. **Every area needs a painter.** A box that shrinks away from cells leaves
   ghosts unless something that paints covers them — the filler trick.
10. **Hand focus on before `remove`.** And forget the ticket after.
11. **Key codes are keys, not letters.** Uppercase ASCII for letters, 32 for
    Space, `input.Keycode.*` for the rest. Typed characters are `Text` events.
12. **A dead ticket is harmless but silent.** When "nothing happens", check
    `ui.valid(c)`.
13. **The scroll box doesn't follow focus.** Down-arrowing past the bottom row
    moves focus off screen. See the first exercise.

---

<a id="15-things-to-try"></a>
## 15. Things to try next

In roughly increasing order:

1. **Keep the focused row in view.** After `set_focus` in `row_input`, compare
   `ui.rect(next)` with the scroll box's `ui.rect` and call
   `ui.scroll_box_scroll_by(scroll, ±1)`.
2. **A "clear done" key.** In `process`, on an unhandled Ctrl+D, walk `g_list`'s
   children and `remove` the ticked ones. (Read `next_sibling` *before* removing.)
3. **A real button.** Make a `button` kind that paints `[ Clear done ]`, emits
   `ui.SIG_PRESSED` on click and on Space/Enter, and put it in an `hbox` with the
   line edit.
4. **Edit a row.** On Enter over a row, put its text in the line edit and
   remember which row; on `submitted`, update that row instead of adding one — and
   this time it's `mark_layout`, because the width changed.
5. **A confirm dialog** before deleting: `ui.dialog`, a label and your button
   inside, `dialog_open` on Delete, `dialog_close` on the answer.
6. **Save and load** the list to a file in `ready` / `shutdown`.

When you want to see how a big program is put together, `ide/` at the root of
this tree is an editor built on exactly these pieces: `ide/view.dado` builds its
tree, `ide/find.dado` wires a line edit to a search, and `ide/palette.dado` is a
dialog with a filtered list.
