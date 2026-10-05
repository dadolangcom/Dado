# `vendor:stb`

Dado bindings for [stb](https://github.com/nothings/stb) — Sean Barrett's
single-file public-domain C libraries. Dual-licensed MIT / Unlicense; see
`LICENSE`, which is upstream's own file unedited.

Drafted by Dado's binding generator from the vendored header, then
hand-edited to add the clauses `bindgen` does not emit, and verified by
compiling and running against it.

## Layout

One package per directory — a package is a directory — each holding two files:
the binding and the header it restates.

```
vendor/stb/
  LICENSE
  README.md
  stb_truetype/  stb_truetype.dado  stb_truetype.h
```

**There is no third file.** No `.c` sidecar and no `link` file: the sections
below are what stands in for both.

```dado
import "vendor:stb/stb_truetype"

ref stb_truetype.stbtt_fontinfo info = #new(stb_truetype.stbtt_fontinfo, alloc)
i32 offset = stb_truetype.stbtt_GetFontOffsetForIndex(&bytes[0], 0)
_ = stb_truetype.stbtt_InitFont(info, &bytes[0], offset)
```

## What is vendored, and from where

| file | upstream | version | sha256 |
|---|---|---|---|
| `stb_truetype/stb_truetype.h` | `nothings/stb`, `master` | v1.26 | `ecd30b05e0dd4fea3a13c26810dd9e1992dc379049482c393d5a19e6b5090aab` |
| `LICENSE` | `nothings/stb`, `master` | — | `bebfe904b14301657e4e5d655c811d51fd31b97c455b9cc2d8600d6bac6cff63` |

Fetched 2026-09-06. The hashes are here so a later reader can tell a vendored
file that was updated from one that was edited, which a version number alone
does not answer.

## How stb is built here

stb is **header-only**: each `.h` is both the API and the implementation, and a
`STB_<NAME>_IMPLEMENTATION` macro is what turns the second one on. The binding
carries it as a `define` clause on its own `foreign` block:

```dado
foreign "stb_truetype.h" define "STB_TRUETYPE_IMPLEMENTATION" link "-lm":
```

A `define` clause is emitted **above every `#include`** in Dado's generated C,
not merely before its own header, so the implementation compiles inside Dado's
single translation unit. Nothing else is compiled and nothing else is linked in
— which is why there is no `.c` sidecar, and the way to see it is the build
manifest:

```
$ ./dado emit example/fontsmoke --emit=build
target x86_64-pc-linux-gnu
os #LINUX
arch #X86_64
include …/example/fontsmoke
include …/collections/core/mem/libc
include …/collections/core/mem
include …/collections/core/io
include …/collections/core/strings
include …/collections/app/font
include …/collections/core/os/fs
include …/collections/vendor/stb/stb_truetype
source …/collections/core/os/fs/fs.c
link -lm
header stdlib.h
header stdio.h
header ./fs.h
header stb_truetype.h
live font
live fs
live io
live libc
live main
live mem
live stb_truetype
live strings
```

**No `source` line for stb.** A `source` line is a second translation unit the
driver compiles and links; one there means a sidecar came back. (`core:os/fs`
does contribute a `source` line to that program — that is `fs`'s own sidecar,
not stb's.)

`link "-lm"` because the rasteriser calls `sqrt`, `floor`, `ceil`, `fabs`,
`pow`, `cos` and `acos`. The implementation also reaches for `<stdlib.h>`,
`<string.h>`, `<math.h>` and `<assert.h>` of its own accord, through the
`STBTT_malloc` / `STBTT_memcpy` / `STBTT_sqrt` / `STBTT_assert` hooks it leaves
undefined; none of those needs a clause here, because it is the header that
includes them.

**No feature-test macros are needed**, which is the difference from
`vendor:sokol`: stb_truetype touches no platform API, only C89 and libm, so
there is no `_POSIX_C_SOURCE` chain and no `when #OS ==` around anything.

## Warnings

Measured 2026-09-06, inside Dado's own translation unit, under
`-std=c99 -Wall -Wextra -pedantic -Werror -O2`: **clean on gcc 13.3 and on
clang 18.1**, and clean again under `-fsanitize=address,undefined` with
`app:font`'s test rasterising a real face through it. There is no suppression
here and none is wanted; if a future stb release stops being warning-clean, that
is the thing to report rather than the thing to silence.

## Three types come across opaque

`stbtt_fontinfo`, `stbtt_pack_context` and `stbrp_rect` are emitted as opaque
`type` declarations with no fields. That is `bindgen` reading the header
correctly rather than giving up: each carries a `typedef struct X X;` line
*ahead* of its struct body, so the declaration the typedef resolves to is the
forward one.

It costs nothing and buys something. An opaque `foreign` type allocates through
the ordinary verbs — `#new(stbtt_fontinfo, alloc)` emits `sizeof(stbtt_fontinfo)`
and the **C** compiler answers it — so a caller can own one without Dado knowing
its shape, and there is no layout assertion over a struct whose shape is stb's
business and changes between releases.

`stbrp_rect` is the interesting one and it is **not** incomplete here, though
the declaration section makes it look so. The public part of the header only
forward-declares it (it is `stb_rect_pack.h`'s type); the *implementation*
section then supplies its own fallback definition when `stb_rect_pack.h` has
not been read first — which is exactly the case in this translation unit. So
`#new(stbrp_rect, alloc)` compiles and runs. Measured, not assumed: the first
version of this paragraph said the type could not be allocated, and a
three-line program disproved it.

## The `type` field, and `@c`

`stbtt_vertex` has a C field called `type`, which is a Dado keyword. The binding
spells it:

```dado
type stbtt_vertex: (i16 x, i16 y, i16 cx, i16 cy, i16 cx1, i16 cy1, u8 @c("type") type_, u8 padding)
```

`@c("…")` between the field's type and its Dado name is the escape, and the
binding generator emits it without being asked. It is the only one this
header needs.

## Testing

`app:font` is the consumer and its `@test` file rasterises a real face through
this binding when the machine has one, skipping — and saying it skipped — when
it does not. `example/fontsmoke` writes the atlas out as a PGM and
`example/fontsmoke/inspect.py` reads it back and judges it.

A binding checked on its own is `ERR0301` (*no `main` in package*) — that is
what a library package is, not drift. Check one by building a program that
imports it.

## What's not here

Only `stb_truetype.h` is vendored. The rest of stb — `stb_image.h`,
`stb_image_write.h`, `stb_rect_pack.h`, `stb_ds.h`, `stb_sprintf.h`, … — is
not, because nothing in the tree needs it yet. Each is the same shape and
the same three steps: drop the header in a directory of its own, run the
binding generator, add the `define` clause and whatever `link` its
implementation costs.

**`stb_rect_pack.h` is the one to expect next, and it is an upgrade rather than
an unblocking.** The `stbtt_PackBegin` / `stbtt_PackFontRanges` path is bound
here and does work: with `stb_rect_pack.h` absent, stb_truetype's own
implementation supplies a fallback rect packer of its own, which is a simple
skyline-free row packer. Vendoring `stb_rect_pack.h` beside this and defining
`STB_RECT_PACK_IMPLEMENTATION` would replace it with the real skyline packer and
waste less atlas. **It has not been exercised by anything in this tree** —
`app:font` packs its own shelves and never calls `stbtt_Pack*` — so treat those
six declarations as bound-but-unproven, which is a different claim from the one
this paragraph used to make.
