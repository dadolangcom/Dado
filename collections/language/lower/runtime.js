// dados:runtime: what every module dadoc emits for a DadoScript class imports.
//
// DadoScript `int` is exactly i64, wrapping at 2^63, held in the smallest of three
// canonical tiers: a Number that is an int32, a Number that is an integer within
// +-(2^53 - 1), or a BigInt beyond. Every function below answers in that tier,
// so two equal ints are one representation and `===`, `<` and friends compare
// them as they are.
//
// The first six are the fork's intrinsics: in code QuickJS-ng (the fork) loads
// with JS_EVAL_FLAG_DADOS a call of one of these names compiles to one opcode
// (patch 0006) and the definition here is never called; on any other engine it
// is the function. Both give the same answer for every pair of ints, and
// `x / 0` and `x % 0` raise "integer division by zero" on both: the fork's
// opcode as the engine's RangeError, the function here as a DadoScript `Error`
// (`__dados_fault`, below), and a catch or a host sees both as the same
// `Error`, code 70 (`__dados_failure`).
// The rest are ordinary functions everywhere.
//
// `string(x)` of a float is rendered here and nowhere else: the shortest
// digits that read back as the same double (the engine's own `toExponential`)
// laid out as Python's `repr` lays them out, which is what Dado's
// `put_float_shortest` writes: `1.0`, `0.1`, `1e+16`, `1e-05`, `-0.0`, `nan`,
// `inf`.

const MAX_SAFE = 9007199254740991;
const MAX_SAFE_N = 9007199254740991n;

// A BigInt holding an i64, in its canonical tier.
function canon(r) {
    return r >= -MAX_SAFE_N && r <= MAX_SAFE_N ? Number(r) : r;
}

// Both operands as i64 BigInts, the operation, wrapped at 2^63.
function wide(a, b, op) {
    const x = BigInt(a), y = BigInt(b);
    switch (op) {
        case 0: return canon(BigInt.asIntN(64, x + y));
        case 1: return canon(BigInt.asIntN(64, x - y));
        case 2: return canon(BigInt.asIntN(64, x * y));
        case 3: return canon(BigInt.asIntN(64, x / y));
        case 4: return canon(BigInt.asIntN(64, x % y));
        case 5: return canon(BigInt.asIntN(64, x & y));
        case 6: return canon(BigInt.asIntN(64, x | y));
        case 7: return canon(BigInt.asIntN(64, x ^ y));
        default: return canon(BigInt.asIntN(64, x & ~y));
    }
}

function is_i32(v) {
    return typeof v === "number" && (v | 0) === v;
}

// `+ - *` on two Numbers: exact whenever the answer is within 2^53 - 1, since
// the exact integer rounds monotonically and 2^53 is a double. `+ 0` turns the
// -0 of `-5 * 0` into 0.
export function __dados_int_add(a, b) {
    if (typeof a === "number" && typeof b === "number") {
        const r = a + b;
        if (r >= -MAX_SAFE && r <= MAX_SAFE) return r + 0;
    }
    return wide(a, b, 0);
}

export function __dados_int_sub(a, b) {
    if (typeof a === "number" && typeof b === "number") {
        const r = a - b;
        if (r >= -MAX_SAFE && r <= MAX_SAFE) return r + 0;
    }
    return wide(a, b, 1);
}

export function __dados_int_mul(a, b) {
    if (typeof a === "number" && typeof b === "number") {
        const r = a * b;
        if (r >= -MAX_SAFE && r <= MAX_SAFE) return r + 0;
    }
    return wide(a, b, 2);
}

function zero_divisor(b) {
    if (b === 0 || b === 0n) throw __dados_fault("integer division by zero");
}

// Truncating, as Dado's `/` is. On two Numbers `a % b` is exact (a double
// remainder always is), so `a - a % b` is an exact multiple of `b` and the
// division exact too.
export function __dados_int_div(a, b) {
    zero_divisor(b);
    if (typeof a === "number" && typeof b === "number") return (a - a % b) / b + 0;
    return wide(a, b, 3);
}

// The sign of the dividend, as Dado's `%` is.
export function __dados_int_mod(a, b) {
    zero_divisor(b);
    if (typeof a === "number" && typeof b === "number") return a % b + 0;
    return wide(a, b, 4);
}

// Two int32s: JavaScript's 32-bit operator is the i64 one, sign bits and all.
export function __dados_int_and(a, b) {
    if (is_i32(a) && is_i32(b)) return a & b;
    return wide(a, b, 5);
}

export function __dados_int_or(a, b) {
    if (is_i32(a) && is_i32(b)) return a | b;
    return wide(a, b, 6);
}

export function __dados_int_xor(a, b) {
    if (is_i32(a) && is_i32(b)) return a ^ b;
    return wide(a, b, 7);
}

// Dado's `&~`, AND NOT; its `~` between two ints is XOR, above.
export function __dados_int_andnot(a, b) {
    if (is_i32(a) && is_i32(b)) return a & ~b;
    return wide(a, b, 8);
}

// `-x`, wrapping: the negation of i64's minimum is itself.
export function __dados_int_neg(a) {
    if (typeof a === "number") return 0 - a;
    return canon(BigInt.asIntN(64, -a));
}

export function __dados_int_not(a) {
    if (is_i32(a)) return ~a;
    return canon(BigInt.asIntN(64, ~BigInt(a)));
}

// A count outside 0..<64 has moved every bit out: `<<` answers 0, and `>>`
// (arithmetic) answers the sign, -1 or 0. That is Dado's rule for every width.
function in_range(n) {
    return n >= 0 && n < 64;
}

export function __dados_int_shl(a, n) {
    if (!in_range(n)) return 0;
    if (typeof a === "number") {
        // A double times a power of two is exact.
        const r = a * 2 ** Number(n);
        if (r >= -MAX_SAFE && r <= MAX_SAFE) return r + 0;
    }
    return canon(BigInt.asIntN(64, BigInt(a) << BigInt(n)));
}

export function __dados_int_shr(a, n) {
    // An int32 and a count under 32: JavaScript's `>>` is Dado's.
    if (typeof a === "number" && (a | 0) === a && n >= 0 && n < 32) return a >> n;
    if (!in_range(n)) return a < 0 ? -1 : 0;
    if (typeof a === "number") return Math.floor(a / 2 ** Number(n)) + 0;
    return canon(BigInt(a) >> BigInt(n));
}

// `int(x)` of a float: toward zero, saturating: NaN is 0 and anything at or
// past either end of i64 is that end, as Dado's conversion is.
export function __dados_int_of_float(x) {
    if (x !== x) return 0;
    if (x >= 9223372036854775808) return 9223372036854775807n;
    if (x < -9223372036854775808) return -9223372036854775808n;
    const t = Math.trunc(x);
    if (t >= -MAX_SAFE && t <= MAX_SAFE) return t + 0;
    return BigInt(t);
}

// `float(x)` of an int: the nearest double, ties to even.
export function __dados_float_of_int(a) {
    return typeof a === "bigint" ? Number(a) : a;
}

export function __dados_str_int(a) {
    return String(a);
}

export function __dados_str_bool(b) {
    return b ? "true" : "false";
}

export function __dados_str_float(x) {
    if (x !== x) return "nan";
    if (x === Infinity) return "inf";
    if (x === -Infinity) return "-inf";
    if (x === 0) return 1 / x < 0 ? "-0.0" : "0.0";
    // `d.ddde+N`: the fewest significant digits that read back as `x`.
    const text = x.toExponential();
    const neg = text.charCodeAt(0) === 45;
    const e_at = text.indexOf("e");
    const mantissa = text.slice(neg ? 1 : 0, e_at);
    const digits = mantissa.length > 1 ? mantissa[0] + mantissa.slice(2) : mantissa;
    const n = digits.length;
    // The value is 0.d1d2... x 10^dp.
    const dp = Number(text.slice(e_at + 1)) + 1;
    let out = neg ? "-" : "";
    if (dp > -4 && dp <= 16) {
        if (dp <= 0) return out + "0." + "0".repeat(-dp) + digits;
        if (dp >= n) return out + digits + "0".repeat(dp - n) + ".0";
        return out + digits.slice(0, dp) + "." + digits.slice(dp);
    }
    out += digits[0];
    if (n > 1) out += "." + digits.slice(1);
    const e = dp - 1;
    const magnitude = String(e < 0 ? -e : e);
    return out + (e < 0 ? "e-" : "e+") + (magnitude.length < 2 ? "0" : "") + magnitude;
}

// ---------------------------------------------------------------------------
// Containers and strings.
//
// `[]T` is a JavaScript Array and `{K: V}` a Map, both held by reference; a
// `string` is JavaScript's. What raises, raises with Dado's own words where Dado
// has the operation (`index out of bounds: ...`, `window out of bounds: ...`)
// and never answers `undefined`: every read below is checked first. A
// position is an `int` in its canonical tier, so a BigInt position is past
// every array and fails the same check. Every raise is a DadoScript `Error` of code
// 70, a fault (`__dados_fault`, in the failure section below).
//
// A `[]byte` is an Array of Numbers in 0..255: every store goes through
// `__dados_byte`, which keeps an int modulo 256 (the low eight bits of its
// two's complement), as narrowing into a byte does.

function out_of_bounds(i, n) {
    return __dados_fault("index out of bounds: " + i + " is not a position in a run of " + n);
}

function check_at(i, n) {
    if (!(i >= 0 && i < n)) throw out_of_bounds(i, n);
}

function check_window(lo, hi, n) {
    if (!(lo >= 0 && lo <= hi && hi <= n)) {
        throw __dados_fault("window out of bounds: " + lo + "..<" + hi + " is not inside a run of " + n);
    }
}

function empty(method) {
    return __dados_fault(method + " of an empty array: there is no element to answer");
}

// An int kept modulo 256.
export function __dados_byte(v) {
    return typeof v === "bigint" ? Number(BigInt.asUintN(8, v)) : v & 255;
}

// `xs[i]`, checked.
export function __dados_a_at(xs, i) {
    check_at(i, xs.length);
    return xs[i];
}

// The fault of `xs[i]` read or stored where `i` is no position: what an
// emitted site's own check calls (`(xs[i] ?? __dados_a_oob(xs, i))`), so the
// raise is on the site's line, as `__dados_a_at`'s is.
export function __dados_a_oob(xs, i) {
    throw out_of_bounds(i, xs.length);
}

// A fixed array's or a tuple's position, checked.
export function __dados_bounds(i, n) {
    check_at(i, n);
}

// `xs[i] = v`, checked; answers nothing a script reads.
export function __dados_a_set(xs, i, v) {
    check_at(i, xs.length);
    xs[i] = v;
}

// `a[i] = v` into a fixed array or a tuple, a value: a new array, the slot
// replaced, which the store puts in the place `a` names; every other holder
// of `a` keeps it as it was. Checked as `xs[i]` is.
export function __dados_t_with(t, i, v) {
    check_at(i, t.length);
    const copy = t.slice();
    copy[i] = v;
    return copy;
}

// `xs[lo..<hi]`: a new array; `hi` left off is the end.
export function __dados_a_slice(xs, lo, hi) {
    if (hi === undefined) hi = xs.length;
    check_window(lo, hi, xs.length);
    return xs.slice(lo, hi);
}

export function __dados_a_append_array(xs, vs) {
    const n = vs.length;
    for (let i = 0; i < n; i++) xs.push(vs[i]);
}

export function __dados_a_insert(xs, i, v) {
    if (!(i >= 0 && i <= xs.length)) {
        throw __dados_fault("index out of bounds: " + i + " is not a position in, or the end of, a run of " + xs.length);
    }
    xs.splice(i, 0, v);
}

export function __dados_a_pop(xs) {
    if (xs.length === 0) throw empty("pop");
    return xs.pop();
}

export function __dados_a_pop_front(xs) {
    if (xs.length === 0) throw empty("pop_front");
    return xs.shift();
}

export function __dados_a_front(xs) {
    if (xs.length === 0) throw empty("front");
    return xs[0];
}

export function __dados_a_back(xs) {
    if (xs.length === 0) throw empty("back");
    return xs[xs.length - 1];
}

export function __dados_a_remove_at(xs, i) {
    check_at(i, xs.length);
    return xs.splice(i, 1)[0];
}

export function __dados_a_clear(xs) {
    xs.length = 0;
}

// Truncate, or grow with `zero()` per new element (a container's zero is a
// new one each time).
export function __dados_a_resize(xs, n, zero) {
    if (n < 0) throw __dados_fault("resize to " + n + ": a length is never negative");
    if (n > 4294967295) throw __dados_fault("resize to " + n + ": an array holds at most 4294967295 elements");
    n = Number(n);
    if (n <= xs.length) {
        xs.length = n;
        return;
    }
    for (let i = xs.length; i < n; i++) xs.push(zero());
}

// DadoScript's `==`: `===` over canonical tiers, so NaN is found nowhere.
export function __dados_a_contains(xs, v) {
    return xs.indexOf(v) !== -1;
}

export function __dados_a_index_of(xs, v) {
    const i = xs.indexOf(v);
    if (i === -1) throw __dados_fault("not found: index_of found no element equal to the value");
    return i;
}

export function __dados_a_last_index_of(xs, v) {
    const i = xs.lastIndexOf(v);
    if (i === -1) throw __dados_fault("not found: last_index_of found no element equal to the value");
    return i;
}

export function __dados_a_find_if(xs, p) {
    const n = xs.length;
    for (let i = 0; i < n; i++) if (p(xs[i])) return i;
    throw __dados_fault("not found: find_if found no element its predicate accepts");
}

// Ascending, stable. A NaN sorts after every number, and -0 beside 0.
function ascending(a, b) {
    if (a < b) return -1;
    if (b < a) return 1;
    if (a !== a) return b !== b ? 0 : 1;
    if (b !== b) return -1;
    return 0;
}

export function __dados_a_sort(xs) {
    xs.sort(ascending);
}

export function __dados_a_sort_by(xs, less) {
    xs.sort((a, b) => (less(a, b) ? -1 : less(b, a) ? 1 : 0));
}

// The text the bytes encode, each invalid or cut-short sequence one U+FFFD
// and the byte that cut it read again: WHATWG's decoder, TextDecoder's.
export function __dados_a_decode_utf8(bs) {
    let out = "";
    const n = bs.length;
    let i = 0;
    while (i < n) {
        const b = bs[i++];
        if (b < 0x80) {
            out += String.fromCharCode(b);
            continue;
        }
        let need = 0, cp = 0, lo = 0x80, hi = 0xbf;
        if (b >= 0xc2 && b <= 0xdf) { need = 1; cp = b & 0x1f; }
        else if (b >= 0xe0 && b <= 0xef) {
            need = 2; cp = b & 0x0f;
            if (b === 0xe0) lo = 0xa0;
            if (b === 0xed) hi = 0x9f;
        } else if (b >= 0xf0 && b <= 0xf4) {
            need = 3; cp = b & 0x07;
            if (b === 0xf0) lo = 0x90;
            if (b === 0xf4) hi = 0x8f;
        } else {
            out += "\ufffd";
            continue;
        }
        let ok = true;
        for (let k = 0; k < need; k++) {
            const c = i < n ? bs[i] : -1;
            if (c < lo || c > hi) { ok = false; break; }
            lo = 0x80; hi = 0xbf;
            cp = cp * 64 + (c & 0x3f);
            i++;
        }
        out += ok ? String.fromCodePoint(cp) : "\ufffd";
    }
    return out;
}

// `m[k]`, raising when `k` is no key; `key` renders it for the message. One
// `get`: only an `any` value can be `undefined`, so `has` is asked only then.
export function __dados_m_at(m, k, key) {
    const v = m.get(k);
    if (typeof v === "undefined" && !m.has(k)) throw __dados_m_absent(k, key);
    return v;
}

// The fault of `m[k]` where `k` is no key: what an emitted site's own check
// calls (`(m.get(k) ?? __dados_m_absent(k, render))`), on the site's line.
export function __dados_m_absent(k, key) {
    throw __dados_fault("key not found: " + key(k));
}

// `k in m` and `v in xs`, the operand written first evaluated first.
export function __dados_m_in(k, m) {
    return m.has(k);
}

export function __dados_a_in(v, xs) {
    return xs.indexOf(v) !== -1;
}

export function __dados_m_get(m, k, d) {
    return m.has(k) ? m.get(k) : d;
}

// `s[i]`: one UTF-16 code unit, as a string.
export function __dados_s_at(s, i) {
    check_at(i, s.length);
    return s[i];
}

// `s[i]`'s fault, as `__dados_a_oob` is `xs[i]`'s.
export function __dados_s_oob(s, i) {
    throw out_of_bounds(i, s.length);
}

export function __dados_s_slice(s, lo, hi) {
    if (hi === undefined) hi = s.length;
    check_window(lo, hi, s.length);
    return s.slice(lo, hi);
}

export function __dados_s_index_of(s, t) {
    const i = s.indexOf(t);
    if (i === -1) throw __dados_fault("not found: index_of found no \"" + t + "\" in the string");
    return i;
}

export function __dados_s_last_index_of(s, t) {
    const i = s.lastIndexOf(t);
    if (i === -1) throw __dados_fault("not found: last_index_of found no \"" + t + "\" in the string");
    return i;
}

// Every `a` replaced by `b`, `b` taken literally. Split and joined rather
// than `replaceAll` (ES2021's, past the subset's library) or `replace` with a
// string (which reads `$&` and its kin in `b` as patterns). An empty `a`
// matches before every UTF-16 unit and at the end, as `replaceAll` does.
export function __dados_s_replace_all(s, a, b) {
    if (a.length === 0) return s.length === 0 ? b : b + s.split("").join(b) + b;
    return s.split(a).join(b);
}

export function __dados_s_repeat(s, n) {
    if (n < 0) throw __dados_fault("repeat " + n + " times: a count is never negative");
    return s.repeat(Number(n));
}

// The UTF-8 encoding, a lone surrogate as U+FFFD (TextEncoder's rule).
export function __dados_s_bytes(s) {
    const out = [];
    for (const c of s) {
        let cp = c.codePointAt(0);
        if (cp >= 0xd800 && cp <= 0xdfff) cp = 0xfffd;
        if (cp < 0x80) out.push(cp);
        else if (cp < 0x800) out.push(0xc0 | (cp >> 6), 0x80 | (cp & 63));
        else if (cp < 0x10000) out.push(0xe0 | (cp >> 12), 0x80 | ((cp >> 6) & 63), 0x80 | (cp & 63));
        else out.push(0xf0 | (cp >> 18), 0x80 | ((cp >> 12) & 63), 0x80 | ((cp >> 6) & 63), 0x80 | (cp & 63));
    }
    return out;
}

export function __dados_s_code_point_at(s, i) {
    check_at(i, s.length);
    return s.codePointAt(i);
}

const INT_TEXT = /^[+-]?[0-9]+$/;

// Canonical decimal text - what `string(n)` writes: no `+`, no leading zero,
// no `-0` - of an int within 2^53 is read by the engine's own number reader
// and answered when writing the number back gives the same text, which holds
// for that text and no other. Anything else, every malformed text and every
// int past 2^53, takes the exact path below, which raises.
export function __dados_s_to_int(s) {
    const read = +s;
    if (((read | 0) === read || (read % 1 === 0 && read >= -MAX_SAFE && read <= MAX_SAFE)) && String(read) === s) {
        return read;
    }
    if (!INT_TEXT.test(s)) throw __dados_fault("to_int: \"" + s + "\" is not a decimal integer");
    const v = BigInt(s);
    if (v < -9223372036854775808n || v > 9223372036854775807n) {
        throw __dados_fault("to_int: \"" + s + "\" is past the range of int");
    }
    return canon(v);
}

const FLOAT_TEXT = /^[+-]?([0-9]+(\.[0-9]*)?|\.[0-9]+)([eE][+-]?[0-9]+)?$/;
const FLOAT_WORD = /^([+-]?)(inf|infinity|nan)$/i;

// core:strings/parse's grammar: digits with an optional point and exponent,
// or inf, infinity or nan in any case, each with an optional sign; nothing
// else, no space. The nearest double, ties to even, as Number reads it.
export function __dados_s_to_float(s) {
    if (FLOAT_TEXT.test(s)) return Number(s);
    const word = FLOAT_WORD.exec(s);
    if (word === null) throw __dados_fault("to_float: \"" + s + "\" is not a number");
    if (word[2].toLowerCase() === "nan") return NaN;
    return word[1] === "-" ? -Infinity : Infinity;
}

// Rendering as `print` and `string(x)` do, Dado's layout: `[1, 2]`, `{a: 1}`,
// `(1, a)` for a tuple, strings unquoted, a map in insertion order, and an
// instance as `Name(field, ...)`, its fields in declaration order, the
// parent's first (`$name()` and `$parts()`, which every class carries when
// a program may render one). `f` renders an element. An array, a map or an
// instance met again while it is being rendered - one that holds itself -
// renders as `[...]`, `{...}` or `Name(...)` (each `...` one character),
// instead of rendering forever.
const rendering = new Set();

function guarded(v, again, f) {
    if (rendering.has(v)) return again;
    rendering.add(v);
    try {
        return f();
    } finally {
        rendering.delete(v);
    }
}

export function __dados_str_string(s) {
    return s;
}

export function __dados_str_seq(xs, f) {
    return guarded(xs, "[\u2026]", () => {
        let out = "[";
        const n = xs.length;
        for (let i = 0; i < n; i++) out += (i > 0 ? ", " : "") + f(xs[i]);
        return out + "]";
    });
}

export function __dados_str_map(m, fk, fv) {
    return guarded(m, "{\u2026}", () => {
        let out = "{";
        let first = true;
        for (const [k, v] of m) {
            out += (first ? "" : ", ") + fk(k) + ": " + fv(v);
            first = false;
        }
        return out + "}";
    });
}

// A tuple: each slot by its own renderer, `fs[i]`.
export function __dados_str_tuple(t, fs) {
    let out = "(";
    for (let i = 0; i < t.length; i++) out += (i > 0 ? ", " : "") + fs[i](t[i]);
    return out + ")";
}

export function __dados_str_obj(v) {
    const name = v.$name();
    return guarded(v, name + "(\u2026)", () => name + "(" + v.$parts().join(", ") + ")");
}

// ---------------------------------------------------------------------------
// Failure.
//
// A DadoScript `Error` is this runtime's own class, `__dados_Error`, and not the
// engine's: constructing one captures nothing. It holds a message, a code (a
// nonzero i32; 70, BSD's EX_SOFTWARE, when none is given or the one given is
// past i32, and the host's reserved `script.ERROR`, INT32_MIN, when the one
// given is 0 - a failure's code is never 0) and a location, a
// `__dados_Location`. **A raise captures the
// engine's stack once** - `fail` (`__dados_raise`), a fault (`__dados_fault`), an
// assertion - and nothing reads it until the trace is asked for: then each
// engine frame is mapped to a DadoScript position through the line table every
// emitted module registers (`__dados_lines`), frames outside DadoScript (this
// runtime, a host, a library) dropped. The trace is innermost first, and
// leaves out the raising frame when it is the location's own line.
//
// Whatever else is thrown - the engine's RangeError from the fork's division
// opcode, a TypeError from the seam's glue, a library's throw of any value -
// becomes an `Error` where it is caught (`__dados_failure`): its message, code
// 70 unless it carries an integer `code` (a Dado function's failure does), and
// its stack read as a raise's. A host reads an uncaught one the same way,
// through the global `__dados_failure` this module installs.
//
// An `Error` renders as `error <code>: <message>` (`toString`), and its full
// report - what a host prints - adds one `  at <file>:<line>:<column>
// (<function>)` line for the location and one per trace entry (`stack`).

const DEFAULT_CODE = 70;

// `core:script`'s `ERROR`: the first of the host's reserved codes, what a
// code of 0 becomes.
const ERROR_CODE = -2147483648;

// The code an `Error` keeps: a nonzero i32, `ERROR_CODE` for 0, else 70.
function code_of(code) {
    if (code === 0) return ERROR_CODE;
    return typeof code === "number" && (code | 0) === code ? code : DEFAULT_CODE;
}

// `Location`: a file as the compiler names it, a line and a column from 1,
// and the function, `package.Class.method`. A value: never written after it
// is made. The zero value is nowhere and renders `<unknown>`.
export class __dados_Location {
    constructor(file, line, column, fn) {
        this.file = file;
        this.line = line;
        this.column = column;
        this.function = fn;
        Object.freeze(this);
    }
}

export const __dados_nowhere = new __dados_Location("", 0, 0, "");

export function __dados_str_location(l) {
    return l.file === "" ? "<unknown>" : l.file + ":" + l.line + ":" + l.column;
}

// One line of a report: where, and in which function when it is known.
function at_line(l) {
    return "  at " + __dados_str_location(l) + (l.function === "" ? "" : " (" + l.function + ")");
}

// Every emitted module's line table, by the name the engine gives the module
// in a stack frame: `file` is the `.dados` file, `fns` the qualified names of
// its functions, and `table` runs of four numbers - a JavaScript line, the
// DadoScript line and column it was emitted for, and an index into `fns` (-1: no
// DadoScript position) - ascending by JavaScript line. `files`, when given, is each
// function's own file: a trait's default written into a class is the
// trait's file's.
const modules = new Map();

// The engine's stack text from the caller of `above` outward.
function stack_text(above) {
    const holder = {};
    if (typeof Error.captureStackTrace === "function") {
        Error.captureStackTrace(holder, above);
        return holder.stack;
    }
    return new Error().stack;
}

// `at name (file:line:col)` and `at file:line:col`, the shape both QuickJS-ng
// and V8 write a frame in.
const FRAME = /^\s*at (?:.* \()?(.+?):(\d+):(\d+)\)?$/;

function frames_of(text) {
    const out = [];
    if (typeof text !== "string") return out;
    for (const line of text.split("\n")) {
        const m = FRAME.exec(line);
        if (m !== null) out.push([m[1], Number(m[2])]);
    }
    return out;
}

// Called once by each emitted module, at its top level: the module's own name
// in a frame is read off the stack of this very call.
export function __dados_lines(file, fns, table, files) {
    const here = frames_of(stack_text(__dados_lines));
    if (here.length > 0) modules.set(here[0][0], { file, fns, table, files });
}

// The DadoScript position of one engine frame, or null outside DadoScript.
function position(frame) {
    const m = modules.get(frame[0]);
    if (m === undefined) return null;
    const t = m.table;
    let lo = 0, hi = t.length / 4 - 1, found = -1;
    while (lo <= hi) {
        const mid = (lo + hi) >> 1;
        if (t[mid * 4] <= frame[1]) {
            found = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    const fn = t[found * 4 + 3];
    if (found < 0 || fn < 0) return null;
    const file = m.files === undefined ? m.file : m.files[fn];
    return new __dados_Location(file, t[found * 4 + 1], t[found * 4 + 2], m.fns[fn]);
}

export class __dados_Error {
    constructor(message, code, location) {
        this.message = message;
        this.code = code_of(code);
        // null until read: the raise's own position, found in its stack.
        this.__at = location;
        // The engine's stack text at the raise; null until raised.
        this.__raw = null;
        // The raise's DadoScript frames, mapped on first read.
        this.__frames = null;
    }

    __mapped() {
        if (this.__frames === null) {
            const out = [];
            for (const f of frames_of(this.__raw)) {
                const p = position(f);
                if (p !== null) out.push(p);
            }
            this.__frames = out;
        }
        return this.__frames;
    }

    get location() {
        if (this.__at === null) {
            const f = this.__mapped();
            this.__at = f.length > 0 ? f[0] : __dados_nowhere;
        }
        return this.__at;
    }

    get trace() {
        const f = this.__mapped();
        const at = this.location;
        const first = f.length > 0 && f[0].file === at.file && f[0].line === at.line;
        return f.slice(first ? 1 : 0);
    }

    toString() {
        return "error " + this.code + ": " + this.message;
    }

    // The report's frame lines: the location, then the trace.
    get stack() {
        let out = at_line(this.location) + "\n";
        for (const l of this.trace) out += at_line(l) + "\n";
        return out;
    }
}

export function __dados_str_error(e) {
    return e.toString();
}

// `fail e`, and every other raise: the stack captured, once - an `Error`
// raised again keeps the trace of its first raise.
export function __dados_raise(e) {
    if (e.__raw === null) e.__raw = stack_text(__dados_raise);
    return e;
}

// `fail n`.
export function __dados_failed(n, location) {
    const e = new __dados_Error("failed with code " + n, typeof n === "bigint" ? DEFAULT_CODE : n, location);
    e.__raw = stack_text(__dados_failed);
    return e;
}

// A fault - an index out of range, a missing key, a division by zero: an
// `Error` of code 70 in Dado's words, located where the script was when it
// happened.
export function __dados_fault(message) {
    const e = new __dados_Error(message, DEFAULT_CODE, null);
    e.__raw = stack_text(__dados_fault);
    return e;
}

// `assert cond, "message"` that does not hold.
export function __dados_assert(message, location) {
    const e = new __dados_Error(message === null ? "assertion failed" : "assertion failed: " + message, DEFAULT_CODE, location);
    e.__raw = stack_text(__dados_assert);
    return e;
}

// Whatever was thrown, as an `Error`: what a `try ... else e:` binds and what a
// host reports.
export function __dados_failure(thrown) {
    if (thrown instanceof __dados_Error) return thrown;
    let message;
    let code = DEFAULT_CODE;
    let raw = null;
    if (thrown instanceof Error) {
        message = String(thrown.message);
        if (typeof thrown.code === "number") code = thrown.code;
        if (typeof thrown.stack === "string") raw = thrown.stack;
    } else {
        try {
            message = typeof thrown === "string" ? thrown : String(thrown);
        } catch (_) {
            message = "a thrown value that has no text";
        }
    }
    const e = new __dados_Error(message, code, null);
    e.__raw = raw;
    return e;
}

// A lowered unit's failure (rung 1), raised by the script's call of the unit
// as the `Error` the script's own code raises: the unit's record `f` is
// [kind, a, b, callee, then line, column and method index per position,
// innermost first]; `fns` names each method of the class by index, `file` is
// the class's file and `msgs` each unit's `fail "text"`s. The trace is the
// positions inside the units, then the engine's frames past the call's own.
export function __dados_unit_raise(f, fns, file, msgs) {
    let message;
    let code = DEFAULT_CODE;
    switch (f[0]) {
        case 1: message = "integer division by zero"; break;
        case 2: message = "index out of bounds: " + f[1] + " is not a position in a run of " + f[2]; break;
        case 3:
            message = "failed with code " + f[1];
            code = typeof f[1] === "bigint" ? DEFAULT_CODE : f[1];
            break;
        case 4: message = msgs[f[2]][f[1]]; break;
        default:
            message = "`" + f[3] + "` failed with code " + f[1];
            code = f[1];
            break;
    }
    const at = [];
    for (let i = 4; i + 2 < f.length; i += 3) {
        at.push(new __dados_Location(file, f[i], f[i + 1], fns[f[i + 2]]));
    }
    const e = new __dados_Error(message, code, at.length > 0 ? at[0] : __dados_nowhere);
    e.__raw = stack_text(__dados_unit_raise);
    // The first DadoScript frame is the call of the unit itself, which the
    // positions above stand for.
    const outer = [];
    let dropped = false;
    for (const fr of frames_of(e.__raw)) {
        if (!dropped && modules.has(fr[0])) {
            dropped = true;
            continue;
        }
        const p = position(fr);
        if (p !== null) outer.push(p);
    }
    e.__frames = at.concat(outer);
    return e;
}

// Where a host finds the rule above without importing this module.
Object.defineProperty(globalThis, "__dados_failure", { value: __dados_failure, configurable: true });

// ---------------------------------------------------------------------------
// `any`.
//
// A value enters an `any` as it is. It leaves one into a typed place through
// `__dados_from_any(v, kind)`, which answers the value in DadoScript's
// representation or raises, so `any` is never an unchecked cast. `kind` is
// "int", "float", "bool", "string", "byte", or ["[]", kind] for an array and
// ["{}", key, value] for a map; an array or a map is checked element by
// element and kept (the same array, an element canonicalised in place).
// JavaScript has one number, so a whole number leaves as an int or a float
// alike, and an int past 2^53 arrives from JavaScript as a BigInt.
//
// A script's class (`["class", Goblin, "Goblin"]`), `Error` and `Location`
// leave by `instanceof`: an instance of a subclass is one of its parent's.
//
// The one place an `any` raises is `any_mismatch`: a fault, a DadoScript `Error` of
// code 70 (`__dados_fault`), located where the script let the `any` go.

function any_kind_text(kind) {
    if (typeof kind === "string") return kind;
    if (kind[0] === "class") return kind[2];
    if (kind[0] === "[]") return "[]" + any_kind_text(kind[1]);
    return "{" + any_kind_text(kind[1]) + ": " + any_kind_text(kind[2]) + "}";
}

function any_article(text) {
    return /^[aeiou]/.test(text) ? "an " : "a ";
}

// What an `any` holds, for a message: a string quoted, the rest as `print`
// writes it, cut short.
function any_shown(v) {
    const text = typeof v === "string" ? JSON.stringify(v) : __dados_str_any(v);
    return text.length > 60 ? text.slice(0, 57) + "..." : text;
}

function any_mismatch(v, kind, inside) {
    const text = any_kind_text(kind);
    throw __dados_fault("any is not " + any_article(text) + text + ": " + (inside === undefined ? any_shown(v) : inside));
}

// The value an int is, in its canonical tier, or `undefined`.
function any_int(v) {
    if (typeof v === "number") {
        return Number.isInteger(v) && v >= -MAX_SAFE && v <= MAX_SAFE ? v + 0 : undefined;
    }
    if (typeof v === "bigint" && BigInt.asIntN(64, v) === v) return canon(v);
    return undefined;
}

// `v` as `kind`, or `undefined` when it is not one (a value can never be
// `undefined` once it has left).
function any_as(v, kind) {
    switch (kind) {
        case "any": return v;
        case "int": return any_int(v);
        case "byte": {
            const n = any_int(v);
            return n !== undefined && n >= 0 && n <= 255 ? n : undefined;
        }
        case "float": return typeof v === "number" ? v : undefined;
        case "bool": return typeof v === "boolean" ? v : undefined;
        case "string": return typeof v === "string" ? v : undefined;
    }
    if (kind[0] === "class") return v instanceof kind[1] ? v : undefined;
    if (kind[0] === "[]") {
        if (!Array.isArray(v)) return undefined;
        for (let i = 0; i < v.length; i++) {
            const e = any_as(v[i], kind[1]);
            if (e === undefined) any_mismatch(v, kind, "element " + i + " is " + any_shown(v[i]));
            if (e !== v[i]) v[i] = e;
        }
        return v;
    }
    if (!(v instanceof Map)) return undefined;
    for (const [k, e] of v) {
        if (any_as(k, kind[1]) !== k) any_mismatch(v, kind, "the key " + any_shown(k));
        const w = any_as(e, kind[2]);
        if (w === undefined) any_mismatch(v, kind, "the value at " + any_shown(k) + " is " + any_shown(e));
        if (w !== e) v.set(k, w);
    }
    return v;
}

export function __dados_from_any(v, kind) {
    const out = any_as(v, kind);
    if (out === undefined) any_mismatch(v, kind);
    return out;
}

// The slot names of the instance an `any` holds, for `for name, value in
// slots(a)`: its class's own, which every class a walk may meet carries as
// `$slots()`. Anything else is a fault, as an `any` leaving into a class is.
export function __dados_slots(v) {
    if (v !== null && typeof v === "object" && typeof v.$slots === "function") return v.$slots();
    throw __dados_fault("any is not an instance of a class: " + any_shown(v));
}

// What `print` and `string(x)` write for an `any`: a script's values as DadoScript
// writes them (a whole number as an int), an array and a map in Dado's layout,
// `undefined` and `null` as JavaScript names them, a function by its name,
// an `Error` or a `Location` as `print` writes it, an instance of a script's
// class as `Name(field, ...)` when the program renders instances and by its
// class, `<Goblin>`, when it carries no `$parts`. A tuple is an array here.
// A promise - an instance's `async T` field - is `<async>`: what it will hold
// is not there yet.
export function __dados_str_any(v) {
    switch (typeof v) {
        case "string": return v;
        case "boolean": return v ? "true" : "false";
        case "bigint": return String(v);
        case "number": return any_int(v) !== undefined ? String(v + 0) : __dados_str_float(v);
        case "undefined": return "undefined";
        case "function": return "<function " + (v.name || "anonymous") + ">";
    }
    if (v === null) return "null";
    if (Array.isArray(v)) return __dados_str_seq(v, __dados_str_any);
    if (v instanceof Map) return __dados_str_map(v, __dados_str_any, __dados_str_any);
    if (v instanceof __dados_Location) return __dados_str_location(v);
    if (typeof v.$parts === "function") return __dados_str_obj(v);
    if (v instanceof Promise) return "<async>";
    if (v.toString === Object.prototype.toString && typeof v.constructor === "function" && v.constructor !== Object) {
        return "<" + v.constructor.name.replace(/\$/g, ".") + ">";
    }
    try {
        return String(v);
    } catch (e) {
        return "<object>";
    }
}

// ---------------------------------------------------------------------------
// The math globals and signals: the self-hosted compiler's alone.

export function __dados_round(x) {
    return x < 0 ? -Math.round(-x) : Math.round(x);
}

export function __dados_int_abs(a) {
    return a < 0 ? __dados_int_neg(a) : a;
}

// Signals: a list per instance per signal, in
// connect order; a connection is a method of an instance (`inst`, its name
// `f`) or a lambda (`inst` null). An emit walks the list as it stood when
// the outermost emit began; connects and disconnects made meanwhile apply
// when it ends, thrown out of or not.
function sig_of(o, name) {
    if (o.$sigs === undefined) o.$sigs = new Map();
    let s = o.$sigs.get(name);
    if (s === undefined) {
        s = { list: [], depth: 0, snap: null, pending: [] };
        o.$sigs.set(name, s);
    }
    return s;
}

function sig_apply(s, add, inst, f, bound) {
    let at = -1;
    for (let i = 0; i < s.list.length; i += 1) {
        if (s.list[i].inst === inst && s.list[i].f === f) at = i;
    }
    if (add && at < 0) s.list.push({ inst, f, bound });
    if (!add && at >= 0) s.list.splice(at, 1);
}

// `bound`: Dado's method `f` bound to the address `inst`.
export function __dados_sig_connect(o, name, inst, f, bound) {
    const s = sig_of(o, name);
    if (s.depth > 0) s.pending.push([true, inst, f, bound]);
    else sig_apply(s, true, inst, f, bound);
}

export function __dados_sig_disconnect(o, name, inst, f, bound) {
    const s = sig_of(o, name);
    if (s.depth > 0) s.pending.push([false, inst, f, bound]);
    else sig_apply(s, false, inst, f, bound);
}

export function __dados_sig_emit(o, name, args) {
    const s = sig_of(o, name);
    if (s.depth === 0) s.snap = s.list.slice();
    const snap = s.snap;
    s.depth += 1;
    try {
        for (const c of snap) {
            if (c.bound) c.f(c.inst, ...args);
            else if (c.inst === null) c.f(...args);
            else c.inst[c.f](...args);
        }
    } finally {
        s.depth -= 1;
        if (s.depth === 0) {
            s.snap = null;
            const ps = s.pending;
            s.pending = [];
            for (const p of ps) sig_apply(s, p[0], p[1], p[2], p[3]);
        }
    }
}
