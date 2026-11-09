// dados/glue.js: the boundary between a JavaScript caller and a package of DadoScript
// scripts built as a library. The package's `index.js` publishes each exported
// class as a facade over the script's own class, and each entry - a constructor,
// a method, a field read or write, a published function - passes every argument
// through `$in` and every answer through `$out`. Calls between the scripts go to
// the scripts' own classes and never pass through here.
//
// A kind says what a value is at the boundary: "int", "float", "bool", "string",
// "byte", "any", "void", "error", "location", or ["[]", k], ["{}", k, v],
// ["()", [k, ...]] (a tuple), ["enum", [name, ...]], ["fn", [k, ...], r],
// ["async", k], ["class", <the script's class>, "<its name>"].
//
// `$in` checks a value a caller hands the script as an `any` leaving into the
// kind is checked: an int is a Number holding an integer within 2^53 - 1 or a
// BigInt within i64, canonicalised to its tier; an array, a map and a tuple are
// checked element by element; a facade becomes the script's instance it holds.
// A wrong value raises a DadoScript `Error` of code 70 naming the entry and the
// argument. `$out` hands a value the script holds to the caller as it is - an
// int a Number up to 2^53 - 1 and a BigInt past it - except that an instance
// of a published class leaves as its facade (the same facade every time), an
// `Error` as a `DadosError`, a function as one that checks what it is passed and
// a promise as one that settles to what `$out` makes of its value.
//
// An array or a map a caller hands in is the script's from then on: an int in
// it is canonicalised in place. One whose elements are facades, errors or
// functions is copied, since the script holds its own instances, never facades.
// An array or a map the script answers is handed out as it is, and copied only
// where an element leaves as something else.

import { __dados_Error, __dados_Location, __dados_failure, __dados_fault, __dados_str_any } from "./runtime.js";

const MAX_SAFE = 9007199254740991;
const MAX_SAFE_N = 9007199254740991n;

// Each facade and the script's instance it holds, both ways.
const instances = new WeakMap();
const facades = new WeakMap();

// Each published class's facade, by the script's class.
const published = new Map();

// Each `DadosError` and the script's `Error` it carries, both ways.
const raised = new WeakMap();
const thrown = new WeakMap();

// Set while `DadosError`'s constructor adopts an `Error` the script made.
let adopting = null;

// What a value is, for a message: a string quoted, the rest as `print` writes it.
function shown(v) {
    let text;
    try {
        text = typeof v === "string" ? JSON.stringify(v) : __dados_str_any(instances.has(v) ? instances.get(v) : v);
    } catch (e) {
        text = "<value>";
    }
    return text.length > 60 ? text.slice(0, 57) + "..." : text;
}

function kind_text(kind) {
    if (typeof kind === "string") return kind === "error" ? "Error" : kind === "location" ? "Location" : kind;
    switch (kind[0]) {
        case "[]": return "[]" + kind_text(kind[1]);
        case "{}": return "{" + kind_text(kind[1]) + ": " + kind_text(kind[2]) + "}";
        case "()": return "(" + kind[1].map(kind_text).join(", ") + ")";
        case "enum": return kind[1].map((m) => JSON.stringify(m)).join(" | ");
        case "fn": return "function";
        case "async": return "async " + kind_text(kind[1]);
        default: return kind[2];
    }
}

function article(text) {
    return /^[aeiou]/.test(text) ? "an " : "a ";
}

function refuse(where, kind, v) {
    const text = kind_text(kind);
    throw __dados_fault(where + " is not " + article(text) + text + ": " + shown(v));
}

// An int in its canonical tier, or `undefined`.
function as_int(v) {
    if (typeof v === "number") {
        return Number.isInteger(v) && v >= -MAX_SAFE && v <= MAX_SAFE ? v + 0 : undefined;
    }
    if (typeof v === "bigint" && BigInt.asIntN(64, v) === v) {
        return v >= -MAX_SAFE_N && v <= MAX_SAFE_N ? Number(v) : v;
    }
    return undefined;
}

// Whether converting a value of `kind` can answer another object than the one
// handed in: then a container holding one is copied rather than written.
function swaps(kind) {
    if (typeof kind === "string") return kind === "any" || kind === "error";
    switch (kind[0]) {
        case "[]": case "{}": case "()": return swaps(kind[1]) || (kind[0] === "{}" && swaps(kind[2])) ||
            (kind[0] === "()" && kind[1].some(swaps));
        case "enum": return false;
        default: return true;
    }
}

// `v`, handed in by a JavaScript caller at `where`, as the script holds a `kind`.
export function $in(kind, v, where) {
    switch (kind) {
        case "any": {
            if (instances.has(v)) return instances.get(v);
            if (raised.has(v)) return raised.get(v);
            return v;
        }
        case "int": {
            const n = as_int(v);
            if (n === undefined) refuse(where, kind, v);
            return n;
        }
        case "byte": {
            const n = as_int(v);
            if (n === undefined || n < 0 || n > 255) refuse(where, kind, v);
            return n;
        }
        case "float":
            if (typeof v !== "number") refuse(where, kind, v);
            return v;
        case "bool":
            if (typeof v !== "boolean") refuse(where, kind, v);
            return v;
        case "string":
            if (typeof v !== "string") refuse(where, kind, v);
            return v;
        case "error":
            if (raised.has(v)) return raised.get(v);
            if (v instanceof __dados_Error) return v;
            return refuse(where, kind, v);
        case "location":
            if (!(v instanceof __dados_Location)) refuse(where, kind, v);
            return v;
    }
    switch (kind[0]) {
        case "[]": {
            if (!Array.isArray(v)) refuse(where, kind, v);
            let out = v;
            for (let i = 0; i < v.length; i++) {
                const e = $in(kind[1], v[i], where + "[" + i + "]");
                if (e !== v[i]) {
                    if (out === v && swaps(kind[1])) out = v.slice();
                    out[i] = e;
                }
            }
            return out;
        }
        case "{}": {
            if (!(v instanceof Map)) refuse(where, kind, v);
            let copy = swaps(kind[1]) || swaps(kind[2]);
            const entries = [];
            for (const [k, e] of v) {
                const at = where + "[" + shown(k) + "]";
                const key = $in(kind[1], k, at + "'s key");
                const value = $in(kind[2], e, at);
                if (key !== k) copy = true;
                entries.push([k, key, e, value]);
            }
            if (copy) {
                const out = new Map();
                for (const [, key, , value] of entries) out.set(key, value);
                return out;
            }
            for (const [k, , e, value] of entries) {
                if (value !== e) v.set(k, value);
            }
            return v;
        }
        case "()": {
            if (!Array.isArray(v) || v.length !== kind[1].length) refuse(where, kind, v);
            let out = v;
            for (let i = 0; i < v.length; i++) {
                const e = $in(kind[1][i], v[i], where + "[" + i + "]");
                if (e !== v[i]) {
                    if (out === v) out = v.slice();
                    out[i] = e;
                }
            }
            return out;
        }
        case "enum":
            if (typeof v !== "string" || !kind[1].includes(v)) refuse(where, kind, v);
            return v;
        case "fn": {
            if (typeof v !== "function") refuse(where, kind, v);
            const params = kind[1];
            const ret = kind[2];
            return (...args) => {
                const out = [];
                for (let i = 0; i < params.length; i++) out.push($out(params[i], args[i]));
                const r = v(...out);
                return ret === "void" ? undefined : $in(ret, r, where + "'s answer");
            };
        }
        case "class": {
            const raw = instances.has(v) ? instances.get(v) : v;
            if (!(raw instanceof kind[1])) refuse(where, kind, v);
            return raw;
        }
    }
    throw __dados_fault(where + ": a value of no kind the boundary knows");
}

// `v`, which the script holds as a `kind`, as a JavaScript caller receives it.
export function $out(kind, v) {
    switch (kind) {
        case "int": case "float": case "bool": case "string": case "byte": case "location":
            return v;
        case "void":
            return undefined;
        case "error":
            return error_of(v);
        case "any":
            if (v instanceof __dados_Error) return error_of(v);
            if (v !== null && typeof v === "object" && published_of(v) !== undefined) return facade_of(v);
            return v;
    }
    switch (kind[0]) {
        case "[]": {
            let out = v;
            for (let i = 0; i < v.length; i++) {
                const e = $out(kind[1], v[i]);
                if (e !== v[i]) {
                    if (out === v) out = v.slice();
                    out[i] = e;
                }
            }
            return out;
        }
        case "{}": {
            if (!swaps(kind[1]) && !swaps(kind[2])) return v;
            const out = new Map();
            for (const [k, e] of v) out.set($out(kind[1], k), $out(kind[2], e));
            return out;
        }
        case "()": {
            let out = v;
            for (let i = 0; i < v.length; i++) {
                const e = $out(kind[1][i], v[i]);
                if (e !== v[i]) {
                    if (out === v) out = v.slice();
                    out[i] = e;
                }
            }
            return out;
        }
        case "enum":
            return v;
        case "fn": {
            const params = kind[1];
            const ret = kind[2];
            return (...args) => {
                try {
                    const given = [];
                    for (let i = 0; i < params.length; i++) {
                        given.push($in(params[i], args[i], "a script's function: argument " + (i + 1)));
                    }
                    return $out(ret, v(...given));
                } catch (e) {
                    throw $thrown(e);
                }
            };
        }
        case "async":
            return v.then((x) => $out(kind[1], x), (e) => {
                throw $thrown(e);
            });
        case "class":
            return facade_of(v);
    }
    return v;
}

// The published class an instance of the script's leaves as: its own class's,
// or the nearest published one it extends.
function published_of(raw) {
    let c = raw.constructor;
    while (typeof c === "function") {
        const f = published.get(c);
        if (f !== undefined) return f;
        c = Object.getPrototypeOf(c);
    }
    return undefined;
}

// The facade over the script's instance `raw`: the same one every time.
function facade_of(raw) {
    const known = facades.get(raw);
    if (known !== undefined) return known;
    const Facade = published_of(raw);
    if (Facade === undefined) throw __dados_fault("an instance of a class that is not published left the package");
    const f = Object.create(Facade.prototype);
    instances.set(f, raw);
    facades.set(raw, f);
    return f;
}

// `index.js`, once per published class: its facade, and the script's class.
export function $publish(Facade, Raw) {
    published.set(Raw, Facade);
}

// A facade's constructor: `f` holds the instance `raw` the script just made.
export function $bind(f, raw) {
    instances.set(f, raw);
    facades.set(raw, f);
}

// The script's instance a facade holds; refused for any other `this`.
export function $self(f, name) {
    const raw = instances.get(f);
    if (raw === undefined) throw __dados_fault(name + " was called on what is not a " + name.split(".")[0]);
    return raw;
}

// A DadoScript `Error`, as a JavaScript caller catches it and hands it back.
export class DadosError extends Error {
    constructor(message, code) {
        const raw = adopting !== null ? adopting : new __dados_Error(String(message), code === undefined ? 70 : code, null);
        adopting = null;
        super(raw.message);
        this.name = "DadosError";
        raised.set(this, raw);
        thrown.set(raw, this);
    }

    get code() {
        return raised.get(this).code;
    }

    get location() {
        return raised.get(this).location;
    }

    get trace() {
        return raised.get(this).trace;
    }

    toString() {
        return raised.get(this).toString();
    }
}

function error_of(raw) {
    const known = thrown.get(raw);
    if (known !== undefined) return known;
    adopting = raw;
    try {
        return new DadosError(raw.message, raw.code);
    } finally {
        adopting = null;
    }
}

// Whatever an entry threw, as a JavaScript caller catches it: a `DadosError`.
export function $thrown(e) {
    if (e instanceof DadosError) return e;
    return error_of(__dados_failure(e));
}
