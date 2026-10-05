<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:cfg

core:cfg — a native INI reader/writer over core:io and core:os/fs.

    import "core:cfg"
    import "core:mem"

    (cfg.Config c, bool ok, i32 bad) = cfg.load("settings.ini", alloc)
    defer cfg.destroy(&c)
    i32 width = cfg.get_i32(c, "window", "width", 1280, alloc)
    cfg.set_i32(&c, "window", "width", width, alloc)
    cfg.save(c, "settings.ini", alloc)

The format is INI, and a vendored C parser was ruled out in favour of
this one — zero dependency, and the result is a queryable
Dado value rather than an opaque handle behind a getter that only speaks
`"section:key"` strings.

**The format.** `[section]` headers, `key = value` lines, `;` and `#`
comments, blank lines — the common subset every INI reader agrees on.
Whitespace around `=` and around a section name is trimmed; nothing else
is. Lines before the first header land in the `""` section, so a caller
that never writes `[section]` still has a flat key=value file.

**Forgiving on read, exact on write.** A hand-edited config is not a wire
protocol: a stray line that isn't a comment, a header or a `key = value`
pair is skipped rather than failing the whole load, the same stance
`numbers.to_i32` (`core:strings/parse`) takes on unparsable text.
`bad_lines` says how many were dropped and `ok` is `bad_lines == 0`, so a
caller that cares can still tell the difference between "booted clean" and
"booted anyway".

**Every value a `Config` holds is its own allocation, and `destroy` ends
them.** Section and key names become map keys, and a map already copies
those on a fresh insert and frees them at `#delete`; the *value* half of
every entry is a `ref []char8` the map neither copies nor frees, so
`parse`/`set_*` clone it in and `destroy(&c)` frees every value, every
section's map and the outer map. A `set_*` over a key that already has a
value frees the old one first, so a reload or an overwrite does not leak.
The result is a `Config` that does not borrow from the text it was parsed
from — `load` frees its read buffer the moment `parse` returns — and one
that can be ended: **the caller that made it calls `destroy`**, or made it
under a `using` arena whose end is the free. `get`/`get_string` answer a
`string8` **window** into the stored value, good until that key is set
again or the `Config` is destroyed.

**Why the value type is `ref []char8` and not `string8`**:
a `string8` is a window, `#delete` of a window is ERR0504, and so a value
stored as one could be freed by nobody — `clone` minted a `ref`, returned a
window over it, and the `ref`, the only thing with authority to free, died
at the end of `clone`. LeakSanitizer measured it: every value of every
`Config` ever parsed, and a program reloading its config leaked the whole
config each time. Storing the `ref` keeps the authority where `destroy` can
reach it; the getters still answer `string8`, so no reader changed.

## Declarations

22 declarations, 21 public.

* `type Config: {string8: {string8: ref []char8}}` — section -> key -> value. `""` is the implicit default section. Each value…
* `type Bytes: []char8` — `string ⇄ []char8` is free, but `[]T` is not a callable…
* `Config fresh((rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — A fresh, empty config. The allocator is explicit, not `#default`
* `ref []char8 clone(string8 s, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — A value's own copy, independent of whatever `s` was a window into. Every…
* `void destroy(^Config cfg)` — End a `Config`: every value, every section's map, and the outer map. `cfg`
* `(string8 name, bool ok) parse_section(string8 ln)` — `[name]`, trimmed and unwrapped. False on anything that isn't a `[...]`
* `(string8 key, string8 val, bool ok) parse_kv(string8 ln)` — `key = value`, split on the first `=`. False when there is no `=` at all…
* `(Config cfg, bool ok, i32 bad_lines) parse(string8 text, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — Parse INI text into a `Config`. See the package doc comment for the format…
* `bool exists(cstring path)` — Whether `path` names a file `load` would actually read (as opposed to…
* `(Config cfg, bool ok, i32 bad_lines) load(cstring path, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — Read `path` and parse it. A missing file is not an error — it returns a…
* `void write_pairs(^io.FILE f, {string8: ref []char8} pairs)` — Write every key of one section (no header) — the shared body `save` uses…
* `bool save(Config cfg, cstring path, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — Write `cfg` to `path` as INI text: the default section's keys first with…
* `(string8 val, bool found) get(Config cfg, string8 section, string8 key)` — The stored value as a window into the `Config`'s own run: good until the…
* `string8 get_string(Config cfg, string8 section, string8 key, string8 fallback)`
* `i32 get_i32(Config cfg, string8 section, string8 key, i32 fallback, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)`
* `f64 get_f64(Config cfg, string8 section, string8 key, f64 fallback, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — `fallback` too when the value is not exactly a number: `to_f64` is strict,…
* `bool get_bool(Config cfg, string8 section, string8 key, bool fallback)` — Accepts true/1/yes/on and false/0/no/off (case-sensitive — a hand-written…
* `void set_string(^Config cfg, string8 section, string8 key, string8 val, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — **The map is taken by pointer, and it has to be.** A `Config` is a map, a…
* `void set_i32(^Config cfg, string8 section, string8 key, i32 val, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)`
* `void set_f64(^Config cfg, string8 section, string8 key, f64 val, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)`
* `void set_bool(^Config cfg, string8 section, string8 key, bool val, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)`
