<!-- dadoc 1.0.0-rc.8 -->
<!-- commit f4253b706f10 (dirty) -->
# core:chars

core:chars — one byte's class and case, UTF-8 code points, and how wide a
character is on a terminal.

**Tier N W H. Script class S.** Pure Dado: no `foreign` block, no
allocation, no package state that changes. The package builds and runs
freestanding, on the web and hosted (Linux, macOS, Windows), and answers
the same for the same bytes on every one of them.

**What raises into a script**: every function here, each promised by the
`#assert_raises` on the line above it. There is nothing to leave out: the
width tables are package data, and every walk is over a `string8`.

**The byte half (this file) is ASCII, and says so.** `is_alpha` through
`is_graph`, `upper` and `lower` answer for the 128 ASCII bytes the way C's
`<ctype.h>` does in the "C" locale, and answer *no* (or hand the byte back
unchanged) for every byte from 0x80 up. They are written as comparisons,
not borrowed from `<ctype.h>`: the header is not on the web or on a
freestanding target, and its answers follow `setlocale`, so the same byte
could be a letter in one process and not in the next. A byte of a
multi-byte character is not a character, so no locale could make a
one-byte answer right for UTF-8 anyway: `is_alpha` asked about either byte
of `é` answers no.

**The case mapping cannot express Unicode's.** `ß` upper-cases to the two
letters `SS`, one code point to two, and a `char8 → char8` function has
nowhere to put the second. There is no `char32` case mapping in `core`.

**The segmentation is code points, not graphemes.** `decode`, `encode`,
`count`, `valid`, `prev_start`, `decode_last`, `width`, `display_width` and
`fit` (`utf8.dado`) are correct UTF-8 and correct per code point (bytes held
as `[]u8` are decoded as the text they view, `decode(string8(b), at)`);
`cluster_end` (`cluster.dado`) groups a base with the zero-width code points
after it, which is what a cell grid needs and is **not** Unicode's extended
grapheme cluster — `cluster.dado` says where the two differ. `count` counts
code points, so `e` + U+0301 is two, and so is a flag.

**What a caller must bring themselves**, because nothing here does it:
Unicode case mapping and case folding (the full tables, one-to-many
mappings, and the locale-sensitive ones such as Turkish dotted and dotless
`i`); normalization, without which `é` as one code point and `e` + U+0301
are different strings to every comparison here and to `==`; character
properties beyond ASCII (is this code point a letter, a digit, a space);
extended grapheme cluster, word and line segmentation; and collation. A
program that needs any of them takes a library that carries the Unicode
tables — ICU, utf8proc — through a `foreign` block.

**Offsets are `i64`**, as `core:strings`' are: every byte index and byte
count (`decode`, `encode`, `prev_start`, `cluster_end`, `fit`, `count`, …)
is one, so a string past 2 GiB is walked rather than wrapped. Widths and
column counts (`width`, `display_width`, `cluster_width`,
`cluster_columns`) are `i32`.

**Thread-safety.** Nothing here is written after the program starts, so
any number of threads may call anything at once.

## Declarations

42 declarations, 32 public.

* `bool is_alpha(char8 c)` — Whether `c` is an ASCII letter, `A`..`Z` or `a`..`z`.
* `bool is_digit(char8 c)` — Whether `c` is an ASCII decimal digit, `0`..`9`. A hex digit's letters are…
* `bool is_xdigit(char8 c)` — Whether `c` is a hexadecimal digit: `0`..`9`, `a`..`f` or `A`..`F`.
* `bool is_alnum(char8 c)` — Whether `c` is an ASCII letter or digit.
* `bool is_space(char8 c)` — Whether `c` is one of C's six spaces: space, `\t`, `\n`, `\v` (0x0B), `\f`
* `bool is_upper(char8 c)` — Whether `c` is an ASCII upper-case letter, `A`..`Z`.
* `bool is_lower(char8 c)` — Whether `c` is an ASCII lower-case letter, `a`..`z`.
* `bool is_punct(char8 c)` — Whether `c` is ASCII punctuation in C's sense: printable, not a space, and…
* `bool is_print(char8 c)` — Whether `c` is printable ASCII, 0x20 (space) through 0x7E (`~`).
* `bool is_graph(char8 c)` — Whether `c` is printable ASCII other than the space, 0x21 through 0x7E: a…
* `bool is_cntrl(char8 c)` — Whether `c` is an ASCII control: 0x00 through 0x1F, and 0x7F (DEL). The C1…
* `bool is_ascii(char8 c)` — Whether `c` is ASCII at all, below 0x80. A byte at or above it is part of…
* `char8 upper(char8 c)` — `c` in upper case when it is `a`..`z`, and `c` unchanged otherwise.
* `char8 lower(char8 c)` — `c` in lower case when it is `A`..`Z`, and `c` unchanged otherwise.
* `i32 digit_value(char8 c)` — The value of the hex digit `c` (0..15), or -1 when `c` is not one. Decimal…
* `i64 cluster_end(string8 s, i64 at)` — The byte index just past the cluster that begins at `at`. `at` at or past…
* `i32 cluster_width(string8 s, i64 at)` — Columns of the cluster that begins at `at`: its base's `width`, which is 0…
* `i64 cluster_count(string8 s)` — How many clusters `s` holds: how many cells a grid gives it, counting a…
* `i32 cluster_columns(string8 s)` — How many columns `s` takes on a grid: the sum of its clusters' widths. This…
* `i64 cluster_start(string8 s, i64 at)` — The byte index where the cluster **before** `at` begins — the step a caret…
* `const char32 REPLACEMENT = char32(0xFFFD)` — U+FFFD, the replacement character — what a malformed sequence decodes to.
* `(char32 cp, i64 size) decode(string8 s, i64 at)` — Decode one character from `s` beginning at byte `at`.
* `i64 encoded_size(char32 cp)` — How many bytes `cp` encodes to. Zero for a value that is not a character.
* `i64 encode(char32 cp, []char8 into, i64 at)` — Encode `cp` into `into` at byte `at`. Answers how many bytes were written,…
* `i64 count(string8 s)` — How many characters are in `s`, as opposed to how many bytes.
* `bool valid(string8 s)` — Whether `s` is well-formed UTF-8 throughout: no byte that `decode` would…
* `i64 prev_start(string8 s, i64 at)` — The byte index where the code point that ends just before `at` begins, as…
* `(char32 cp, i64 size) decode_last(string8 s, i64 end)` — Decode the code point that ends just before byte `end`: `decode`'s mirror,…
* `@const bool AMBIGUOUS_WIDE = false` — East Asian Ambiguous characters — Greek, Cyrillic, box drawing, much of…
* `i32 width(char32 cp)` — How many columns `cp` occupies: 0, 1 or 2.
* `i32 display_width(string8 s)` — How many columns `s` occupies. This is the measurement every layout in a…
* `i64 fit(string8 s, i32 columns)` — The byte index just past the last character that fits in `columns` columns.
