# Changelog

Every user-visible change to Dado, newest first: the language, the compiler, the `dado` tool and
the collections (`core:`, `app:`, `vendor:`). The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions are
[semantic versions](https://semver.org/) with release-candidate suffixes.

Until 1.0.0, a release candidate may break the language and the standard library. Every break
is under **Breaking** (a removal is under **Removed**, and breaks a program that used it), with
the old name or signature, the new one, and what to write instead. Diagnostic codes are the
ones `dado explain` describes.

## 1.0.0-rc.7.1 (2026-10-22)

### Breaking

_Nothing yet._

### Added

_Nothing yet._

### Changed

- The escape check's stale-read diagnostic is a warning: `ERR0803` is now `WRN0803`, a warning and never an error, `--strict` included, so it no longer fails `dado check`, `build`, `run` or `test`. rc.7's hardened check refused real programs that read nothing freed — the three shapes it added under **Breaking** in rc.7 among them; one user's project went from 0 errors on rc.6 to 239 on rc.7 — and the check cannot yet tell a read that must follow a move or free from one that only may. Same detection, same message and `help:` line; only the severity and the letters change, and `dado explain ERR0803` still answers (as the former spelling of `WRN0803`). rc.8 splits the check: a read proven to follow a move or free is an error again, and one that only may stays this warning.

### Fixed

_Nothing yet._

### Removed

_Nothing yet._

### Known issues

- A read that may follow a move or free of the block under it — `^i32 p = &xs[1]`, then `#append(xs, 9)`, then `p^ = 3` — now compiles, with `WRN0803`, including one that certainly reads freed memory. `dado test` catches it at run time with AddressSanitizer (`heap-use-after-free`) when the C compiler has one — `--cc clang` or `--cc gcc`; the pinned zig has none and sanitizes with UBSan alone, which does not see it.

## 1.0.0-rc.7 (2026-10-21)

### Breaking

- The escape check (`ERR0803`) ties a `string8` (and `string16`, `string32`) a call answers to the `ref` lent to it, as a `[]u8` answer already was: `string8 s = id(b)`, `strings.trim(b)`, `strings.split(b, ",")` and `#copy(strings.split(b, ","))` are refused when read after `b` is freed or grown, each accepted before (a break for code that compiled). A `ref` lent to a `string8` binding or slot (`string8 v = b`, `P p = (input, toks)`) is a view of its block, not a second handle; a view a function answers from a `ref` handed to it by value (`look(b)` answering `b[:]`, `strings.text(bl)` of a `strings.Builder`), or from a record holding one handed by value (`look(r)` answering `r.h[1..<2]`, a method on `self` by value answering `self.data`), is a view into that `ref`'s block, refused when read after it grows or is freed; and a pointer held in a slot of a local record or array (`P s = (&b, 1)` then `s.a^`, `[&b, &c]` then `ps[0]^`, `s.p^ = split(b[:])`) is followed to what it points at.
- The escape checks refuse three more shapes, each accepted before (a break for code that compiled): a function that hands over the block its `^ref` points at (`b^ = nil` after reading `b^`) on only some paths no longer counts as handing it over on all, so its answer is a copy of `b`'s handle when it does not (`ERR0803`); a window over a block filled with views of `b`, handed beside `&b` to a function that grows `b` and then reads the window, is `ERR0827` as the window over `b` itself is; and a pointer to the frame stored through a pointer read from a package variable (`^F p = GP; p^ = (local[:], 1)`) or out of storage the function does not own (`h.p^ = …` with `h` a heap record from a call) is `ERR0826` ("…through `h.p`, a pointer read out of storage `put` does not own, which may be package storage or the heap and outlive `put`").
- The escape check (`ERR0803`) refuses views and handle copies stored where it did not follow them, each accepted before (a break for code that compiled): views of a block of the function's stored in package storage — written (`GV = b[:]`), appended (`#append(G, (d[0..<1], 1))`) or kept by a callee (`fill_g(b[:])`) — refuse a later move or free of that block, at the move ("freeing `b` here may move or free the block under `b`, and views into that block were stored in `GV` at …"; help: store a `#copy`, or keep the block alive and unmoved); views stored into a block of the function's (`c[0] = b[:]`, `#append(c, g[0])`), through its own `ref` handed to a callee (`fill(&g, b[:])`), an out parameter (`split_into(b[:], &g)`), a pointer (`p^ = split(b[:])`) or a record slot written through a `^` refuse reads through that block after `b` moves; a package handle stored through an out parameter (`out^ = G`), a handle copied out of a walked block (`for h in hs: k = h`), and a handle copied into a record (`w = (t, nil)`, `W v = (t, nil)`, `w = v`) are copies, stale when the original moves.
- The escape check (`ERR0803`) refuses more programs that read freed memory, each accepted before (a break for code that compiled and ran into freed memory, or would on other input). What a call answers is now tied to what it may copy or view: a call through a trait value (`v.finish()`, with `Fin v = b`, or `x.finish()` with `^Fin x`) is read through every implementer's method, with the trait value as its receiver; a handle answered through any pointer — a parameter, a pointer local, `&G` of a package record, a receiver — may be a copy of a handle what it points at holds, and one answered from a record or fixed array handed by value, or through a window of pointers, or by a function value, of one those hold; `p^` with `p = &b` is `b`; a block filled with views copied (`#copy(split(b[:]))`) or indexed where it is made (`split(b[:])[0]`) holds those views; a function answering `split(G[:])` says so to its callers; a view answered out of a record a `^` points at (`w.look()` answering `self.aux[:]`) points into that record's blocks, when its type can (a `^Vertex` read out of a block of `(^Vertex, i32)` cannot); a record copied out of a package record (`R r = GR`) holds copies of its handles; and a destructuring `let` (`(ref []u8 g, i32 n) = pair()`) is followed as any binding is. Each is refused where the copy or view is used after the block moves, with the repair the existing messages name.
- A project's root is marked by `dado.json` (was `dado.ini`), for `dado`, `dado-lsp` and `dadoc`'s `project:` imports. An existing `dado.ini` no longer marks one: rename it to `dado.json` (an empty file still counts, read as `{}`), or rely on the top of the git work tree, which still marks a project when no `dado.json` is found.

### Added

- `dado package` (new): releases a library from its git repository. It reads `dado.json` (keys `name`, `version`, `description`, `licence`, `repository`, `targets` and optionally `dadoc`; an unknown key is refused by name), asking once for what it lacks with defaults from the repository (the folder's name, the README's `## Summary`, the LICENSE file, `origin`'s URL as `https://`, `["native"]`). It refuses a detached HEAD, a tree with uncommitted changes, a field holding a control character, a `.dado` or `.dados` file that imports through `lib:` or `imports:`, a name the package index lists for another repository, and failing tests (`dado test` on this machine; `--no-test` skips it), all before anything is written. It then takes the version (`x.y.z`, or `patch`, `minor`, `major`, after the newest of `dado.json`'s and the tags here and on `origin`; semver only; a first release offers `0.1.0`), writes and commits `dado.json`, tags `v<version>` (a failed tag undoes the commit) and, after asking, pushes both to `origin`. Flags: `--version=`, `--category=`, `--yes`, `--no-pr`, `--no-test`, `--refresh`, `--cc`, and the library's directory.
- The package index: the `README.md` of `github.com/dadolangcom/DadoPKG`, one line per library, `- [Name](https://…) — description`, under `## Category` headings; lines inside fenced code blocks are examples, not entries. When a released library is not listed yet and its repository is public, `dado package` offers a pull request adding its line in name order: with `gh` when it is logged in, otherwise it opens GitHub's editor and prints the line and heading to paste. The local copy is a shallow clone in the user's cache, fetched again when more than a day old or with `--refresh`; `DADO_INDEX` sets its folder (one holding other files is refused, never deleted, and a copy with local changes is read but not refreshed) and `DADO_INDEX_REPO` the repository it is cloned from.
- `dado new <library|program> <name>` makes a project in a new folder, and `dado init <library|program>` sets up the folder it runs in (its name is the folder's; a name is letters, digits, `-` and `_`, starting with a letter, and its package form, lowercase with `-` as `_`, is neither a keyword nor `main`; a library's is also not a standard collection's), writing only what is missing and naming each file it left alone and why. A program gets `main.dado` and a starter `<name>_test.dado`; a library gets its package at the folder's root, `<name>.dado` and `<name>_test.dado` (the name lowercased; `package <name>`, with `-` as `_`, and one documented function), so a fetched copy is imported as `import "lib:<name>"` (or `import my_geo "lib:my-geo"` when the name holds a `-`). Both get `dado.json` (`name`, `targets: ["native"]`), `.gitignore` (`build/`), `AGENTS.md` and a `README.md` with the title, a tests marker (`<!-- dado:tests -->`…`<!-- /dado:tests -->`), for a library an install marker holding `dado get <Name>`, and a `## Summary` whose placeholder is a comment, so `dado package` asks for the description. With no kind, both list the kinds and ask on a terminal, and exit 1 off one. With no git repository at or above the folder, `git init` is asked on a terminal; off one nothing is run and the note says how; `--git` and `--no-git` answer beforehand. Nothing is committed.
- `dado package` rewrites a library's README between its markers, when it has them, and commits it with `dado.json` in the release commit: the tests marker with the run it just did (`Tests: passing on linux-x86_64 with Dado 1.0.0-rc.7`, or `Tests: not run for v<version>` under `--no-test`) and the install marker with `dado get <Name>`. The README is read after the tests run, and may differ from its committed text only inside the markers; any other uncommitted or staged change to it refuses the release, as for any other file.
- `dado get` (new): fetches a library with git and pins it in the project. `dado get <Name>` looks the name up in the package index (ignoring case; the local copy is refreshed when stale, or with `--refresh`, and once more before a name it does not list is refused, so a library listed today is found) and takes the newest release, the newest semver tag that is not a pre-release; `dado get <Name>@<version>` takes that release (`v` optional; a pre-release, or a tag that is not semver, by its name); `dado get <url>[@<version>]` fetches from any git address without the index, naming the library after the address's last part. A library listed under `## Unmaintained` is fetched with a warning. By default it goes to the install's `collections/lib/<name>/` (the name lowercased) as a git checkout, one per name, moved between versions with a shallow fetch through its `origin`; a folder with changed, untracked, ignored or hidden (`--assume-unchanged`, `--skip-worktree`) files, of another repository, with no commit, or of files that are not a checkout is refused, naming it. With `--project` it is vendored: fetched into a temporary checkout, checked there, and its tracked files (but `.gitignore`, `.gitattributes` and any other `.git…` file) copied into the project's `collections/lib/<name>/` with no `.git`, to be committed with the project; that copy wins over the install's. An edited vendored copy is never replaced, nothing is written or removed through a symbolic link, the new copy is swapped in only once complete, and files of it the project's git would ignore are named in a warning. Either way `import "lib:<name>/<package>"` resolves with nothing else to set up, and `dado get` prints the import line for each package the library holds, in the alias form when the folder is not the package's name (`import my_geo "lib:my-geo"`). Nothing in a fetched tree is run (git runs with hooks, fsmonitor and LFS filters off), and a library is refused on arrival when it holds a symbolic link or any `.dado` or `.dados` file imports through `lib:` or `imports:` (what this run made is removed, a moved checkout put back). An address starting with `-` is refused wherever it comes from. The project's `dado.json` (the nearest, else one made at the top of the git work tree; never a fetched library's own) records `"libraries": {"<Name>": {"repository", "version", "commit"}}`, plus `"hash"` for a vendored copy (SHA-256 over its files' sorted paths, lengths and contents, hidden files included and `.git…` ones left out, CR LF read as LF), its other keys kept; re-pinning a version whose tag moved warns. A bare `dado get` puts every pinned library back: a checkout at its commit, a vendored copy that is missing; from the tag at the pinned commit or, if the tag moved or is gone, the commit itself: the second machine's step.
- `build`, `run`, `check` and `test` check each library the project's `dado.json` pins: a vendored copy must hash to its pin (refused, when edited, with how to keep the edits and `dado get --project <Name>@<version>`) and hold no symbolic link and no `.git…` file at any depth (the hash passes over those, and a `.gitignore` there could keep the copy's files out of the project's commits; refused with the file to delete, and `dado get` will not replace or restore such a copy either), and an install checkout must be at its commit with nothing changed, ignored or hidden (refused with `help: dado get <Name>@<version>`). A project folder that is a git checkout, or holds a copy its pin does not describe, is refused with how to convert it. A project that pins nothing reads nothing more.
- The repository's `.gitignore` ignores `collections/lib/`, so libraries fetched into an install are invisible to `dado update` and survive it.

### Changed

- `runtime.init_scratch` takes its blocks from `#default` and nothing else: made under a nil allocator (`using (nil, nil): runtime.init_scratch()`) it answers `false`, as on `#NONE` at depth 0, where it configured the scratch over `#heap`. At depth 0 on a hosted target it takes C's heap, as before.
- `dado new <name>` is still a program, and now writes the files `dado new program <name>` does: the starter test, `dado.json`, `.gitignore` and `README.md` beside `main.dado` and `AGENTS.md`; `main.dado` prints through a `greeting()` the test checks. Off a terminal it does not `git init`, and says so.
- A library's name whose package form is a Dado keyword or `main` (`Type`, `For`, `Main`) is refused by `dado package` and the index's lines.
- `dado package` reads the README's `## Summary` as only the paragraphs under it: the section ends at the next heading, and HTML comments, over one line or several, are skipped (text beside one on its line is kept), and fenced code is never read. An empty summary was read as the next section's first paragraph.

### Fixed

- On the web, a script's call of a Dado function written inline at its site takes every value `dadoc --emit=strategies` says it does (`encoded`, `checked`), before and after `main` returns: a `string8` argument longer than the 16 KiB scratch is encoded into a block of the module's heap, one holding a lone surrogate is encoded as natively (each surrogate its own three bytes), and a `char32` or `char16` argument that is not one character raises the `Error` it raises natively (`` `main.tail`'s parameter `ch` is a character, which takes a string of exactly one code point ``). Each was sent down the generic crossing instead, which after `main` returned threw "the Dado function `…` of `dado:main` was called with no VM running". A dev build's per-site `fallbacks` count (`tools/web/sites.mjs`) counts these calls.
- On the web, a message a Dado function sends after `main` returned, on a channel a script `receive`s, raises an `Error` in the script's call ("a message was sent on a channel a script receives after `main` returned … it waits in the channel, undelivered"), where it vanished. Only the call that sent it raises: a message `main` left waiting, or one an earlier call sent, makes no later call raise, and a call that sends nothing answers its value. A script's handlers are still called only while `main` runs.
- On the web, what a program writes to standard output after `main` returned (`fmt.println` or `fmt.print` in a Dado function a page's callback calls) is handed over at the end of each write, a partial line too; it stayed in the runtime's 4 KiB buffer and was never shown. On a page it is shown, where the loader dropped everything written once `main` returned, and a partial line reaches the console when the task that wrote it ends.
- `dadoc` no longer panics ("a bound function the live set does not hold") on a program that calls a script's package function (`game.back(t)`) whose script calls a Dado function of its own package that nothing else calls, with no instance of the class made: the call now makes the package live when it has a Dado half (the host was pruned to the packages it reaches and dropped `game.dado`), and keeps the Dado functions the class calls, as `#script_new` does.
- `dado test core:script --target=web` passes: the six tests that drive the native engine's module loader over modules embedded as strings (`script.add_module`), which the web never compiles, now say they are skipped there, where they failed with "could not load module '…': the build wrote no file for it". Programs whose scripts are classes the build writes were not affected. On the web, loading a module registered only as a string still fails, and the message now says why: a script's modules there are the files the build writes, and a module registered as a string is never compiled.
- `#default` read as a value at depth 0 (and `#return` and `#caller` there) is the root allocator, C's heap, on a hosted target and on the web in a program that allocates nothing through `#new`, `#make`, `#format` or a map: it read as two nil words (`proc == nil`), so a program that only passed the ambient allocator on, or called it, went through a null pointer. Reading the ambient now roots it as allocating does. `#NONE` is unchanged: there is no heap, and the root stays nil.
- `#line` directives (written by `dado build`, `run` and `test`, and by `dadoc --line-directives`) name each file's own path when two packages' files share a short name, as `core:net/enet` and the `vendor:enet` it imports do (both `enet/enet.dado`): one package's directives named the other's file, so a C compiler's error, a debugger and a sanitizer report pointed at the wrong file. With it, `dado api core:net/enet` and `dado inspect core:net/enet` list the package's declarations, where they listed none (they read `vendor:enet`'s file).
- `dado test --filter=<text>` that matches no test file of the package fails with `error: --filter '<text>' selects no test of <package> (N in the package)` and exit status 1, where it printed `no @test files in <package> matching '<text>'` and exited 0, so a run naming a test that had since been renamed passed with nothing tested. A package with no `@test` file, tested with no `--filter` or `--shard`, still prints `no @test files in <package>` and exits 0 (as `dado package` relies on); `--shard` selecting nothing already failed.
- The escape check (`ERR0803`) accepts a slot a call hands over: `defer w.destroy()` beside the text `w.finish()` answers, for `core:json`'s and `core:cbor`'s `Writer`, where it was refused since 1.0.0-rc.5 ("returns a copy of the handle to the block under `text`, and a `defer` pending at this `return` — freeing `w.out` inside `w.destroy` — …"). A function that leaves a `ref` slot of what a `^` parameter points at `nil` on every way out (`defer self.out = nil`, or `c.tris = nil` before the `return`) hands that slot's block to the caller, so freeing the slot afterwards (`w.destroy()`, `release(&c)`, deferred or written after) no longer makes the answer stale, whether the answer is bound first or returned directly. Still refused: a slot written `nil` on only some paths or written again before the `return` (directly, or through a pointer that may reach it, such as `w.po^ = t` with `w.po` pointing at `w.out`), a parameter rebound or its address taken before it, and an answer put back into the slot (directly, through a pointer, a call or a tuple) before the slot is freed. The answer is still taken for a copy of any other handle the record holds.
- A function whose parameter is a trait (`i32 first(i32 n, Test t)`, `^Test t`), or whose `where` names a trait or a type (`where T has Test`), called from another package: the name is read in the package that declares the function, where such a call was refused with `ERR0920` "`Test` names no type here", pointing into that package, whenever it spelled the name bare. So `core:io`'s `read_all(&r)` and `copy` are callable outside `core:io` (they were `ERR0920` "`Reader` names no type here"), and `core:cfg`'s `load` and `save` now read their file through `io.read_all`.
- `ERR0919` and `ERR0537`, for an argument whose declaration does not take a trait on, name a repair the caller can write: the trait spelled as that declaration's file imports it (add `using lib.Test` to `Plain`'s declaration in `main`, where it said `using Test`, a name `main` cannot see); an implementer to pass only if it is one the caller can name, never another package's private one; and, for a trait private to its package, that the function cannot be called from outside that package, where it suggested a `using` that is refused. A declaration that takes on another package's trait of the same name is told apart (`Elsewhere` does not take `lib.Test` on: it takes `other.Test` on, package `other`'s). A function's `where T has Test` that does not hold now names the `using` to add in its `help:`, where it said only to call it with an argument its `where` admits.
- A call through a trait value to a `^self` method of an implementer whose slots are all one type (`type Pair using Counter: i32 a, i32 b`, laid out as `[2]i32`) compiles: the C the call goes through passed the implementer's struct pointer where the method takes a pointer to its array, which `gcc` and `clang` refuse (`incompatible pointer types`), so such a program built nothing.

### Removed

_Nothing yet._

### Known issues

_Nothing yet._

## 1.0.0-rc.6 (2026-10-14)

### Breaking

- `int` and `float` are now type names in Dado: `int` is `i64` and `float` is `f64` (see Added). Written as a type they no longer draw `ERR0401` ("a numeric category and not a type"), so a signature such as `float dot([$N]float a, [N]float b)` is now an ordinary function over `f64` elements rather than refused. After `is` and in a `where` they still name every integer width and both floats (`where T is int` is unchanged). In front of a binding, `int $T` and `float $T`, they are still `ERR0401`, which now names the `where` form; `scalar` and the retired `number` are refused as before.
- DadoScript: a script's `int` and `float` are Dado's own, so a type the compiler reports (`--emit=types`, a diagnostic) lists `int` and `float` beside `i64` and `f64` as entries of their own; they are one type with those two.
- DadoScript: a tuple whose slots are all one type is a fixed array, so it prints as one, `[1, 2]` (was `(1, 2)`), and a diagnostic spells its type `[2]int` (was `(int, int)`).
- DadoScript: a Dado fixed array, and a Dado type whose fields are all one type (`type Vec2: (f32 x, f32 y)`), now reaches a script as a fixed array of as many elements, `[2]float`, where it was an array `[]float`. It keeps Dado's operators there (`v + v`, `v * 2.0`) and is a value: assigning it copies it, so a second name no longer sees a change made through the first. This applies to what a Dado function answers a script, a Dado constant a script reads, and a Dado type a script names (`entities.Vec2 p = entities.pos()`); an 8-bit integer element is an `int` (was a `byte`), and wraps where it crosses back into Dado as a lone `int` does. A script that used it as a `[]float` declares it `[2]float` (or by the Dado type's name) where it wrote `[]float`, and calls the `[]T` methods a fixed array does not have (`append`, `pop`, `sort`, `index_of` and the rest) on a slice of it, `v[0..<2]`, which is a new `[]float`; `len()` stays, as a fixed array has it. A `[]float` still passes where a Dado function takes a `[2]f32`, its length checked when the call is made.
- DadoScript: a script method Dado calls whose signature names a Dado type of one type throughout (`entities.Vec2`) answers Dado a `[2]f64` where it answered a `ref []f64` — hold it as `[2]f64`, and drop the `#delete` — and a parameter of that type takes a Dado array or type of as many elements, no longer a slice: pass `v` rather than `v[:]`.
- DadoScript: a script's `int` value passed to a Dado function's `f32` or `f64` parameter, or as an element of a fixed array or a slice of floats (`[2]f64`, `[]f32`), is refused (`ERR1201`, help `float(x)`) where it became the nearest float. A number never changes kind by itself, in either dialect: write `half(float(n))`. A literal still adapts to the float it meets, as in Dado, so `half(2)` and `sum2([1, 2])` are unchanged; a constant declared with a type (`const i32 STEPS = 3`) is a value, and is refused like one.
- DadoScript: a Dado enum reaches a script as the script's enum, where it was the integer behind it. A Dado function taking or answering one, a Dado constant of one and a Dado enum a script names (`Dir`, or `compass.Dir` through an import) now give the script a `Dir`, whose value is a member (`Dir.north`, which prints as `north`) or the number no member names. Passing an `int` where the Dado function's signature names the enum is refused (`ERR1201`), naming `Dir(n)`: write `turn(Dir(n))`, and read a value's number with `int(d)`. A parameter written `i32` still takes an `int`, and refuses a `Dir`. A script's `switch` over one needs an `else`, as Dado's does (`ERR0705`). An enum inside an array or a tuple crosses the same way, so a Dado `[]Dir` is the script's `[]Dir` (was `[]int`).
- DadoScript: a script method Dado calls whose parameter or answer is written as a Dado enum (`compass.Dir d`) takes a value of that enum by Dado's rules, and answers Dado that enum where it answered an `i64`: hold the answer as a `Dir`. A field written as a Dado enum is no longer read or written through a handle (`ERR1219`): give the class a method that answers or sets it.
- DadoScript: what a script writes into a Dado value it was passed now comes back to Dado (under Fixed), so a write Dado's value cannot take fails the call with `script.TYPE`, naming the argument, where it was dropped with the script's copy: growing or shrinking a Dado slice (`xs.append(4)`; a slice keeps its length, as a Dado `[]T` parameter does), writing a `string8` element of one (a view, which takes no new text), and writing a Dado map whose values are views or own storage (`{i64: string8}`) or one inside a tuple or array passed by value that outgrows its table. Grow or write a copy instead (`[]int mine = xs.copy()`). A program whose script wrote a Dado slice or map and relied on Dado's value staying as it was passes a copy.
- DadoScript: a Dado enum backed by no integer (`enum (f32, f32) Heading`) no longer crosses into a script as the type it is backed by: a Dado function taking or answering one is refused (`ERR1206`), and so are a constant of one (`ERR1208`) and the script naming one; give the script a function that answers the member's value.
- `const []T` is no longer a type (`ERR0401`, help `write []const T`): a read-only window is written `[]const T`, the word on its elements. `const []T` put the word on the window's two-word header and promised the elements nothing, so an element write through it was refused while a `[]T` parameter it was handed wrote the caller's elements. A parameter or slot written `const []T` is read as `[]const T` after the error, so handing it to a `[]T` is then refused as any read-only window's is (`ERR0504`). A function head `const []T f(…)` was `ERR0201` ("expected `=`") and now names `[]const T f(…)`; a local `const []T w = a[:]` is still a constant that cannot be folded (`ERR0901`), whose note now names `[]const T w = a[:]`.
- A `$T` bound from a `const` place binds the type under the word: from an element of a `[]const i32`, a `const i32` parameter or a walk over a read-only window, `T` is `i32`, where it was `const i32`. So a template written `[]$T`, which may write its elements, no longer takes a read-only window (`ERR0504`, "slot 0 of `f` is []i32, and this is []const i32"); a template that only reads writes `[]const $T`, and takes a writable window, a fixed array's `a[:]` and a `ref []T` as well.

### Added

- `int` and `float` in Dado: `i64` and `f64` on every target, one type with them for identity, layout, `#size`, generic binding and the C, so a `[]int` passes as a `[]i64` and `typeof` of an `int` is `i64`. A value whose type is written `int` or `float` adapts to whatever width of its own kind it meets, in either direction: `u8 b = n` and `f32 x = speed` need no conversion written. Where a narrowing happens at run time the value wraps, or a `float` rounds, as the written conversion `u8(n)` does; a constant that does not fit is linted as `u8 b = 300` is (`LNT0505`, an error under `--strict`). In an operator the written width decides from either side, `i32 + int` and `int + i32` being `i32` arithmetic; an assignment, compound ones included, takes its target's width; a shift its value's; a comparison compares the two values without narrowing either (`i32 x < int big`). A tuple, array or type of `int`/`float` slots adapts slot by slot to one of the same length with written widths (`[2]float` to `[2]f32`). An integer still never becomes a float, nor a number a string, by itself: write `float(n)`. A generic binding binds the data, so `$T` bound from an `int` is `i64`. For example:

  ```dado
  f32 halved(float speed):
      return speed / 2.0     // `float` arithmetic, rounded into the f32 return
  i32 doubled(int width):
      return width * 2       // `int` arithmetic, narrowed into the i32 return
  ```
- A type of two slots or more is built from one value per slot by calling it: `Vec2(1, 2)` for `type Vec2: f32 x, f32 y`, where `Vec2((1, 2))` converted a tuple. The slots may be named instead, every one of them once, in any order, as a call's arguments are (`Vec2(y: 2, x: 1)`). Each value enters its slot as an assignment would: a literal adapts, an `int` or a `float` adapts to the slot's width, an explicit width only widens (`Wide(k, n)` for an `i32 k` into `i64` slots), and a change of kind is still written (`Vec2(f32(n), 0)`). A `distinct` type over a shape (`Metres(0, 0)`), a fixed array's name (`type Pair: [2]f32`, `Pair(1, 2)`), a nested shape, a `foreign` struct (`raylib.Vector2(1, 2)`) and a constant (`const Vec2 ONE = Vec2(1, 2)`, its slots in order) are built the same way, in DadoScript too, by the same rules: a script's own `type` (`Vec2(1.0, 2.0)`) and a Dado declaration a script names (`entities.Vec2(y: 2.0, x: 1.0)`). One value is still a conversion (`Vec2(q)`), a type of one slot is never built from several, and a union is not built from slots (`Shape(c)` picks the member). The wrong number of values is `ERR0601`, naming the slots and both spellings; a name no slot has is `ERR0602`, a slot named twice `ERR0603`, and named and positional values mixed `ERR0604`. `Vec2((1, 2))` still works. For example:

  ```dado
  Vec2 half(Vec2 v):
      return Vec2(v.x / 2, v.y / 2)
  ```
- DadoScript: fixed arrays, `[N]T`, written as Dado writes them (`[2]float`, `[4]int`, `[3][3]float`), `N` a literal or a constant the script sees. A fixed array is a value: assigning, passing and returning one copies it. Its operators are Dado's, element by element, with Dado's rules for which apply: `+ - * /` over two of one type or over one and a scalar (`v * 2.0`), `%` and the bit operators over `[N]int`, unary `-`, and `==` and `!=` over every element; an `[N]int` element wraps as an `int` does. An array literal where a fixed array is wanted is one (`[2]float v = [1.0, 2.0]`), and a `[]T` elsewhere as before. `v[i]` reads and `v[i] = x` stores an element (a written position past the end is refused `ERR0519`, a computed one raises `index out of bounds`), `for x in v` and `for x, i in v` walk it, `x in v` scans it, `v.len()` is its length and `v[a..<b]` is a new `[]T`. A fixed array prints as Dado prints one, `[1.0, 2.0]`.
- DadoScript: fixed arrays cross between Dado and a script both ways, as the same type. A script passes its `[2]float` to a Dado function taking a `[2]f32` or a Dado type of two `f32` fields, such as `raylib.Vector2`, with no conversion written: each element rounds as a lone `float` passed to an `f32` does, and an `int` element passed to an `i32` wraps as a lone `int` does. Dado calls a script method taking or answering a fixed array (`[2]float`, `[3]int`, `[2][2]float`, `[2]bool`), passing its own arrays and same-typed types, and receives `[2]f64`, `[3]i64` or `[2]bool`. A fixed array of another length, or one whose elements do not cross, is refused `ERR1201`; so is one whose length is a constant (`[STEPS]float`) in the signature of a method Dado calls — write the length as a number there.
- DadoScript: a `foreign` C struct whose fields are all one type, such as `raylib.Vector2`, crosses as the fixed array it is one type with, `[2]float`, where it was refused (`ERR1206` for a Dado function taking or answering one, `ERR1208` for a script naming it). A script calls a Dado function taking a `raylib.Vector2` with its `[2]float`, and can declare `raylib.Vector2 p` and read `p.x`. A `foreign` function itself is still called through a Dado function.
- The run-time narrowing warning. In a dev build, where an `int` or a `float` narrows to a smaller width at run time with no conversion written and the value changes — an integer the width cannot hold, which wraps, or a finite `float` too large for `f32`, which becomes infinity — the first time at each place writes a warning naming the place, the value and the width: `warning: main.dado:12:12: implicit narrowing changed the value: the int 300 does not fit in u8, and wraps to 44`. Later ones at that place are counted, and at exit each place that changed a value more than once is written again with its count, so a narrowing every frame says so once. The value narrows exactly as before; rounding to the nearest `f32` is not reported. The warning goes to standard error, to the page's `console.warn` on the web, and, for a script's `int` or `float` passed to a narrower Dado parameter, to the script engine's error sink. On a target with no C library (`#NONE`) it is handed to `narrowing_warning` when the program defines one (`export void narrowing_warning(cstring where, cstring width, i64 value, f64 fvalue)`), and goes nowhere otherwise.
- `--release` builds drop the run-time narrowing warning; `--narrowing-warnings` keeps it in a release build, and `--no-narrowing-warnings` drops it from a dev one (`dado build`, `dado run`, `dado test` and `dadoc`). Neither changes a value or a diagnostic.
- `dadoc --emit=narrowings`: every place a program narrows an `int` or a `float` to a smaller width at run time without a conversion written, one line each with the two types, a script's arguments into Dado among them. For code that wants to audit where it relies on `int` and `float` adapting; nothing is linted.
- DadoScript: tuples take Dado's operators element by element where they were refused (`ERR0607`): `==` and `!=` compare every slot, a `string` slot by its contents, and `+ - * /` apply when they apply to every slot (`(float, int) + (float, int)`). A slot is read and stored by a written position, `t[1]`, and `t.len()` is its count of slots. A tuple holding an array or a map still has no `==`.
- DadoScript: `type` and `union` declared in a class, spelled as in Dado: named slots one per line (`type Vec2:` over `float x` and `float y`) or a shape after the colon (`type Pair: [2]float`, `type Hp: int`), and `union Shape: (Circle, Rect, string)`. A type is a value, where a class is held by reference: assigning, passing and returning one copies it, `==` compares its slots, and its operators are Dado's for its shape (`Vec2` is a `[2]float`, with its `+` and `* 2.0`; a type of mixed slots is a tuple). A slot holds a script's types (`int`, `float`, `bool`, `string`, fixed arrays, `[]T`, maps, enums, tuples and other types); a `[]T` or map slot is shared by a copy, as a Dado `ref []T` field is. A type is built as Dado builds one, from one value per slot (`Vec2(1.0, 2.0)`), by a literal where the type is wanted (`Vec2 v = (1.0, 2.0)`) or by converting one value (`Vec2(q)`); its slots are read and written by name (`v.x = 3.0`), and `Vec2.names()` lists them. A union is built by converting a value of a member's type (`Shape(c)`) and read with Dado's narrowing `switch`, whose arms name a member and may bind it (`case Circle k:`); one naming every member needs no `else`. A script's type crosses to and from Dado as the Dado shape it is: a script passes its `Vec2` to a Dado function taking a `[2]f32` or a Dado type of two `f32` fields, and Dado calls a script method taking or answering one with its own shapes, receiving `[2]f64` (or a tuple, for mixed slots). A script's types are data for now: a method written on one, `using` and binders are refused (`ERR0001`); a width, `ref` or a pointer in a slot, and `distinct`, are Dado's (`ERR1301`); a script's enum does not cross to Dado (`ERR1201`), and a script's union crosses as described below.
- DadoScript: an inner class may declare types, unions, enums and traits, as an outer class does. Each is named by its bare name inside the class that declares it, through that class's name in the outer class (`Board.Cell`), and through its whole path in the package or from an importing one (`Game.Board.Cell`). A type, union or enum holds no declarations, and a class inside an inner class is still refused.
- DadoScript: a class embeds a `type` by naming it as a member on a line of its own, as a Dado `type` embeds one: `Vec2` in a class body makes each named slot of `Vec2` a field of the class, reached as any field is (`x`, `self.x`, `ship.x`) and starting at its zero. The type may be one the class or the class around it declares (`Vec2`, `Hold.Cargo`) or a Dado type the script names (`Vec2`, `entities.Vec2`), and one it embeds in turn brings its slots too. The type's name, as a member, is the embedded value whole: `Vec2`, `self.Vec2` or `ship.Vec2` reads a copy of the slots as one `Vec2`, and `Vec2 = v` stores each of them (a plain `=`; a compound one is refused `ERR0606`). A Dado type's `self` methods are promoted: `ship.length()` calls the Dado method with a copy of the slots; a `^self` method is refused at the call (`ERR0606`), and one a trait's default gives the type is not reached yet (`ERR0001`). A collision is refused, never an override, naming both: a slot or promoted method named like a field or method of the class or one it inherits (`ERR0409`, `ERR0534`), and two embedded types bringing one slot or one method (`ERR0407`). A class is never embedded — inherit it with `extends` or hold it in a field — and neither is a union, a scalar or a type whose slots are not all named (`ERR1310`). Embedding takes on no trait: a class's `using` is answered only by the methods it writes. A host does not reach an embedded type's slots through a handle yet. For example, where `entities.Vec2` writes `f64 length(self)`:

  ```
  class Ship
  import "entities"
  entities.Vec2
  void step():
      x += 1.0
      entities.Vec2 p = Vec2
      Vec2 = p * 2.0
      float l = length()
  ```
- DadoScript: Dado's enums, `distinct` types and unions cross between Dado and a script, both ways, natively and on the web, each as the script's own by the type the signature names. **An enum** is the script's enum, open: a member crosses as its name and a value no member names as its number, so every value crosses and comes back as itself, and a value two members name crosses as the one declared first; `Dir(5)` makes one of a number, `int(d)` reads one's number, `Dir.names()` lists the members, and a `switch` over one needs `else`. **A `distinct` type** (`distinct type Meters: f32`) is nominal in the script and the number it is over when it runs: `Meters(1.5)` makes one, `float(m)` and `int(m)` read it, Dado's operator rules for it hold (`Meters + Meters`, never `Meters + float`), and a `float` passed where a `Meters` is taken is refused (`ERR1201`), naming `Meters(x)`. A Dado function taking or answering a `distinct` type is called from a script where it was refused (`ERR1206`). **A union** (`union Value: (i32, f64, string8)`) is the script's union of the types its members cross as, read with Dado's narrowing `switch` (`case int n:`) and made with `Value(x)`; one two of whose members a script holds as one type (`(i32, i64)`) does not cross (`ERR1206`). Dado calls a script method whose signature names one with its own values, by Dado's rules (a `Meters` parameter takes a `Meters`), and an enum or a union answered arrives as Dado's own (a `Meters` answer as an `f64`, as a `float` answer does). A constant of an enum or a `distinct` type is read by a script, where it was refused (`ERR1208`). For example:

  ```
  class Walker
  import "compass"
  void step():
      compass.Dir d = compass.turn(compass.Dir.north)
      switch d:
          case compass.Dir.north:
              println("still north")
          else:
              println("now ", d, ", number ", int(d))
  ```
- DadoScript: a script method Dado calls may take one of the script's own unions: Dado passes a value of one of its members' types (`held.size(3)` for a `Shape` of `(int, string)`), and the script receives the union holding it. A value no member takes, or more than one, is refused at the call (`ERR1201`), and so is a method answering one, which Dado has no type to hold.
- DadoScript: Dado's maps cross between Dado and a script, both ways, natively and on the web, as the script's own map. A Dado function a script calls may take or answer a map (`{string8: i64}` is the script's `{string: int}`), and Dado may pass one to a script method and receive one back. Each crossing copies the map entry by entry, every key and value crossing as it would alone, so a value may be an array, a string or another map. The key is an integer, a `bool`, a `string8` or, where a Dado function's signature names one, a Dado enum; a map keyed by anything else (a tuple, an array) is refused (`ERR1206`, `ERR1201`), and a script's map keyed by a `float` has no Dado map to arrive as. What a script writes into a map Dado passed it reaches Dado's map when the call returns (below, under Fixed), and a map a Dado function answers a script is ended once it is copied, as a `ref([]T)` answered is (a `ref([]T)` of maps is refused, `ERR1206`). A map a script's method answers is a new Dado map on the ambient allocator, which Dado ends with `#delete`; its string keys are the map's own, and a `ref([]char8)` value is ended by itself, as an answered array's elements are. A value of the wrong kind raises when the call is made, naming the entry (`at [1][1]` for the second entry's value), and a Dado map lent to a script is locked for the run as an array is (`script.LENT`). Dado promises no map order. For example:

  ```
  class Tally
  {string: int} doubled({string: int} m):
      {string: int} out = {}
      for k, v in m:
          out[k] = v * 2
      return out
  ```

  and in Dado, `{string8: i64} d = held.doubled(ages)`, then `#delete(d)`.
- DadoScript: a `string16`, a `string32` and a `cstring` cross into and out of a script as its `string`, transcoded: as a Dado function's parameter or answer, an array's element or a map's value, and as Dado's argument to a script method's `string`. A `string16` crosses as its UTF-16 units, an unpaired surrogate included; a `string32` as the code points of the UTF-8 a `string8` is read as, a value past U+10FFFF written as U+FFFD; a `cstring` up to its NUL, so a script's string holding a NUL character raises when it crosses into one. A script's `string` answered to Dado is still a new `ref([]char8)`. A field written through a handle and a script's package function still take a `string8` (`ERR1201`, `ERR1226`).
- DadoScript: a Dado matrix crosses as the shape it tags, its columns: a `[2,3]f32` is a script's `[3][2]float`, both ways, and a script's `[3][2]float` passes where a `[2,3]f32` is taken. A `#soa [N]P` crosses as the array of its elements, `[N]P`, gathered from its lanes and scattered back into them; one whose element holds an array or another container is refused (`ERR1206`, `ERR1201`).
- DadoScript: a Dado function crosses into a script as a value. A Dado function a script calls may answer one (`i64(i64, i64) pick()`), and Dado may pass one to a script method taking a function (`int apply(int(int, int) f)`); the script holds it as its own function type and calls it, each call crossing as a call of that Dado function does — a failure thrown with its code. Its parameters and its answer must cross as values (`ERR1206` names the one that does not). A script's method answering a function is refused at Dado's call (`ERR1201`): Dado keeps a script's function only through a `(proc, rawptr data)` pair a Dado function takes.
- A package variable's initializer may window another package variable's fixed array: `private [2]([]f64) ROWS = [T[:], geom.SIZES[1..<3]]` is one static initializer holding the arrays' addresses, whichever is declared first and in whichever package, where it was refused (`ERR0901`, "a slice, which windows storage that exists when the program runs"). A write through a row is a write to the array. A window into a `@thread_local` array, into a constant or of another window is still `ERR0901`, naming which.
- A constant folds a conversion to a float: `const f64 REACH = f64(TRIES / 2) * 1.4` over an integer constant, and `const f32 E = f32(M / 8.0)` over an `f64` one, each rounded as the C conversion rounds it, where both were refused (`ERR0901`, "a conversion between two widths"). `f64(E)` of an `f32` constant is the `f32` value. A conversion of arithmetic done in `f32`, `f64(E * 3.0)`, is refused (`ERR0901`, "a conversion of `f32` arithmetic"), naming `f64(E) * 3.0`.

### Changed

- DadoScript: a Dado function that answers a script's function back to it (`void(i32) answer(void(i32) f): return f`) is refused as keeping the function past the call (`ERR1220`), where it was refused as answering a function at all (`ERR1206`).
- A web module that runs scripts imports two more host calls from `dados`, `map_new` and `map_entries`, which the toolchain's loader answers; a loader of your own that answers the `dados` module needs them too.
- DadoScript: a written type with more than one mistake in it (`{Foo: Bar}`, `(Foo, Baz)` with neither declared) reports its first one only, as a Dado type does, instead of one error per unknown name plus an `ERR0001` on the whole type.
- DadoScript: a construct the compiler does not check in a script yet (`ERR0001`, "not built yet") is refused before any type checking, so it is reported alone rather than beside type errors elsewhere in the program, and it is refused in every `when` arm, selected or not, as a Dado-only spelling already is.
- DadoScript: a field of a Dado type whose fields are of different types, which a script holds as a tuple, is written by name (`u.hp = 3`) as one of a same-typed type is, where it was refused (`ERR1208`).
- DadoScript: calling a Dado declaration a script names is Dado's call of a type: one value converts, so `Hp(5)` for a `type Hp: i32` is the `int` it crosses as where it was refused (`ERR1208`), and `Vec2(1.0)` for a type of two slots is refused as Dado refuses it (`ERR0504`, naming `Vec2(x, y)`) where it was `ERR0601`.
- A constant built by calling a type of one slot with several values (`const Slot X = Slot(1, 2)` for `distinct type Slot: u32`) is refused for its count (`ERR0601`) where it was refused as not a constant (`ERR0901`).
- The repair offered for a written-out type called on several values, `[3]f32(1.0, 1.0, 1.0)`, builds the named type from them, `F32x3(1.0, 1.0, 1.0)`, where it converted a tuple, `F32x3((1.0, 1.0, 1.0))`.
- A `type`, `union`, `enum`, `trait` or `class` written in a type's body, in either dialect, is refused saying a type holds no declarations, where it was read as a slot with a bit width and refused for that.
- `dadoc --emit=strategies` names a Dado function taking a slice as crossing `through its generated reader, and Dado's writes are copied back`, where it said `through its generated reader`.
- `#copy` of a `[]const T` answers a `ref []T`, and of a `^const T` a `ref T` (were `ref []const T` and `ref const T`): a copy is a new block nobody else holds, so its caller writes it. A position that names the read-only handle, `ref []const T c = #copy(v)`, still gets one.
- `core:slices`: the functions that only read a run take `[]const $T` where they took `[]$T` (`contains`, `index_of`, `last_index_of`, `count_of`, `equal`, `starts_with`, `ends_with`, the `_run` searches, `all`, `any`, `none`, `count_if`, `find_if`, `min_index`, `max_index`, `is_sorted`, `min`, `max`, `is_sorted_ascending`, `lower_bound`, `upper_bound`, `binary_search`, `find_sorted`, `chunk_count`, `window_count`, `clone` and `copy_into`'s source), so a read-only window reaches them: `slices.contains(view, x)` for a `[]const i32 view` was refused (`ERR0504`, "binds `T` of `contains` to both const i32 and i32"). A writable window, a fixed array's `a[:]` and a `ref []T` reach them as before.
- `core:geom/polygon`: `offset` takes a window, `[][2]$T`, where it took `ref [][2]$T`: a fixed array's `ring[:]` and a borrowed window reach it, and an owned ring still does, lending its window.
- `ERR0504` for a tuple literal that does not fit a declared shape names the shape and the first slot that does not take its value ("the declared shape is Spec, whose slot 2 is i64, and this literal's is f32"), where it printed both whole shapes, which ran to kilobytes over a large type.
- `ERR0701` for a returned `ref` whose view is not what the function answers says what the `ref` does lend (help: "ref([]i32) lends []i32 and nothing else"), and carries the `cstring` and text-window repairs `ERR0504` gives a binding; it had no help.

### Fixed

- DadoScript: a write to a received slice now reaches the sender, both ways, natively and on the web. A slice that crosses is lent for the call: a script method's writes to a Dado slice Dado passed it (`xs[1] = 20`) are in Dado's slice when the call returns, where they were lost; a Dado function's writes to a script's array it received are in the script's array when it returns, where they were lost. The writes come back whether the call succeeded or failed, and before any call back into the sender, so a Dado function the script calls mid-method reads the script's writes so far, and a script method a Dado function calls back into sees Dado's. A slice inside an array, a tuple or another slice comes back the same way, element by element and in place, and a `ref []char8` element takes a script's new text on its own allocator; what cannot come back fails the call (under Breaking). An array, a Dado type and a tuple are values: each side keeps its own, and only the slices inside one come back. What a script keeps past the call is its own copy. A sender writing what it lent while the call runs still fails it (`script.LENT` in Dado, an error in the script), and then nothing more comes back.
- DadoScript: one storage passed for two parameters is one storage on the other side: `held.f(xs, xs)` hands the script one array, so a write through one parameter is read through the other, and a script's `f(a, a)` hands a Dado function taking two slices of one type one slice. A script method compiled to native C that took one array for two parameters saw two copies; it now sees one.
- DadoScript: a Dado map a script wrote (`m["k"] = 1`) is cleared and rebuilt with the script's entries when the call returns, on the map's own allocator, where the script's writes were lost. It keeps its table while the entries fit and moves to a new one otherwise, which the variable passed then holds (another copy of the map's handle keeps the old table, as with a `ref` grown through one copy).
- DadoScript: a Dado function a script calls that stores a `string8` (a literal, or a view of its own text) or another slice into an array the script passed it no longer frees memory it does not own when the call returns, and the script's array holds what Dado stored.
- DadoScript: a Dado function answering a script a `ref []string8` or a `ref [][]T` no longer frees the strings or the arrays its elements view once the answer is copied: only the block it owns is given back. Such a program failed to build where the release's C had nothing to call, and freed memory it did not own where it did.
- DadoScript: a Dado `#soa` array passed to a script method was accepted, and the C written for the call did not build; it now crosses element by element.
- DadoScript: a class's `type` that embeds another of the class's types (`type Body:` over `Vec2` and `float mass`) names the embedded slots, so `b.x` reads one, where its slots had no names and `b.x` was refused (`ERR0410`).
- A DadoScript method with no parameters and no answer (`void init(): return`, `void _draw(): return`) that was compiled to native C failed to build under `-Werror` with `unused parameter 'ctx'`; it now builds.
- DadoScript: a package that declares its own `type Location` (or `Error`) and a script of it that writes `Location(…)`: the call made DadoScript's built-in `Location` where the written type named the package's, so `Location loc = Location(…)` was refused. The package's own declaration now answers the name in a call as in a type, and the built-in is the name only where nothing of the package or the class declares it.
- A Dado program calling a script method whose parameter is one of the script's enums stopped the compiler with an internal error; it is now refused at the call (`ERR1201`), as an answer of one already was, saying the enum has no number to cross as and naming the Dado `enum` to declare instead.
- `#len` of a constant array, `#len(XS)` for `const [3]i32 XS`, is the count its type holds, in a body and in another constant's initializer (`const i64 N = #len(XS)`), where it was refused (`ERR0504`, "takes an **inert** place", with a help that said to bind the value first).
- `p.K.w`, a slot of a constant another package declares, reads the slot, where it was refused (`ERR0408`, "`p` is not a value in scope"); another package's variable's slot, `p.V.x`, reads, writes and takes its address the same way.
- A bare name in a `when` condition or a `#static_assert` is its own package's constant when the package declares one, where a name another package declared too was refused (`ERR0903`). So two packages may each guard code with a `@const` of one name, and `-D package.NAME=value` sets one of them: `-D libtess2.FAULT_HOOKS=true` leaves another package's `FAULT_HOOKS` at its default, as `docs/language.md` now describes.
- A conversion of an integer literal to a float made no float: `f64(7) / f64(2)` computed integer division, 3, both while the program ran and when folded in a `const`; it is now 3.5. `f32(1) / f32(3)` was 0 and is now a third, and `f64(-7) / f64(2)` was -3 and is now -3.5. The emitted C writes the conversion as C's cast, `(double)(7)`, where it wrote the bare `7`.
- The operand of a conversion to a float is checked as its own expression and then converted, so `f32(1 << 23)` is the integer shift made an `f32`, 8388608.0, where it was refused (`ERR0607`, "`<<` is not defined for f32"), and `f32(7 / 2)` is the integer division made a float, 3.0. A literal in such an operand takes its own type (`i32`, or `f64` for `1.5`) rather than the target's, so `f32(1.0 / 3.0)` is computed in double precision and rounded once. A number written alone is still the target's literal (`f32(1.5)`), and a conversion to an integer is unchanged (`i64(1 << 40)` is a 64-bit shift).
- Arithmetic over literals alone, written where a float is wanted, is done in that float, the type the compiler gives the literals: `f64 c = 7 / 2` was the integer division, 3, both while the program ran and when folded in a `const`, and is now 3.5. `x + 1 / 2` for an `f64 x` added 0 and now adds 0.5, `f32 h = 1 / 3` was 0 and is now a third, and `f64 n = -7 / 2` was -3 and is now -3.5. The same holds in an argument (`half(7 / 2)` for an `f64` parameter), a `return` from a function answering `f64`, a default argument, a package variable and a `const`, a script reading that constant included, so Dado now computes what DadoScript's `float f = 7 / 2` already did, 3.5. The emitted C writes such a literal as C's floating constant, `7.0` (`1.0f` for an `f32`), where it wrote `7`, so `f64 big = 4000000000 * 4000000000`, which the C compiler refused as an integer overflow, now builds. Arithmetic over integer literals for an integer is unchanged (`i32 n = 7 / 2` is 3), and so is a conversion's operand, its own expression: a program that wants the integer division in a float writes `f64(7 / 2)`, 3.0. `x / 0` for an `f64 x` is still refused (`ERR0902`).
- A value read from a `const` place converts, indexes and meets a literal as the plain type does: `f64(x)` of a `const f32` and `i64(x)` of a `const i32` element widen, where they were refused (`ERR0504`, "a conversion changes a `distinct` tag or a numeric width; const f32 and f64 are neither"); a `const u32` walked out of a `[]const u32` indexes (`a[r]`), where it was `ERR0504` ("an index is an integer; this is const u32"); and a float literal passed to a `const f32` parameter is an `f32`, where it stayed `f64` and was refused.
- A run written in place fills a `[]const T` parameter or binding (`ro([a, 2, 3])`), where it was refused (`ERR0504`) though it filled a `[]T` one.
- A `ref []T` argument lends its window to a generic `[]$T` parameter, and a `ref T` its pointer to a `^$T` one, before `T` is read: `first(r)` for `$T first([]T xs)` and a `ref []f32 r` was refused (`ERR0504`, "`first` writes this parameter as []T and this argument is ref([]f32)"), and `r[:]` was needed.
- A failable function answering an instantiation of a generic type keeps its `!`: `!Box(T) f(…)`, `!Box(f64) f(…)` and `Why!Box(T) f(…)` were not failable, so their `fail` was refused (`ERR0714`) with a help naming a doubled `!!Box(T)`.
- A `return` lends as a binding does: a `ref []T` returned from a function answering `[]T`, `[]const T` or (for a `ref []char8`) `string8`, and `return #new(Node)` from one answering `^Node`, were refused (`ERR0701`), and binding the value to a local first was needed.
- A tuple literal's slot takes what a declaration of the slot's type takes: a `ref []T` lends its `[]T` (or `[]const T`, or a `ref []char8` its `string8`), a `[]T` gains `const` on its elements, and a `^T` stands where a `^const T` is wanted, as a binding, an argument or a return: `Spec s = (a, 1)` with a `ref []i32 a` and `Holder h = (&x, 1)` with a `^const i32` slot were refused (`ERR0504`).

### Known issues

Every known issue of 1.0.0-rc.5 is still present, and a few of this release's own:

- ERR0803 refuses `defer w.destroy()` beside `return w.finish()` (the writer pattern of `core:json` and `core:cbor`), though `finish` hands the text over. New in rc.5 and not yet fixed; until it is, drop the `defer` and call `destroy` on each early return.
- The escape and borrow checks still accept some unsafe programs (views stored where the checker does not follow them, package handles copied through places it does not follow, a callee that hands a block over on only some paths, a bound view returned past a `defer #delete` of its block), and still refuse two safe ones (`keep(p)` through a pointer variable; a slot handed over and then released). The language reference's limitations section lists them.
- A script's `when` folds literals and `Platform` only: `when DEBUG:` over a Dado `@const` is refused (`ERR0906`). A script reads the constant, `-D` included, with `if DEBUG:`.
- `core:script`'s tests fail on the web (six, "could not load module … the build wrote no file for it"); they pass natively.
- `app:config`'s and `app:net`'s tests fail on the web: they save a file and listen on a port, which the web has no host for.
- The web: an inline crossing still takes the slow path, and throws after `main` returns, for a stored `string8` longer than 16 KiB or a `char32`/`char16` argument that is not one character; `--emit=strategies` reports it as `encoded` or `checked`.
- `#default` read as a value at depth 0 is nil in a program that allocates nothing; `runtime.init_scratch` falls back to `#heap`.
- `io.read_all` can be called only inside `core:io` (ERR0920 from another package), and calling a trait-taking function across packages is ERR0920.
- Traits: an implementer whose only slot is a pointer cannot read it in its methods (ERR0410); dispatch to an implementer whose slots all have one type emits C that does not compile.
- `dado api` lists each per-platform version of a function separately, lists a private type's method as public in `core:path`, and shows no declarations for `core:net/enet`.
- `dado install` still refuses DadoScript on Windows.
- A `--web-threads` worker's 1 MiB stack is not guarded against overflow.
- DadoScript: the run-time narrowing warning does not yet cover an array or a tuple a script passes into narrower Dado elements, nor a script method compiled to native C.
- DadoScript: Dado does not keep a script's function as a value (`ERR1201`); a Dado function takes it through a `(proc, rawptr data)` pair.
- DadoScript: `#script_reload` of a script that changes a `type`'s slots or a fixed array's length is not handled; rebuild the program.
- DadoScript: a `[]const T` parameter of a Dado function a script calls is refused (`ERR1206`); write `[]T`.

## 1.0.0-rc.5 (2026-10-11)

**`dado build` no longer optimises by default: build what you ship with `--release`.** Most of
the standard library reaches its 1.0 shape in this release, so most programs need edits; the
breaks are listed package by package below.

### Breaking

The build:

- `dado build`, `dado run` and `dado test`: dev builds compile at `-O0` (were `-O2`), native and web; `--release` compiles at `-O2`, `--debug` at `-g -O0`. A build you ship must pass `--release`.

The checker (each refuses a program that was accepted before):

- ERR0803: a view a call returns over a package-level `ref` (`[]const i32 r = ro()`) is tied to that `ref`; reading it after a call that grows the `ref`, an `#append` to it, or a loop's next pass is refused, naming the call. Such a read used freed memory.
- ERR0522: a window over, or a walk by address of, a whole `#soa` element that is itself an array (`#soa [2][2]f64`) is refused, as `&s[i]` already was. `s[i][k]` is lane `k` of element `i`.

Failure codes, in every `core:` package that fails:

- Each package's failure codes are one `distinct enum i32 Error` with UPPER_CASE members and a `message(Error why)` function, and its functions fail `Error!T`. Codes that were `const i32` (`os.NOT_FOUND`) are members (`os.Error.NOT_FOUND`), and `message(i32 code)` is `message(Error why)`.
- Code ranges: `os` 1–99 (shared by `os`, `os/fs`, `io` and `os/proc`), `cfg` 1300–1399, `flags` 1400–1407, `geom/bvh` 1500, `geom/earcut` 1600–1601, `geom/halfedge` 1700, `geom/tess` 1800–1801, `net/enet` 2000–2099, `path` 2200–2299, `slices` 2300–2399, `strings/parse` 2400–2499, `threads` 2500–2503, `time` 2600–2604. `http` has its own `http.Error`.
- `path`'s codes no longer collide with `os`'s 1–3, `geom/earcut`'s no longer collide with `geom/tess`'s, and `time.Error.MALFORMED` no longer shares a value with `net`'s `UNSUPPORTED`; code that compared raw numbers must compare members.

`core:strings`, `core:strings/parse`, `core:chars`:

- `strings.clone_to_cstring(string8 s)` and `strings.clone_from_cstring(cstring c)` take no allocator argument; they allocate from the current allocator (`#default`, or a `using` block's).
- `strings.offset_in(parent, window)` answers `!i64` (was `(i64 at, bool ok)`).
- `strings.is_space` and the `strings.Bytes` type are private; use `chars.is_space`.
- `strings/parse`: `to_bool`, `to_f32`, `to_f64`, `to_i32`, `to_i64` and `to_u64` fail `parse.Error` (`MALFORMED`, `RANGE`, `RADIX`).
- `strings/parse.to_i32(string8 s, i32 radix = 10)` is the strict parser (was `to_i32_strict`); the lenient `to_i32(cstring)` is removed.
- `chars`: byte offsets and counts are `i64` (`decode`, `decode_last`, `count`, `encode`, `encoded_size`, `fit`, `prev_start`, `cluster_count`, `cluster_start`, `cluster_end`, `cluster_width`'s `at`).
- `chars.REPLACEMENT` is a `char32` (was a `u32`).

`core:slices`, `core:fmt`:

- `slices.min(xs)` and `slices.max(xs)` answer `Error!T`, failing `EMPTY` on an empty slice (were `(T value, bool found)`).
- `slices`: the gap buffer is the generic `Gap(T)` with methods (`init`, `get`, `set`, `push`, `insert`, `remove`, `move`, `at`, `len`, `cap`, `clear`, `destroy`, `bytes_moved`); the `gap_*` functions are gone, `init` takes no allocator, and edits fail `slices.Error` telling out of range from out of memory.
- `fmt`: `zeros` pads after the sign: `-007` (was `00-7`).
- `fmt.is_nan` and `fmt.is_infinite` are private; use `math.is_nan` and `math.is_infinite`.
- `fmt.put_float`, `put_float_pad` and `render_float`: the precision parameter is named `places` (was `decimals`).
- `fmt.put_repeat(char8 b, i64 n, Sink out)` takes an `i64` count.

`core:os`, `core:os/fs`, `core:path`, `core:io`, `core:os/proc`:

- `os.get_env(name)` answers `Error!ref []char8` (was `(ref []char8 value, bool found)`); `home_dir`, `set_env`, `unset_env`, `cache_dir`, `config_dir`, `hostname`, `user_name`, `catch_interrupt`, `random_bytes` and `executable_path` fail `os.Error`.
- `os.from_system` and `os.last_error` answer `os.Error`; new `os.fail_last()` and `os.fail_with(why)`.
- `os.random` is a pure-Dado generator: the same seed gives a different sequence than before.
- `os/fs`: every failable function fails `os.Error` (`read_file`, `write_file`, `read_dir`, `glob`, `info`, `rename`, `remove_all`, …).
- `fs.walk(root)` and `fs.make_dirs(path)` take no allocator; `make_dirs` answers `os.Error!void` (was `bool`).
- `fs`'s `cstring` predicates take `string8`: `exists`, `is_dir`, `is_file`; `executable` is `is_executable`, `is_symlink` is `is_link`; `make_executable` and `create_private_dir` (was `make_private_dir`) answer `os.Error!void`.
- `fs.open_file(path, Open flags)`: the open flags are `enum Open` (were `READ`, `WRITE`, `APPEND`, … constants); `File.seek` takes an `io.Whence` (`Start`, `Current`, `End`; were `FROM_*`); file kinds are `enum Kind` (were `KIND_*`).
- `fs`: `release(Listing)` is the method `Listing.destroy()`; `os.Environment`'s `release()` is `destroy()`.
- `path.match` and `path.rel` fail `path.Error` (`BAD_PATTERN`, `DIFFERENT_ROOTS`, `UNCLIMBABLE`).
- `io.open(string8 path, string8 mode)` answers `os.Error!^FILE` (was `^FILE open(cstring, cstring)`); `open_read`, `open_write`, `open_append` likewise take `string8` and fail.
- `io.Reader` is a trait (`Cursor` and `StreamReader` implement it); `read_all` and `copy` take `^Reader`; `reader()` and `stdin_reader()` answer a `StreamReader`; `cursor_reader` is gone.
- `io.read_line(^FILE)` answers `os.Error!ref []char8` on the current allocator (was a `cstring` in a static 8 KiB buffer); `read_line_into` answers `os.Error!i64`.
- `io.write` and `io.write_line` take `string8` (were `cstring`).
- `os/proc`: a finished process is `union Ended: (Exited, Signaled)` (were `(bool exited, i32 status)`); `run`, `close`, `exec`, `wait`, `finish`, `finished` and `stop` answer it; `ok(Ended e)`.
- `os/proc`: `run`, `read`, `write`, `spawn`, `exec` and `duplex` take `string8` commands and `[]string8` argv, and fail `os.Error`.
- `os/proc`: the pipe calls (`drain`, `send`, `send_all`, `receive`, `receive_errors`) fail `proc.Pipe` (were counts with `DRAINED`/`WAITED`/`BROKE` sentinels); `duplex`'s error mode is `enum Errors` (was `ERRORS_*`).
- `os/proc` refuses to build for `#WEB` and `#NONE`, saying why.

`core:cfg`, `app:config`:

- `cfg`: every allocator parameter is gone (14 functions: `parse`, `empty`, `get_f64`, `get_i32`, `render`, `render_over`, the `set_*` family, `load`, `save`); allocation follows the current allocator.
- `cfg.exists`, `cfg.load` and `cfg.save` take `string8` paths (were `cstring`); so do `app:config`'s `load`, `load_with` and `save`.
- `cfg.get(cfg, section, key)` answers `Error!string8`, failing `NOT_FOUND` (was `(string8 val, bool found)`).
- `cfg.parse(text)` answers `(Config cfg, i32 bad_lines)` (its `ok` is gone).

`core:flags`:

- `Flags.parse(args)` answers `flags.Error!string8`; the codes are `flags.Error` with a new `NO_MEMORY`.

`core:time`, `core:threads`, `core:log`, `app:log`, `core:jobs`:

- `time.from_utc`, `put_rfc3339`, `format_rfc3339` and `parse_rfc3339` fail `time.Error`.
- `threads`: handles are typed: `Thread`, `Mutex`, `Cond`, `RwLock` (were `rawptr`); `spawn`, `mutex_new`, `cond_new`, `rwlock_new` and every lock call use them.
- `threads.join_result(Thread t)` answers `Error!rawptr` (was `bool` with an out-parameter); `semaphore`, `wait_group` and `barrier` answer `Error!`, and their `ok()` method is gone.
- `threads`: timed waits take a `time.Duration`: `cond_wait_for(c, m, d)`, `acquire_for(d)`, `wait_for(d)` (were milliseconds).
- `log.Sink` is a trait (was a `(fn, data)` pair); `Filter`, `Fanout` and `StreamSink` implement it, replacing the `filtered`, `fanout` and `file_sink` functions; `Fanout.add` is a method.
- `jobs`: per-job scratch arenas come from `#default`, not C's heap.

`core:math`, `core:hash`:

- `math.magnitude(x)` is `math.abs(x)`, as in `core:linalg`.
- `hash.xxh64(u64 seed, []u8 data)` and `xxh64_text(u64 seed, string8 text)` take the seed first.

`core:mem/*`, `core:runtime`:

- `mem.Proc` names the allocator procedure's type, and every allocator signature spells `(mem.Proc proc, rawptr data)`.
- `mem/bump.over([]u8 storage)` takes a slice (was `over(rawptr block, u64 cap)`).
- `mem/arena`'s `Block` and `mem/slab`'s `Chunk`, `Large`, `Link`, `CHUNK_HEADER` and `LARGE_HEADER` are private.
- `mem/gc` is outside the 1.0 compatibility promise, and refuses to build for `#WEB` and `#NONE`.
- `runtime.init_scratch_over(backing, …)` is folded into `runtime.init_scratch(first_block, can_grow)`, which builds on `#default`.

`core:net`, `core:net/enet`, `vendor:enet`:

- `net.format_ip` and `net.format_endpoint` answer `Error!ref []char8` (were nil on failure).
- `enet.init`, `host_server`, `host_client`, `connect` and `send` answer `enet.Error!`.
- `enet.poll(host, timeout_ms)` takes no allocator and answers `Error!Event`; `Event` is a union of `Idle`, `Connected`, `Disconnected`, `TimedOut` and `Received` (was a record with `Kind`).
- `vendor:enet`: `event_type` answers `EventType`; `packet_copy(packet, []u8 out)` and `address_get_host(addr, []char8 out)` take slices.

`core:http`, `core:http/server`:

- `http.accept`, `receive` and `send` answer `http.Error!` and move bytes as `[]u8`; `set_read_timeout` is `set_timeout` (both directions).
- `http.listen(string8 host, u16 port)`: host first, and no default host.
- `http.request_end`, `response_end` and `chunked_end` answer `Error!Extent`, a union of `Whole`, `Pending` and `ToClose` (were `i64` with negative sentinels); `head_end`, `unchunk_to`, `unchunk` and `parse_response` fail `http.Error`.
- `http`'s client (`get`, `post`, `request`, `read_response`) fails `http.Error`.
- `server.parse(raw)` answers `Parsed`, a union of `Request`, `http.Pending` and `Refused`.
- `App.listen(port)` answers `http.Error!void` (was `bool`); a `Route`'s action is the union `Action: (Handler, RawHandler, Files)`; `match`, `expected_size` and `same_fold` are private.

`core:geom/*`:

- `tess.tesselate` is `tess.tessellate`; `tess`'s enum members are PascalCase: `AbsGeqTwo`, `ConnectedPolygons`, `BoundaryContours`.
- `clip.union_polygon` is `clip.union`; `clip.release(^Contours)` is `clip.destroy`.
- `halfedge.get_triangles` is `halfedge.triangles`; `halfedge.baked_destroy` is `halfedge.destroy_baked`; `BoxFace` is backed by `i32`.
- `point.point_on_line`, `point_on_ray`, `point_on_segment`, `point_in_polygon`, `point_in_rect`, `point_in_triangle` are `on_line`, `on_ray`, `on_segment`, `in_polygon`, `in_rect`, `in_triangle`.
- `rect.rx`, `ry`, `rw`, `rh` are `rect.x`, `y`, `w`, `h`; `area`, `perimeter`, `grow`, `grow_sides`, `grow_to_point`, `grow_to_rect`, `positive`, `hslice`, `vslice` and `contains_rect` take a `[2][2]T` rectangle (were position and extent).
- Tolerance parameters are named `eps` (were `tol` in `sat`, `epsilon` in `clip.simplify`).
- `polygon.offset_by(polygon, dt)` takes no allocator and answers `tess.Error!ref [][2]T` (returned nil silently on failure).
- `clip`'s set operations and `triangulate*`, and `polygon.offset_outline*`, fail `tess.Error` (`earcut`'s codes are mapped to `tess`'s).

`vendor:tui`:

- `tui.open` and `tui.open_headless` keep the `#default` they see for the UI's life.
- `tui.log_open(budget)` and `tui.snapshot()` take no allocator; `snapshot` answers an owned `ref []char8` (was `string8`).
- `tui.color_rgb(c)` answers `!u32` (was `(u32 value, bool has)`).
- The colour depth is `enum ColorDepth: (None, Colors16, Colors256, TrueColor)`: `color_depth()` answers it and `colors(depth)` takes it (were `COLORS_16`/`COLORS_256`).

### Added

- Dev builds are split into a `runtime.c` and units of about 512 KiB of C each, grouped by package, compiled in parallel against an object cache keyed on each unit's preprocessed text, so an edit recompiles only the units it changed. `DADO_UNIT_BYTES` sets the unit size (`0` gives one unit per package). `--release`, `--one-file`, freestanding builds and `dado test` stay one C file.
- A `--release` web build loads its scripts as one file: every module the page's scripts compile to is bundled into `registry.js`, with one `modulepreload` and one `integrity`; nothing is evaluated from a string, so a strict Content Security Policy holds. The scripts' own JavaScript modules and `dado-exports.js` stay separate files. Dev builds are unchanged.
- `dado build --release` for the web warns when the page still loads more than ten files.
- `dado test --release` builds the tests at `-O2`.
- DadoScript: a script reads Dado's constants, folded into its JavaScript where read: its own package's by the bare name, another package's as `lib.X`. Integers, floats, `bool`, characters, strings of any width, and arrays and tuples of those (a declared shape keeps its slot names); `-D` and `@const` reach them as they reach Dado.
- DadoScript constants that cannot cross are refused, naming why: enum, `distinct`, foreign, function and type constants and any holding a slice (ERR1208); another package's private constant (ERR0413); a store into a constant (ERR0606).
- `strings.clone(s)`: the owned copy of a `string8`.
- `fmt.eprint` and `fmt.eprintln`, printing to standard error.
- `fs.read_bytes(path)` and `fs.write_bytes(path, data)`, and the `File` methods `read_bytes` and `write_bytes`, over `[]u8`.
- `hash.hex(digest)`, `hash.digest_equal(a, b)` and `hash.equal_hex(digest, text)`, generic over any `[N]u8` digest.
- `mem.zero(p, n)` and `mem.copy(dst, src, n)`.
- `log.discard()`, a `Sink` that drops every record.
- `net.dial(host, port, timeout_ms = 5000)`, resolving the host and trying each address.
- `geom/line`'s intersection functions name their result slots `(at, hit)`; `rect.intersection` names them `(both, hit)`.
- `DADO_HOST_PRUNED=0` restores the previous type-check sequence for a program with scripts (see Changed).

### Changed

- Every binary targets its architecture's baseline CPU (`-mcpu=baseline`): `dadoc launch`'s cached driver, `build`, `run` and `test`, dev and release. A binary built on a newer machine runs on an older one of the same architecture; set `DADO_CFLAGS=-mcpu=native` to opt back in.
- The web: every link puts the stack first (`--stack-first`), with one 8 MiB stack for dev, test and release builds, so a recursion that fits a test fits the shipped page. Each page starts with about 7 MiB more memory.
- `dado build` runs `dadoc` once per build and asks the pinned zig its version once per run.
- A program with scripts whose build removes unused packages is type-checked twice, not three times.
- `dado` builds its own tools (the dashboard among them) with `--release`.
- A private package variable that nothing reads is no longer written into the C.
- A dev build's C is stable under an edit (local names numbered per function, constant tables named by content), so an edit moves only the units it touches.
- Every `core:` package passes its tests on `x86_64-windows-gnu`.
- Every public function of the `core:` packages listed above that can raise into a script is promised by `#assert_raises`.
- `core:http` is built on `core:net`.

### Fixed

- The web: a script calling a Dado function with more than eight parameters fell back to a path that throws once `main` has returned (in a DOM or host callback), while `dado emit --emit=strategies` reported it inline; it now crosses inline at any arity, and the report agrees.
- A store into a position inside a `#soa` element that is itself an array (`c[1][1] = 2.5` on `#soa [2][2]f64`) was written into a temporary and dropped, with the sanitizers silent.
- Embedding a type not yet defined (an imported one, or one lower in the file) spliced its bare name in as one slot.
- The web: a stack overflow overwrote globals; it now traps, ending the run with status 134 and `dado: the module trapped` after everything the program printed.
- The web: `-O0` and `--debug` builds failed to link; the web compile passes `-fno-sanitize=undefined` (native `dado test` still runs UBSan over the same C).
- The web: a program that only prints no longer declares host imports it never calls (`dado.exit`, `spawn`, `cores`).
- ERR0803 and ERR0826 no longer refuse these safe programs: views appended through a `^ref []T`; a scalar from a tuple `try` beside an arena's `ref`; a copy of what a `^const T` points at, through a view slot or a string slot; a `ref` made fresh on each pass of a loop.
- A generic type first used as a slot of a `type` had no methods.
- An allocator pair spelled through `mem.Proc` read its slots only if something else had already used the allocator shape.
- A package's own trait named `Sink` is what its files mean by the bare name, over the compiler's `#render` `Sink`.
- A generic function's failure enum is resolved in the function's own file, so an unqualified `Error` no longer resolves in the caller's package.
- gcc's `-Werror=type-limits` no longer flags the C of a comparison the types decide (`if x < 0:` over a `u8`), and gcc `-O2`'s `-Wmaybe-uninitialized` no longer flags a failable `try` whose `else` carries on.
- `#fence` compiles under ThreadSanitizer.
- `dado update` rebuilt the driver as a dev build, leaving an object cache in the install's checkout that made the next update refuse; it rebuilds with `--release`.
- `dado test` on Windows reads the C runtime's abort exit code 3 as 134, so a `@test 134` file passes.
- `strings.repeat("", n)` looped `n` empty rounds; `repeat` and `replace_n` refuse a size an `i64` cannot count.
- `strings.clone_to_cstring` and `clone_from_cstring` no longer write through a failed allocation.
- `cfg`, `flags`, `strings` and `vendor:tui`: every refused allocation reaches the failure answer, instead of storing nil, leaking, or indexing a failed handle.
- `cfg`: a setter or `parse` whose copy failed stored nil or leaked; `put` keeps the old value, and `parse` counts such lines as bad.
- `cfg.slurp` no longer reports every read failure as `IS_DIR`.
- `flags`: `error()` no longer answers `""`, and `help` no longer answers text with words missing, when memory runs out.
- `fs.glob` no longer reports a malformed pattern as not found.
- `os`: on Windows, `get_env`, `set_env` and `unset_env` fail `NO_MEMORY` (were `INVALID`) when a copy is refused, and `set_env` no longer takes `"a\0b"` as `"a"`.
- `os`: on Windows, a seek before the start of a file fails `INVALID`, as on POSIX (was `UNKNOWN`).
- `io` and `os`: `EBUSY` and a Windows sharing violation are both `BUSY`.
- `math.fused_multiply_add` on Windows answered NaN when `x * y` underflowed below the smallest subnormal; it uses Dado's correctly rounded fma there.
- `enet.send` no longer leaks a packet ENet refused, and `enet.poll` reports a service error instead of nothing happening.
- `http`: `build_request` no longer answers a truncated request when memory runs out, and `read_response`'s limit no longer misses a response that arrives whole.
- `mem/slab.reset` no longer re-zeroes a whole kept chunk.
- `mem/libc` clamps a zero-size allocation.
- `slices.sort` stays fast over many equal keys.
- `vendor:tui`: cell grids come from the allocator given (were always C's heap), and a frame no longer runs on a grid that could not be allocated.

### Removed

- `core:time`: `now`, `since`, `ms_since` and `nap`; use `instant()`, `elapsed()`, `Duration` and `sleep(milliseconds(n))`.
- `core:mem/heap`, the whole package (deprecated, with two memory bugs).
- `core:http`'s global-buffer server (`poll`, `method`, `path`, `body`, `respond`, `respond_cors`, `Method`, `Accepted`, `valid`, `open`); use `core:http/server`.
- `core:os`: the `cstring` functions `arg`, `all`, `has_arg`, `env` and `has_env` (use `os.args` and `os.get_env`), and `os.magnitude`.
- `core:os/fs`: the `cstring` generation: `Dir`, `open`, `next`, `close`, `opened`, `cwd`, `set_cwd`, `bytes`, `make_dir`, `symlink`, `read_link`, `real_path`; use `read_dir`, `current_dir`, `change_dir`, `info`, `create_dir`, `make_symlink`, `link_target`, `canonical`.
- `core:io`: `try_open` (use `open`), `delete`, `move`, `try_delete`, `try_move` (use `fs.remove_file` and `fs.rename`), `opened`, `line_capacity`, and the error constants (use `os.Error`).
- `core:os/proc`: `opened`, `spawned` and `command_line_utf8`.
- `core:strings/parse`: the lenient `to_i32(cstring)`; `to_i32` is the strict parser.
- `core:slices.clone_with(xs, a)`; write `using a: slices.clone(xs)`.
- `core:hash`: `sha1_hex`, `sha256_hex`, `sha512_hex`, `sha1_equal`, `sha256_equal`, `sha512_equal`, `sha1_equal_hex`, `sha256_equal_hex`, `sha512_equal_hex`; use `hex`, `digest_equal` and `equal_hex`.
- `core:log`: `level_name` (use `level_label`) and `emit`.
- `core:threads`: the `ok()` method of semaphores, wait groups and barriers.
- `core:net/enet.host_valid`.
- `vendor:tui`: `open_over` and `open_headless_over`; `open` and `open_headless` inside a `using` block take its allocator.

### Known issues

- `core:script`'s tests fail on the web (six, "could not load module … the build wrote no file for it"); they pass natively.
- `app:config`'s and `app:net`'s tests fail on the web: they save a file and listen on a port, which the web has no host for.
- ERR0803 refuses `defer w.destroy()` beside `return w.finish()` (the writer pattern of `core:json` and `core:cbor`), though `finish` hands the text over. This false refusal is new in rc.5 and will be fixed in rc.6; until then, drop the `defer` and call `destroy` on each early return.
- The escape and borrow checks still accept some unsafe programs (views stored where the checker does not follow them, package handles copied through places it does not follow, a callee that hands a block over on only some paths), and still refuse two safe ones (`keep(p)` through a pointer variable; a slot handed over and then released). The language reference's limitations section lists them; each was accepted or refused before rc.5 too.
- The web: an inline crossing still takes the slow path, and throws after `main` returns, for a stored `string8` longer than 16 KiB or a `char32`/`char16` argument that is not one character; `--emit=strategies` reports it as `encoded` or `checked`.
- `#default` read as a value at depth 0 is nil in a program that allocates nothing; `runtime.init_scratch` falls back to `#heap`.
- `io.read_all` can be called only inside `core:io` (ERR0920 from another package), and calling a trait-taking function across packages is ERR0920.
- Traits: an implementer whose only slot is a pointer cannot read it in its methods (ERR0410); dispatch to an implementer whose slots all have one type emits C that does not compile.
- `dado api` lists each per-platform version of a function separately, lists a private type's method as public in `core:path`, and shows no declarations for `core:net/enet`.
- `dado install` still refuses DadoScript on Windows.
- A `--web-threads` worker's 1 MiB stack is not guarded against overflow.

## Earlier releases

1.0.0-rc.1 to 1.0.0-rc.4 have no entries here; what they changed is in the repository's git
history and on each release's GitHub page.
