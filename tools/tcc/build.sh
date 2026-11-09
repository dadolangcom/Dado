#!/bin/sh
# build.sh — tcc for dev builds, as a toolchain component: copy the pristine
# upstream source (src/, at the commit UPSTREAM names), apply the fork's patch
# series (patches/, in order), and build and install it into a directory keyed
# by the content of all three (src/, patches/, this script).
#
# **The one build of the fork, and the one home of its key.** `dado bootstrap
# tcc` calls this to build, and the driver calls `--print-key` to find the
# build a dev build compiles with (it keeps the answer by the files' stats, so
# a build whose tcc did not move runs no shell). An edit to a source file or a
# patch moves the key, and the next `dado bootstrap tcc` builds a new tcc
# beside the old one. Shell, not Dado, for the reason tools/quickjs/build.sh
# is: a sequence of toolchain invocations with no logic of its own, run before
# the driver has a C compiler to build anything with.
#
#   build.sh [options]
#     --cc CC        the C compiler that builds tcc (default: cc); several
#                    words are allowed (`zig cc`)
#     --into DIR     where builds go, one directory per key (default:
#                    $DADO_TOOLCHAIN_DIR/tcc, else ~/.cache/dado/toolchain/tcc)
#     --print-key    print the key and the build's directory, build nothing
#     --quiet        say nothing on a cache hit
#
# Prints the build's directory (bin/tcc, lib/tcc, PROVENANCE) last. `tcc -v`
# says `dado@<key>` where upstream's says its git commit.
set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
. "$HERE/UPSTREAM"

CC=cc INTO= PRINT_KEY=0 QUIET=0
while [ $# -gt 0 ]; do
  case $1 in
    --cc) CC=$2; shift 2;;
    --into) INTO=$2; shift 2;;
    --print-key) PRINT_KEY=1; shift;;
    --quiet) QUIET=1; shift;;
    *) echo "build.sh: unknown option $1" >&2; exit 2;;
  esac
done
if [ -z "$INTO" ]; then
  INTO=${DADO_TOOLCHAIN_DIR:-${XDG_CACHE_HOME:-$HOME/.cache}/dado/toolchain}/tcc
fi
say() { [ "$QUIET" = 1 ] || echo "$*" >&2; }

if command -v sha256sum >/dev/null; then SHA=sha256sum; else SHA="shasum -a 256"; fi

# --- the key: every file of the source, the series and this script ----------
# One line per file, `<sha256>  <path>`, in byte order of path; the notes
# beside them are prose and move nothing.
PLIST=$(ls "$HERE"/patches/[0-9][0-9][0-9][0-9]-*.patch 2>/dev/null | LC_ALL=C sort || true)
KEY=$(cd "$HERE" && find src patches build.sh UPSTREAM -type f 2>/dev/null | LC_ALL=C sort | xargs $SHA | $SHA | cut -c1-16)
OUT="$INTO/$KEY"
if [ "$PRINT_KEY" = 1 ]; then echo "$KEY"; echo "$OUT"; exit 0; fi

if [ -x "$OUT/bin/tcc" ] && [ -f "$OUT/PROVENANCE" ]; then
  say "cache hit: tcc $KEY"
  echo "$OUT"; exit 0
fi

# --- one builder per key (a directory lock: `mkdir` is atomic everywhere) ----
mkdir -p "$INTO"
LOCK="$OUT.lock"
while ! mkdir "$LOCK" 2>/dev/null; do
  holder=$(cat "$LOCK/pid" 2>/dev/null || true)
  if [ -n "$holder" ] && ! kill -0 "$holder" 2>/dev/null; then rm -rf "$LOCK"; continue; fi
  sleep 0.2
done
echo $$ > "$LOCK/pid"
trap 'rm -rf "$LOCK"' EXIT INT TERM
if [ -x "$OUT/bin/tcc" ] && [ -f "$OUT/PROVENANCE" ]; then echo "$OUT"; exit 0; fi

# --- the patched tree: a copy, never the checkout ----------------------------
# Built in a copy outside the checkout, so the checkout stays pristine and
# upstream's Makefile finds no git work tree to take a commit from.
TREE="$OUT.src"
rm -rf "$TREE" "$OUT"
cp -R "$HERE/src" "$TREE"
PNAMES=
for f in $PLIST; do
  (cd "$TREE" && patch -p1 -s < "$f") \
    || { echo "build.sh: $(basename "$f") does not apply to tinycc ${commit%"${commit#???????}"}" >&2; exit 1; }
  PNAMES="$PNAMES $(basename "$f" .patch)"
done

say "building tcc (tinycc $commit${PNAMES:+ +$PNAMES}) with $CC into $OUT"
t0=$(date +%s)
# GITHASH and GITMODF are upstream's Makefile's own: it would take them from
# git, and the copy is no work tree (git's complaints go to the log).
(
  cd "$TREE"
  ./configure --prefix="$OUT" --cc="$CC"
  make -j4 GITHASH="dado@$KEY" GITMODF=
  make install GITHASH="dado@$KEY" GITMODF=
) > "$TREE.log" 2>&1 || { tail -20 "$TREE.log" >&2; echo "build.sh: tcc did not build (in $TREE; log $TREE.log)" >&2; exit 1; }
t1=$(date +%s)
{
  echo "upstream=$repo $commit ($date)"
  echo "patches=${PNAMES:-none}"
  echo "key=$KEY"
  echo "cc=$CC: $($CC --version 2>/dev/null | head -1)"
  echo "build_seconds=$((t1 - t0))"
  echo "built=$(date -u +%Y-%m-%dT%H:%M:%SZ) on $(uname -sm)"
} > "$OUT/PROVENANCE"
rm -rf "$TREE" "$TREE.log"
echo "$OUT"
