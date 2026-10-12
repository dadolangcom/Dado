<!-- dadoc 1.0.0-rc.4 -->
<!-- commit d5b92d013902 -->
# core:cfg

core:cfg — INI text to a queryable `Config` and back, and the same to and
from a file.

    import "core:cfg"

    (cfg.Config c, bool ok, i32 bad) = cfg.load("settings.ini")
    defer cfg.destroy(&c)
    i32 width = cfg.get_i32(c, "window", "width", 1280)
    cfg.set_i32(&c, "window", "width", width)
    _ = cfg.save(c, "settings.ini")

    try string8 title = cfg.get(c, "window", "title") else why:
        println(cfg.message(why))       // "no such key in that section"
        return 1

**Tier N W H, the file layer W H. Script class H.** The text half —
`parse`, `render`, `render_over`, the getters, setters and removers — is
Dado over `core:strings`, `core:strings/parse` and `core:slices`, and
builds and runs freestanding, on the web and hosted. The file half
(`load`, `save`, `exists`, in `file.dado`) is a thin layer over `core:io`
on top of it: it is not declared on `#NONE`, which has no files, and on the
web it answers what `core:io` does there — no file opens yet, so `load`
answers an empty `Config` and `save` answers false.

**What raises into a script.** Every function of the text half, each
promised by the `#assert_raises` above it. The file layer is not: it
reaches C's streams through `core:io`.

The format is INI, and a vendored C parser was ruled out in favour of
this one — zero dependency, and the result is a queryable
Dado value rather than an opaque handle behind a getter that only speaks
`"section:key"` strings.

**The format.** `[section]` headers, `key = value` lines, `;` and `#`
comments on lines of their own, blank lines — the common subset every INI
reader agrees on. Whitespace around `=` and around a section name is
trimmed; nothing else is, and a `;` or `#` after a value is part of the
value. Lines before the first header land in the `""` section, so a caller
that never writes `[section]` still has a flat key=value file.

**Quoted values.** A value that starts with `"` is quoted: it ends at the
matching `"`, which must end the line, and `\"`, `\\`, `\n`, `\r` and `\t`
inside it are escapes. That is how a value keeps leading or trailing
spaces, a line break or a leading quote. `render` quotes exactly the values
that need it, so every value round-trips; a value that merely contains a
`"` or a `\` after its first byte is written bare.

**Forgiving on read, exact on write.** A hand-edited config is not a wire
protocol: a stray line that isn't a comment, a header or a `key = value`
pair — or a quoted value that does not close — is skipped rather than
failing the whole parse. `bad_lines` says how many were dropped, so a
caller that cares can still tell "booted clean" (0) from "booted anyway".

**Comments survive a round trip.** `render(c)` writes a `Config` out fresh,
in a fixed order: the `""` section's keys, then each section, both sorted
by name. `render_over(c, original)` writes it over the text it was parsed
from instead: every comment, blank line and unparsed line of a section
that still exists stays where it was, every key whose value did not change
keeps its line byte for byte, a changed value is rewritten in place, a
removed key or section is dropped, and new keys go after the last line of
their section, new sections at the end. `save` renders over the file
already at the path, so a hand-written file keeps its comments and its
order through a load, a set and a save.

**Every value a `Config` holds is its own allocation, and `destroy` ends
them.** Section and key names become map keys, and a map already copies
those on a fresh insert and frees them at `#delete`; the *value* half of
every entry is a `ref []char8` the map neither copies nor frees, so
`parse` and the setters copy it in and `destroy(&c)` frees every value,
every section's map and the outer map. A setter over a key that already
has a value frees the old one first, so a reload or an overwrite does not
leak. The result is a `Config` that does not borrow from the text it was
parsed from, and one that can be ended: **the caller that made it calls
`destroy`**, or made it under a `using` arena whose end is the free.
`get` and `get_string` answer a `string8` **window** into the stored
value, good until that key is set again or removed, or the `Config` is
destroyed.

**Why the value type is `ref []char8` and not `string8`**: a `string8` is
a window, `#delete` of a window is ERR0504, and so a value stored as one
could be freed by nobody. Storing the `ref` keeps the authority where
`destroy` can reach it; the getters still answer `string8`.

**Allocation is ambient.** Everything this package allocates comes from
`#default`, so `cfg.parse(text) using arena.allocator(&a)` puts the whole
`Config` in `a`; a value set later is copied onto whatever the setter is
called under.

**A failed allocation drops what it was for, and nothing else.** A setter
whose copy cannot be made leaves the key as it was (the old value, or
absent); `parse` counts a line whose value or section cannot be stored in
`bad_lines`; `render` and `render_over` answer `nil`.

**Thread-safety.** No package state. A `Config` is a map: one thread at a
time may change one, and any number may read one nobody is changing.

**Error codes: `Error`, 1300 to 1399**, a range no other package's codes
share, and only `get` fails. A malformed line is counted, a typed getter
answers the caller's fallback for a missing key, and the file layer
answers `bool`s.

## Declarations

48 declarations, 26 public.

* `type Config: {string8: {string8: ref []char8}}` — section -> key -> value. `""` is the implicit default section. Each value…
* `distinct enum i32 Error` — Why `get` failed. The numbers are this package's, 1300 to 1399, apart from…
* `string8 message(Error why)` — A sentence for each of this package's codes, and one for any other value.
* `Config empty()` — An empty `Config`, its table on `#default`. `destroy` ends it.
* `void destroy(^Config cfg)` — End a `Config`: every value, every section's map, and the outer map. `cfg`
* `(Config cfg, i32 bad_lines) parse(string8 text)` — Parse INI text into a new `Config` on `#default`. See the package comment…
* `ref []char8 render(Config cfg)` — `cfg` as INI text, in a new run on `#default` the caller `#delete`s: the…
* `ref []char8 render_over(Config cfg, string8 original)` — `cfg` written over `original`, the text it was parsed from, in a new run…
* `Error!string8 get(Config cfg, string8 section, string8 key)` — The stored value, as a window into the `Config`'s own run: good until the…
* `string8 get_string(Config cfg, string8 section, string8 key, string8 fallback)` — The stored value, or `fallback` for a missing key: `get` with the answer…
* `i32 get_i32(Config cfg, string8 section, string8 key, i32 fallback)` — The stored value as an `i32` when it is exactly a decimal integer in range…
* `i64 get_i64(Config cfg, string8 section, string8 key, i64 fallback)` — The stored value as an `i64` when it is exactly a decimal integer in range,…
* `u64 get_u64(Config cfg, string8 section, string8 key, u64 fallback)` — The stored value as a `u64` when it is exactly a decimal integer from 0 to…
* `f64 get_f64(Config cfg, string8 section, string8 key, f64 fallback)` — The stored value as an `f64` when it is exactly a number…
* `bool get_bool(Config cfg, string8 section, string8 key, bool fallback)` — `true` for true/1/yes/on and `false` for false/0/no/off (case-sensitive —…
* `void set_string(^Config cfg, string8 section, string8 key, string8 val)` — Store `val` under `section` and `key`, creating either as needed.
* `void set_i32(^Config cfg, string8 section, string8 key, i32 val)` — Store `val` in decimal.
* `void set_i64(^Config cfg, string8 section, string8 key, i64 val)` — Store `val` in decimal; `get_i64` reads it back exactly.
* `void set_u64(^Config cfg, string8 section, string8 key, u64 val)` — Store `val` in decimal; `get_u64` reads it back exactly.
* `void set_f64(^Config cfg, string8 section, string8 key, f64 val)` — Store `val` in the shortest decimal that reads back as the same `f64`.
* `void set_bool(^Config cfg, string8 section, string8 key, bool val)` — Store `true` or `false`.
* `bool remove_key(^Config cfg, string8 section, string8 key)` — Remove `key` from `section`, freeing its value, and answer whether it was…
* `bool remove_section(^Config cfg, string8 section)` — Remove `section` and every key in it, freeing their values, and answer…
* `bool exists(string8 path)` — Whether `path` names a file `load` would read: one that opens and…
* `(Config cfg, bool ok, i32 bad_lines) load(string8 path)` — Read `path` and parse it onto `#default`; the caller ends the `Config`
* `bool save(Config cfg, string8 path)` — Write `cfg` to `path` as INI text. When a file is already there, `cfg`
