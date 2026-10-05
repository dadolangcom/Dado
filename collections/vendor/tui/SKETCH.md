# `vendor:tui` — what laying out a UI actually looks like

A worked example against the proposed API, so the ergonomics can be judged
rather than described. Nothing here compiles yet.

The screen is the Build tab of `dado dashboard`: a package sidebar, a settings
column of selectors and checkboxes, a live build log, and a status bar.

---

## 1. The thing that makes nesting work

Indentation in Dado only opens after a `:`. That sounds like it forbids nested
containers — but every container returns a `bool`, so **every container is an
`if`, and an `if` indents.** Nesting comes out looking like nesting:

```dado
if tui.row(gap: 1):
    defer tui.end()

    if tui.panel("Packages", width: 28):
        defer tui.end()
        tui.list(&pkg, packages, height: tui.REST)

    if tui.column(width: tui.REST, gap: 1):
        defer tui.end()

        if tui.panel("Settings", height: 12):
            defer tui.end()
            tui.select("compiler", &cc, "cc", "clang", "gcc", "tcc")
            tui.checkbox("sanitize", &asan)

        if tui.panel("Output", height: tui.REST):
            defer tui.end()
            tui.log(&build_log, follow: true)
```

That is three levels deep and it reads like three levels deep. The whole cost is
`defer tui.end()` — one line per container, always the same line, and it is the
shape the reader already knows from `defer #delete(x)`.

---

## 2. Three spellings of the same container

### (a) `if` + `defer` — the default

```dado
if tui.panel("Settings", width: 34):
    defer tui.end()
    tui.checkbox("release", &release)
    tui.checkbox("sanitize", &asan)
```

Correct on every exit path the language has, because that is what `defer` is
for. Two lines of ceremony.

### (b) `for` — the syrup

`for cond:` is Dado's only `while`, and it re-evaluates the condition after each
pass. So a container can close *itself*: the first call opens the region and
answers `true`, the body runs, the second call sees its own region on top of the
stack and closes it, answering `false`.

```dado
for tui.panel("Settings", width: 34):
    tui.checkbox("release", &release)
    tui.checkbox("sanitize", &asan)
```

One line, no `end`, no `defer`, and it nests exactly the same way. `continue`
is safe — it re-evaluates the condition, which is the close.

The hazard is real and worth stating: **`break` skips the condition, so the
region never closes**, and every widget after it lands inside a panel it was not
meant to be in. `return` out of the frame function is harmless (the frame is
over, and `end_frame` closes whatever is left), but `break` is not. The library
can detect it — a region closed by `end_frame` rather than by its own call is a
`#breakpoint` in a debug build — but it cannot prevent it.

### (c) A body function — for a panel worth naming

```dado
tui.panel("Settings", settings_body, width: 34)

private void settings_body():
    tui.checkbox("release", &release)
    tui.checkbox("sanitize", &asan)
```

No ceremony at all, composes, reusable, and the right answer once a panel is
more than a screenful. The cost is that the body cannot see the enclosing
function's locals — but in a UI whose state is package-level anyway, that
usually costs nothing.

**My recommendation: ship all three, document (b) as the default and (a) as the
one to reach for when the body has a `break` or a `return` in it.** (b) is the
one that makes this feel like a Python library.

---

## 3. The whole screen

```dado
package dashboard

import tui "vendor:tui"
import "core:os/proc"

// ── the app's own state. The library holds none of it. ────────────────────
private i32 tab = 0
private i32 pkg = 0
private i32 cc = 0
private i32 opt = 2
private bool release = false
private bool asan = true
private bool ubsan = true
private bool warn_error = true
private f32 progress = 0.0
private bool building = false
private tui.LogView out_log
private ref []char8 filter

private []string PACKAGES = ["core:strings", "core:fmt", "core:io", "core:mem"]

// ── one frame ─────────────────────────────────────────────────────────────
private void frame():
    // A header row: title on the left, live status on the right.
    for tui.row(height: 1):
        tui.title("dado dashboard")
        tui.align(tui.RIGHT)
        if building: tui.spinner("building")
        else:        tui.badge("idle", color: tui.now().muted)

    tui.tabs(&tab, "Build", "Test", "Constants", "Flags", "Diagnostics")
    tui.rule()

    switch tab:
        case 0: build_tab()
        case 1: test_tab()
        else:   tui.paragraph("Not built yet.")

    // The footer. `hints()` renders every binding registered this frame,
    // so it can never disagree with what the keys actually do.
    tui.align(tui.BOTTOM)
    for tui.row(height: 1):
        tui.hints()
        tui.align(tui.RIGHT)
        tui.kv("pkg", PACKAGES[pkg])

// ── the Build tab ─────────────────────────────────────────────────────────
private void build_tab():
    for tui.row(gap: 1, height: tui.REST):

        // Left: a filterable package list.
        for tui.panel("Packages", width: 30):
            tui.line_edit("", &filter, placeholder: "filter…")
            tui.space()
            if tui.list(&pkg, PACKAGES, height: tui.REST, filter: tui.edited(filter)):
                start_build()          // Enter or a double click on a row

        // Right: settings above, output below.
        for tui.column(width: tui.REST, gap: 1):

            for tui.panel("Toolchain", height: 9):
                // A two-column form. `pct` splits the panel's own width.
                for tui.row(gap: 2):
                    for tui.column(width: tui.pct(50)):
                        tui.select("compiler", &cc, "cc", "clang", "gcc", "tcc")
                        tui.stepper("optimise", &opt, min: 0, max: 3)
                        tui.checkbox("release", &release)
                    for tui.column(width: tui.REST):
                        tui.checkbox("address sanitizer", &asan)
                        tui.checkbox("undefined sanitizer", &ubsan)
                        tui.checkbox("warnings are errors", &warn_error)

                tui.rule()
                // The command line that all of the above adds up to,
                // rebuilt every frame out of the frame arena and never freed.
                tui.paragraph(command_line())

            for tui.panel("Output", height: tui.REST):
                if building:
                    tui.progress(progress, label: "compiling")
                tui.log(&out_log, follow: !building)

    // Bindings. Each one also becomes a footer hint.
    tui.bind("ctrl+b", "build",  start_build)
    tui.bind("ctrl+t", "test",   start_test)
    tui.bind("ctrl+d", "debug",  start_debug)
    tui.bind("q",      "quit",   tui.quit)

// The command line is a string built per frame. `tui.mem` is the frame arena:
// it is reset at the top of every frame, so nothing here is ever freed and
// nothing here leaks.
private string command_line():
    fmt.Cursor c = tui.scratch(256)
    fmt.put_string("dadoc --collections=collections -O", tui.line(&c))
    fmt.put_signed(opt, tui.line(&c))
    if asan:        fmt.put_string(" -fsanitize=address", tui.line(&c))
    if ubsan:       fmt.put_string(" -fsanitize=undefined", tui.line(&c))
    if warn_error:  fmt.put_string(" -Werror", tui.line(&c))
    fmt.put_string(" ", tui.line(&c))
    fmt.put_string(PACKAGES[pkg], tui.line(&c))
    return fmt.written(c)

i32 main():
    if !tui.open(): #panic("dado dashboard needs a terminal")
    defer tui.close()
    tui.theme(tui.DARK)

    // `step` rather than `run`, because this program also has to drain a
    // child process while the UI stays live. 30 ms is the drain cadence,
    // not the repaint rate — a frame is only composed when something changed.
    for tui.step(frame, 30):
        if building: pump_build()
    return 0
```

---

## 4. The parts of that worth pointing at

**Nesting is visually nesting.** Four levels in `build_tab` — row, column, panel,
row, column — and the indentation tells you so. No `end` calls to line up by eye
and no way to close the wrong one.

**Sizes are three sentinels and a number.** `width: 30`, `width: tui.REST`,
`width: tui.pct(50)`, `height: 9`. There is no flex solver, no constraint
system, and no second pass — and for a terminal that is not a compromise, it is
just enough.

**Named arguments are the whole sugar engine.** `tui.list(&pkg, PACKAGES,
height: tui.REST, filter: ...)` writes only the three slots it means out of the
nine the signature has, and a named call may omit *any* defaulted parameter, not
merely a trailing run of them. That is a language feature this library is built
to spend.

**Variadic labels.** `tui.tabs(&tab, "Build", "Test", "Constants", …)` and
`tui.select("compiler", &cc, "cc", "clang", "gcc", "tcc")` — a `..string` pack is
a compound literal in the caller's frame, so it allocates nothing and needs no
array bound first.

**The state is all the app's, and it is all ordinary variables.** `&pkg`, `&asan`,
`&opt`. Nothing is registered, nothing is subscribed, nothing has an id the app
has to hold. The library's own state — which row is focused, where the log is
scrolled, whether the spinner is on frame 3 — never appears.

**`switch tab:` is the screen router**, and it works because the frame is total:
a tab that is not selected simply is not called, and the widgets it would have
placed do not exist this frame. In a retained model that is a tree edit; here it
is an `if`.

**Alignment is one call and one pass.** `tui.align(tui.RIGHT)` before the status
badge; `tui.align(tui.BOTTOM)` before the footer. Possible because a widget's
size is known before it is placed.

---

## 5. What this asks of `core:`

Two additions, both small, both things `core:` wants anyway:

* **`core:os/proc` needs a non-blocking drain.** `run` blocks and `open` is
  `popen`-shaped; `pump_build()` above needs "give me whatever this child has
  written, right now, and do not wait." That is one `select` over the stream's
  fd, ~~and it is the same `select` `tty.c` already contains — so it is a
  function in an existing sidecar, not a new one.~~
  **Built, and not the way this predicted.** It is `core:os/proc`'s `drain`, it
  is Dado over `poll` rather than C over `select`, and there is **no sidecar for
  it to be a function in** — `tty.c` and `tty.h` were deleted on 2026-09-13 and
  `tty.dado` is pure Dado too. **`drain.dado` then became the precedent for every
  `when #OS` arm in `tty.dado`**, so the dependency ran the other way round from
  the one sketched here.
* **`core:fmt` needs a `Cursor` over a caller-supplied run**, which it very
  nearly has: `fmt.Cursor` exists and `fmt.written` exists. `tui.scratch(n)`
  above is a `Cursor` over `n` bytes of the frame arena, which is a three-line
  helper here rather than a change there.

Neither is on the critical path for steps 1–3 of the build order.
