# Dado, for agents

Dado is a systems language that transpiles to portable C11. A program is a directory of `.dado`
files (a package); `dado` builds it with a C compiler (the pinned `zig cc` by default) into a
native program, or into WebAssembly for a page. Blocks are indented under a `:`, a declaration
leads with its type, and every builtin is spelled with a `#`. Memory is explicit: values live
on the stack unless an allocator made them, and a failure is a code, never an exception.

This file is the whole primer, and it is short because the compiler's diagnostics carry the
rest: each one names the rule that was broken and what to write instead.

## The loop

1. Write or edit code.
2. Run `dado check <package>` (it type-checks without building; seconds, not minutes).
3. Read the first diagnostic. Its `help:` line is the repair. `dado explain <CODE>` gives the rule
   behind it and an example.
4. Fix it and go back to 2. Then `dado run <package>` or `dado test <package>`.

Fix the first diagnostic first: later ones are often the same mistake seen again.

## Where you will guess wrong

Dado looks like several languages and is none of them. These are the guesses that fail first,
with the diagnostic you will see:

| You might write | Dado takes | You will see |
|---|---|---|
| `fn add(a: i32) -> i32 {` or `func`, `def` | `i32 add(i32 a):` and an indented body | ERR0201 `expected ':'` |
| `let x = 1`, `var x`, `x := 1` | `i32 x = 1` (`auto x = f()` takes a call's type) | ERR0401 / ERR0201 |
| `struct P { x: i32 }` | `type P:` with one `i32 x` per indented line | ERR0201 |
| `len(xs)`, `append(xs, v)`, `make(...)` | `#len(xs)`, `#append(xs, v)`, `#make([]T, n)` | ERR0805 / ERR0405 |
| `println("hi")` with no import | `import "core:fmt"` then `fmt.println("hi")` | ERR0405 |
| `fmt.println("n=%d", n)` | `fmt.println("n=", n)`: arguments are printed in order, there are no format verbs | (it prints `n=%d3`) |
| `if n:`, `if p:` | `if n != 0:`, `if p != nil:`; a condition is a `bool` | ERR0702 |
| `while c:` | `for c:` (and `for i in 0..<n:`, `for x in xs:`) | ERR0201 |
| `elif` | `else if` | ERR0201 |
| `and`, `or`, `not` | `&&`, `\|\|`, `!` | ERR0201, ERR0408 |
| `i++`, `i--` | `i += 1` | ERR0201 |
| `for i in 0..n:` | `0..<n` (up to n) or `0..=n` (through n); a bare `..` spreads an argument list | ERR0607, ERR0408 |
| `null`, `NULL`, `None` | `nil` | ERR0408 |
| `a + b` on strings | `strings.concat(a, b)` or `#format(a, b)` | ERR0607 |
| `i64 n = x` where `x` is `i32`, `x * 1.5` | widening is implicit; narrowing and mixing are `i32(n)`, `f64(x) * 1.5` | ERR0504 |
| `f(a = 2)` | `f(a: 2)` for a named argument | ERR0604 |
| `[]` for an empty slice | `[]f64 xs` with no value is empty; `[1, 2]` is a fixed `[2]` | ERR0504 |
| `map[string]int`, `HashMap` | maps are built in: `{string8: i32} m = {}`, `m[k] = v`, `m[k] += 1`, `k in m`, `for k, v in m:` | ERR0201, ERR0401 |
| `x = m[k]` for a key that may be missing | a read is failable: `try c = m[k] else:` | ERR0611 |
| `try i32 c = f() else: c = 0` | the `else` of a declaring `try` must leave; declare `c` first and write `try c = f() else:` | ERR0716 |
| `import "os"`, `import "strings"` | `import "core:os"`; collections are `core:`, `app:`, `vendor:` | ERR0303 |
| `string` | `string8` | ERR0401 |
| `main()` reading `argv` | `i32 main(i32 argc, ^rawptr argv):` and `[]string8 args = os.args(argc, argv)` (`core:os`; `args[0]` is the program) | ERR0306 |
| `string8(buf)` on a `ref []char8` | `string8 s = buf`: binding lends the bytes as a view | ERR0504 |
| `switch` over an enum with some members | name every member and write `else:` (an enum value need not be a member) | ERR0705, LNT0731 |
| a `Result` or an exception for errors | `!T` on the return type, `fail <code>`, and `try … else code:` at the call | ERR0611 |
| `*p`, `p->x` | `p^`, `p^.x`; `&x` takes an address; `^T` is a pointer type | ERR0201 |
| an unused variable | delete it, or bind `_` | LNT0412 (an error under `--strict`) |

Integer and float widths are always written: `i8`…`i64`, `u8`…`u64`, `f32`, `f64`, `bool`,
`char8`, `string8` (UTF-8 bytes; `s[i]` is a `char8`, `s[a..<b]` a view), `rawptr`, `cstring`.

## A complete program

```dado
package main

import "core:fmt"
import "core:strings"

// A point: a tuple whose slots have names.
type Point:
    i32 x
    i32 y

// Fails with a code when the text is empty.
!i32 word_count(string8 text):
    if #len(text) == 0:
        fail 1
    i32 n = 1
    for i in 0..<#len(text):
        if text[i] == ' ':
            n += 1
    return n

i32 area(Point p):
    return p.x * p.y

i32 main():
    Point p = (3, 4)
    fmt.println("area: ", area(p))
    try i32 n = word_count("the quick brown fox") else code:
        fmt.println("failed with ", code)
        return 1
    ref []char8 shout = strings.concat("hello", "!")
    string8 said = shout
    fmt.println(said, " ", n)
    ref []i32 xs = #make([]i32, 0)
    _ = #append(xs, 7)
    #delete(xs)
    ^Point q = &p
    q^.x = 10
    return 0
```

`dado run hello` prints `area: 12` and `hello! 4`, and exits 0. `main` returns the exit status;
`quit n` exits from anywhere.

Allocation: `#make` takes from the current allocator (C's heap unless a scope says
otherwise); `ref` marks a value that owns what it points at, and `#delete` gives it back. A
`using arena.allocator(&a):` block (`core:mem/arena`) makes every allocation in it come from an
arena freed at once. `defer` runs a statement at the end of its scope.

## A package, and its tests

```
hello/
  main.dado          package main, with i32 main()
  words.dado         more of package main: every file in the directory is one package
  words_test.dado    a test: first line @test, and i32 test() returning 0 for a pass
```

Another package is a directory too, imported by path (`import "core:strings"`, or
`import "../util"` beside yours) and used qualified (`strings.trim(s)`). Declarations are
public unless marked `private`. A package that names itself anything but `main` (`package util`)
is a library: it needs no `main`, and `dado check util` checks it as one. A test file:

```dado
@test
package main

i32 test():
    bool ok = #expect(area((2, 3)) == 6, "area((2, 3)) is ", area((2, 3)))
    return 0 if ok else 1
```

`dado test hello` builds each test file into its own program and runs it under the sanitizers.

## The collections

Packages that come with the toolchain, imported as `"<collection>:<path>"`. `dado api <package>`
lists a package's signatures; `dado doc <package>` prints its page.

* core (portable, no graphics): `fmt` (printing), `strings` (search, split, trim, build),
  `strings/parse` (text to numbers), `chars`, `slices` (sort, search), `math`, `linalg`,
  `hash` (SHA-256), `io` (files and standard streams), `os` (arguments, environment, errors),
  `os/fs` (files and directories), `os/proc` (run a program, read its output), `path`,
  `flags` (command lines), `time`, `log`, `cfg` (INI), `threads`, `jobs`, `mem` (the allocator
  interface) with `mem/arena`, `mem/bump`, `mem/slab`, `mem/heap`, `mem/libc`, `mem/gc`,
  `runtime`, `net`, `http`, `http/server`, `geom/*`, `script` (DadoScript's runtime), `shader`.
* app (games and tools with a window): `app` and `graphical`/`headless` (the runtime),
  `display`, `draw`, `text`, `font`, `sprite`, `graph` (scene graph), `body2d`, `input`,
  `events`, `log`, `net`, `config`, `ui` (cell-grid UI).
* vendor (C libraries, bound): `raylib`, `sokol/*`, `stb/stb_truetype`, `enet`, and `tui`
  (terminal UIs).

## How to ask for more

Everything below answers from the toolchain you have installed, and takes `--format=json` for a
stable machine-readable answer.

| Ask | Command |
|---|---|
| what a diagnostic means | `dado explain ERR0504` |
| what a package offers | `dado api core:strings` |
| a package's page, or the language reference | `dado doc core:os/fs`, `dado doc reference` |
| search the docs and diagnostics | `dado search "named argument"` |
| one declaration, its doc comment, and every name it uses | `dado inspect core:strings.trim` |
| what a name means inside a declaration | `dado peek mypkg.main count` |
| everything that uses a declaration | `dado refs mypkg.area` |
| change whole declarations, checked before anything is written | `dado inspect mypkg.area -o area.dado`, edit the file, `dado apply area.dado` |
| where the toolchain is, and whether it is healthy | `dado where`, `dado doctor` |
| a new project | `dado new hello` |
| this guide | `dado agent` |

`dado apply` refuses (and writes nothing) when the new text does not parse or a name in it does
not resolve, and writes the change with its type errors listed otherwise; `--dry-run` checks
without writing, and `--strict` refuses on any error. For a small edit inside one declaration an
ordinary text edit and `dado check` is just as good.
