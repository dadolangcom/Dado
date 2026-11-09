# language:syntax

Dado's front end, in Dado (PORT.md P1): the lexer, interning, the parser
(flat tree, recovery, today's parse diagnostics), item hashes and the
normalised tree dump.

## Layout

| file | what |
|---|---|
| `token.dado` | tags (`u8`, dado_lex's `TokenKind` order), dump names, spellings, `keyword` |
| `lex.dado` | `lex(src) -> Tokens`: SoA `tag []u8` + `start []u32`, lex `Diag`s, `trivia` (comments and blank runs, `(start, end)` pairs); `token_at` (the one cursor: the token holding a byte, else the next); `token_end` re-derives a token's end; the lambda-body region rules |
| `literal.dado` | `int_value`, `char_value`, `string_value` (multi-line margin), `unescape` |
| `dump.dado` | `dump_tokens` (`--emit=tokens`, byte for byte) and `render_diags` / `render_parse_diags` (dadoc's human form) |
| `intern.dado` | `Names` (one table for many files) and `intern(&t, src, &names)`, which fills `Tokens.sym`; the positional words (`switch`, `using`, `@c`'s `c`, `#channel` …) are interned first, so the parser tests a word by id |
| `tree.dado` | node tags and layouts, `Ast` (SoA: `tag u8`, `main`, `a`, `b`, `lo`, `hi` `u32`; lists in `extra`), `kids` |
| `parse.dado` | `parse(src, &t, dados) -> Ast`, the `Parser` (tokens, marks/rewind, the scratch stack lists are built on, diagnostics, recovery) |
| `decl.dado`, `foreign.dado`, `types.dado`, `expr.dado`, `stmt.dado`, `misc.dado` | the grammar, by dado_parse's modules: items and members, attributes and `foreign`, types, Pratt expressions and lambdas, statements, the misplaced-`try` and misread-conversion refusals, classes |
| `hash.dado` | per-item `sig`/`body` hashes (128-bit) into `Ast.hashes` |
| `tree_dump.dado` | `dump_tree`, `port/oracle/tree-dump.md`'s format |

`Tokens` is 13 B/token with ends and interning (`end` is stored: the parser
needs every node's byte span). A node is a row of 21 B plus its list in
`extra`; a production answers a node, `NONE` or `BAD`, and a rewind truncates
every array to its mark. Diagnostics carry dado_span's marks (`\x1f` why,
`\x1e` repair), split as `Diagnostic::marked` splits them.

Item hashes: for each top-level item, `sig` covers its significant tokens
outside function bodies (all of them when the head binds a `$T`, `$N` or a
pack) and `body` those inside, each as (tag, offset from the item, length,
text); layout tokens are left out, so a moved item keeps both
(`hash_test.dado`). A `Diag` is code, span and offsets into one shared
text run. Regions keep their levels on the one `indents` stack (a base index
per region), so nothing is copied when a lambda body opens.

The program `port/syntax-tokens` prints `dadoc --emit=tokens` output; the files
it is given are one compilation (offsets and dadoc's end-of-file `1:1` quirk
follow). dadoc's other `--` flags are ignored. `--bench N files…` times `lex`.

## Numbers (2026-11-03, aarch64 VM, one core, gcc -O2)

* Identity: 532/532 files of corpus, collections, tools, example (`.dado` and
  `.dados`) give byte-identical stdout, and identical lex diagnostics (code,
  span, text, help), when given the files dadoc loads. 750/750 mutated files
  (every lex code, ERR0101–0111 and the joining-operator ERR0201) agree too.
* Speed: 531 files, 191,604 lines, 1.27M tokens in 24–25 ms: **~7.7M lines/s**,
  50M tokens/s, lexing only (no interning, no trivia). Dev and `--release`
  builds are the same speed.
* Allocations: about 8.5 per file per lex (arrays reserved from the file's size).

The program `port/syntax-tree` is the Dado side of the `trees`,
`parse-diagnostics` and `rejects` oracles (`syntax-tree FILE`), and
`--bench N files…` times lex + intern + parse.

## Parser numbers (2026-11-04, same machine)

* Oracles: trees 546/546, parse-diagnostics 546/546, rejects 2136/2136
  mutants; 6380 more mutants (deleted brackets and tokens, swaps, stray
  keywords, attributes and literals) give identical trees and diagnostics
  (18,430 diagnostics, 25 codes).
* Speed: 542 files, 197,495 lines in 71 ms with interning: **~2.7M lines/s**
  (lex + intern + parse, best of 5).
* Allocations: about 45 allocator calls per file (24 fresh, the rest growth),
  constant in the file's size but for a few doublings.

## Trivia

`Tokens.trivia` holds dado_lex's side table as `(start, end)` pairs in source
order: a comment's text starts with `/` (a line comment stops before its
`\r`), a blank run does not (adjacent blank lines merge, newlines included).
One array reserved from the file's size, so allocations stay per file; lexing
with it is within 2% of lexing without (7.3M against 7.4M lines/s, release).
Its readers are `language:fmt` (`dado fmt`) and `tools/lsp`; both oracles below
exercise every piece of it.

## Over this package

* `language:fmt`, the formatter (dado_parse's unparse, format, align and
  style), and `tools/fmt`, the program (`dadoc --format`'s contract):
  byte-identical to the Rust formatter on all 581 files, under the tree's
  `.dadofmt` and under the defaults, and idempotent (oracles `fmt`,
  `fmt-defaults`, `fmt-idempotent`).
* `tools/lsp`, the language server's syntax features in process: diagnostics,
  formatting, document symbols (dadoc's declarations dump and the outline over
  this tree) and folding ranges (new; the Rust server has none). Oracles `lsp`
  and `lsp-errors` compare scripted sessions with the Rust dado-lsp.
