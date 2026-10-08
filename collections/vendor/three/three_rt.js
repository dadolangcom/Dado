// three_rt.js: the JS half of the dtsgen wrappers over lib/three.module.js. Generated; retired by Dado 1.1 CP-1.
import * as M from "./lib/three.module.js";

const W = Symbol("dtsgen.wrapper");
const wrappers = new WeakSet();
let skips = 0;
let pending;

// A wrapper's init: build (or adopt) the object and tie it to its wrapper; a
// base class's init under a subclass's builds nothing.
export function dts_make(self, name, ...args) {
    if (skips > 0) {
        skips--;
        return undefined;
    }
    let o = pending;
    pending = undefined;
    if (o === undefined) o = new M[name](...args);
    else if (!(o instanceof M[name])) throw new Error(`dtsgen: ${name} wraps ${o?.constructor?.name}`);
    o[W] = self;
    wrappers.add(self);
    return o;
}

export function dts_skip() {
    skips++;
}

export function dts_adopt(o, name) {
    if (o === null || o === undefined) throw new Error(`dtsgen: ${name} is ${o}`);
    pending = o;
}

// The wrapper already tied to `o` when it is a `name` (or a subclass of one).
export function dts_cached(o, name) {
    const w = o === null || o === undefined ? undefined : o[W];
    for (let p = w && Object.getPrototypeOf(w); p; p = Object.getPrototypeOf(p)) {
        if (p.constructor?.name === name) return w;
    }
    return undefined;
}

export function dts_nil(v) {
    return v === null || v === undefined;
}

export function dts_u(v) {
    if (wrappers.has(v)) return v.h;
    if (v instanceof Map) return dts_obj(v);
    return v;
}

export function dts_obj(m) {
    return Object.fromEntries([...m].map(([k, v]) => [k, dts_u(v)]));
}

export function dts_hs(xs) {
    return xs.map((x) => x.h);
}

export function dts_cb(target, method) {
    return (...a) => target[method](...a);
}

export function dts_null() {
    return null;
}

export function dts_call(o, name, ...args) {
    return o[name](...args);
}
