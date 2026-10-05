# `vendor:tui` — a terminal UI you can throw together in seconds

```dado
import tui "vendor:tui"

private bool notify = true
private i32 hits = 0

private void frame():
    tui.title("Preferences")
    tui.checkbox("Notify me", &notify)
    tui.kv("clicks", hits)
    if tui.button("Press"): hits = hits + 1
    if tui.button("Quit"):  tui.quit()

i32 main():
    if !tui.open(): #panic("not a terminal")
    defer tui.close()
    tui.run(frame)
    return 0
```

Twelve lines, and it is themed, bordered, focusable with Tab, clickable with the
mouse, redraws only the cells that changed, restores the terminal on Ctrl-C and
costs no CPU while nobody is typing.

`DESIGN.md` is the argument for the shape. This is what the shape turned out to
be. **Where the two disagree, this file is the newer one and the compiler is
still right about both.**

---

## It is immediate mode, because Dado has no closures

That is the whole design, and everything else follows from it.

A retained, callback-driven widget in this language costs a package-level
handler function *and* a package-level variable *and* an id, per widget —
because there are no closures, no methods, no overloading and no nested
functions. A form with eight fields is eight of each. That is what the
predecessor asked for, and it is why nothing was ever built on it twice.

Immediate mode makes the callback problem vanish: the code that would have gone
in the handler is the code on the next line, in a scope that can see every local
it needs.

Three consequences, and they are the model:

* **You own the value; the library owns the interaction.** `&notify` is yours.
  Which widget is hot, which has focus, where a pane is scrolled, where a caret
  sits — ours, keyed by an id derived from where the widget was called.
* **A widget call answers `bool`, and it means "activated or changed on this
  frame."** One rule, every widget. Anything else is a question about the widget
  just placed: `hovered()`, `focused()`, `rect_of()`.
* **A frame is total and cheap.** The frame function runs from the top every
  time; the compositor writes only the cells that differ. An idle UI runs no
  frames at all.

### Backgrounds nest

A region that paints a background says so with `region_bg`, and `panel`,
`table_row` and the rest already do. Widgets draw on **the innermost enclosing
background**, found by a walk — not on "the panel's surface", because
backgrounds nest: a table row inside a panel inside the window. A widget that
guesses paints over the row it is in, and a zebra stripe with holes in it
exactly where the text is is the artefact a cell compositor is supposed to make
impossible.

It is a **singleton** — one terminal per process. The alternative is a `^Ui`
first parameter on every call, which is the entire sugar budget spent on
nothing.

---

## Two things the language asks of you

**A call is all positional or all named.** So `tui.panel("Build", width: 34)` is
refused; write `tui.panel(title: "Build", width: 34)`. Named calls may omit
*any* defaulted parameter, not merely a trailing run, which is what makes the
signatures wide and the calls short.

**A generic value cannot be an untyped literal.** `tui.kv("n", 41)` is refused,
because a literal supplies nothing to infer `$T` from. Bind it first, or write
`i32(41)`. Values in a real program are variables, so this rarely bites.

---

## Containers, and the one sharp edge

Every container answers `bool`, which buys two spellings.

**The `for` form** is the default. `for cond:` is Dado's only `while` and it
re-evaluates after each pass, so the first call opens the region and the second
recognises its own region on top of the stack and closes it:

```dado
for tui.panel(title: "Settings", width: 34):
    tui.checkbox("release", &release)
```

Nesting indents, because every container is an `if`/`for` with a suite.
`continue` is safe — it *is* the close.

> **A container may not be a direct child of a container with the same tag, and
> `break` inside one skips its close.**

The two calls are indistinguishable to the library: an opening call for a
same-tagged nested container sees exactly the state a closing call sees, and
there is no call-site information in the language to separate them. `panel` uses
its title, so two panels collide only when they share one. `row` and `column`
share one default each, which is why both take `tag:` — a row inside a row is
`for tui.row(tag: "inner"):`.

Closing wins the tie, because the alternative turns `for tui.panel("x"): pass`
into a stack overflow, and a wrong layout beats a hang. **The library says so on
screen**: a container closed with nothing in it, or left open at the end of a
frame, draws a red `layout:` banner in the corner, and `layout_warning()`
reports it.

It is drawn and not printed, and that is not a style choice — `println` and
`#breakpoint` write to the terminal this library is drawing on, so a diagnostic
about the layout would arrive *as* a corrupted layout: the message overwrites
the screen, the compositor's front buffer no longer matches what is there, and
every frame after it is diffed against a lie. A TUI library may not report a
problem by printing.

**The `if` form** has no ambiguity at all, because the close is written:

```dado
if tui.panel(title: "Settings"):
    defer tui.end()
    ...
```

Use it where the body has a `break` or an early `return` — including a `return`
out of a *helper* the frame calls, which is the same thing one stack frame down
and is what the dashboard's Constants tab needed.

### Destructive actions

`confirm_button` arms on the first press and acts on the second, disarms on
`esc`, and expires by itself after a few seconds:

```dado
if tui.confirm_button(label: "Clean", armed: "clean build/?"): clean()
```

It is a widget and not a program idiom because the state it needs — armed, and
since when — is per-widget and keyed by id, which is what the library holds and
an immediate-mode caller does not. A modal is the other answer and is worse for
a toolbar: it takes the keyboard, needs somewhere to live in the layout, and
costs two more interactions than the mistake it prevents.

It is sized from the wider of its two labels, always. A button that grows when
you arm it shoves everything beside it sideways at the moment you are about to
click again, which is how a confirm turns into a misclick.

---

## Overlays

Three of them, and they are built out of the same pieces as everything else:

```dado
if tui.modal(title: "Remove build/?", open: &confirming):
    defer tui.end()
    tui.paragraph("This deletes 41 files, 12.3 MB.")
    for tui.row(tag: "buttons"):
        if tui.button("Remove"): clean()
        if tui.button("Cancel"): confirming = false

tui.toast("build finished")
tui.toast("3 tests failed", tui.now().err)

i32 chose = tui.palette(open: &palette_open, query: &palette_query,
                        items: COMMANDS[:])
if chose >= 0: run(chose)
```

`modal` is the `if` form and not the `for` form, because a modal body almost
always has a `return` or a `break` in it. It dims what is behind it, closes on
`esc` and on a click outside — both by writing `false` through *your* pointer, so
the state a modal is in is a state your program can see and set.

`toast` can be called from **anywhere, including outside a frame**, which is the
point: a build finishing is not a frame event. They draw themselves at the end of
every frame, stacked up from two rows above the bottom — clear of the hints bar,
because a toast that covers the keys you would use to react to it is a toast that
is in the way. The loop keeps waking while one is counting down.

`palette` is a `modal` holding a `line_edit` and a `list`, and it answers **the
index into the array you passed**, not into the filtered view. Resolving that
here is the whole difference between a palette you can act on and one whose
answer you have to re-derive the meaning of. Filtering is case-insensitive, and
opening clears the query.

### An overlay is drawn last and takes input first

Those two pull in opposite directions in a one-pass library: it lands on top
because it is composed *after* everything else, but the widgets it is on top of
already ran and already read the mouse.

The answer is the one the claiming rule already uses — **last frame's state is
what is in force**. While an overlay was up on the previous frame, every input
query outside an overlay answers false: `key`, `typed`, `clicked`, `hovering`,
the wheel, and focus registration. So a global `q` does not quit behind a modal,
and Tab does not walk into the panels underneath. The cost is one frame at the
moment a modal opens — during which the modal is not drawn yet either, so there
is nothing to aim at. Invisible rather than merely cheap.

> **Declare your overlays last in the frame.** They draw in call order like
> everything else, and a modal written above the panels would be under them.

---

## Layout is one pass

A region is a rectangle, an axis, two cursors and a gap. There is no flex solver
and no second pass, because in a terminal a widget's size is known before it is
placed.

Sizes are cells, or `AUTO`, `REST`, `pct(n)`, or `FLOW` — the last being "fill
the cross axis, natural along the main one", which is what makes a stack of
labels full-width and a row of them packed.

Two cursors is the whole alignment story: `align(START)` packs forward from the
near edge, `align(END)` backward from the far one.

**The one thing one-pass layout asks of you** is `reserve(n)`. A `REST`-sized
widget takes everything left *at the moment it is placed*, so anything after it
finds nothing. `reserve` says how much is spoken for first:

```dado
tui.reserve(7)                        // a 6-row pane and a footer row
for tui.panel(height: tui.REST): ...  // REST now stops 7 short
for tui.panel(height: 6): ...         // spends 6 of the promise
tui.align(tui.BOTTOM)
tui.hints()                           // spends the last one
```

---

## The roster

| | |
|---|---|
| **containers** | `column` `row` `panel` `scroll` `table` `table_row` `end` |
| **fillers** | `space` `fill` `rule` `reserve` `align` |
| **static** | `text` (with `anchor:`) `title` `label` `muted` `paragraph` `kv` `kv_text` `kv_float` `badge` `badge_quiet` `dot` |
| **indicators** | `spinner` `progress` |
| **controls** | `button` `confirm_button` `checkbox` `radio` `select` `list` `tabs` `line_edit` (+ `edited` `set_text`) |
| **panes** | `log` (+ `log_open` `log_feed` `log_say` `log_at` `log_clear`) `table` `table_row` |
| **overlays** | `modal` `toast` (+ `toasts_clear` `toasts_showing`) `palette` `overlay_up` `dim_rect` |
| **input** | `key` `bind` `hint` `hints` `typed` `mouse` `clicked` `clicked_in` `hovering` `wheel` |
| **identity** | `push_id` `pop_id` |
| **style** | `theme` `now` `rgb` `hex` `ansi` `mix` `DARK` `LIGHT` `MONO` |
| **lifetime** | `open` `open_over` `close` `run` `step` `quit` `wake` `clock` `closed` |
| **the terminal** | `suspend` `resume` `suspended` `shell_out` `stop` |
| **headless** | `open_headless` `compose` `snapshot` `cell_at` `color_rgb` `sink` |

---

## Keys are named, and who gets them is a rule

```dado
if tui.key("ctrl+s"): save()
if tui.key("up"):     move(0 - 1)
tui.bind("q", "quit", tui.quit)      // and it appears in the hints bar
```

Zero or more of `ctrl+`, `alt+`, `shift+`, then a name: a character, `enter`,
`tab`, `esc`, `space`, `bksp`, `del`, `ins`, `home`, `end`, `pgup`, `pgdn`, the
arrows, `f1`..`f12`.

> **A focused text-entry widget claims plain characters and the editing keys.
> Everything else stays global.**

So typing `q` into a filter field does not quit, and `ctrl+s` still saves while
you are typing. The authority is the *previous* frame's answer, because a global
binding is usually written before the focused widget has been placed, and a rule
that depended on statement order would be one nobody could predict.

Focus is automatic: Tab and Shift-Tab walk the focusable widgets in declaration
order, Enter and Space activate, arrows navigate inside a list or a tab strip.

**`tui.hints()` renders every binding registered this frame**, so a program that
binds its keys gets its help bar for free and it cannot drift from the bindings.

---

## Theming

A theme is two ramps, not a foreground and a background — because the two things
that make a TUI look shoddy are flat surfaces and loud idle borders.

* the **surface ramp**: `base` `mantle` `surface` `overlay`
* the **text ramp**: `text` `subtext` `muted` `faint`
* plus `accent`/`on_accent`, `ok` `warn` `err` `info`, `border`/`border_focus`,
  a `BorderSet`, and `wide_glyphs`.

It is an ordinary structural tuple, so tweaking one is copy-and-overwrite:

```dado
tui.Theme t = tui.DARK
t.accent = tui.hex(0xF5A97F)
tui.theme(t)
```

Truecolour is detected once at `open` and every colour downgrades to the nearest
256-colour index when it is absent — greys against the greyscale ramp as well as
the cube, because a surface ramp lives entirely among them.

---

## Giving the terminal back

A TUI owns the terminal, so anything else that wants it — an editor, a pager,
the shell you get from Ctrl-Z — has to be handed it and given it back.

```dado
_ = tui.shell_out(cstring("${EDITOR:-vi} +11 src/main.dado"))
```

`suspend` and `resume` are the pair; `shell_out` is the form to use, because the
pair is what is easy to get wrong — a `defer` inside it puts the terminal back
on every path, including the ones where the child never started. The next frame
after a resume is a full repaint, because whatever ran in between wrote to the
same screen and the compositor's front buffer is a description of something no
longer there.

> **You do not need to bind Ctrl-Z, and you cannot.** With `ISIG` on — which it
> is, so Ctrl-C still works — the line discipline turns `^Z` into SIGTSTP before
> the program sees a byte, so `key("ctrl+z")` can never fire. **`tty.dado` handles
> the signal** (it was `tty.c` until 2026-09-13): it restores the terminal, stops
> for real with the default disposition,
> and the loop re-enters raw mode when SIGCONT arrives. **Ctrl-Z works in any
> program using this library, with nothing written.** `tui.stop()` is the same
> thing asked for from inside the program and takes the identical path.

## Rendering

Widgets write `(glyph, fg, bg, attr)` into a cell. **There is no per-box string
cache and no SGR parser anywhere in the library** — the only place an escape
sequence is produced is the compositor, and there is no place one is consumed.
The predecessor painted by writing escape codes into a 4 KiB buffer per box and
parsing them back out to recover the style.

A cell carries a width, so a double-width character occupies its cell and marks
the next a continuation. `core:chars` does the decoding and the width table.

Measured, on a 100×26 screen: **a toggle costs 52 bytes.** A full repaint is
thousands.

The frame goes to a `Sink`, not to stdout, which is what makes the library
testable: `open_headless` + `compose` + `snapshot` runs a whole UI with no
terminal anywhere.

---

## Storage

One arena, **reset at the top of every frame**, exposed as `tui.mem`:

```dado
tui.text(#format("hits: ", n, tui.mem))
```

has no cleanup and no leak.

> **Nothing built from `tui.mem` may outlive the frame that built it.** A string
> formatted there and kept in a package variable is pointing at reclaimed
> memory by the time anything reads it, and what you see is a few bytes of the
> *next* frame's formatting where your value should be. Anything that outlives a
> frame needs an allocator that does too — `open`'s backing, or `#heap`. This is
> the one thing the arena makes easy to get wrong, and it is worth the arena. After the first frame the arena is the size the
program needs and steady state asks the heap for nothing. The cell grids and a
line edit's buffer come from the allocator `open` was given.

There are no fixed ceilings on widget counts. The predecessor's `MAX_BOXES` was
a `#panic`.

---

## The C floor — **there isn't one any more**

~~Four facts about the platform, in `tty.c`, each argued in `tty.h`.~~
**`tty.c` and `tty.h` are deleted (2026-09-13) and `tty.dado` is the whole of
it.** The facts are the same facts and they are worth keeping in front of you,
because they are what makes this package POSIX rather than portable: raw mode
needs a `struct termios` whose layout is the platform's; the window size is a
variadic `ioctl`; a read with a timeout is `select` over an `fd_set`; stopping
is `raise(SIGTSTP)` with the default disposition; and the signal handlers are
what make Ctrl-C and Ctrl-Z leave a usable terminal behind. **What changed is
that Dado can now say all five**, so there is no sidecar and no second language
in the package. Windows' four are the console API's, in the same file.

**Two reasons that file gave for existing are worth reading, because one was
false for a whole milestone before anybody re-measured it.** *"TOY has no
`volatile`, and therefore no `sig_atomic_t`"* stopped being true when the four
C qualifiers landed as positional words; `tty.dado`'s
`private volatile SigAtomic g_continued` emits
`static DADO_UNUSED volatile sig_atomic_t …`, which is exactly the declaration
the C had. And *"a handler may call `write` and may not call `printf`"* is true
and never needed C: `write` restates in one line, the handlers are ordinary Dado
functions installed with `signal`, and **what Dado lacks is a way to *mark* a
function handler-safe — which C lacked too.** The C file held that discipline by
being small; so does `tty.dado`, which states the guarantee as a sentence and
keeps it true by being read. `SIG_DFL` and `errno` are macros, and both bind as
ordinary `foreign` constants: the declaration emits nothing and the use site
writes the bare name for the preprocessor to expand.

Raw mode clears `IXON` as well as `ECHO`/`ICANON`, because with it on `Ctrl-S` is
XOFF and stops the terminal's output instead of reaching the program — a UI that
binds `ctrl+s` appears to freeze, and `Ctrl-Q` unsticks it and nobody knows that.

~~Symbols are `dado_sys_tty_*`: `dado_tty_rows` is exactly what a package `tty`
with a function `rows` mangles to, and the predecessor owned that name.~~ **The
collision it was avoiding is gone by construction: there is no package `tty`.**
Everything in `tty.dado` is `private` — so it emits `static` and has no external
symbol at all — and is prefixed `tty_`, because the package it now lives in
(`tui`) already has a `cols`, a `rows`, a `measure`, an `is_terminal` and a
`stop` of its own. **One package and one prefix answers both sides of what used
to need two naming conventions.**

---

## Testing

* `layout_test.dado` — headless compose and snapshot, no terminal.
* `tty_test.dado` — the platform arm with its streams on pipes, on every OS
  and under Wine: no terminal is a refusal and not a failure, the size falls
  back to 80x24, and on Windows a console's UTF-16 (a split surrogate pair
  included) reaches the decoder as UTF-8.
* the repository's pty harness, `test_dashboard.py` — the flagship driven as
  a program: package discovery, filtering, the constants table and its `-D`
  round trip, and a real `./dado test` streaming into the log pane; the test
  runner's queue, the diagnostics list, and that `q` does not quit while a
  text field has focus.
* the same harness's `test_input.py` — 55 checks under a **real pty**: the
  diff compositor, the SGR stream, key decoding, focus, mouse, wheel, resize,
  the claiming rule, and that quitting restores the terminal. It runs after
  `./dado build archive/dashboard/smoke`. The harness parses the emitted
  escape stream back into a grid, so the assertions are about what a terminal
  would show.

`pty_drive.py` can also render the live screen as HTML, which is how the
dashboard's screenshots are taken.

---

## Cross-compiling

**Windows has its own arm**, the console API in `tty.dado` behind a
`when #OS == #WINDOWS`: `SetConsoleMode` for raw input with
`ENABLE_VIRTUAL_TERMINAL_INPUT` and for `ENABLE_VIRTUAL_TERMINAL_PROCESSING` on
output — so the keys arrive as the escape sequences the decoder already reads
and the compositor's SGR stream is drawn — `GetConsoleScreenBufferInfo` for the
size, and `ReadConsoleInputW` after `WaitForSingleObject` for the timed read,
with the characters handed on as UTF-8. Ctrl-C stays a control event, as `ISIG`
stays on here, and a console control handler restores both modes before the
process ends. A console has no job control, so `tui.stop` there is a suspend
and a resume. A TUI program cross-compiles with `zig cc -target
x86_64-windows-gnu` like everything else in the package.

**The `#DARWIN` arms in `tty.dado` are compiled and not run.** `zig cc -target
aarch64-macos` (and `x86_64-macos`) builds the dashboard and this package's
tests against zig's bundled macOS headers, so every layout, member-type and
signature assertion over the Darwin `termios`, `struct timeval` (`tv_usec` is
an `int` there) and `fd_set` is checked by a C compiler; nothing here runs the
result on a Mac.

Cross-compiling anything is `DADO_TARGET=<triple>` — the driver's own override —
plus a C compiler that can emit for it, and `zig cc` is what makes that one
install rather than a toolchain per platform:

    DADO_TARGET=aarch64-macos ZIG_TARGET=aarch64-macos \
        ./dado build corpus/pangram --cc scripts/zigcc

`scripts/zigcc` exists because `--cc` names one *program* and `zig cc` is two
words. It reads `ZIG_TARGET`, so one shim serves every target, and unset it is
an ordinary host `cc`. The two variables say the same thing to the two halves of
the build — `DADO_TARGET` is what `dadoc` resolves `#OS` and `#ARCH` from, and
`ZIG_TARGET` is what the C compiler emits for; setting one without the other
gives a program whose `when` branches disagree with its object code.

Verified building and `#OS`-resolving for `x86_64-linux-gnu`,
`aarch64-linux-gnu`, `x86_64-macos`, `aarch64-macos`, `x86_64-windows-gnu` and
`aarch64-windows-gnu`.

## The consumer

`dashboard/dash` — a build tool for Dado. It discovers the collections, filters
them, drives `./dado build|check|test` through `core:os/proc`, and streams the
compiler's output into a `log` pane with `proc.drain` while the UI keeps
drawing. Its loop is `tui.step`, which is the reason `step` exists.

Its target selector is **gated on `zig` being on the PATH** and says so when it
is not, rather than offering targets that would fail at the link step; and the
Test button is off for a non-native target, because a cross-built test binary
cannot be run on the machine that built it.

## Why it is in `vendor/` and not in `core/`

It inherits the slot from the tuibox port, which it replaces. What it keeps from
that port is the idea of a diffing cell compositor and the shape of the C floor;
the box model, the callback design, the caches and the storage ceilings are all
gone. Whether a TUI is `core:` surface Dado commits to is a question worth asking
separately from whether this one works.
