#!/bin/sh
# build.sh — the DadoScript engine as a toolchain component: fetch the pinned
# QuickJS-ng release (UPSTREAM), apply the fork's patch series (patches/), and
# build libquickjs.a into a cache keyed by (triple, compiler, flags, patch set).
#
# **The one build of the engine, and the one home of the patch series.** `./dado`
# calls this when a program's build manifest carries an `engine quickjs` line
# (a program whose emitted C holds the DadoScript runtime library, `core:script`),
# and links the archive it answers; the benchmarks and measurements under
# `toys-lab/quickjs-ng` call it too, through a forwarding script there. A
# program's build never compiles the engine: a cold build is ~20 s at -O2 on
# two cores and runs once per key, and every build after it is a cache hit.
#
# **Shell, not Dado — the recorded exception to "all tooling for Dado is written
# in Dado".** This is the build of a C library: clone a pinned commit, apply
# patches, run the C compiler four times and `ar` once, keyed by a digest. It
# is a sequence of toolchain invocations with no logic of its own to test, and
# `./dado` (itself shell, the floor under every Dado build) runs it *before* the
# program it is building can link. Written in Dado it would be a program `./dado`
# had to build first — a second bootstrap stage on the first build of every
# script-hosting program, on every machine — to replace a script that only
# calls other programs. When `./dado` itself moves to Dado, this moves with it.
#
#   build.sh [options]
#     --cc CC           compiler (default: cc)
#     --triple T        target triple (default: the compiler's -dumpmachine);
#                       passed as --target=T when CC is clang. gcc builds only
#                       for its own triple and is refused any other
#     --opt FLAGS       optimisation flags (default: -O2)
#     --extra FLAGS     more compile flags, part of the key: `./dado` passes a
#                       sanitized program's -fsanitize flags here, so the engine
#                       it links is instrumented like the program
#     --no-ndebug       keep QuickJS-ng's asserts and dump hooks (no -DNDEBUG); the
#                       default builds with -DNDEBUG, as upstream's Release build does
#     --patches LIST    comma-separated patch numbers (0001,0003), "all" (default) or "none"
#     --cache DIR       cache root (default: $DADO_QJS_CACHE or ~/.cache/dado-quickjs)
#     --upstream URL    clone from URL instead of UPSTREAM's repo (e.g. a local mirror;
#                       default $DADO_QJS_UPSTREAM, which reaches the driver's builds)
#     --sysroot DIR     a target sysroot (clang cross builds); part of the key
#     --compile-only    build the archive only (no host link), for a cross triple
#     --quiet           say nothing on a cache hit (the driver's mode)
#     --test            also build qjs + run-test262 + api-test (cmake, native only)
#                       and run QuickJS-ng's tests/*.js suite and its API test
#     --test262 DIR     with --test, also run test262 (fast conf) from a checkout in DIR
#     --print-key       print the cache key and archive path, build nothing
#
# Prints the archive directory (containing libquickjs.a, quickjs.h, PROVENANCE) last.
# The archive is built once per key: a second run with the same key is a cache hit.
# Concurrent runs for one key wait for each other (a lock beside the archive),
# so a gate testing many packages at once builds it once; runs for different
# keys build side by side.
# Engine sources: dtoa.c libregexp.c libunicode.c quickjs.c. No quickjs-libc.
#
# The key names the compiler by its resolved binary and its version, not by
# the name it was called by, so `cc` and `gcc` naming one gcc share an archive.
set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
. "$HERE/UPSTREAM"

CC=cc TRIPLE= OPT=-O2 EXTRA= PATCHES=all CACHE=${DADO_QJS_CACHE:-$HOME/.cache/dado-quickjs}
UPURL=${DADO_QJS_UPSTREAM:-$repo} COMPILE_ONLY=0 TEST=0 T262= PRINT_KEY=0 SYSROOT= NDEBUG=-DNDEBUG QUIET=0
while [ $# -gt 0 ]; do
  case $1 in
    --cc) CC=$2; shift 2;;
    --triple) TRIPLE=$2; shift 2;;
    --opt) OPT=$2; shift 2;;
    --extra) EXTRA=$2; shift 2;;
    --patches) PATCHES=$2; shift 2;;
    --cache) CACHE=$2; shift 2;;
    --upstream) UPURL=$2; shift 2;;
    --sysroot) SYSROOT=$2; shift 2;;
    --no-ndebug) NDEBUG=; shift;;
    --compile-only) COMPILE_ONLY=1; shift;;
    --quiet) QUIET=1; shift;;
    --test) TEST=1; shift;;
    --test262) T262=$2; shift 2;;
    --print-key) PRINT_KEY=1; shift;;
    *) echo "build.sh: unknown option $1" >&2; exit 2;;
  esac
done

# --- the patch set, in order ---------------------------------------------------
PLIST=
if [ "$PATCHES" = all ]; then
  PLIST=$(ls "$HERE"/patches/[0-9][0-9][0-9][0-9]-*.patch 2>/dev/null | sort || true)
elif [ "$PATCHES" != none ]; then
  for n in $(echo "$PATCHES" | tr ',' ' '); do
    f=$(ls "$HERE"/patches/$n-*.patch 2>/dev/null | head -1)
    [ -n "$f" ] || { echo "build.sh: no patch $n" >&2; exit 2; }
    PLIST="$PLIST $f"
  done
  PLIST=$(for f in $PLIST; do echo "$f"; done | sort)
fi
PNAMES=$(for f in $PLIST; do basename "$f" .patch; done | tr '\n' ' ' | sed 's/ $//')
PHASH=$( (echo "$commit"; for f in $PLIST; do cat "$f"; done) | sha256sum | cut -c1-12)

# --- the toolchain key ---------------------------------------------------------
# CC may be several words (`zig cc -target x86_64-windows-gnu`): the first is the
# program, and the rest go wherever the compiler runs, unquoted.
CCPROG=${CC%% *}
CCPATH=$(command -v "$CCPROG" 2>/dev/null) || { echo "build.sh: no C compiler named $CCPROG" >&2; exit 2; }
CCREAL=$(readlink -f "$CCPATH" 2>/dev/null || echo "$CCPATH")
case "$(basename "$CCPROG")" in
  zig*) KIND=zig;;
  *) case $($CC --version 2>/dev/null | head -1) in
       *clang*) KIND=clang;;
       *) KIND=gcc;;
     esac;;
esac
HOSTTRIPLE=$($CC -dumpmachine)
[ -n "$TRIPLE" ] || TRIPLE=$HOSTTRIPLE
TFLAG=
if [ "$KIND" = clang ]; then
  TFLAG="--target=$TRIPLE"
elif [ "$KIND" = zig ]; then
  # zig names its target with `-target`, already among CC's words.
  :
elif [ "$TRIPLE" != "$HOSTTRIPLE" ]; then
  echo "build.sh: $CC builds for $HOSTTRIPLE and cannot build the engine for $TRIPLE; use clang (--cc clang) for a cross triple" >&2
  exit 2
fi
# The version line without the name the compiler was called by: `cc (Ubuntu …)`
# and `gcc (Ubuntu …)` are one compiler, and the resolved path says which.
CCARGS=${CC#"$CCPROG"}
CCVER="$CCREAL${CCARGS} $($CC --version | head -1 | sed 's/^[^ ]* //')"
CFLAGS="$OPT${NDEBUG:+ $NDEBUG} -D_GNU_SOURCE -funsigned-char${EXTRA:+ $EXTRA}${TFLAG:+ $TFLAG}"
if [ -n "$SYSROOT" ]; then CFLAGS="$CFLAGS --sysroot=$SYSROOT -isystem $SYSROOT/include"; fi
KEYSTR="triple=$TRIPLE|cc=$CCVER|flags=$CFLAGS|upstream=$commit|patches=$PHASH"
KEY=$(printf '%s' "$KEYSTR" | sha256sum | cut -c1-16)
OUT="$CACHE/lib/$TRIPLE/$KEY"
if [ "$PRINT_KEY" = 1 ]; then echo "$KEYSTR"; echo "$OUT"; exit 0; fi

say() { [ "$QUIET" = 1 ] || echo "$*" >&2; }

# --- the locks: one builder per key, and one preparer of the source ----------
#
# Directories, because `mkdir` is atomic everywhere and `flock` is not on a
# stock macOS. The holder writes its pid; a waiter takes over a lock whose
# holder is gone. Taken only on a miss, and the miss is asked again under the
# lock, so a run that waited finds what the holder built. Two locks, so two
# keys (gcc's archive and clang's, or a plain and a sanitized one) compile side
# by side, while the clone and the patched tree they share are made once.
HELD=
unlock() { [ -z "$HELD" ] || rm -rf "$HELD"; HELD=; }
lock() {
  mkdir -p "$(dirname "$1")"
  waited=0
  while ! mkdir "$1" 2>/dev/null; do
    holder=$(cat "$1/pid" 2>/dev/null || true)
    if [ -n "$holder" ] && ! kill -0 "$holder" 2>/dev/null; then
      rm -rf "$1"; continue
    fi
    [ "$waited" -gt 0 ] || say "waiting for another build of the engine ($1)"
    waited=$((waited + 1))
    # Twenty minutes: a cold sanitized build on a loaded two-core box is about
    # a minute, so a lock older than this is not a build.
    [ "$waited" -le 12000 ] || { rm -rf "$1"; continue; }
    sleep 0.1
  done
  echo $$ > "$1/pid"
  HELD=$1
  trap unlock EXIT INT TERM
}

if [ -f "$OUT/libquickjs.a" ]; then
  say "cache hit: $KEYSTR"
  [ "$TEST" = 1 ] || { echo "$OUT"; exit 0; }
fi
lock "$CACHE/.prepare.lock"

# --- source: clone once, verify the commit ------------------------------------
SRC="$CACHE/src/$commit"
if [ ! -d "$SRC/.git" ]; then
  mkdir -p "$CACHE/src"
  rm -rf "$SRC.tmp"
  say "fetching QuickJS-ng $tag from $UPURL"
  git clone -q --no-checkout "$UPURL" "$SRC.tmp" \
    || { echo "build.sh: cannot clone $UPURL (the engine's source is fetched once, then cached in $CACHE/src; pass --upstream with a local mirror to build offline)" >&2; exit 1; }
  git -C "$SRC.tmp" -c advice.detachedHead=false checkout -q "$tag"
  mv "$SRC.tmp" "$SRC"
fi
got=$(git -C "$SRC" rev-parse HEAD)
[ "$got" = "$commit" ] || { echo "build.sh: $tag is $got, expected $commit" >&2; exit 1; }

# --- the patched tree, once per patch set -------------------------------------
TREE="$CACHE/tree/$PHASH"
if [ ! -f "$TREE/.patched" ]; then
  rm -rf "$TREE.tmp"; mkdir -p "$CACHE/tree"
  git -C "$SRC" worktree prune
  cp -r "$SRC" "$TREE.tmp"
  for f in $PLIST; do
    git -C "$TREE.tmp" apply --whitespace=nowarn "$f" || { echo "build.sh: $f does not apply" >&2; exit 1; }
  done
  echo "$PNAMES" > "$TREE.tmp/.patched"
  rm -rf "$TREE"; mv "$TREE.tmp" "$TREE"
fi
unlock

# --- the archive, once per key --------------------------------------------------
lock "$OUT.lock"
if [ -f "$OUT/libquickjs.a" ]; then
  say "cache hit: $KEYSTR"
else
  echo "building the DadoScript engine (QuickJS-ng $tag + ${PNAMES:-no patches}) for $TRIPLE with $CC — once per compiler and flags, then cached in $OUT" >&2
  rm -rf "$OUT.tmp"; mkdir -p "$OUT.tmp"
  t0=$(date +%s.%N)
  for s in dtoa libregexp libunicode quickjs; do
    $CC $CFLAGS -c "$TREE/$s.c" -o "$OUT.tmp/$s.o"
  done
  t1=$(date +%s.%N)
  AR=ar; [ "$KIND" = clang ] && command -v llvm-ar >/dev/null && AR=llvm-ar
  $AR rcs "$OUT.tmp/libquickjs.a" "$OUT.tmp"/*.o
  rm -f "$OUT.tmp"/*.o
  cp "$TREE/quickjs.h" "$OUT.tmp/"
  {
    echo "upstream=$repo $tag $commit ($date)"
    echo "patches=${PNAMES:-none}"
    echo "patchset=$PHASH"
    echo "triple=$TRIPLE"
    echo "cc=$CCVER"
    echo "cflags=$CFLAGS"
    echo "key=$KEY"
    echo "compile_seconds=$(echo "$t1 - $t0" | bc)"
    echo "built=$(date -u +%Y-%m-%dT%H:%M:%SZ) on $(uname -m) $(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | cut -d: -f2 | sed 's/^ //')"
  } > "$OUT.tmp/PROVENANCE"
  rm -rf "$OUT"; mv "$OUT.tmp" "$OUT"
fi
unlock

# --- optional: the engine's own tests on this patch set (native only) ---------
if [ "$TEST" = 1 ]; then
  [ "$COMPILE_ONLY" = 0 ] || { echo "build.sh: --test needs a native build" >&2; exit 2; }
  B="$TREE/build-test"
  cmake -S "$TREE" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER="$CC" >/dev/null
  ninja -C "$B" qjs run-test262 api-test >/dev/null
  (cd "$TREE" && "$B/run-test262" -c tests.conf) | tail -1
  # The embedding API, the fork's additions with it (0008's JS_GetFastArray).
  (cd "$TREE" && "$B/api-test") && echo "api-test: ok"
  if [ -n "$T262" ]; then
    rm -rf "$TREE/test262"; ln -s "$T262" "$TREE/test262"
    (cd "$TREE" && "$B/run-test262" -c test262.conf -c test262-fast.conf -a -t 1) | head -1
  fi
fi
echo "$OUT"
