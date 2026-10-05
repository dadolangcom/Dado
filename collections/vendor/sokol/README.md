# `vendor:sokol`

Dado bindings for [sokol](https://github.com/floooh/sokol) — Andre Weissflog's
single-file C libraries (zlib licensed; see `LICENSE`). Drafted by Dado's
binding generator from the vendored headers, then edited by hand, and verified
by compiling against them.

## Layout

One package per directory — a package is a directory — each holding two files:
the binding and the header it restates.

```
vendor/sokol/
  LICENSE
  README.md
  sokol_gfx/   sokol_gfx.dado   sokol_gfx.h
  sokol_app/   sokol_app.dado   sokol_app.h
  sokol_glue/  sokol_glue.dado  sokol_glue.h
  sokol_audio/ …
  sokol_time/  …
  sokol_args/  …
  sokol_log/   …
  sokol_fetch/ …
```

**There is no third file and there used to be two.** A `.c` implementation
sidecar sat beside each binding until 2026-09-06 and a `link` file beside that
until 2026-09-01; both are gone, and the sections below are what replaced them.

Import each by its path; the qualifier is the package name:

```dado
import "vendor:sokol/sokol_time"

sokol_time.stm_setup()
u64 t = sokol_time.stm_now()
```

## How sokol is built here

sokol is **header-only**: each `.h` is both the API and the implementation, and
`SOKOL_IMPL` is what turns the second one on. Every binding carries it as a
`define` clause on its own `foreign` block:

```dado
foreign "sokol_gfx.h" define "SOKOL_IMPL":
```

A `define` clause is emitted **above every `#include`** in Dado's generated C, not
merely before its own header, so the implementation compiles inside Dado's single
translation unit. Nothing else is compiled and nothing else is linked in — which
is the whole reason the `.c` sidecars are gone, and the way to see it is the
build manifest:

```
$ ./dado emit example/gfxsmoke --emit=build -D BACKEND=dummy
target x86_64-pc-linux-gnu
os #LINUX
arch #X86_64
include /home/…/example/gfxsmoke
include /home/…/collections/vendor/sokol/sokol_gfx
header sokol_gfx.h
live main
live sokol_gfx
```

**No `source` line.** A `source` line is a second translation unit the driver
compiles and links; one there means a sidecar came back.

The same clause set carries the POSIX feature-test macros the GNU libc build
needs, under a `when` that keeps them off Apple and Windows — where
`_POSIX_C_SOURCE` alone *hides* the platform APIs sokol reaches for:

```dado
when #OS != #DARWIN && #OS != #WINDOWS:
    foreign "sokol_gfx.h" define "_POSIX_C_SOURCE=200809L" define "_DEFAULT_SOURCE=1"
```

`sokol_app` adds `define "SOKOL_NO_ENTRY"`, because a Dado program has its own
`main` and calls `sapp_run` from it.

`sokol_glue` is the one package with an ordering problem rather than a macro
problem: `sokol_glue.h` `#error`s if `sokol_gfx.h` has not been read before it,
and its implementation `#error`s if `sokol_app.h` has not. Headers are emitted in
the order their `foreign` blocks are written, so `sokol_glue.dado` writes two
clause-only lines for gfx and app above its own block and the order holds
whatever order a program's `import` lines take.

## Backend selection

**The render backend is a Dado value.** `sokol_gfx` declares it:

```dado
@const string BACKEND = "auto"
```

and a `when` chain beside it turns that value into the backend macro, the
libraries it costs, and — on Apple — the one compiler flag it needs. A `-D`
replaces it at the build, either spelling:

```sh
./dado run example/gfxsmoke -D BACKEND=dummy              # bare
./dado run example/gfxsmoke -D sokol_gfx.BACKEND=dummy    # qualified
```

| `BACKEND` | macro | notes |
|---|---|---|
| `"auto"` (default) | by `#OS` | Metal on Apple, D3D11 on Windows, GLES3 under Emscripten, GL elsewhere |
| `"gl"` | `SOKOL_GLCORE` | |
| `"gles3"` | `SOKOL_GLES3` | |
| `"metal"` | `SOKOL_METAL` | |
| `"d3d11"` | `SOKOL_D3D11` | |
| `"wgpu"` | `SOKOL_WGPU` | the WebGPU implementation library is the program's to choose |
| `"vulkan"` | `SOKOL_VULKAN` | experimental in the header |
| `"dummy"` | `SOKOL_DUMMY_BACKEND` | headless: no GPU, no window, no libraries |

**There is one `BACKEND` for the whole collection and there cannot be two.**
`sokol_app` reads `sokol_gfx.BACKEND` rather than declaring its own: the window
and the renderer create and draw into one context, so one constant has to decide
for both. It is also what the compiler requires — with two packages in a
compilation declaring that name, a bare `BACKEND` in a `when` is `ERR0903`
(*"declared by more than one package … one name, two answers"*) and a bare
`-D BACKEND=` is `ERR1103`. The bare form above works because this is the only
`@const` named `BACKEND` in the tree; the qualified form works whether or not
that stays true.

The chain ends on `== "auto"` rather than on a bare `else`, so a value it does
not know defines no backend macro at all and sokol answers for itself —
*"Please select a backend with SOKOL_GLCORE, …"*. A misspelt backend silently
picking GL is the failure that shape refuses.

**There is no `sokol_app` under `"dummy"`.** The dummy renderer is headless and
sokol_app has no headless mode, so a program that imports `sokol_app` under a
dummy selection gets sokol_app.h's own *"unknown 3D API selected"*. A headless
program imports `sokol_gfx` and stops there — `example/gfxsmoke` is the worked
case.

## Link dependencies

sokol needs no third-party libraries, only OS frameworks and libs. Each is a
`link` clause on the arm of the `when` that chose it, so a build links what its
backend actually uses and nothing else:

```
$ ./dado emit example/gfxsmoke --emit=build -D BACKEND=gl    | grep link
link -lGL
$ ./dado emit example/gfxsmoke --emit=build -D BACKEND=dummy | grep link
$
```

That second command printing nothing is the point. Until the backend became a
Dado value it was a C macro passed in `DADO_CFLAGS`, which a `when` cannot see, so
Linux linked `-lGL` into every build including the headless ones.

`sokol_app`'s cost splits in two, because the two halves are keyed on different
things: the windowing libraries are the platform's and are the same whichever
backend runs (`-lX11 -lXi -lXcursor -lm -ldl -lpthread` on Linux,
`-framework Cocoa -framework QuartzCore` on macOS,
`-lkernel32 -luser32 -lshell32 -lgdi32` under MinGW), while the 3D library
follows `sokol_gfx.BACKEND`.

Windows arms name their libraries with `-l` because `./dado` builds Windows
through MinGW gcc, which reads none of the headers' `#pragma comment(lib, …)`
lines.

## `-Wno-pedantic`, and which builds pay for it

`sokol_app`'s GLX loader resolves nineteen entry points with `dlsym` and
`glXGetProcAddress` and casts each between an object pointer and a function
pointer — which is what those two calls are *for*, and which ISO C does not
define. gcc refuses every one under `-Werror=pedantic`, which `./dado` passes by
default; clang does not diagnose it at all. Measured on a program whose only
import is `vendor:sokol/sokol_app`: 19 errors under gcc, 0 under clang.

It is vendor code and the conversion is correct, so the repair is a clause and
not an edit to the header:

```dado
foreign "sokol_app.h" define "SOKOL_GLCORE" flags "-Wno-pedantic" link "-lGL"
```

**There is no narrower flag.** gcc names the diagnostic `[-Wpedantic]` itself,
so there is nothing more specific to switch off; `-std=gnu99` still errors 19
times because `-pedantic` outranks the dialect, and `-Wno-error=pedantic` buys
the same weaker bar plus 19 warnings on every build. What would be narrow is
`-isystem` on the package directory instead of `-I` — the emitted include is
`<sokol_app.h>`, so gcc would exempt the header and hold Dado's own emitted C to
`-pedantic` as before — but a `flags` string is a literal and a package cannot
spell its own absolute path, so that is a `./dado` change and not a clause one.

So the narrowing is by **configuration**, and it is taken: the clause is on the
two GLCORE arms and on the Windows platform arm, and nowhere else.

| build | `-pedantic` | why |
|---|---|---|
| GLCORE (`"gl"`, and `"auto"` on Linux) | **off** | the GLX loader; measured, 19 gcc errors without it |
| `"gles3"` | on | the EGL loader casts nothing; measured clean under gcc |
| `"metal"`, `"d3d11"`, `"wgpu"`, `"vulkan"`, `"dummy"` | on | no loader in the path |
| any backend on Windows | **off** | `_sapp_win32_init_dpi` resolves `user32`/`shcore` entry points the same way, under every backend. Unmeasured — no MinGW here — and read off the header's text |

The cost, plainly: a program that reaches `sokol_app` under one of those
configurations compiles its **whole** translation unit, Dado's emitted C
included, without `-pedantic`. `-Wall -Wextra -Werror` and the two interop
`-Werror=` flags are unaffected, and every `collections/` package that does not
reach `sokol_app` still compiles pedantically under both compilers.

## Objective-C on Apple

sokol_gfx's Metal implementation and sokol_app's Cocoa implementation are
Objective-C. That used to be a hard stop: they were compiled from `.c` sidecars,
`dadoc` discovers only `.c` sidecars, and the driver compiles them as C.

With the implementation inside Dado's own generated `.c` — which is valid
Objective-C — the whole translation unit can be compiled as Objective-C, and the
Apple arms say so themselves:

```dado
foreign "sokol_gfx.h" define "SOKOL_METAL" flags "-x objective-c" link "-framework Metal -framework Foundation"
```

`flags` is the compile-step clause; `./dado` threads it onto the `cc` line. No
`.m` extension, no change to the driver, no wrapper script.

**Verified as far as the manifest, not as far as a Mac.** Under
`--target=aarch64-apple-darwin` the Apple arm's `flags -x objective-c` and its
`link -framework …` lines reach `--emit=build`; the container this was measured
in has no macOS and no Apple toolchain, so the Metal and Cocoa *compile* is
unverified here. Say what breaks if it does.

## Testing without a GPU

`BACKEND=dummy` is a real sokol backend, not a stub: `sg_setup`, resource
creation and state queries all work, with no GPU and no window.
`example/gfxsmoke` is that test, and `./dado run example/gfxsmoke -D BACKEND=dummy`
builds clean under the default `-Werror` on gcc and on clang.

A binding checked on its own is `ERR0301` (*no `main` in package*) — that is what
a library package is, not drift. Check one by building a program that imports it.

## Regenerating a binding

Dado's binding generator drafts these from the headers, and every one of them is
hand-edited past the draft — the `define`, `link` and `flags` clauses and the
`when` chains that carry them are what the sections above are about, and a
generator that read a header cannot derive a line of them.

So the generator does not own the file. It owns the two regions between
`// bindgen:begin …` and `// bindgen:end …`, and `--merge` rewrites exactly
those and nothing else. Each binding's `bindgen.toml` names its header, its
package and `merge = true`, so a regeneration is the generator run over that
file.

Running it twice over the same file gives the same bytes. **Without `--merge`
it writes a whole file** and the hand-written half is gone — without
`define "SOKOL_IMPL"` the binding declares functions nothing defines and the
link fails, and without the backend chain the header refuses to compile at all.

`sokol_gfx.dado` carries the markers. The other seven bindings do not yet, so
`--merge` refuses one of those and says which pair it wanted; a first merge
needs the pair added by hand — around the enum block and inside the `foreign`
block — and one run *without* `--merge` into a scratch directory is how to see
what the regions should hold before splicing them.

## What's not here

* **The text and quad path, as sokol ships it.** `sokol_gl.h`,
  `sokol_debugtext.h`, `sokol_shape.h`, `sokol_color.h`, `sokol_fontstash.h`,
  `sokol_imgui.h`, `sokol_nuklear.h`, `sokol_memtrack.h` and `sokol_spine.h` are
  not vendored, and are not going to be: they ship their own shaders and a fixed
  bitmap font. `app:draw` writes the pipeline and `app:font` the glyph atlas,
  over `sokol_gfx` directly.

**And nothing else: `sokol_gfx` skips nothing.** Twelve `sg_*` declarations
used to sit under *"Skipped (needs a human)"* — five types whose single field
is an array or a nested struct (`sg_image_data`, `sg_sampler_info`,
`sg_shader_info`, `sg_pipeline_info`, `sg_view_info`), one that contains one of
them (`sg_image_desc`), and the six functions that return them, which is the
`sg_query_*_info` reflection surface plus image *creation*. `dadoc` mis-shadowed
a one-slot foreign struct and `bindgen` skipped past it rather than emit a
binding that would not build. That is fixed, the binding is regenerated,
`sg_make_image` takes a real `^sg_image_desc`, and the count line at the foot
of the block reads `skipped 0`.

`sokol_glue` **is** here — the sentence that said it was omitted was written
before it was added, and it checks clean.
