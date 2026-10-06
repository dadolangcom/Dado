<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# Collection packages

Every package in the collections, with the first line of its own comment. `dado doc <package>` prints a page, and `dado api <package>` only its signatures.

## app

* [app:app](app/app.md) — the runtime's engine room.
* [app:body2d](app/body2d.md) — Godot's `CharacterBody2D`: an axis-aligned box swept against…
* [app:config](app/config.md) — self-provisioning settings, over core:cfg.
* [app:display](app/display.md) — the display server.
* [app:display/src](app/display/src.md) — the private sokol substrate behind app:display.
* [app:draw](app/draw.md) — a retained 2D graphics server over `vendor:sokol/sokol_gp`.
* [app:events](app/events.md) — retired onto channels. This file is the migration note, and the…
* [app:font](app/font.md) — a glyph atlas and the metrics a layout engine needs.
* [app:graph](app/graph.md) — the neutral scene graph: parent/child, composed…
* [app:graphical](app/graphical.md) — the graphical runtime: a real sokol backend, a window, a…
* [app:headless](app/headless.md) — the same runtime with nothing on the other end of it.
* [app:input](app/input.md) — a poll-based input subsystem.
* [app:input/sokol](app/input/sokol.md) — the sokol decode: `sapp_event` in, `app:input`'s queue out.
* [app:log](app/log.md) — thread-safe logging from any worker, flushed at the barrier.
* [app:net](app/net.md) — the RPC-and-packets escape hatch on top of `core:net`: framed…
* [app:sprite](app/sprite.md) — a sheet cut into frames, a resource holding **named**
* [app:text](app/text.md) — the pen loop, written once, recorded into a canvas item.
* [app:ui](app/ui.md) — a UI framework whose layout and text are bound to a monospace cell…
* [app:ui/cell](app/ui/cell.md) — the vocabulary of a cell-grid UI: the cell, the rectangle in…
* [app:ui/grid](app/ui/grid.md) — the cell-grid renderer: a screen of `cell.Cell`s, the glyph…

## core

* [core:cfg](core/cfg.md) — a native INI reader/writer over core:io and core:os/fs.
* [core:chars](core/chars.md) — C's `<ctype.h>`, told the truth about.
* [core:flags](core/flags.md) — command lines: flags, positionals, subcommands, and the help…
* [core:fmt](core/fmt.md) — where rendered bytes go, and what shape they arrive in.
* [core:geom/line](core/geom/line.md)
* [core:geom/point](core/geom/point.md)
* [core:geom/polygon](core/geom/polygon.md)
* [core:geom/rect](core/geom/rect.md)
* [core:geom/xform](core/geom/xform.md) — a 2D affine transform, the scene graph's mechanism half.
* [core:hash](core/hash.md) — hashes over bytes. Today: SHA-256.
* [core:http](core/http.md) — a minimal, non-blocking HTTP/1.1 server over TCP.
* [core:http/server](core/http/server.md) — a small web server that reads like Python's.
* [core:io](core/io.md) — files and the standard streams, through C's `<stdio.h>`.
* [core:jobs](core/jobs.md) — a general task system and the worker pool the runtime parallelises…
* [core:linalg](core/linalg.md) — vector and matrix maths, written once, over generics.
* [core:log](core/log.md) — levels, a fixed-shape log record, and pluggable sinks.
* [core:math](core/math.md) — the parts of C's `<math.h>` a program reaches for.
* [core:mem](core/mem.md) — the allocator interface, and the one helper an allocator with…
* [core:mem/arena](core/mem/arena.md) — a bump arena that **grows**, behind the allocator interface.
* [core:mem/bump](core/mem/bump.md) — a cursor over a block, behind the allocator interface.
* [core:mem/gc](core/mem/gc.md) — a conservative mark-sweep collector behind `mem.Allocator`.
* [core:mem/heap](core/mem/heap.md) — a growable `List`, on the heap, by hand.
* [core:mem/libc](core/mem/libc.md) — C's heap behind the `Allocator` interface.
* [core:mem/slab](core/mem/slab.md) — size classes over large chunks, behind the allocator interface.
* [core:net](core/net.md) — a thin `Reliable`/`Unreliable` wrapper over `vendor:enet`.
* [core:os](core/os.md) — the process, and the arguments it was started with.
* [core:os/fs](core/os/fs.md) — what is on the disk.
* [core:os/proc](core/os/proc.md) — running another program, and reading what it says.
* [core:path](core/path.md) — the lexical algebra of file paths: join, clean, split, base,…
* [core:runtime](core/runtime.md) — the scratch allocator, its checkpoints, and `TEMP_GUARD`, one…
* [core:script](core/script.md) — the native runtime a DadoScript program runs on: QuickJS-ng, the…
* [core:shader](core/shader.md) — what a compiled DadoGL shader looks like to the program that…
* [core:slices](core/slices.md) — ordering, searching and rearranging a run of anything.
* [core:strings](core/strings.md) — the operations `==` does not give you.
* [core:strings/parse](core/strings/parse.md) — text to numbers.
* [core:threads](core/threads.md) — portable threads, locks, condition variables and atomics.
* [core:time](core/time.md) — measuring how long something took, and waiting.

## vendor

* [vendor:enet](vendor/enet.md) — Dado bindings for the zpl-c/enet single-header fork of ENet…
* [vendor:raylib](vendor/raylib.md) — a restatement of the parts of raylib 6.x a game reaches for.
* [vendor:sokol/sokol_app](vendor/sokol/sokol_app.md) — Generated by Dado's binding generator from sokol_app.h.
* [vendor:sokol/sokol_args](vendor/sokol/sokol_args.md) — Generated by Dado's binding generator from sokol_args.h.
* [vendor:sokol/sokol_audio](vendor/sokol/sokol_audio.md) — Generated by Dado's binding generator from sokol_audio.h.
* [vendor:sokol/sokol_fetch](vendor/sokol/sokol_fetch.md) — Generated by Dado's binding generator from sokol_fetch.h.
* [vendor:sokol/sokol_gfx](vendor/sokol/sokol_gfx.md) — Generated by Dado's binding generator from sokol_gfx.h.
* [vendor:sokol/sokol_glue](vendor/sokol/sokol_glue.md) — bridges sokol_app's window to sokol_gfx's swapchain.
* [vendor:sokol/sokol_gp](vendor/sokol/sokol_gp.md) — Generated by Dado's binding generator from sokol_gp.h.
* [vendor:sokol/sokol_log](vendor/sokol/sokol_log.md) — Generated by Dado's binding generator from sokol_log.h.
* [vendor:sokol/sokol_time](vendor/sokol/sokol_time.md) — Generated by Dado's binding generator from sokol_time.h.
* [vendor:stb/stb_truetype](vendor/stb/stb_truetype.md) — Generated by Dado's binding generator from stb_truetype.h.
* [vendor:tui](vendor/tui.md) — a terminal UI you can throw together in seconds.
