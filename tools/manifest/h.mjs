// The official C header extension of the build manifest (`dadoc --emit=manifest`,
// schema `manifest/1`): every Dado `export` as a C prototype, with the shapes,
// includes and macros its signature needs. It reads the manifest on standard
// input and writes the header to standard output.
//
//     dadoc --emit=manifest --package game | node tools/manifest/h.mjs > game.h
//     ./dado build game --manifest=h
//
// What it writes, in order: the include guard; the includes the signatures
// need (`dado.header.includes`); the attribute macros the prototypes wear —
// `DADO_FAILABLE` (the failure convention's mark, which a binding generator
// reads back as `!`), `DADO_NORETURN`, `DADO_UNUSED`, `DADO_SYMBOL` — each only
// when a prototype wears it, spelled as `dadoc --emit=h` spells it so a unit
// holding both headers sees one definition; the shapes' declarations
// (`dado.header.declarations`, the compiler's C for the shapes it invents);
// one `DADO_FAILABLE_<symbol>` mark per failable export; the prototypes, built
// from each function's `c` record; and the manifest's layouts as
// `_Static_assert`s, compiled only when the includer defines
// `DADO_MANIFEST_CHECK_LAYOUT` — a claim about the manifest's target, which a
// consumer on another ABI has no reason to be refused over.
//
// Self-contained, like `dts.mjs`: no import beyond node's process streams.

const SCHEMA = "manifest/1";

const MACROS = {
  noreturn: [
    "/* Functions whose every path panics or quits — `_Noreturn` is C11. */",
    "#if defined(__GNUC__)",
    "#define DADO_NORETURN __attribute__((noreturn))",
    "#else",
    "#define DADO_NORETURN",
    "#endif",
  ],
  unused: [
    "/* A parameter an exported body cannot read — a failable that only fails. */",
    "#if defined(__GNUC__)",
    "#define DADO_UNUSED __attribute__((unused))",
    "#else",
    "#define DADO_UNUSED",
    "#endif",
  ],
  failable: [
    "/* The failure convention's mark, which a binding generator reads back as `!`. */",
    "#if defined(__clang__)",
    '#define DADO_FAILABLE(kind) __attribute__((annotate("dado:failable:" #kind)))',
    "#else",
    "#define DADO_FAILABLE(kind)",
    "#endif",
  ],
  symbol: [
    "/* What Dado has no type for and the C compiler has to be told — `@symbol(…)`. */",
    "#if defined(__GNUC__)",
    "#define DADO_SYMBOL(a) __attribute__((a))",
    "#else",
    "#define DADO_SYMBOL(a)",
    "#endif",
  ],
};

// A C comment's text, with nothing in it that could close the comment.
function comment(text) {
  return text.replace(/\*\//g, "* /");
}

function prototype(f) {
  const c = f.c;
  const params = c.params.length
    ? c.params.map((p) => `${p.unused ? "DADO_UNUSED " : ""}${p.type} ${p.name}`).join(", ")
    : "void";
  let head = "";
  if (f.failable) head += `DADO_FAILABLE_${c.symbol} `;
  for (const a of c.attributes) head += `DADO_SYMBOL(${a}) `;
  if (c.noreturn) head += "DADO_NORETURN ";
  return `${head}${c.returns} ${c.symbol}(${params});`;
}

export function header(manifest) {
  if (manifest.schema !== SCHEMA) {
    throw new Error(`this extension reads ${SCHEMA}, and the manifest says ${JSON.stringify(manifest.schema)}`);
  }
  const dado = manifest.dado;
  const fns = dado.functions;
  // No C was emitted (scripts alone): a header saying there is no C surface.
  const head = dado.header || { guard: `DADO_PKG_${manifest.package.root.toUpperCase().replace(/[^A-Z0-9_]/g, "_")}_H`, includes: [], declarations: "" };
  const guard = head.guard;
  const lines = [];
  lines.push(`/* Generated from the build manifest of package \`${comment(manifest.package.root)}\` (${SCHEMA}) by its C header extension. Do not edit. */`);
  lines.push(`/* The C surface of every Dado \`export\`, and the shapes they name, for ${comment(manifest.target.triple)}. */`);
  lines.push("");
  lines.push(`#ifndef ${guard}`);
  lines.push(`#define ${guard}`);
  lines.push("");
  if (head.includes.length) {
    for (const inc of head.includes) lines.push(`#include ${inc}`);
    lines.push("");
  }
  const wears = {
    noreturn: fns.some((f) => f.c.noreturn),
    unused: fns.some((f) => f.c.params.some((p) => p.unused)),
    failable: fns.some((f) => f.failable),
    symbol: fns.some((f) => f.c.attributes.length > 0),
  };
  for (const name of ["noreturn", "unused", "failable", "symbol"]) {
    if (wears[name]) lines.push(...MACROS[name], "");
  }
  const declarations = head.declarations.trimEnd();
  if (declarations) lines.push(declarations, "");
  const failable = fns.filter((f) => f.failable);
  if (failable.length) {
    lines.push("/* failable: each answers 0 when it succeeded and wrote its destination, or a nonzero code when it wrote nothing */");
    for (const f of failable) {
      lines.push(`#define DADO_FAILABLE_${f.c.symbol} DADO_FAILABLE(${f.failable.destination ? "destination" : "code"})`);
    }
    lines.push("");
  }
  if (fns.length) {
    lines.push("/* prototypes */");
    for (const f of fns) {
      if (f.doc) lines.push(`/* ${comment(f.doc).split("\n").join("\n * ")} */`);
      lines.push(`/* ${comment(f.signature)} */`);
      lines.push(prototype(f));
    }
    lines.push("");
  }
  const laid = dado.types.filter((t) => t.layout);
  if (laid.length) {
    lines.push(`/* The manifest's layouts, for a target whose pointer is ${manifest.target.pointer_bytes} bytes: define DADO_MANIFEST_CHECK_LAYOUT to have the C compiler hold them. */`);
    lines.push("#if defined(DADO_MANIFEST_CHECK_LAYOUT)");
    lines.push("#include <stddef.h>");
    for (const t of laid) {
      const what = comment(t.name || t.dado).replace(/"/g, "'");
      lines.push(`_Static_assert(sizeof(${t.c}) == ${t.layout.size}, "manifest: ${what} is ${t.layout.size} bytes");`);
      lines.push(`_Static_assert(_Alignof(${t.c}) == ${t.layout.align}, "manifest: ${what} aligns to ${t.layout.align}");`);
      for (const m of t.members) {
        if (m.offset === null) continue;
        lines.push(`_Static_assert(offsetof(${t.c}, ${m.name}) == ${m.offset}, "manifest: ${what}.${m.name} is at ${m.offset}");`);
      }
    }
    lines.push("#endif");
    lines.push("");
  }
  lines.push(`#endif /* ${guard} */`);
  return lines.join("\n") + "\n";
}

let text = "";
process.stdin.setEncoding("utf8");
for await (const chunk of process.stdin) text += chunk;
try {
  process.stdout.write(header(JSON.parse(text)));
} catch (e) {
  process.stderr.write(`h: ${e.message}\n`);
  process.exitCode = 1;
}
