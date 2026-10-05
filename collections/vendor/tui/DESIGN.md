# `vendor:tui` — the design

> **Status: a proposal.** Nothing here is built. This file is the argument for a
> shape, written before the code so the shape can be argued with cheaply.

`vendor:tui` replaces the tuibox port. It keeps two of that port's ideas — the
diffing cell compositor, and the small honest C floor — and throws the rest
away, because the rest was a 2015-era retained box model designed for a language
with closures, and Dado does not have closures.

The brief: **you should be able to throw together a really nice looking TUI in
seconds.** That is the whole specification. Everything below is downstream of it.

---

## 1. The one decision everything else follows from

### Dado has no closures. Therefore the library is immediate-mode.

Write out what a retained, callback-driven widget costs in this language. There
are no closures, no methods, no UFCS, no overloading, and no nested functions, so
a handler is a package-level function taking an id, and every piece of state it
touches is a package-level variable:

```dado
private i32 name_field
private bool notify_on

private void on_notify(i32 id, i32 x, i32 y, bool down):
    if down: notify_on = !notify_on

i32 main():
    ...
    i32 c = tui.add(4, 3, 20, 1, 0, draw_notify, on_notify, nil)
```

A form with eight fields is eight globals, eight draw functions, eight handlers
and eight ids. That is not "in seconds". It is not even in an afternoon, and it
is what the tuibox port asked for.

Now the same thing immediate-mode:

```dado
tui.checkbox("Notify me", &notify_on)
```

The callback problem evaporates, because there is no callback: the code that
would have gone in the handler is the code on the next line, in a scope that can
see every local it needs. **Immediate mode is not a performance style here, it is
the answer to a language constraint**, and it is the reason a Dado TUI library can
be more ergonomic than a C one rather than less.

Three consequences, stated up front because they are the whole model:

* **The app owns the value; the library owns the interaction.** `&notify_on` is
  yours. Which widget is hot, which is focused, where a scroll pane sits, where
  the caret is in a line edit — ours.
* **A widget call returns `bool`, and it means "activated or changed on this
  frame."** One rule for every widget in the library. `if tui.button("Save"):
  save()`.
* **A frame is cheap and total.** The frame function runs from the top every
  time, paints an 80×24 grid of cells, and the compositor writes only the cells
  that differ. An idle UI runs no frames at all.

### It is a singleton

One terminal per process, and the alternative is a `^Ui` first parameter on every
call in the library — which is precisely the sugar budget spent on nothing. It is
a singleton, it is stated out loud, and `open` refuses a second one.

---

## 2. Hellope

```dado
package demo

import tui "vendor:tui"

private bool notify = true
private i32 hits = 0

private void frame():
    tui.title("Preferences")
    tui.rule()
    tui.checkbox("Notify me", &notify)
    tui.kv("clicks", hits)
    tui.space()
    if tui.button("Press"): hits += 1
    if tui.button("Quit"):  tui.quit()

i32 main():
    if !tui.open(): #panic("not a terminal")
    defer tui.close()
    tui.run(frame)
    return 0
```

Twelve lines, and it is bordered, themed, centred, focusable with Tab, clickable
with the mouse, redraws only what changed, restores the terminal on Ctrl-C and
costs nothing while nobody is typing. That is the bar.

---

## 3. Layout

### The model: nested regions, one pass, no flex solver

A region is a rectangle, an axis, a cursor and a gap. Widgets are *placed* into
the region at the cursor and advance it along the axis. Regions nest.

There is **no two-pass flex layout**, deliberately. A one-pass model is possible
because in a terminal **a widget's size is known before it is placed** — text is
its own measurement — and one pass is what keeps immediate mode honest.

```dado
type Rect: (i32 x, i32 y, i32 w, i32 h)
```

### Sizes

A size is an `i32` in cells, or one of three sentinels:

| written | means |
|---|---|
| `12` | twelve cells |
| `tui.AUTO` | fit the content (the default for a widget's cross axis) |
| `tui.REST` | everything the region has left on this axis |
| `tui.pct(40)` | forty percent of the region's extent on this axis |

`AUTO` is answerable in one pass for everything the library ships, because every
widget can measure itself from its own arguments.

### Opening a region

Dado's indentation only opens after a `:`, so a container cannot be written as a
free-standing indented block. Two spellings, and both are honest:

**The inline form** — an `if` suite, closed by a `defer`:

```dado
if tui.panel("Build", width: 34):
    defer tui.end()
    tui.checkbox("release", &release)
    tui.checkbox("sanitize", &asan)
```

`panel` returns `false` when the region is collapsed, clipped away or off-screen,
in which case the body is skipped entirely and there is nothing to close. This is
Dear ImGui's `if (BeginChild(...))` and it is well-worn. `defer tui.end()` runs on
every exit path the language has, including `return`, `break` and `continue` —
which is exactly the property `defer #delete(x)` already has, so it is a shape
the reader knows.

**The body form** — a `void()` function reference:

```dado
tui.panel("Build", build_body, width: 34)
```

Composes, is reusable, needs no `end`, and reads better once a panel is more than
a handful of lines. Both are shipped. The inline form is the documented default.

### The containers

```dado
tui.column(gap: 0, pad: 0, width: AUTO, height: REST)   // stack downward
tui.row(gap: 1, pad: 0, ...)                            // pack rightward
tui.panel(title: "", ...)                               // a bordered column
tui.scroll(height: REST)                                // a clipped, scrollable column
tui.overlay()                                           // absolutely positioned, above
tui.at(x, y, w, h)                                      // the escape hatch
tui.end()
```

and the fillers:

```dado
tui.space(n = 1)          // n blank cells along the axis
tui.fill()                // take the rest of the axis (a flexible gap)
tui.rule(title = "")      // a themed separator, optionally with an inset caption
```

### Alignment, one-pass

Right- and centre-alignment do not need a second pass, because the widget's size
is known before placement. `align` sets the anchor within the current region and
lasts until changed or until the region closes:

```dado
tui.row()
tui.text("dadoc 0.6 — pangram")
tui.align(tui.RIGHT)
tui.text(status_line)             // placed against the right edge
tui.end()
```

Anchors are `LEFT`, `CENTER`, `RIGHT` on a row and `TOP`, `MIDDLE`, `BOTTOM` on a
column. This is the whole alignment story and it is enough.

### The root

The root region is a **column, padded by one cell, gap zero, filling the
terminal.** That opinion is why the twelve-line program above looks composed
rather than jammed into the corner. `tui.root(pad:, gap:)` changes it.

---

## 4. Identity

Immediate-mode widgets need stable identity across frames to carry hot/active/
focus state, and Dado has no `#LINE` intrinsic to derive one from a call site.

The rule: **a widget's id is a hash of (the enclosing region's id path, the
widget's ordinal within that region, its label).** Stable while the frame code is
stable, which it always is. For loops over dynamic data, where the ordinal is the
only thing distinguishing two rows, push an explicit scope:

```dado
for t, i in tests:
    tui.push_id(i)
    defer tui.pop_id()
    if tui.button(t.name): run_test(t)
```

`tui.push_id` takes an `i32` or a `string`. Nothing else needs to know this
exists; it is documented under "lists of things".

---

## 5. Style

### Colours

```dado
distinct type Color: u32

tui.rgb(137, 180, 250)      tui.hex(0x89B4FA)
tui.ansi(4)                 // a 0..255 palette index
tui.DEFAULT                 // the terminal's own
```

Truecolour is detected once at `open` (`COLORTERM`, then `TERM`), and when it is
absent every colour is **downgraded to the nearest 256-colour index
automatically**. A program never writes that branch. That is what "opinionated"
buys.

### Themes

A theme is an ordinary structural tuple, so a program tweaks one by copying and
overwriting a slot — no builder, no registry:

```dado
type Theme: (
    Color bg, fg, muted, faint,
    Color accent, on_accent,
    Color ok, warn, err, info,
    Color surface, border, border_focus,
    Border border,           // Round | Single | Double | Thick | Ascii | None
    bool  wide_glyphs,       // may the theme use ◆ ● ▁▂▃ etc.
)

const Theme DARK
const Theme LIGHT
const Theme MONO             // no colour at all — for pipes, CI, and `TERM=dumb`

tui.theme(tui.DARK)
```

`DARK` is the default and it is a real palette, not eight ANSI colours. A library
that ships one carefully chosen theme is the difference between "a TUI" and "a
nice TUI", and it is the cheapest quality this project can buy.

### Local style

```dado
tui.push_style(fg: tui.theme_now().err, bold: true)
defer tui.pop_style()
```

with named arguments doing all the work — every parameter is defaulted, so you
write only the slot you mean.

---

## 6. The widget roster

Every one of these returns `bool`, meaning *activated or changed this frame*.
Every one takes named, defaulted arguments beyond the first two or three.

### Static

```dado
tui.text(s)                            // one line, clipped to the region
tui.paragraph(s)                       // word-wrapped to the region width
tui.title(s)                           // themed heading
tui.label(s, color:, dim:)
tui.kv(key, $T value)                  // an aligned "key ......... value" line
tui.badge(s, color:)                   // a pill
tui.rule(title:)
tui.icon(Glyph g)
```

`kv` is generic over the value: `#format(v, tui.mem)` renders anything Dado can
render, so `tui.kv("target", triple)`, `tui.kv("ratio", 0.75)` and
`tui.kv("size", (w, h))` all work with no conversion written. This is the answer
to "there is no string interpolation": you almost never need one.

### Indicators

```dado
tui.progress(f32 v, label:, width:)    // a themed bar
tui.gauge(f32 v, min:, max:)
tui.spinner(label:)                    // animates; registers a repaint timer
tui.sparkline([]f32 xs, height:)
tui.dot(Color c)                       // a status pip
```

### Interactive

```dado
bool tui.button(label, style:, enabled:, hint:)
bool tui.checkbox(label, ^bool on)
bool tui.toggle(label, ^bool on)                        // a switch
bool tui.radio(label, ^i32 choice, i32 value)
bool tui.select(label, ^i32 index, ..string options)    // a popup chooser
bool tui.slider(label, ^f32 v, min:, max:, step:)
bool tui.stepper(label, ^i32 v, min:, max:, step:)
bool tui.line_edit(label, ^ref []char8 buf, placeholder:, secret:, width:)
bool tui.list(^i32 selected, []string items, height:, filter:)
bool tui.tabs(^i32 active, ..string labels)
bool tui.tree(^TreeState st, ...)
```

Two notes worth arguing.

**`select` and `tabs` take a variadic pack**, so the common case reads
`tui.tabs(&page, "Build", "Test", "Constants", "Flags")` with no array bound
first. A pack is a `[]T` window in the callee and allocates nothing — it is a
compound literal in the caller's frame. There is a `_of` sibling
(`tui.tabs_of(&page, labels)`) taking a `[]string` for when the labels are data.

**`line_edit` takes a `^ref []char8`**, which is the language's own growth idiom
(`#append` through a pointer to the handle — the repair ERR0813 names). A
no-allocation program uses `tui.line_edit_fixed(label, []char8 buf, ^i32 len)`
instead. `tui.edited(buf)` lends the current contents as a `string`.

### Queries about the last widget

The uniform `bool` return covers activation. Everything else is a question about
the widget just placed, which keeps the one-liner form intact:

```dado
tui.hovered()   tui.focused()   tui.pressed()   tui.submitted()   tui.rect_of()
```

### Containers with a job

```dado
tui.table(..string headers)            // then rows; cells are ordinary widgets
tui.row_of_table()
tui.log(^LogView v, ..)                // scrollback with follow-tail
tui.modal(title, ^bool open)           // centred, dims the rest, Esc closes
tui.toast(s, kind:)                    // transient, expires on a timer
tui.palette(^PaletteState st, ..string commands)   // a command palette
tui.status(..)                         // the bottom bar
tui.hints()                            // auto-rendered key hints, from the bindings
```

`tui.hints()` is worth a sentence: every `tui.bind` and every widget's `hint:`
argument registers a `(key, description)` pair for the current screen, and
`hints()` renders them along the footer. **A program that binds its keys gets its
own help bar for free**, and it can never drift from the bindings, because it is
generated from them.

---

## 7. Input

### Keys are named, not escape sequences

The tuibox port matched raw byte prefixes: `tui.bind("\x1b[A", up)`. The new
library decodes a real key event and names it.

```dado
bool tui.key(string spec)          // "ctrl+s", "up", "shift+tab", "f5", "q", "esc"
bool tui.chord(string spec)        // a two-key sequence: "g g", "space b"
void tui.bind(string spec, string description, Action f)   // for the hints bar
type Action: void()
```

The spec grammar is one line: zero or more of `ctrl+`, `alt+`, `shift+`, then a
name (`a`, `1`, `enter`, `tab`, `esc`, `space`, `up`, `home`, `pgdn`, `f1`..`f12`,
`bksp`, `del`, `ins`). Parsed at compile time where it can be — a `const` spec
folds to a `KeyEvent` — and at run time otherwise.

Decoding covers: plain bytes, C0 controls, the CSI arrow/edit families in both
the `1;5A` modifier form and the legacy one, SS3 (`\x1bO`), `alt+` as an ESC
prefix, and bracketed paste as a single `Paste` event rather than a hundred
keystrokes. UTF-8 sequences arrive as one `char32`.

### Who gets the key

One rule, stated so it is predictable:

> **A focused text-entry widget claims plain characters and the editing keys.
> Everything else — function keys, `ctrl+`/`alt+` combos, `esc`, `tab` — is
> global and reaches `tui.key`.**

So typing `q` into a filter box does not quit the program, and `ctrl+s` saves
while you are typing. Focus itself is automatic: **Tab and Shift-Tab walk the
focusable widgets in declaration order, Enter/Space activates, arrows navigate
inside a list, Esc leaves the innermost overlay.** No program writes any of that.

### Mouse

```dado
(i32 x, i32 y) tui.mouse()     bool tui.clicked(i32 button = 0)
i32 tui.wheel()                bool tui.dragging()
```

SGR-1006 decode, as in the port. Hit testing is against the region rectangles the
layout already produced, so `hovered()` and `clicked()` on a widget need no
registration.

---

## 8. Rendering

### A cell grid, and a diff — the one thing kept wholesale

```dado
private type Cell: (u32 glyph, Color fg, Color bg, u16 attr, u8 width)
private ref []Cell back, front
```

Widgets write cells. **There is no per-box string cache and no SGR re-parsing.**
The port painted by writing escape codes into a 4 KiB buffer per box and then
parsing them back out to recover the style — clever, and one indirection more
than saying it. A widget here sets `(glyph, fg, bg, attr)` directly, and the only
place an escape code is produced is the compositor.

Consequences: `BOX_CACHE` and `room()` and "a handler cannot overrun its buffer"
all cease to exist as concepts, and so does the 512 KiB of `.bss` they lived in.

### Wide characters are real

A `Cell` carries a width. A double-width glyph occupies its cell and marks the
next one a continuation; a combining mark is width zero and attaches to the cell
before it. A small built-in width table (the East Asian Wide/Fullwidth ranges,
combining marks, and the emoji presentation ranges) is Dado, about a hundred lines
of range checks. The port counted bytes, so a box sized from a CJK string was
sized wrong; a library that claims "nice looking" has to get this right.

### The frame is written to a `Sink`, not to stdout

```dado
void tui.sink(Sink out)          // default: stdout
```

`core:fmt`'s `Sink` is already the right shape. This is a testing requirement, not
a feature: **the entire library can be driven headless**, a whole frame composed
into a `fmt.Cursor` and the bytes asserted, with no TTY, in an ordinary `@test`
file under the existing gate. The port could not be tested at all.

`tui.snapshot(alloc) -> string` renders the current grid as plain text for
golden-file tests.

---

## 9. The loop, and staying alive while something else works

```dado
type Frame: void()

void tui.run(Frame f)                        // the whole loop, until quit()
bool tui.step(Frame f, i32 timeout_ms)       // one iteration; the caller owns the loop
void tui.quit()
void tui.wake()                              // a background thing changed state
bool tui.closed()                            // stdin ended
```

`run` sleeps until something happens. The timeout is computed, not fixed: the
next animation deadline if a spinner or a toast is live, otherwise infinite. **An
idle TUI uses no CPU**, which is not true of a fixed-tick loop and is the reason
`wake()` exists.

`step` is the form the flagship needs, and is the reason `poll(timeout)` existed
in the port: a program that also has to drain a child process, service a socket
or watch a clock must have somewhere to put that work.

```dado
for tui.step(frame, 30):
    drain_build_output()
```

### Storage

Two allocators and one opinion.

* **A frame arena**, exposed as the package-level `tui.mem`, **reset at the top of
  every frame.** Anything a frame builds — a formatted string, a wrapped
  paragraph, a filtered list — is allocated from it and never freed by anybody.
  `tui.text(#format("hits: ", n, tui.mem))` has no cleanup and no leak, and that
  is the sugar that makes formatting inside a frame free to write.
* **A persistent arena** for the cell grids and the widget state cache, sized to
  the terminal and remade on resize.

Both are `core:mem/arena` over an allocator the caller may supply
(`tui.open(alloc: my_alloc)`), defaulting to `#heap`. Unlike the port, there are
no fixed ceilings — the widget count is not bounded by a constant and going past
128 of anything is not a `#panic`.

---

## 10. The C floor

> **Overtaken 2026-09-13: there is no C floor.** `tty.c` and `tty.h` are deleted
> and `tty.dado` is the whole of it. **The facts below are still the facts** — they
> are what makes this package POSIX — and the proposal was right about every one
> of them. What it could not know is that Dado would learn to *say* all of them,
> so the argument for a `.c` expired rather than being refuted. `README.md`'s
> *"The C floor"* section is the current account. The text below is the proposal
> as written.

Unchanged in substance from `tuibox/tty.c`, which is already the right argument:
raw mode needs a `struct termios` whose layout is the platform's, the window size
is a variadic `ioctl`, and a read with a timeout is `select` over an `fd_set`.
Four facts, one `.c`, everything else Dado.

Three changes:

1. ~~**The symbols are renamed `dado_sys_tty_*`.**~~ **The collision is gone by
   construction instead: there is no package `tty`.** A package `tty` with a
   function `rows` emits exactly `dado_tty_rows`, which is what the predecessor
   owned; `tty.dado` declares `package tui` and everything in it is `private` —
   so it emits `static` and has no external symbol at all — and prefixed `tty_`,
   because `tui` has a `rows` of its own about the grid. *(The original text
   pointed at a work order for the record of that collision. **That document is
   gone** — the board it lived on expired and was deleted — which is the
   argument for stating a fact rather than citing where it was filed.)*
2. `core:time` supplies the clock (`time.now`, `time.ms_since`), so no second C
   file appears for animation timing. **Held**, and `core:time` is itself Dado now.
3. The epilogue string gains bracketed-paste-off and, if we enable it,
   kitty-keyboard-off.

---

## 11. The flagship: `dado dashboard`

The library exists to be pointed at, and the thing it will be pointed at is a
build dashboard for `dadoc` itself. It is worth naming now, because it is what
decides which widgets are non-negotiable.

What it needs, and which part of the design pays for it:

| the dashboard wants | the library part |
|---|---|
| tabs across Build / Test / Constants / Flags / Diagnostics | `tabs`, screens |
| a sidebar of packages, a main pane | `row` + `panel(width: 28)` + `panel(width: REST)` |
| `@const` overrides read from `dadoc --emit=constants`, editable in place, typed per kind | `table` of `line_edit` / `checkbox` / `stepper`, keyed by kind |
| C-side selectors: compiler, `-O` level, target triple, sanitizers | `select`, `radio`, `checkbox` |
| the resulting `dadoc` command line, live | `paragraph` over the frame arena |
| a build runner whose output streams in while the UI stays live | `tui.step` + `log` with follow-tail |
| a test list from `--emit=tests`, run individually or all, with pass/fail/timing | `list` with per-row `dot` and `kv`, `push_id` |
| diagnostics with codes, colourised, jump-to-source | `list` + `paragraph` + theme colours |
| a debug runner | the same, with a different command |
| `?` for help, `ctrl+p` for anything | `hints()`, `palette` |

**Two gaps this exposes, and neither is the TUI library's:** `core:os/proc` has
`run` (blocking) and a `popen`-shaped `open`, but nothing that drains a child's
output without blocking, and `dadoc` has no machine-readable form of
`--emit=constants` / `--emit=tests` promised as stable. Both are prerequisites for
the dashboard and neither is a reason to change anything above. They belong on the
plan as their own items.

---

## 12. Shape of the package

A package is a directory, so this is one package in several files:

```
collections/vendor/tui/
    README.md          the argument, as tuibox/README.md is
    tui.dado            lifetime, the loop, the singleton, re-exports
    theme.dado          Color, Theme, DARK/LIGHT/MONO, style stack
    layout.dado         Rect, Region, the stack, sizes, alignment
    cells.dado          Cell, the grids, clipping, the width table
    render.dado         the diff, SGR emission, the Sink
    input.dado          the decoder, key specs, focus, mouse
    widget.dado         static + indicator widgets
    control.dado        button, checkbox, line_edit, list, select, ...
    overlay.dado        modal, toast, palette, hints
    tty.c / tty.h      the four facts        <- DELETED 2026-09-13; it is
                                                tty.dado, and the package has
                                                no C in it at all
    *_test.dado         headless, per module
```

**`ls collections/vendor/tui` is the current shape** — this listing is the
proposal's, and the package grew files it did not predict (`panes.dado`,
`state.dado`) as well as losing the two above. Read the directory, not this block.

## 13. Order of work

1. **Floor and frame.** `tty.c` renamed, `open`/`close`/`run`, the cell grid, the
   diff, the `Sink`, `tui.text`. A hello-world TUI that repaints one character.
   *(Done — and the floor was later rewritten in Dado and the `.c` deleted, which
   is step 9 that nobody wrote down.)*
2. **Layout.** Regions, sizes, alignment, `column`/`row`/`panel`/`space`/`fill`/
   `rule`, the root opinion. Headless layout tests.
3. **Theme.** `Color`, downgrade, `DARK`, borders, the style stack.
4. **Input.** The decoder, key specs, focus ring, mouse, the claiming rule.
5. **Controls.** `button`, `checkbox`, `toggle`, `radio`, `select`, `stepper`,
   `slider`, `line_edit`, `list`, `tabs`.
6. **Containers.** `scroll`, `table`, `log`, `modal`, `toast`, `palette`,
   `status`, `hints`.
7. **Polish.** Width table, truecolour downgrade, resize, `MONO`, snapshot tests,
   a `demo` program exercising every widget on one screen.
8. **Retire the tuibox port.**
9. **The dashboard**, plus its two prerequisites.

Each step ends with something runnable, and steps 1–3 already produce a TUI worth
looking at.
