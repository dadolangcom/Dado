//@ toys/dialect 40
//@ ### A script calls Dado
//@
//@ A script calls a function of its package's Dado files by its bare name,
//@ as it calls a method of its own class — a method of the class wins over a
//@ Dado function of the same name — and a function of a Dado package it
//@ imports through the import's name, as a Dado file does. It sees what a Dado
//@ file of its package sees. The function takes and answers scalars — a
//@ sized integer, `f32` or `f64`, `bool`, `char8`, `char16`, `char32`, a
//@ string of any width or a `cstring` — and slices, arrays, tuples and maps
//@ of them, and it may answer a new string or array as `ref([]char8)` or
//@ `ref([]T)`, or a Dado function as a value. A Dado array, and a shape
//@ whose slots share one type, is the script's fixed array, `[2]f32` its
//@ `[2]float` (see *Arrays and tuples cross the seam*); a matrix and a
//@ `#soa` array are the arrays they are, and a map is the script's map (see
//@ *Maps, strings of every width, matrices and functions cross the seam*).
//@ A Dado enum, `distinct` type or union is the script's own, by the type
//@ the signature names (see *Enums, `distinct` types and unions cross the
//@ seam*). It may take or answer a handle of a script class, `npc.Goblin`,
//@ which the script passes and receives as its own instance. It may be
//@ failable.
//@
//@ The script passes its own values and the compiler converts them. A
//@ script's `int` is Dado's, so it adapts to the width it meets as a Dado
//@ `int` does: an int passed to a narrower integer wraps, as Dado's written
//@ conversion and a literal that does not fit do, so 300 passed to a `u8`
//@ arrives as 44;
//@ an int passed to an `i64` is exact and one passed to a `u64` goes by its
//@ bits. A float passed to an
//@ `f32` rounds. A number never changes kind by itself, in either
//@ direction: a float passed to an integer parameter is refused, naming
//@ `int(x)`, and an int passed to an `f32` or `f64` — or as an element of
//@ an array or a slice of them — is refused, naming `float(x)`. A literal
//@ adapts to the float it meets, as in Dado, so `half(2)` is a call. A string crosses as a copy,
//@ read as UTF-8, and a `char32` takes a string of exactly one code point.
//@ A `char8` is a byte, which a script holds as an int: it wraps into one as
//@ a `u8` does. A `char16` takes a string of exactly one UTF-16 code unit —
//@ JavaScript's own character — and any other string raises when the call
//@ is made. What comes back is the script's own type: every integer and a
//@ `char8` an int, `f32` and `f64` a float, a `char32`, a `char16` or a Dado
//@ string of any width a string.
//@
//@ Narrowing is never an error. Under `--strict` it is a warning
//@ (`WRN1207`) wherever the argument is not a constant the parameter holds
//@ exactly. At run time a dev build checks each value the script computes
//@ where it calls, and the first that changes as it crosses at each place
//@ is reported on the engine's error sink, as Dado's own narrowing is (see
//@ *`int` and `float` adapt to the width they meet*). A parameter or a result that cannot cross — a pointer, alone
//@ or as an element, a `ref` other than a new string or array answered, a
//@ map keyed by what a script's map cannot hold, an enum backed by no
//@ integer, a union two of whose members a script holds as one type —
//@ makes the function one a script cannot call (`ERR1206`), and the refusal
//@ names the part and what to write instead.
//@
//@ A failable function's failure is thrown into the script as an error whose
//@ message names the function and the code. Nothing in the script has to
//@ mark it: uncaught, it leaves the method, and the Dado program's
//@ `#script_run` fails.
//@
//@ A Dado function may take a script's function. A parameter of a function
//@ type, `void(i32) f`, takes a script's `void(int)` — a lambda or a
//@ method — which Dado may call while the call runs and not after: one the
//@ Dado function keeps, storing it or handing it to a function that does, is
//@ refused (`ERR1220`). A `(proc, rawptr data)` pair whose procedure takes
//@ its `data` first carries a script's function past the call: a `Sink`
//@ takes a `void(string)`, so `core:fmt` writes into a script, and a pair
//@ of the program's own, `(!void(rawptr, i64) proc, rawptr data)`, takes a
//@ `void(int)`. Dado passes the function scalars as it answers them, and the
//@ function answers a scalar or nothing. Each call enters the script's VM,
//@ so one from another thread fails `script.THREAD`, and one through a
//@ pair whose VM was deleted fails `script.STALE`; a failable procedure
//@ answers the code. A Dado function the script's function calls sees the
//@ `using` allocator in effect when Dado called it. When the script's
//@ function fails during a Dado function the script called, the script's own
//@ error reaches it as that Dado function returns.
