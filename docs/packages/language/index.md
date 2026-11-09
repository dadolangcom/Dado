<!-- dadoc 1.0.0-rc.9 -->
<!-- self-hosted (tools/dadoc) -->
# language:index

Loading a compilation: the main package (a directory, or one file), the
packages its imports reach, each file lexed, interned and parsed once.

## Declarations

34 declarations, 23 public.

* `const u32 NONE = 0xFFFFFFFF`
* `type Import`
* `type Source`
* `type Package`
* `type Program`
* `string8 Program.text(^self, u32 sym)`
* `string8 Program.spell(^self, u32 f, u32 n)` — The token text of node `n`'s main token in file `f`.
* `u32 Program.sym(^self, u32 f, u32 n)`
* `ref [](ref []char8) g_keep` — Texts held for the compilation (the process).
* `string8 hold(ref []char8 s)`
* `string8 clean(string8 path)` — `a/b/../c` as `a/c`, `./` dropped: one key per directory.
* `type Lookups` — The driver's lookups (Rust's `packages::Imports`), per run:
* `Lookups g_look`
* `bool g_stdin_on` — `--stdin`: the input file's text; `--package`: a file argument loads its…
* `string8 g_stdin`
* `string8 g_stdin_path`
* `bool g_package`
* `string8 coll_dir(string8 root, string8 coll, string8 rest)` — The directory `<coll>:<rest>` names, whether or not it exists.
* `Program load(string8 path, string8 root)` — `path` (a `.dado` file or a package directory) and every package it…
* `ref []string8 g_tests` — `--test=`, in order; each one's identity (canonical); a load refusal.
* `ref []string8 g_test_ids`
* `string8 g_load_err`
* `bool test_header(string8 src)` — Rust's `test_header`: `@test` on the first line, a word.
