//@ build/manifest 10
//@ ### The manifest: the build's public surface, as JSON
//@
//@ `dadoc --emit=manifest` writes one JSON document describing what a
//@ build publishes — **public declarations only**, from both dialects:
//@
//@     dadoc --emit=manifest --package game
//@     ./dado build game --manifest=h,dts
//@
//@ * **Dado's `export`s** (`dado.functions`): each one's Dado signature, its C
//@   symbol and C prototype, the failure convention a failable one keeps —
//@   an `int32_t` code, and the destination its value is written through —
//@   and on the web its wasm signature;
//@ * **the shapes those signatures reach** (`dado.types`): tuples, slices,
//@   strings, maps, unions, enums and `distinct` tags, each with its C name,
//@   its members and, where the compiler can say, its size, alignment and
//@   every member's offset — for the target named at the top
//@   (`target.pointer_bytes`), never another;
//@ * **what the scripts publish** (`dados.classes`, `dados.functions`): each
//@   exported class with its constructor, fields and methods, each package
//@   function, every type as the DadoScript spelling and as what a JavaScript
//@   caller receives;
//@ * **the public channels** (`dado.channels`): name, kind, capacity and the
//@   index a reader of the channel directory finds it at.
//@
//@ Every entry carries its `dialect`, its `span` (file, line, column and
//@ byte range) and its `doc`: the `//` comment lines directly above the
//@ declaration, with no blank line between. Private declarations and
//@ function bodies are never in it.
//@
//@ The first field, `"schema": "manifest/1"`, names the format: a reader
//@ checks it, and a change that would break a reader changes the number.
//@ The format is `tools/manifest/schema.json`, a JSON Schema the gate
//@ validates every manifest against.
//@
//@ **Other formats are extensions**: separate programs that read the
//@ manifest on standard input and write their one output to standard
//@ output. Two are official — `h`, a C header declaring every Dado export
//@ with the types it needs, and `dts`, the TypeScript declarations of what
//@ the scripts publish (`--emit=js-package`'s `index.d.ts` is written by
//@ it). `./dado build --manifest=h,dts` runs them beside the build; a path
//@ in the list names a third-party one.
