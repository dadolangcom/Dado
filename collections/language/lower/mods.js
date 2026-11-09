// dados:mods: what a script loading another by its path calls - `Script.load(path,
// ..args)` - and what takes a loaded script's object as a class of the program's.
//
// A mod is the module `dadoc --emit=mod` wrote: one class, importing nothing but
// `dados:runtime`. The host (`dados:host`, natively `core:script`'s `mods.dado`)
// reads `<path>.js`, compiles and evaluates it in the loading script's engine,
// and refuses any other import, naming it; this module constructs its class with
// the arguments the loader gave and marks the object as a mod's.
//
// A mod's object leaves an `any` into a class of the program's structurally
// (`__dados_from_mod`): every method of the class but `init` must be a function
// of the same arity on it, and every field must be on it. What the conversion
// answers is an object of the class's prototype whose methods and fields are the
// mod's, each answer and each read checked as an `any` leaving into its type,
// and the same object every time the same mod object is converted to the same
// class, so `==` stays identity.

import { __dados_fault, __dados_from_any } from "dados:runtime";
import { load as host_load } from "dados:host";

// `Script.load(path, ..args)`.
export function __dados_load(path, args) {
    const where = "Script.load(" + JSON.stringify(path) + ")";
    let ns;
    try {
        ns = host_load(path);
    } catch (e) {
        throw __dados_fault(where + ": " + (e instanceof Error ? e.message : String(e)));
    }
    const names = Object.keys(ns).filter((k) => !k.includes("$") && typeof ns[k] === "function");
    if (names.length !== 1) {
        throw __dados_fault(where + ": `" + path + ".js` exports " + (names.length === 0 ? "no class" : names.length + " classes") +
            ", and a script loaded by its path is one class, the module `dadoc --emit=mod` writes");
    }
    const o = new ns[names[0]](...args);
    Object.defineProperty(o, "$dados$mod", { value: path });
    return o;
}

// Every conversion made, by mod object and then by class.
const adapted = new WeakMap();

// `kind` is `["class", K, "Name", [methods, fields]]`, each method
// `[name, arity, answer kind or null]` and each field `[name, kind]`.
export function __dados_from_mod(v, kind) {
    if (v instanceof kind[1] || v === null || typeof v !== "object" || v.$dados$mod === undefined) {
        return __dados_from_any(v, kind);
    }
    let per = adapted.get(v);
    if (per === undefined) {
        per = new Map();
        adapted.set(v, per);
    }
    const done = per.get(kind[1]);
    if (done !== undefined) return done;
    const [methods, fields] = kind[3];
    const missing = [];
    for (const [name, arity] of methods) {
        if (typeof v[name] !== "function") missing.push("no method `" + name + "`");
        else if (v[name].length !== arity) missing.push("`" + name + "` takes " + v[name].length + " and `" + kind[2] + "." + name + "` takes " + arity);
    }
    for (const [name] of fields) {
        if (!(name in v)) missing.push("no field `" + name + "`");
    }
    if (missing.length > 0) {
        throw __dados_fault("the script loaded from `" + v.$dados$mod + "` is no " + kind[2] + ": " + missing.join(", "));
    }
    const a = Object.create(kind[1].prototype);
    for (const [name, , answer] of methods) {
        const f = answer === null
            ? (...xs) => { v[name](...xs); }
            : (...xs) => __dados_from_any(v[name](...xs), answer);
        Object.defineProperty(a, name, { value: f });
    }
    for (const [name, k] of fields) {
        Object.defineProperty(a, name, {
            get: () => __dados_from_any(v[name], k),
            set: (x) => { v[name] = x; },
        });
    }
    Object.defineProperty(a, "$dados$mod", { value: v.$dados$mod });
    per.set(kind[1], a);
    return a;
}
