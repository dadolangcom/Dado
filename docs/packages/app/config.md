<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:config

app:config — self-provisioning settings, over core:cfg.

    import "app:config"

    void seed(^config.Config c):
        config.set_i32(c, "window", "width", 1280)
        config.set_i32(c, "window", "height", 720)

    (config.Config c, bool created, bool ok, i32 bad) = config.load("settings.ini", seed)
    defer config.destroy(&c)
    i32 width = config.get_i32(c, "window", "width", 1280)

**Self-provisioning** is the whole point: `load` on a path with
nothing there yet calls the caller's `DefaultsFn` to seed sane values into
a fresh `Config`, writes it, and hands that back with `created` true. A
later run finds the file and just reads it — same call either way, so a
program does not carry two code paths for "first run" vs "every run
after." `example/` demonstrates the round-trip end to end.

**Neutral**: this imports `app:app` for its persistent allocator
and nothing backend-specific, so a server reads config the same way a
terminal or graphical program does.

**Opinion over mechanism.** Everything here forwards to `core:cfg`, fixing
one allocator (`app.allocator()`) so a program never plumbs one through —
that is the entire value add. A program that wants its own allocator, or
the untyped `Config` value's own shape, reaches for `core:cfg` directly;
nothing here is otherwise reachable only through this package.

**Where a `Config` lands, and how a caller says so.** `core:cfg` takes an
allocator in every signature, which is the convention this collection does
not use: an allocator is not threaded through parameter lists here, it is
the ambient one and `using` is how it is changed. So the two forms this
package offers are the ambient and the opinion, and neither is a parameter:

    config.load(path, seed)                              // app.allocator()
    config.load_with(path, seed)                         // the caller's own default
    config.load_with(path, seed) using arena.allocator(&a)   // in `a`

`load` is now literally `load_with` under a `using`, which is the whole of
what "fixing one allocator" means and is one line rather than an argument
threaded down. `load_with` writes `#default` where `core:cfg` insists on a
value, and `#default` *is* the ambient — so the caller's `using` reaches the
`core:cfg` call underneath without this file naming a place.

**Who ends a `Config`.** The caller that loaded it, with `destroy(&c)` —
every value, every section and the map, wherever `load`/`load_with` put
them; or, for one loaded under a `using` arena, the arena's own end, and no
`destroy` at all. A `set_*` over an existing key frees the value it
replaces, so a program that reloads or rewrites its settings holds one
config's worth, not one per reload (`core:cfg` has the why).
One seam: the `set_*` forwarders below allocate from `app.allocator()`, not
the ambient, so a value *set* on an arena-loaded `Config` is not the
arena's — `destroy` such a config too, before the arena ends (a `Free`
through an arena is a no-op, so that is safe).

`DefaultsFn` lost its allocator for the same reason. A seed function is
called from inside `load_with`, under whatever the caller arranged, so an
allocator parameter on it was a value with exactly one correct argument.

## Declarations

14 declarations, 14 public.

* `type Config: cfg.Config` — section -> key -> value: **`core:cfg`'s `Config` itself**, by a qualified…
* `type DefaultsFn: void(^Config)` — Called once, only when `load` finds nothing at the path, to seed a fresh…
* `(Config c, bool created, bool ok, i32 bad_lines) load(cstring path, DefaultsFn defaults)` — Load `path` with `app.allocator()`. See `load_with` for the form that takes…
* `(Config c, bool created, bool ok, i32 bad_lines) load_with(cstring path, DefaultsFn defaults)` — Load `path`, or self-provision it. If `path` exists, this is exactly…
* `bool save(Config c, cstring path)` — Write `c` back to `path` — after a program has changed a value at…
* `void destroy(^Config c)` — End `c`: every value, every section's map, and the map. See the package…
* `string8 get_string(Config c, string8 section, string8 key, string8 fallback)`
* `i32 get_i32(Config c, string8 section, string8 key, i32 fallback)`
* `f64 get_f64(Config c, string8 section, string8 key, f64 fallback)`
* `bool get_bool(Config c, string8 section, string8 key, bool fallback)`
* `void set_string(^Config c, string8 section, string8 key, string8 val)` — The four setters take the map **by pointer**, following `core:cfg`'s, and…
* `void set_i32(^Config c, string8 section, string8 key, i32 val)`
* `void set_f64(^Config c, string8 section, string8 key, f64 val)`
* `void set_bool(^Config c, string8 section, string8 key, bool val)`
