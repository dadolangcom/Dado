<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:path

core:path — the lexical algebra of file paths: join, clean, split, base,
dir, ext, volume, is_abs, abs, rel and glob match.

**Tier N W H. Script class S.** Pure: no function here asks the operating
system anything — not the working directory, not whether a file exists,
not where a link points. That is why `abs` takes the directory to resolve
against (`core:os/fs` answers the working directory) and why `rel` refuses
what only the file system could answer. So the package builds and runs
freestanding, on the web and hosted, and answers the same on every host
for the same target.

**The separator follows the target.** Built for Windows (`#OS ==
#WINDOWS`) every function keeps Windows' rules; built for anything else,
POSIX's:

* **POSIX.** `/` separates; `\` is an ordinary byte of a name. A path is
  absolute when it starts with `/`.
* **Windows.** `\` and `/` both separate on input, and `\` is what is
  written. A path may start with a **volume**, which no function cleans
  away or splits: a drive letter (`C:`), a UNC share (`\\server\share`), or
  a device path (`\\.\COM1`, `\\?\C:`, `\??\C:`, `\\.\UNC\server\share`).
  A path is absolute when it has a UNC or device volume, or a drive letter
  followed by a separator. `C:foo` is relative to the current directory
  *of drive C* and `\foo` to the root of the current drive; neither is
  absolute. Volume names compare without regard to ASCII case.

**The rules follow Go's `path/filepath`** (Clean, Join, Split, Base, Dir,
Ext, IsAbs, Rel, Match and VolumeName), and its own test tables, both
styles, are this package's tests. Where the answers differ, on purpose:

* `volume` answers a window into the path as written, where `VolumeName`
  rewrites `/` to `\`; its length is the same.
* Windows' `match` treats `/` and `\` alike, as every other function here
  does: `*` and `?` cross neither, and a separator in the pattern matches
  either one in the name. Go's compares the bytes, so on Windows its `*`
  crosses a `/`.
* `match` reports a malformed pattern even when an earlier part of it has
  already failed to match; Go's answers `false` without looking further.
* Windows' names are compared with ASCII case folding, where Go folds all
  of Unicode. Windows' own rule is its volume's upcase table, which no
  pure function can read; ASCII is the part every volume agrees on.
* `abs` is lexical (Go's calls the OS). Windows' drive-relative `C:foo`
  resolves against the base when the base is on drive C, and against
  `C:\` otherwise: the per-drive current directories Windows keeps are
  process state.
* Reserved device names (`NUL`, `CON`, `COM1`, …) are ordinary names.

**How the tests reach Windows' rules on Linux, and POSIX's on Windows.**
Every algorithm is written once, over a private `Style`, and the public
functions pass the target's. The tests are files of this package, so they
call the private `_in` forms with each `Style` and test both rule sets on
every host. The public surface stays one set of functions with no style
parameter, which is the promise worth keeping; a public `Style` can be
added later without changing anything here.

**Allocation.** What makes new text (`join`, `clean`, `dir`, `abs`,
`rel`) answers a `ref []char8` on the ambient allocator, `nil` when the
allocation fails; under a `using` arena nothing needs freeing, and
outside one the caller `#delete`s it. What only cuts (`split`, `base`,
`ext`, `volume`) answers windows into its argument and allocates nothing.
Lengths and indices are `i64`.

**Error codes.** `rel` and `match` are failable; `message(code)` answers a
sentence for each code:

* `BAD_PATTERN` — `match`'s pattern is malformed.
* `DIFFERENT_ROOTS` — `rel`'s paths are on different volumes, or one is
  rooted and the other is not.
* `UNCLIMBABLE` — `rel`'s base climbs out through `..` further than the
  target, so the way back down needs the file system.

**Thread-safety.** No package state: every function is safe to call from
any number of threads at once.

## Declarations

47 declarations, 19 public.

* `const i32 BAD_PATTERN = 1` — `match`'s code: the pattern is malformed — a `[` with no `]`, an empty…
* `const i32 DIFFERENT_ROOTS = 2` — `rel`'s code: the two paths have no common root — different volumes, or…
* `const i32 UNCLIMBABLE = 3` — `rel`'s code: the base reaches further up through `..` than the target,…
* `string8 message(i32 code)` — A sentence for each of this package's codes, and one for any other.
* `const char8 SEPARATOR = '\\'` — The separator this target writes.
* `const char8 SEPARATOR = '/'` — The separator this target writes.
* `bool is_separator(char8 c)` — Whether `c` separates path elements on this target: `/`, and on Windows…
* `ref []char8 join(..string8 parts)` — The parts joined by the separator, then cleaned. Empty parts are skipped;…
* `ref []char8 clean(string8 p)` — The shortest path naming what `p` names, by lexical processing alone:
* `(string8 dir, string8 file) split(string8 p)` — `p` cut after its last separator: `dir` keeps that separator, `file` is…
* `string8 base(string8 p)` — The last element, trailing separators ignored. `.` for the empty path, and…
* `ref []char8 dir(string8 p)` — Everything but the last element, cleaned. `.` for a path with no…
* `string8 ext(string8 p)` — The extension: from the last `.` of the last element to its end, `.`
* `string8 volume(string8 p)` — The leading volume: on Windows `C:`, `\\server\share`, `\\.\COM1`, …, as…
* `bool is_abs(string8 p)` — Whether `p` is absolute: it names the same file whatever the current…
* `ref []char8 abs(string8 p, string8 from)` — `p` made absolute against `from`, cleaned. An absolute `p` is only…
* `!ref([]char8) rel(string8 from, string8 to)` — The path that names `to` when joined to `from`, by lexical processing…
* `!bool match(string8 pattern, string8 name)` — Whether the whole of `name` matches the glob `pattern`:
* `void put(^self, char8 c)`
