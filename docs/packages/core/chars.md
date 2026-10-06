<!-- dadoc 1.0.0-rc.2 -->
<!-- commit 7e1d5137cae1 (dirty) -->
# core:chars

std/chars — C's `<ctype.h>`, told the truth about.

Every one of these C functions returns `int`, not a boolean: a non-zero
value means yes and the *particular* non-zero value is unspecified. So the
`foreign` block below says `i32`, which is what the header says, and the
wrappers turn that into a `bool`.

Declaring `bool isalpha(i32 c)` instead would compile and would be a lie the
C compiler could not catch — `_Bool r = isalpha(c);` converts silently. This
is the small, dull reason a standard library wraps rather than re-exports.

**The wrappers take a `char8`, and the `foreign` block still takes `i32`.**
That gap is the second thing they exist to close, and it needed a written
width conversion at all: until that landed there was no way to write
`i32(c)`, so these signatures said `i32` and a character could not reach a
character function. Declaring the *parameter* `char8` instead would
have been the wrong fix — C's integer promotion accepts `isalpha(uint8_t)`
without a word, so it is a restatement C cannot check, which is exactly the
hole a restatement exists to keep shut.

── what this package is, and what it is not ──────────────────────────────

**The case mapping here is ASCII.** `upper` and `lower` are `char8 → char8`
over `toupper`/`tolower`: one byte in, one byte out. Under the "C" locale —
which is what a program runs in unless it calls `setlocale` itself — they
map `a`..`z` and `A`..`Z` and hand every other byte back unchanged, and no
locale can make them right for UTF-8, because a byte of a multi-byte
character is not a character. The shape cannot express Unicode case
mapping at all: `ß` upper-cases to the two letters `SS`, one code point to
two, and a `char8 → char8` function has nowhere to put the second. There is
no `char32` case mapping anywhere in `core`. The predicates are the same:
`is_alpha` asked about either byte of `é` answers no, because neither byte
is a letter on its own.

**The segmentation is code points, not graphemes.** `decode`, `encode`,
`count`, `width`, `display_width` and `fit` (`utf8.dado`) are correct
UTF-8 and correct per code point; `cluster_end` (`cluster.dado`) groups a
base with the zero-width code points after it, which is what a cell grid
needs and is **not** Unicode's extended grapheme cluster — `cluster.dado`
says where the two differ. `count` counts code points, so `e` + U+0301 is
two, and so is a flag.

**What a caller must bring themselves**, because nothing here does it:
Unicode case mapping and case folding (the full tables, one-to-many
mappings, and the locale-sensitive ones such as Turkish dotted and dotless
`i`); normalization, without which `é` as one code point and `e` + U+0301
are different strings to every comparison here and to `==`; character
properties beyond ASCII (is this code point a letter, a digit, a space);
extended grapheme cluster, word and line segmentation; and collation. A
program that needs any of them takes a library that carries the Unicode
tables — ICU, utf8proc — through a `foreign` block.

## Declarations

45 declarations, 24 public.

* `bool is_alpha(char8 c)`
* `bool is_digit(char8 c)`
* `bool is_alnum(char8 c)`
* `bool is_space(char8 c)`
* `bool is_upper(char8 c)`
* `bool is_lower(char8 c)`
* `bool is_punct(char8 c)`
* `bool is_print(char8 c)`
* `char8 upper(char8 c)` — `toupper` and `tolower` return an `int` that is a byte for every byte input,…
* `char8 lower(char8 c)`
* `i32 cluster_end(string8 s, i32 at)` — The byte index just past the cluster that begins at `at`. `at` at or past…
* `i32 cluster_width(string8 s, i32 at)` — Columns of the cluster that begins at `at`: its base's `width`, which is 0…
* `i32 cluster_count(string8 s)` — How many clusters `s` holds: how many cells a grid gives it, counting a…
* `i32 cluster_columns(string8 s)` — How many columns `s` takes on a grid: the sum of its clusters' widths. This…
* `i32 cluster_start(string8 s, i32 at)` — The byte index where the cluster **before** `at` begins — the step a caret…
* `const u32 REPLACEMENT = 0xFFFD` — U+FFFD, the replacement character — what a malformed sequence decodes to.
* `(char32 cp, i32 size) decode(string8 s, i32 at)` — Decode one character from `s` beginning at byte `at`.
* `i32 encoded_size(char32 cp)` — How many bytes `cp` encodes to. Zero for a value that is not a character.
* `i32 encode(char32 cp, []char8 into, i32 at)` — Encode `cp` into `into` at byte `at`. Answers how many bytes were written,…
* `i32 count(string8 s)` — How many characters are in `s`, as opposed to how many bytes.
* `@const bool AMBIGUOUS_WIDE = false` — East Asian Ambiguous characters — Greek, Cyrillic, box drawing, much of…
* `i32 width(char32 cp)` — How many columns `cp` occupies: 0, 1 or 2.
* `i32 display_width(string8 s)` — How many columns `s` occupies. This is the measurement every layout in a…
* `i32 fit(string8 s, i32 columns)` — The byte index just past the last character that fits in `columns` columns.
