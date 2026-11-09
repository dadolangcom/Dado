# tools/tcc: Dado's fork of tcc

Dev builds compile with tcc on Linux, and on macOS arm64 once the work below
lands (port/engine.md, port/tcc-macos/). This directory holds the fork.

| Path | What it is |
|---|---|
| `src/` | **Pristine** upstream source at the commit `UPSTREAM` names. Never edited by hand. |
| `patches/` | The fork's changes: `NNNN-short-name.patch`, applied in order. |
| `UPSTREAM` | Upstream's repository, branch, commit and date. |
| `build.sh` | Copies `src/`, applies `patches/`, builds and installs tcc. |

Upstream: <https://github.com/TinyCC/tinycc> (mirror of `repo.or.cz/tinycc.git`),
branch `mob`, commit `e23dabed154a8495f3ac1ff3cc9d04fa1c207893` (2026-10-16).
`src/` is that commit's tree without `tests/`, `win32/`, `examples/`, `.github/`
and `.gitignore`. Upstream's `.gitignore` would hide `conftest.c` from git.
`COPYING` (LGPL 2.1) and `RELICENSING` are kept. The licence is also in
`THIRD_PARTY_NOTICES`.

## Building

`dado bootstrap tcc` runs `build.sh --cc <the first C compiler here> --into
<toolchain>/tcc`. Nothing is downloaded. The script:

1. Computes the key: the first 16 hex digits of a SHA-256 over every file in
   `src/`, `patches/`, `build.sh` and `UPSTREAM`. This README is not included.
2. Stops if `<toolchain>/tcc/<key>/` already holds a build (a cache hit).
3. Copies `src/` to `<key>.src` outside the checkout, so the checkout stays
   pristine and upstream's Makefile finds no git work tree.
4. Applies each `patches/NNNN-*.patch` in order with `patch -p1`. A patch that
   does not apply fails the build.
5. Runs `./configure --prefix=<toolchain>/tcc/<key>`, `make` and `make install`,
   then writes `PROVENANCE`.

`tcc -v` prints `dado@<key>` where upstream prints its git commit, and the
bootstrap checks for it. The driver finds the build through
`build.sh --print-key`. It stores the answer under `<toolchain>/tcc/keys/`,
filed by the stats of these files, so a dev build where the fork has not
changed runs no shell. Changing a source file, a patch or the script changes
the key, and the next `dado bootstrap tcc` builds the new tcc next to the old
one. `DADO_TCC` still selects a different tcc.

## Patches

Each change is one numbered patch with one concern, in `git format-patch`
form. Its header says why the fork needs it and which workaround in the
emitter or engine it removes. The workaround's removal lands in the same
commit as the patch.

Making a patch: copy `src/` into a scratch git repository (`git init; git add
-A; git commit`). Make the change there and commit it with the reason in the
message. Then run `git format-patch -1 --start-number NNNN -o tools/tcc/patches`.
Run `dado bootstrap tcc` and the corpus under tcc.

## Updating upstream

1. Fetch the new commit: `git fetch --depth 1 https://github.com/TinyCC/tinycc <commit>`.
2. Replace `src/` with its tree, minus the same directories as above.
3. Update `UPSTREAM` and the commit named in this README.
4. Run `dado bootstrap tcc`. Fix each patch that no longer applies, or drop it
   if upstream has absorbed it. Regenerate fixed patches against the new
   `src/` so that none apply with fuzz.
5. Run the corpus programs that have a `.stdout` under tcc, and `tools/dado`'s
   tests.

## Candidate patches (none written yet)

Taken from `port/tcc-macos/` and `port/engine.md`. Each item says what the
patch would remove.

1. **Mach-O thread-locals on arm64 macOS.** Today tcc writes Linux-style
   (ELF) TLS on macOS, so `_Thread_local` is wrong there (probe.sh step 4).
   A patch would add Mach-O TLV descriptors (`__thread_vars`, calls through
   `tlv_get_addr`) in `tccmacho.c` and `arm64-gen.c`. This removes the
   planned thread-local accessor in the emitter (`dado_rt__context` reached
   through `dado_rt_ctx()`, probe2.sh step 6c) and the clang-built
   `libdadotls` dylib it depends on (probe2.sh step 7). It also removes the
   `this_os() == "#LINUX"` condition in `tcc_by_default` and `self_cc`, so
   macOS arm64 gets tcc by default.
2. **Mach-O objects from `tcc -c` on macOS.** Today `-c` writes ELF objects,
   so only tcc can link them (probe.sh step 1), and C built by clang can only
   join through a dylib (probe.sh step 5). This removes the rule that a tcc
   build links only with tcc on macOS. It is the largest patch on the list.
   Patch 1 may be enough without it, if tcc keeps doing the link.
3. **Predefine `__GCC_ATOMIC_{INT,LLONG,POINTER}_LOCK_FREE`** in
   `include/tccdefs.h`. This removes the lock-free part of the engine's
   `TCC_PRELUDE` (`tools/dado/engine.dado`).
4. **`__builtin_ia32_pause` on x86_64** as a builtin, or `pause` in
   `tccdefs.h`. This removes the prelude's pause hint.
5. **`__atomic_*` and `__ATOMIC_*` without `<stdatomic.h>`.** The
   `__ATOMIC_*` order macros would be predefined in `tccdefs.h`, the way gcc
   and clang define them. This removes `TCC_ATOMICS`: the feature-test
   macros and the forced `#include <stdatomic.h>` for C that uses Dado's
   atomic verbs, and the separate `prelude-atomics.h`.
6. **C11's `ATOMIC_{INT,LLONG,POINTER}_LOCK_FREE` in `include/stdatomic.h`.**
   Channel C fails under tcc only because these are missing: its
   `_Static_assert(ATOMIC_LLONG_LOCK_FREE == 2, …)` does not compile. With
   the three defined by hand (`-D…=2`), `corpus/channel_loop` compiled under
   tcc and its output matched `.stdout` (checked 2026-11-04 on Linux
   aarch64). This removes the `_Atomic` case of `tcc_refuses`
   (`engine.dado`), so channel programs stop falling back to another
   compiler. tcc treats `_Atomic` as a qualifier only, so plain loads and
   stores of `_Atomic` objects are not made atomic. The emitter makes every
   such access through `atomic_*_explicit`, so this patch does not need to
   change that.
7. **`_Complex`.** tcc reports "_Complex is not yet supported". This removes
   the `complex numbers` case of `tcc_refuses`. It is large, and no corpus
   program needs it.

The engine also rewrites the atomic-verb guard (`tinyc_guards`:
`!defined(__GNUC__)` becomes `&& !defined(__TINYC__)`). No tcc patch fixes
that, because tcc should not claim to be GNU C. The new emitter will write
the guard itself.
