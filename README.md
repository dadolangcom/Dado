<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="graphics/dado-banner-dark.svg">
    <img alt="Dado" src="graphics/dado-banner.svg" width="640">
  </picture>
</p>

Dado is a programming language with dialects for systems programming, scripting and GPU
programming. The dialects share a single syntax tree and one compiler, and the compiler generates
the glue between them. Systems code compiles to C11, which any C compiler can build into a native
program or WebAssembly. Scripts compile to JavaScript and run in an embedded engine, under Node or
in a browser. DadoGL, the graphics dialect, is designed but not built: the compiler recognises its
files and refuses them with a message saying so.

## A first program

```dado
package main

import "core:fmt"

type Point:
    f64 x,
    f64 y,

f64 dot(Point a, Point b):
    return a.x * b.x + a.y * b.y

i32 main():
    [3]Point path = ((0, 1), (3, 4), (6, 8))
    for p in path:
        fmt.println(p.x, ", ", p.y, " -> ", dot(p, path[1]))
    return 0
```

```
$ dado run hello
0, 1 -> 4
3, 4 -> 25
6, 8 -> 50
```

Everything is a tuple. Function arguments, structs, arrays, unions and multiple return values
are all the same construct, and a type is a shape rather than a name. `Point` is two `f64` slots
with a reading of them as `x` and `y`, which is why the compiler writes it as `Point = [2]f64`
below, and why a tuple literal like `(3, 4)` is already a `Point`. Unless a type is declared
`distinct`, two types with the same shape are the same type, and a name only adds a way of
reading it.

The whole program, imports included, becomes one C file, so there is no link step between
packages.

## Owning memory and viewing it

```dado
package main

import "core:fmt"

i32 main():
    ref []i64 owned = #make([]i64, 3)
    owned[0] = 7
    #append(owned, 8)
    []i64 window = owned[:]
    fmt.println(#len(owned), " ", window[0])
    #delete(owned)
    return 0
```

```
4 7
```

Heap memory is reached only through `ref`. A `ref []i64` owns its storage and carries the
allocator it came from, so it can grow, shrink and be freed. A plain `[]i64` is a view: a pointer
and a length over storage it doesn't own. Views are cheap to pass around, and the compiler refuses
anything that would change the storage behind them. Writing `#append(window, 8)` is an error
(ERR0814), because a view has nothing it could grow. `ref` is a capability, not a borrow checker:
it doesn't track lifetimes, so it won't catch a double free.

## When it's wrong, it says why

Diagnostics name the rule that was broken and show what the compiler saw. Ask for a field that
isn't there:

```
error[ERR0410]: `z` names axis 2 and Point = [2]f64 has 2 slots; its axes are `x`, `y` and `r`, `g`
  --> hello/main.dado:10:26
  10 |     return a.x * b.x + a.z * b.y
                                ^
```

Pass a number where a `Point` belongs:

```
error[ERR0504]: slot 1 of `dot` is Point = [2]f64, and this is f64
  --> hello/main.dado:15:52
  15 |         fmt.println(p.x, ", ", p.y, " -> ", dot(p, path[1].x))
                                                          ^^^^^^^^^
```

Every code has a longer explanation: `dado explain ERR0410`.

## Three dialects

| | Dialect | Files | Compiles to | Status |
|:-:|---|---|---|---|
| <picture><source media="(prefers-color-scheme: dark)" srcset="graphics/dado-dark.svg"><img src="graphics/dado.svg" width="40" alt=""></picture> | Dado: systems | `.dado` | C11, then native code or WebAssembly | usable |
| <picture><source media="(prefers-color-scheme: dark)" srcset="graphics/dados-dark.svg"><img src="graphics/dados.svg" width="40" alt=""></picture> | DadoScript: scripting | `.dados` | JavaScript, run by an embedded engine, Node or a browser | usable |
| <picture><source media="(prefers-color-scheme: dark)" srcset="graphics/dadogl-dark.svg"><img src="graphics/dadogl.svg" width="40" alt=""></picture> | DadoGL: graphics | `.dadogl` | SPIR-V, then WGSL, GLSL, HLSL and MSL | designed, not built |

The dialects meet at declarations. DadoScript code is written as classes, either in `.dados`
files or in `class` blocks inside a Dado package. Dado code creates instances and calls their
methods, and scripts call Dado functions by name. The compiler checks both sides and generates
the code that crosses between them, including the boundary between WebAssembly and JavaScript
when a program runs in a browser.

## Install

You need git and a network connection. On macOS and Linux:

```sh
curl -fsSL https://raw.githubusercontent.com/dadolangcom/Dado/main/install.sh | sh
```

On Windows, in PowerShell:

```powershell
irm https://raw.githubusercontent.com/dadolangcom/Dado/main/install.ps1 | iex
```

The script downloads the release archive for your machine from this repository's
[Releases](../../releases), checks it against the release's `SHA256SUMS`, unpacks it into
`~/dado` (`%LOCALAPPDATA%\dado` on Windows), and runs the installer inside it. Pass options after
`sh -s --`, for example `... | sh -s -- --dir ~/tools/dado --version 1.0.0`.

Downloaded the archive in a browser instead? Unzip it somewhere permanent and run `sh install.sh`
(or `install.bat` on Windows) from a terminal in that folder. Double-clicking the script is not
supported. The script clears the download mark from its own folder, which macOS and Windows put on
files from a browser, and then runs `./dadoc bootstrap`, which you can also run yourself.

What the install does:

1. downloads the pinned version of [Zig](https://ziglang.org) to use as the C compiler, and
   refuses it unless its SHA-256 matches the one built into `dadoc` (a `zig` of exactly that
   version already on your PATH is used instead);
2. clones this repository into `src/` at the tag of the release you downloaded;
3. builds `dado` and the bundled tools from that source with Zig;
4. links `dado`, `dadoc` and `dado-lsp` into `~/.local/bin` (on Windows, adds the install's `bin`
   folder to your user PATH), and tells you the line to add if `~/.local/bin` is not on your PATH;
   it never edits a shell profile;
5. sets up the editors it finds (the VS Code family, and Claude Code if it is installed), after
   asking which;
6. runs `dado doctor`.

Everything else stays in the install folder. `dado uninstall` reverses every change the install
made outside it, newest first, and then deletes the folder.

`dado` asks this repository, at most every 15 minutes, whether the collections have moved. On a
terminal it asks before updating; elsewhere it prints one line and carries on. `dado update`
applies new collections, never past the oldest compiler they need (`DADOC_MIN` in this
repository), and `dado upgrade` installs a newer compiler release. Each has `--rollback`.

Windows is cross-compiled and tested under Wine, not on a Windows machine. Known gaps there:
programs that embed DadoScript cannot be built on Windows yet (the script engine's build is a
POSIX shell script), and the installer has not been run end to end on Windows.

## Documentation

The documentation is generated by the compiler of the release you installed, so it describes
that release. The same pages are in [`docs/`](docs).

| Command | Shows |
|---|---|
| `dado doc reference` | the language reference |
| `dado doc core:strings` | one package's documentation |
| `dado api core:strings` | a package's signatures only |
| `dado explain <code>` | the full explanation of a diagnostic |
| `dado search <words>` | matching pages and diagnostics |
| `dado agent` | the primer for AI coding agents (also [AGENTS.md](AGENTS.md)) |

## This repository

```
collections/core     the standard library: memory, strings, I/O, threads, networking, HTTP
collections/app      windows, input, drawing, UI and fonts for graphical programs
collections/vendor   bindings to enet, raylib, sokol, stb and a terminal UI
tools/               dado itself and the tools it runs: doc, webdev, dashboard
docs/                the generated documentation
```

Releases of the compiler are on the [Releases](../../releases) page. This repository is
generated from the project's development repository on each release and collections update, so
pull requests here cannot be merged directly; issues are welcome.

## How Dado is built

Claude, Anthropic's AI model, writes most of Dado's code, tests and documentation, including
this file, working as a team of agents. One human maintainer designed the language, directs the
work and decides what ships. Work merges into the main branch only after it passes
these checks:

- The compiler is the specification. There is no separate spec document to drift from it, and
  the reference documentation is generated from the compiler.
- The test programs are compiled by two different C compilers, GCC and Clang, and their behaviour
  is compared.
- Generated C must compile without warnings.
- Tests run under AddressSanitizer and UndefinedBehaviorSanitizer, and threaded code under
  ThreadSanitizer.
- The test suite is itself tested: the compiler is deliberately broken in small ways (mutation
  testing), and a test has to fail each time.
- An install from a release archive into an empty home folder, followed by `dado uninstall`, has
  to leave that folder exactly as it was, file for file.

## Versions

`dadoc` and `dado-lsp` share one version, `major.minor.patch`, and follow semantic versioning.
Within major version 1 the language and the diagnostic codes are stable: a program that builds
with 1.0 builds with every 1.x, and `ERR0504` keeps meaning what it means.

The collections and the tools, `dado` included, are not versioned. They are beta, they move
freely, and `dado update` brings them up to date. Each update records the oldest compiler it
works with, and an install never takes collections its compiler is too old for.

## A note from the maintainer

Dado is a passion project originally designed as a toy language so I could explore an idea I had
for a type system. The type system is the "everything is a tuple" shape unification that Dado
practices. This concept actually worked way better than I expected and I continued to develop the
language. Eventually I decided I wanted to use the language in production at work for my real job
and started using Claude to accelerate development. While it would be super fulfilling to spend
the next 10 years of my life developing these concepts personally and maintaining my own language,
I have to work for a living, and I need these tools right now. For that reason, Claude is the
implementor and maintainer of most of the source code and will continue to be.

Dado was never meant to replace any other language out there or even be a legitimate alternative
to anything. It's written for me, so I can write code the way I like to write code, and
specializes in automating the things I don't like doing: namely, the boundaries between wasm/js
and systems/graphics languages. Its memory semantics using the `ref` type also exist to keep the
concept of memory ownership and memory viewership explicit. Every decision Dado makes in its
design is there to cater to my weaknesses, and is not an opinion or statement about how
"programming done right" looks.

## License

MIT. Bundled third-party code keeps its own license; see
[THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES).
