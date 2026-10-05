<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:strings

std/strings — the operations `==` does not give you.

The language gives `string` equality by contents and stops there, because
it says nothing about collation and inventing an ordering is not a
compiler's business. C has one, so this package borrows it.

**This package used to be a wrapper over `<string.h>` and is not any more.**

Every parameter here was declared `cstring`, and a `string` crossed into one
as its `ptr` half with no cast and no check — which was safe only because
a `\0` at `ptr[len]` was guaranteed. That conversion was deleted and
the wrappers went with it: `strcmp`, `strncmp`, `strstr` and `strchr` are
now written in Dado, ten lines of loops over bytes this package can already
index.

That is a better package and not a consolation. `strcmp` answered the C
library's question — *how do these compare up to the first `\0`* — and every
function here was quietly wrong for a string holding an interior one, which
`#len` of a string had already stopped being wrong about. It also means a program that
compares strings no longer carries `<string.h>`, so it stays freestanding
— the freestanding rule, now reaching one more package.

Parsing is not this package's: since `STR-FREESTANDING` `to_i32` and
`to_f64` are **`core:strings/parse`**'s. They moved with the
`private foreign "stdlib.h"` block they were the only users of, which is
why the move was the fix rather than a tidy-up; that package has since
replaced `atoi` and `atof` with Dado and includes nothing.

**Who ends what this package returns.** A window (`substring`, `rest`,
`trim`, the splitters, `bytes`) allocates nothing and is good as long as the
bytes it was cut from; new text (`concat`, `join`, a `Builder`'s `finish`)
is a `ref []char8` from the ambient allocator, so under a `using` arena you
never free it and outside one the caller `#delete`s it.

## Declarations

36 declarations, 35 public.

* `i32 compare(string8 a, string8 b)` — Ordering: byte by byte, and the shorter one first where one is a prefix of…
* `bool before(string8 a, string8 b)`
* `bool after(string8 a, string8 b)`
* `bool starts_with(string8 s, string8 prefix)`
* `bool contains(string8 haystack, string8 needle)` — The naive search, which is the right one at this size: no table to build, no…
* `bool has_byte(string8 s, char8 byte)` — Searching for `'\0'` finds an interior one and reports it, where `strchr`
* `bool ends_with(string8 s, string8 suffix)`
* `bool is_empty(string8 s)` — **`#len(s) == 0` and not `s == ""`**, which is the same answer and a cheaper…
* `i64 index_of(string8 s, char8 byte)` — Where `byte` first occurs, or -1. Written by hand rather than through…
* `i64 last_index_of(string8 s, char8 byte)` — Where it last occurs, or -1. Ranges are ascending only, so the…
* `i64 count_byte(string8 s, char8 byte)` — How many times `byte` occurs. The one a program splitting on a separator…
* `bool is_space(char8 b)` — Whether `b` is one of the four bytes this package treats as space.
* `string8 substring(string8 s, i64 at, i64 count)` — `count` bytes of `s` from `at`, **clamped** rather than refused.
* `string8 rest(string8 s, i64 at)` — Everything of `s` from `at` to the end: `substring(s, at, #len(s))` said…
* `string8 trim_left(string8 s)`
* `string8 trim_right(string8 s)`
* `string8 trim(string8 s)`
* `(string8 head, string8 rest, bool found) split_first(string8 s, string8 sep)` — The **cursor pair**, and the shape every splitter here takes.
* `(string8 head, string8 rest, bool found) line(string8 s)` — One line, and the rest. `\n` is the separator and a trailing `\r` is dropped,…
* `string8 trim_right_cr(string8 s)` — `trim_right`'s one-byte cousin, kept separate because `line` must not eat…
* `(string8 head, string8 rest, bool found) field(string8 s)` — One whitespace-separated field, and the rest. Leading space is skipped, so…
* `(i64 at, bool ok) offset_in(string8 parent, string8 window)` — Where `window`'s bytes start inside `parent`'s, and whether they lie…
* `type Bytes: []char8` — `Bytes` is the name the `string8 → []char8` conversion is reached through:
* `[]char8 bytes(string8 s)` — `s`'s bytes, as a window over them. **A pure reinterpret**: the two words…
* `ref []char8 concat(string8 a, string8 b)` — A new run holding `a` then `b`. See the convention above: allocated from…
* `ref []char8 join([]string8 parts, string8 sep)` — Every part, with `sep` between each two. `join([], sep)` is an empty run,…
* `type Builder: ref []char8` — Text built a piece at a time, when the pieces are not all known at once.
* `Builder builder(i64 room = 16)` — An empty builder with room for `room` bytes before its first growth,…
* `bool append(^Builder b, string8 s)` — `s` on the end. `false`, and nothing appended, when the run could not grow.
* `bool append_char(^Builder b, char8 c)` — One byte on the end. `false`, and nothing appended, when the run could not…
* `string8 text(Builder b)` — What has been built so far, as text. A window into the builder's run, so it…
* `ref []char8 finish(^Builder b)` — The run, handed over: the caller owns it now (see the convention above),…
* `string8 borrow_from_cstring(cstring c)` — The bytes at `c`, as a `string`, **without copying them**.
* `ref []char8 clone_to_cstring(string8 s, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — Copy `s`'s bytes into an owned run and put a `\0` after them, so the result…
* `ref []char8 clone_from_cstring(cstring c, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — Copy a NUL-terminated C string into an owned run, so C's bytes become a value…
