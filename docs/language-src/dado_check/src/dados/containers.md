//@ toys/dialect 25
//@ ### A script's arrays, maps and strings
//@
//@ `[]int` is an array and `{string: int}` a map, written as Dado writes
//@ them — `[1, 2, 3]`, `{"ada": 36}`, and `[]` or `{}` where the type is
//@ declared — and both are **held by reference**: assigning one to another
//@ name, or passing it, hands over the same array, so a change through one
//@ name is seen through the other. A slice, `xs[1..<3]`, is a new array; so
//@ is `copy`. A map keeps its entries in the order they were first put in,
//@ and is keyed by an `int`, a `float`, a `string` or a `bool` — never by a
//@ container (`ERR0801`), whose `==` is not defined (`ERR0607`): which array
//@ or what it holds is a question TypeScript and GDScript answer
//@ differently. `[]byte` is the byte buffer: an element reads as an `int`,
//@ and storing an `int` keeps it modulo 256.
//@
//@ Reading what is not there **raises**, never answers a stand-in: `xs[i]`
//@ past either end, with Dado's own words (`index out of bounds: 4 is not a
//@ position in a run of 4`); `m[k]` when the key is missing (`m.get(k,
//@ default)` is the form that does not); `pop` of an empty array; and a
//@ search that finds nothing — `index_of`, `last_index_of`, `find_if` — so
//@ `contains` asks first. `for x in xs` walks an array (`for x, i in xs`
//@ adds the position), `for k, v in m` a map in its order, and `for c in s`
//@ a string by code point.
//@
//@ A string is JavaScript's: `s.len()` counts UTF-16 code units, `s[i]` is
//@ one of them as a string, and `s[a..<b]` cuts at them; `code_point_at`
//@ reads a code point's number and `bytes` the UTF-8. The methods are the
//@ admitted lists — 26 for an array, 9 for a map, 19 for a string and
//@ `decode_utf8` on a `[]byte` — and another language's name for one is
//@ refused naming DadoScript's (`ERR1305`): `size()` is `len()`, `push(v)` is `append(v)`.
//@ The methods that take a function — `sort`, `filter`, `map`, `reduce`,
//@ `any`, `all`, `find_if` — take a lambda written in place, its types
//@ first, `xs.filter(bool(int v): return v > 2)`, or any function of the
//@ shape, a method's name among them: `xs.filter(is_big)`. `print` and `string(x)`
//@ write a container as Dado does, `[1, 2]` and `{ada: 36}`.
