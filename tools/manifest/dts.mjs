// The official `.d.ts` extension of the build manifest (`dadoc --emit=manifest`,
// schema `manifest/1`): what a package of DadoScript scripts publishes, declared for
// TypeScript. It reads the manifest on standard input and writes the
// declarations to standard output.
//
//     dadoc --emit=manifest --package game | node tools/manifest/dts.mjs > game.d.ts
//     ./dado build game --manifest=dts
//
// `dadoc --emit=js-package` runs this text to write the package's `index.d.ts`
// (dadoc carries it, so the two cannot differ). It declares exactly what the
// package's `index.js` publishes: each exported class with its constructor,
// fields and methods, and each package function; an `int` is
// `number | bigint`, a Number up to 2^53 - 1 and a BigInt past it.
//
// Self-contained on purpose — no import, no file read — because dadoc hands it
// to node as a string (`node --input-type=module -e`).

const SCHEMA = "manifest/1";

const PRELUDE = `/** A DadoScript \`Error\`, as a JavaScript caller catches it from any entry of the package. */
export declare class DadosError extends Error {
    constructor(message: string, code?: number);
    /** A nonzero 32-bit code: 70 for a fault, or what \`fail\` raised. */
    readonly code: number;
    /** Where the script raised it. */
    readonly location: Location;
    /** The script's frames it passed through, innermost first. */
    readonly trace: readonly Location[];
}

/** A position in a script: its file, line and column, and the function. */
export declare class Location {
    private constructor();
    readonly file: string;
    readonly line: number;
    readonly column: number;
    readonly function: string;
}
`;

// A string as a JavaScript literal: printable ASCII as itself, the rest as
// `\uXXXX` UTF-16 units — the spelling `index.js` gives an enum member.
function literal(text) {
  let out = '"';
  for (const c of text) {
    if (c === '"') out += '\\"';
    else if (c === "\\") out += "\\\\";
    else if (c === "\n") out += "\\n";
    else if (c === "\r") out += "\\r";
    else if (c === "\t") out += "\\t";
    else if (c >= " " && c <= "~") out += c;
    else {
      for (let i = 0; i < c.length; i++) {
        out += "\\u" + c.charCodeAt(i).toString(16).padStart(4, "0");
      }
    }
  }
  return out + '"';
}

// A boundary type (`dados.*`'s `type`) as TypeScript.
function ts(t) {
  switch (t.kind) {
    case "int":
      return "number | bigint";
    case "float":
    case "byte":
      return "number";
    case "bool":
      return "boolean";
    case "string":
      return "string";
    case "any":
      return "unknown";
    case "void":
      return "void";
    case "error":
      return "DadosError";
    case "location":
      return "Location";
    case "array": {
      const inner = ts(t.of);
      return /[| ]/.test(inner) ? `(${inner})[]` : `${inner}[]`;
    }
    case "map":
      return `Map<${ts(t.key)}, ${ts(t.value)}>`;
    case "tuple":
      return `readonly [${t.elements.map(ts).join(", ")}]`;
    case "enum":
      return t.members.map(literal).join(" | ");
    case "function":
      return `((${t.params.map((p, i) => `arg${i}: ${ts(p)}`).join(", ")}) => ${ts(t.returns)})`;
    case "async":
      return `Promise<${ts(t.of)}>`;
    case "class":
      return t.name;
    default:
      throw new Error(`a boundary type of kind \`${t.kind}\` is not in ${SCHEMA}`);
  }
}

// A member's name: as written, or quoted when it is `constructor`, which
// would declare the constructor instead.
function member(name) {
  return name === "constructor" ? '"constructor"' : name;
}

function params(list) {
  return list.map((p) => `${p.js_name}: ${ts(p.type)}`).join(", ");
}

export function declarations(manifest) {
  if (manifest.schema !== SCHEMA) {
    throw new Error(`this extension reads ${SCHEMA}, and the manifest says ${JSON.stringify(manifest.schema)}`);
  }
  let out =
    `// index.d.ts: the package \`${manifest.package.name}\`, declared by dadoc from its checked DadoScript scripts.\n` +
    "// An `int` is `number | bigint`: a Number up to 2^53 - 1, a BigInt past it.\n\n";
  out += PRELUDE;
  for (const c of manifest.dados.classes) {
    out += `\nexport declare class ${c.js_name} {\n`;
    out += `    constructor(${params(c.constructor)});\n`;
    for (const f of c.fields) out += `    ${member(f.name)}: ${ts(f.type)};\n`;
    for (const m of c.methods) out += `    ${member(m.name)}(${params(m.params)}): ${ts(m.returns)};\n`;
    out += "}\n";
  }
  for (const f of manifest.dados.functions) {
    out += `\nexport declare function ${f.name}(${params(f.params)}): ${ts(f.returns)};\n`;
  }
  return out;
}

let text = "";
process.stdin.setEncoding("utf8");
for await (const chunk of process.stdin) text += chunk;
try {
  process.stdout.write(declarations(JSON.parse(text)));
} catch (e) {
  process.stderr.write(`dts: ${e.message}\n`);
  process.exitCode = 1;
}
