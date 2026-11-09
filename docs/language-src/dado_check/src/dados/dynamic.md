//@ toys/dialect 30
//@ ### `any`, the untyped value
//@
//@ `any` holds a value whose type is known only when the script runs: a
//@ host object, a JavaScript library's answer, or a script's own value put
//@ where any type will do — `[]any` and `{string: any}` are ordinary
//@ containers. **A value goes into an `any` freely**: `any a = 5`, or
//@ appending a `string` to a `[]any`, writes no conversion. **A value comes
//@ out of one checked**: `int n = a`, returning `a` from a method that
//@ answers an `int`, passing it where a `string` is taken, or writing
//@ `int(a)` or `float(a)`, checks what it holds when the line runs and
//@ raises a fault — an `Error` of code 70, `any is not an int: "abc"` —
//@ when it is something else. An array or a map is checked element by
//@ element as it comes out, and an instance of a script's class, an
//@ `Error` or a `Location` by what it is an instance of (`Goblin g = a`
//@ takes a `Goblin` or an instance of a class that extends it). JavaScript
//@ has one kind of number, so an `any` holding a whole number comes out as
//@ an `int` or a `float` alike.
//@
//@ What an `any` holds is reached the way JavaScript reaches it: a member,
//@ `a.name`; a call through one, `a.name(x)`; a call of the value itself,
//@ `f(x)`; an index by an `int` or a `string`, `a[k]`; and a store through
//@ a member or an index. Each answers another `any`. Nothing computes with
//@ one directly — no operator, no condition, no `for` over it, no map keyed
//@ by it — so the conversion is written first, and that is the one place
//@ its type is checked: `int(a) + 1`, not `a + 1`. `string(a)` and `print`
//@ write whatever it holds.
//@
//@ **`globalThis` is an `any`**: the engine's global object, the door to
//@ what the host provides — `globalThis.Math.sqrt(2.0)`, converted where it
//@ is used, `float r = globalThis.Math.sqrt(2.0)`.
