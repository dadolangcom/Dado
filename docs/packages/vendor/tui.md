<!-- dadoc 1.0.0-dev+f2b6a9e7dc2b.dirty -->
<!-- commit f2b6a9e7dc2b (dirty) -->
# vendor:tui

vendor:tui — a terminal UI you can throw together in seconds.

    import tui "vendor:tui"

    private bool notify = true

    private void frame():
        tui.title("Preferences")
        tui.checkbox("Notify me", &notify)
        if tui.button("Quit"): tui.quit()

    i32 main():
        if !tui.open(): #panic("not a terminal")
        defer tui.close()
        tui.run(frame)
        return 0

── the three rules worth knowing before you use it ────────────────────────

**It is immediate mode, because Dado has no closures.** There are no widget
objects, no handlers to register and no ids to hold: the frame function runs
from the top every time, and the code that would have been a callback is the
code on the next line, in a scope that can see every local it needs. This is
not a performance style, it is the answer to a language constraint — and it
is why a Dado TUI library can be *more* ergonomic than a C one rather than
less.

**You own the value; the library owns the interaction.** `&notify` is yours.
Which widget is hot, which has focus, where a scroll pane sits, where the
caret is in a line edit — ours, keyed by an id derived from where the widget
was called rather than from anything you have to hold.

**A widget call answers `bool`, and it means "activated or changed on this
frame."** One rule for every widget in the library, which is what makes
`if tui.button("Save"): save()` the whole idiom.

── and one more ───────────────────────────────────────────────────────────

**It is a singleton.** One terminal per process, and the alternative is a
`^Ui` first parameter on every call in the library — which is the entire
sugar budget spent on nothing. `open` refuses a second one.

## Declarations

561 declarations, 286 public.

* `type Rect: (i32 x, i32 y, i32 w, i32 h)` — A rectangle in terminal cells. Zero-based and half-open: a `Rect` at (0,0)
* `const Rect NOWHERE = (0, 0, 0, 0)`
* `bool empty(Rect r)`
* `bool holds(Rect r, i32 x, i32 y)`
* `Rect intersect(Rect a, Rect b)`
* `Rect inset(Rect r, i32 by)`
* `i32 cols()`
* `i32 rows()`
* `Rect screen()` — The whole terminal, as a rectangle.
* `Rect clip()`
* `void fill_rect(Rect r, Style st)` — Paint a rectangle in one style, glyph and all. `fill_rect(r, surface)` is…
* `void tint_rect(Rect r, Color bg)` — Repaint a rectangle's background without touching its glyphs — the selection…
* `void dim_rect(Rect r, u32 amount)` — Push everything in `r` toward the window's own background.
* `i32 draw_text(i32 x, i32 y, string8 s, Style st, i32 max_cols)` — Draw `s` at (x, y), stopping after `max_cols` columns. Answers how many…
* `i32 draw_text_fill(i32 x, i32 y, string8 s, Style st, i32 cols_wide)` — `draw_text`, and then blank the rest of the run in the same style. This is…
* `i32 measure(string8 s)` — How wide `s` will be. One call, so a widget that measures and a widget that…
* `void draw_box(Rect r, BorderSet b, Style line, string8 title, Style cap)` — A box, optionally with a caption inset into its top edge.
* `bool scroll(i32 height = REST, i32 width = REST, string8 tag = "scroll", bool follow = false)` — A clipped, scrollable column.
* `void scroll_to(string8 tag, i32 line)` — Scroll a pane by hand, by the tag it was opened with.
* `bool list(^i32 selected, []string8 items, i32 height = REST, string8 tag = "list")` — A list of rows, one selected. `selected` is yours.
* `bool tabs(^i32 active, ..string8 labels)` — A row of tabs. `active` is yours; the answer is whether it changed.
* `bool tabs_of(^i32 active, []string8 labels, string8 tag = "tabs")` — The same, when the labels are data rather than written out.
* `bool select(string8 label, ^i32 index, ..string8 options)` — One of a set, cycled in place: `‹ clang ›`.
* `bool select_of(string8 label, ^i32 index, []string8 options)`
* `bool confirm_button(string8 label, string8 armed = "sure?", i32 width = AUTO, f64 window = 3.0, bool enabled = true)` — A button that asks first: the press arms it, and a second press within…
* `bool armed(string8 label)` — Whether the confirm button with this label is currently waiting for its…
* `bool line_edit(string8 label, ^ref []char8 buf, string8 placeholder = "", bool secret = false, i32 width = FLOW)` — A single-line text field. The buffer is yours, and it is a `^ref []char8`
* `string8 edited(ref []char8 buf)` — The contents of a line edit's buffer, as a `string` to read.
* `ref []char8 set_text(ref []char8 buf, string8 s)` — Put `s` into a line edit's buffer, minting one if there is none. Answers the…
* `const u32 K_NONE = 0`
* `const u32 K_ENTER = K_BASE + 1`
* `const u32 K_TAB = K_BASE + 2`
* `const u32 K_ESC = K_BASE + 3`
* `const u32 K_BKSP = K_BASE + 4`
* `const u32 K_DEL = K_BASE + 5`
* `const u32 K_INS = K_BASE + 6`
* `const u32 K_HOME = K_BASE + 7`
* `const u32 K_END = K_BASE + 8`
* `const u32 K_PGUP = K_BASE + 9`
* `const u32 K_PGDN = K_BASE + 10`
* `const u32 K_UP = K_BASE + 11`
* `const u32 K_DOWN = K_BASE + 12`
* `const u32 K_LEFT = K_BASE + 13`
* `const u32 K_RIGHT = K_BASE + 14`
* `const u32 K_F1 = K_BASE + 20`
* `distinct enum u32 Mod`
* `type KeyEvent: (u32 code, Mod mods)`
* `void animate()`
* `(i32 bytes, i32 keys) input_counters()` — Diagnostics for the pty harness: how many bytes the decoder has been handed…
* `bool key(string8 spec)` — Was this key pressed on this frame? Consumes it, so two widgets cannot both…
* `string8 typed()` — The printable characters typed this frame, in order, as a `string` from the…
* `(i32 x, i32 y) mouse()`
* `bool mouse_down()`
* `bool clicked_in(Rect r)` — Did a press land inside `r` this frame? The hit test every clickable widget…
* `bool hovering(Rect r)`
* `i32 wheel()` — How far the wheel turned this frame, in lines. Negative is up.
* `bool overlay_up()` — Whether an overlay is up, for a program that wants to say so.
* `bool has_focus(u32 id)`
* `type Action: void()`
* `void hint(string8 spec, string8 describe)` — Advertise a key in the hints bar **without claiming it**.
* `void bind(string8 spec, string8 describe, Action f)` — Run `f` when `spec` is pressed, and show it in the hints bar. `describe` may…
* `const i32 AUTO = 0 - 1` — Fit the content. The default cross size for a row's children.
* `const i32 REST = 0 - 2` — Everything the region has left along this axis. The default cross size for a…
* `const i32 FLOW = 0 - 3` — Fill the cross axis, take the natural size along the main one. The default…
* `i32 pct(i32 n)` — A fraction of the region's extent along this axis. `pct(50)` in a row is…
* `enum Axis: (Down, Right)`
* `enum Anchor: (START, MIDDLE, END)` — Where along the *main* axis a widget is placed. `START` packs forward from…
* `const Anchor LEFT = Anchor.START` — The spellings that read right in each direction. They are the same three…
* `const Anchor CENTER = Anchor.MIDDLE`
* `const Anchor RIGHT = Anchor.END`
* `const Anchor TOP = Anchor.START`
* `const Anchor MIDDLE = Anchor.MIDDLE`
* `const Anchor BOTTOM = Anchor.END`
* `const i32 MAX_TABLE_COLS = 12` — A table wider than this is a table that wanted to be a list.
* `void push_id(i32 n)` — Distinguish otherwise identical widgets in a loop.
* `void push_id_str(string8 s)`
* `void pop_id()`
* `Rect available()` — Where the next widget would go, without taking it.
* `void align(Anchor a)` — Set the anchor for everything placed after this, until the region closes.
* `void gap(i32 n)` — Set the gap between everything placed after this.
* `(bool any, string8 what) layout_warning()` — Whether the frame just composed had a layout problem, and what it was.
* `void end()` — Close the innermost region.
* `bool column(i32 width = REST, i32 height = REST, i32 gap = 0, i32 pad = 0, string8 tag = "col")` — A vertical stack. Children fill its width unless they say otherwise.
* `bool row(i32 width = REST, i32 height = 1, i32 gap = 1, i32 pad = 0, string8 tag = "row")` — A horizontal run. Children take their natural width unless they say…
* `bool panel(string8 title = "", i32 width = REST, i32 height = REST, i32 gap = 0, i32 pad = 2)` — A bordered, titled column on the raised surface.
* `void region_bg(Color c)` — Say what a region painted its background, so the widgets inside it draw on…
* `void focus_frame()` — Mark the enclosing panel as focused, so its frame draws in the accent…
* `void space(i32 n = 1)` — Blank cells along the main axis.
* `void fill()` — Take everything left along the main axis, so what follows an `align(END)`
* `void reserve(i32 n)` — Promise `n` cells to something placed later, at either edge.
* `void rule(string8 title = "")` — A themed hairline across the region, optionally with an inset caption.
* `bool modal(string8 title, ^bool open, i32 width = 0 - 1, i32 height = 0 - 1, u32 dim = 150)` — A centred panel above everything else, with the screen behind it dimmed.
* `void toast(string8 s, Color color = DEFAULT, f64 seconds = 4.0)` — Show `s` for `seconds`. The default colour is the accent; pass `err` or `ok`
* `void toasts_clear()`
* `i32 toasts_showing()`
* `i32 palette(^bool open, ^ref []char8 query, []string8 items, string8 title = "Commands")`
* `bool table([]string8 headers, []i32 widths, i32 height = REST, string8 tag = "table")` — Open a table: draws the header row, then leaves a column open for the body.
* `bool table_row(i32 height = 1, string8 tag = "tr", bool selected = false)` — One row of the enclosing table. Zebra striping alternates on the row index,…
* `type Log` — A byte run plus the offset each line starts at. Two growable runs rather than…
* `Log log_open((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc, i32 budget = 262144)` — A log over `alloc`, keeping roughly `budget` bytes of scrollback.
* `void log_close(^Log l)`
* `void log_clear(^Log l)`
* `void log_feed(^Log l, []char8 chunk)` — Append a chunk. It may end mid-line, hold several lines, or be one byte —…
* `void log_say(^Log l, string8 s)` — One whole line.
* `i32 log_lines(Log l)`
* `string8 log_at(^Log l, i32 i)` — One line, as a `string` window over the log's own bytes.
* `type LogPaint: Color(string8)` — The pane. Follows the tail unless the reader has scrolled up, which is the…
* `Color plain(string8 line)` — The default painter: every line the ordinary log colour. It exists because…
* `bool log(^Log l, i32 height = REST, string8 tag = "log", LogPaint paint = plain)`
* `void log_scroll(string8 tag, i32 by)` — Scroll the log pane opened with `tag` by `by` lines, negative for up, as…
* `void log_follow(string8 tag = "log")` — Follow the tail again, from wherever the reader scrolled to.
* `const i32 COLORS_NONE = 0` — How many colours the terminal is sent. The compositor downgrades every…
* `const i32 COLORS_16 = 16`
* `const i32 COLORS_256 = 256`
* `const i32 COLORS_TRUE = 16777216`
* `void sink(Sink out)` — Send frames somewhere other than stdout. The library never opens a stream of…
* `void truecolor(bool on)` — Whether 24-bit colour is available. Decided once at `open` from the…
* `void colors(i32 depth)` — Set the colour depth outright: `COLORS_NONE`, `COLORS_16`, `COLORS_256` or…
* `i32 color_depth()` — The colour depth in force, so a program can choose a theme written for it:
* `(i32 bytes, i32 writes) frame_output()` — The bytes and writes the last `present` sent: `(bytes, writes)`. Writes is…
* `void present()` — Walk the grid and write what changed. `front` is updated as we go, so it…
* `(u32 glyph, Color fg, Color bg, Attr attr, bool cont) cell_at(i32 x, i32 y)` — What is painted at (x, y) in the frame just composed. `cont` marks the right…
* `(u32 value, bool has) color_rgb(Color c)` — The 24-bit value behind a colour, resolved through the theme's default.
* `string8 snapshot((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — The current frame as plain text, one row per line, no colour. For golden…
* `distinct type Color: u32` — ── colour ─────────────────────────────────────────────────────────────────…
* `const u32 RGB = 0x02000000` — The same bit, spelled for the theme tables below. A `const` folds and a call…
* `const Color DEFAULT = Color(KIND_NONE)` — The terminal's own foreground or background — whatever the user configured.
* `Color rgb(u32 r, u32 g, u32 b)` — A 24-bit colour from three components.
* `Color hex(u32 v)` — A 24-bit colour written the way a designer writes one: `hex(0x89B4FA)`.
* `Color ansi(u32 i)` — A 256-colour palette index. 0..7 are the terminal's own eight, 8..15 their…
* `bool is_default(Color c)`
* `Color mix(Color a, Color b, u32 t)` — Blend two colours, `t` in 0..255 parts of `b`. Used for the surface ramp's…
* `distinct enum u16 Attr` — ── attributes ─────────────────────────────────────────────────────────────…
* `bool has_attr(Attr set, Attr one)`
* `type Style` — ── style ──────────────────────────────────────────────────────────────────…
* `const Style PLAIN = (DEFAULT, DEFAULT, Attr.NONE)`
* `Style styled(Color fg, Color bg = DEFAULT, Attr attr = Attr.NONE)`
* `type BorderSet` — ── borders ────────────────────────────────────────────────────────────────…
* `const BorderSet ROUND = ("╭", "╮", "╰", "╯", "─", "│", "├", "┤", "┬", "┴", "┼")` — Rounded corners, light weight. The default, because a 1px-looking box with…
* `const BorderSet SINGLE = ("┌", "┐", "└", "┘", "─", "│", "├", "┤", "┬", "┴", "┼")`
* `const BorderSet DOUBLE = ("╔", "╗", "╚", "╝", "═", "║", "╠", "╣", "╦", "╩", "╬")`
* `const BorderSet THICK = ("┏", "┓", "┗", "┛", "━", "┃", "┣", "┫", "┳", "┻", "╋")`
* `const BorderSet ASCII = ("+", "+", "+", "+", "-", "|", "+", "+", "+", "+", "+")` — For a terminal whose font has no box drawing, and for `TERM=dumb`.
* `type Theme` — ── the theme ──────────────────────────────────────────────────────────────…
* `` const Theme DARK = ( Color(RGB | 0x11111B), // base Color(RGB | 0x0D0D14), // mantle Color(RGB | 0x181825), // surface Color(RGB | 0x1E1E2E), // overlay Color(RGB | 0xCDD6F4), // text Color(RGB | 0xA6ADC8), // subtext Color(RGB | 0x7F849C), // muted Color(RGB | 0x45475A), // faint Color(RGB | 0x89B4FA), // accent Color(RGB | 0x11111B), // on_accent Color(RGB | 0xA6E3A1), // ok Color(RGB | 0xF9E2AF), // warn Color(RGB | 0xF38BA8), // err Color(RGB | 0x94E2D5), // info Color(RGB | 0x313244), // border — one step above `faint`, deliberately quiet Color(RGB | 0x89B4FA), // border_focus ROUND, true, ) `` — The default. A cool, low-chroma dark palette with a genuine four-step…
* `const Theme LIGHT = ( Color(RGB | 0xEFF1F5), // base Color(RGB | 0xE6E9EF), // mantle Color(RGB | 0xFFFFFF), // surface Color(RGB | 0xFFFFFF), // overlay Color(RGB | 0x4C4F69), // text Color(RGB | 0x5C5F77), // subtext Color(RGB | 0x8C8FA1), // muted Color(RGB | 0xBCC0CC), // faint Color(RGB | 0x1E66F5), // accent Color(RGB | 0xFFFFFF), // on_accent Color(RGB | 0x40A02B), // ok Color(RGB | 0xDF8E1D), // warn Color(RGB | 0xD20F39), // err Color(RGB | 0x179299), // info Color(RGB | 0xCCD0DA), // border Color(RGB | 0x1E66F5), // border_focus ROUND, true, )` — The same ramp inverted, for a light terminal. Not an afterthought: the…
* `const Theme MONO = ( DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, ASCII, false, )` — No colour at all. For a pipe, for CI, for `TERM=dumb`, and for anybody who…
* `void theme(Theme t)` — Install a theme. Takes effect on the next frame.
* `Theme now()` — The theme in force, for a widget or a program that wants one of its colours.
* `type COORD: (#c.short X, #c.short Y)`
* `type SMALL_RECT: (#c.short Left, #c.short Top, #c.short Right, #c.short Bottom)`
* `type CONSOLE_SCREEN_BUFFER_INFO`
* `type INPUT_RECORD`
* `const rawptr INVALID_HANDLE_VALUE`
* `const #c.ulong STD_INPUT_HANDLE`
* `const #c.ulong STD_OUTPUT_HANDLE`
* `const #c.ulong WAIT_OBJECT_0`
* `const #c.ulong INFINITE`
* `const #c.ulong ENABLE_PROCESSED_INPUT`
* `const #c.ulong ENABLE_LINE_INPUT`
* `const #c.ulong ENABLE_ECHO_INPUT`
* `const #c.ulong ENABLE_QUICK_EDIT_MODE`
* `const #c.ulong ENABLE_EXTENDED_FLAGS`
* `const #c.ulong ENABLE_VIRTUAL_TERMINAL_INPUT`
* `const #c.ulong ENABLE_PROCESSED_OUTPUT`
* `const #c.ulong ENABLE_VIRTUAL_TERMINAL_PROCESSING`
* `const #c.uint CP_UTF8`
* `rawptr GetStdHandle(#c.ulong which)`
* `#c.int GetConsoleMode(rawptr console, ^#c.ulong mode)`
* `#c.int SetConsoleMode(rawptr console, #c.ulong mode)`
* `#c.int SetConsoleCtrlHandler(TtyCtrlHandler handler, #c.int add)`
* `#c.int GetConsoleScreenBufferInfo(rawptr console, ^CONSOLE_SCREEN_BUFFER_INFO info)`
* `#c.int GetNumberOfConsoleInputEvents(rawptr console, ^#c.ulong count)`
* `#c.int ReadConsoleInputW(rawptr console, ^INPUT_RECORD records, #c.ulong count, ^#c.ulong read)`
* `#c.ulong WaitForSingleObject(rawptr handle, #c.ulong milliseconds)`
* `#c.int WriteConsoleA(rawptr console, const rawptr buffer, #c.ulong count, ^#c.ulong written, rawptr reserved)`
* `#c.int WideCharToMultiByte(#c.uint code_page, #c.ulong flags, ^const u16 from, #c.int from_count, ^#c.char into, #c.int into_count, cstring default_char, ^#c.int used_default)`
* `type @c("sig_atomic_t") SigAtomic: #c.int`
* `const i32 SIGINT`
* `const i32 SIGTERM`
* `const i32 SIGHUP`
* `const i32 SIGTSTP`
* `const i32 SIGCONT`
* `const TtyHandler SIG_DFL`
* `TtyHandler signal(i32 sig, TtyHandler handler)`
* `i32 raise(i32 sig)`
* `type @c("sig_atomic_t") SigAtomic: #c.int`
* `const i32 SIGINT`
* `const i32 SIGTERM`
* `const i32 SIGHUP`
* `const i32 SIGTSTP`
* `const i32 SIGCONT`
* `const TtyHandler SIG_DFL`
* `TtyHandler signal(i32 sig, TtyHandler handler)`
* `i32 raise(i32 sig)`
* `const i32 errno`
* `const i32 EINTR`
* `const i32 STDIN_FILENO`
* `const i32 STDOUT_FILENO`
* `i32 isatty(i32 fd)`
* `#c.long read(i32 fd, rawptr buf, #c.size_t count)`
* `#c.long write(i32 fd, const rawptr buf, #c.size_t count)`
* `const i32 STDIN_FILENO`
* `const i32 STDOUT_FILENO`
* `i32 isatty(i32 fd)`
* `#c.long read(i32 fd, rawptr buf, #c.size_t count)`
* `#c.long write(i32 fd, const rawptr buf, #c.size_t count)`
* `type TermiosT`
* `const #c.ulong ECHO`
* `const #c.ulong ICANON`
* `const #c.ulong IXON`
* `const #c.ulong ICRNL`
* `const i32 VMIN`
* `const i32 VTIME`
* `const i32 TCSAFLUSH`
* `i32 tcgetattr(i32 fd, ^TermiosT t)`
* `i32 tcsetattr(i32 fd, i32 actions, ^const TermiosT t)`
* `type TermiosT`
* `const u32 ECHO`
* `const u32 ICANON`
* `const u32 IXON`
* `const u32 ICRNL`
* `const i32 VMIN`
* `const i32 VTIME`
* `const i32 TCSAFLUSH`
* `i32 tcgetattr(i32 fd, ^TermiosT t)`
* `i32 tcsetattr(i32 fd, i32 actions, ^const TermiosT t)`
* `type WinSize: (u16 ws_row, u16 ws_col, u16 ws_xpixel, u16 ws_ypixel)`
* `const #c.ulong TIOCGWINSZ`
* `i32 ioctl(i32 fd, #c.ulong request, ...)`
* `type TimeVal: (#c.long tv_sec, #c.int tv_usec)`
* `type TimeVal: (#c.long tv_sec, #c.long tv_usec)`
* `type @c("fd_set") FdSet`
* `@macro void FD_ZERO(^FdSet set)`
* `@macro void FD_SET(i32 fd, ^FdSet set)`
* `@macro i32 FD_ISSET(i32 fd, ^FdSet set)`
* `type @c("fd_set") FdSet`
* `@macro void FD_ZERO(^FdSet set)`
* `@macro void FD_SET(i32 fd, ^FdSet set)`
* `@macro i32 FD_ISSET(i32 fd, ^FdSet set)`
* `i32 select(i32 nfds, ^FdSet readfds, ^FdSet writefds, ^FdSet exceptfds, ^TimeVal timeout)`
* `i32 select(i32 nfds, ^FdSet readfds, ^FdSet writefds, ^FdSet exceptfds, ^TimeVal timeout)`
* `type Frame: void()` — What you hand `run` and `step`. It takes nothing and returns nothing,…
* `(rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) mem` — ── storage ────────────────────────────────────────────────────────────────…
* `bool is_terminal()` — Whether there is a terminal to draw on, asked without changing anything. A…
* `bool open(i32 pad = 1)` — Enter raw mode, switch to the alternate screen, turn on SGR mouse reporting…
* `bool open_over((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) backing, i32 pad = 1)` — The same, over an allocator you supply. Everything the library allocates —…
* `void close()` — Put the terminal back, in reverse. Safe to call twice, and `tty_onsignal`
* `void quit()` — End the loop after this frame. The ordinary way a program stops.
* `void wake()` — Ask for a repaint. A program whose state changed outside a frame — a child…
* `bool closed()` — Whether stdin has ended: Ctrl-D, or a closed pipe. A loop that ignores this…
* `f64 clock()` — Seconds since `open`. The clock every animated widget in the library reads,…
* `void suspend()` — Put the terminal back the way it was found, without closing the UI. The…
* `void resume()` — Take it back. The next frame is a full repaint, because whatever ran in…
* `bool suspended()` — Whether the UI is currently handed over.
* `(bool exited, i32 status) shell_out(cstring command)` — Run `command` with the terminal, and take it back afterwards.
* `void stop()` — Stop the way Ctrl-Z does, and come back when continued.
* `bool open_headless(i32 w, i32 h, i32 pad = 1)` — Open a UI of a fixed size with no terminal anywhere: no raw mode, no…
* `bool open_headless_over(i32 w, i32 h, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) backing, i32 pad = 1)`
* `void compose(Frame f)` — Run one frame into the back buffer, without presenting it. Pairs with…
* `bool step(Frame f, i32 timeout_ms)` — One iteration: wait up to `timeout_ms` for input, then compose a frame if…
* `(i32 frames, f64 last_ms, f64 worst_ms, f64 mean_ms, i32 over) frame_times()` — The frames composed since `open`, the last one's time, the worst and the…
* `void frame_budget(f64 ms)` — The time a frame may take before `frame_times` counts it as over.
* `void run(Frame f)` — The whole loop, until `quit`. Sleeps when nothing is happening: the timeout…
* `bool resized()` — Re-read the window size; true when it changed.
* `Rect rect_of()`
* `bool hovered()`
* `bool focused()`
* `bool clicked()` — Was the last widget placed clicked on this frame? The companion to…
* `void text(string8 s, Color color = DEFAULT, Attr attr = Attr.NONE, Anchor anchor = Anchor.START)` — One line of text, clipped to the region.
* `void title(string8 s)` — A heading. Bold, in the theme's loudest text, with a blank line under it —…
* `void label(string8 s)` — Quieter than `text`. Captions, units, the second half of a sentence.
* `void muted(string8 s)`
* `i32 paragraph(string8 s, Color color = DEFAULT)` — Word-wrapped to the region's width. Answers how many rows it took.
* `void kv(string8 key, $T value)` — An aligned key/value line: the key in the quiet text colour, a run of dots,…
* `void kv_text(string8 key, string8 value)` — The same, when the value is already a string.
* `void kv_float(string8 key, $T value, i32 decimals = 2) where T is float` — A key/value line whose value is a number written to a fixed number of…
* `void badge(string8 s, Color color = DEFAULT)` — A pill. Reads as a status rather than as a word, which is what a colour on…
* `void badge_quiet(string8 s)` — A badge for a state that is not news: the surface ramp rather than a hue,…
* `void dot(Color color)` — A status pip. Two cells: the dot and the space after it, so a row of them…
* `void spinner(string8 caption = "")` — A spinner, optionally captioned. Registers an animation, so the loop stops…
* `void progress(f32 value, string8 caption = "", i32 width = REST)` — A progress bar. Sub-cell resolution when the theme allows wide glyphs, which…
* `bool button(string8 label, i32 width = AUTO, bool enabled = true, string8 hint = "")` — A button. `true` on the frame it is activated, by a click or by Enter or…
* `bool checkbox(string8 label, ^bool on)` — A checkbox. `on` is yours; the answer is whether it changed on this frame.
* `bool radio(string8 label, ^i32 choice, i32 value)` — One of a set. `choice` is yours and holds the selected `value`.
* `void hints()` — Every binding registered this frame, along one row. A program that binds its…
