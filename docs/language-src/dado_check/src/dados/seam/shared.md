//@ toys/dialect 46
//@ ### Enums, `distinct` types and unions cross the seam
//@
//@ A Dado enum, `distinct` type or union crosses as the script's own, by the
//@ type the Dado signature names — a parameter written `i32` takes an `int`,
//@ and one written `Dir` a `Dir`, whatever the value — and the script names
//@ the declaration as it names a Dado function: `Dir` of its package,
//@ `compass.Dir` through an import.
//@
//@ **A Dado enum is a script's enum, open.** Its members are written
//@ `Dir.north`, and a value crosses as the name of the member it is, as a
//@ script's own enum member is the string of its name — or, when no member
//@ names it, as its number, so every value of the enum crosses and comes
//@ back as itself. A value two members name crosses as the one declared
//@ first. `Dir(5)` makes one of a number and `int(d)` reads one's number;
//@ two compare with `==`, one keys a map, and one prints as its member's name
//@ or its number. It is open, as Dado holds it, so a script's `switch` over
//@ one needs `else`, as Dado's does. Passing an `int` where the signature
//@ names the enum is refused (`ERR1201`), naming `Dir(n)`. An enum backed
//@ by no integer, and a `foreign` one, whose values are C's, do not cross. A
//@ script's own enum is closed — its members are names with no number behind
//@ them, so it has no `int(x)` — and it crosses into no Dado parameter.
//@
//@ **A `distinct` type is nominal in the script and erased when it runs.**
//@ `distinct type Meters: f32` is the script's `Meters`, over the type its
//@ shape crosses as, and Dado's rules for it hold there: `Meters(1.5)` makes
//@ one, `float(m)` and `int(m)` read it, `Meters` meets `Meters` under an
//@ operator and never a plain `float`, and a `float` passed where a `Meters`
//@ is taken is refused, naming `Meters(x)`. Running, it is the number it is
//@ over, and it crosses as that crosses, with nothing added.
//@
//@ **A Dado union is a script's union**, the pair `[member, payload]`, its
//@ members the types Dado's cross as, in Dado's order: `union Value: (i32,
//@ f64, string8)` is the script's union of `int`, `float` and `string`, read
//@ with Dado's narrowing `switch` — `case int n:` — and made with
//@ `Value(x)`, which picks the member by the value's type. A union two of
//@ whose members a script holds as one type, which no `case` could tell
//@ apart, does not cross (`ERR1206`).
//@
//@ Dado passes them to a script's method that names them and takes them
//@ back: an enum or a union answered arrives as Dado's own, and a
//@ `distinct` type's answer as the type its shape answers as, `f64` for
//@ `Meters`. A script's own union takes the Dado value of one of its
//@ members' types, `held.size(3)` for a `Shape` of `(int, string)`; Dado
//@ has no type of it to hold an answer in, so a method answering one is not
//@ called from Dado (`ERR1201`). A constant of an enum or a `distinct` type
//@ reads as its value does, and an enum inside an array crosses as one does
//@ alone. A field of an enum or a union is not reached through a handle yet.

//@ toys/dialect 46
//@ ### Maps, strings of every width, matrices and functions cross the seam
//@
//@ **A Dado map is a script's map**, a JavaScript `Map`, copied entry by
//@ entry, each key and each value crossing as it would alone — a
//@ `{string8: i64}` is a script's `{string: int}`, and a value may be an
//@ array or another map. A map crosses keyed by an integer, a `bool`, a
//@ string or, where a Dado function's signature names one, a Dado enum: a
//@ float keys no Dado map, so a script's map keyed by one has none to arrive
//@ as. Dado's map is copied into a new script map — and when the script
//@ wrote it, Dado's map is cleared and rebuilt with the script's entries as
//@ the call returns (or calls back into Dado), on its own allocator; one a
//@ Dado function answers is ended once it is copied, as a `ref([]T)`
//@ answered is, and a new array answered holds no map — and a script's map
//@ is read into a new Dado map: the one a Dado function takes
//@ is Dado's for the call, and the one a script's method answers is Dado's
//@ own on the ambient allocator, its string keys the map's copies, ended
//@ with `#delete`, its `ref([]char8)` values each ended by themselves. A
//@ value of the wrong kind raises where it crosses, naming the entry by its
//@ place in the map's order and its key or value: `at [1][1]`. Dado promises
//@ no order: the script's map has its own. The rebuilt map keeps its table
//@ while the script's entries fit in it; when they do not, it moves to a new
//@ one, and the variable passed is given it — another copy of the map's
//@ handle keeps the table it had, as a `ref` grown through one copy leaves
//@ another holding the old block, so pass a map a script may grow as the
//@ variable that owns it. A map inside a tuple or an array passed by value
//@ is rebuilt only in its own table, and a map whose values are views or
//@ own storage — a `string8`, a slice, a map, a `ref` — is not rebuilt: a
//@ script's writes that cannot be taken fail the run with `script.TYPE`. A
//@ Dado map lent to a script is locked for the run, as an array is: a Dado
//@ function the script calls that writes it fails the run with
//@ `script.LENT`.
//@
//@ **A `string16`, a `string32` and a `cstring` are a script's `string`**,
//@ transcoded each way, as parameters, answers and elements. A `string16`
//@ is UTF-16, JavaScript's own, so its units cross as they are, an unpaired
//@ surrogate included. A `string32` is read through the UTF-8 a `string8`
//@ is read as, so it keeps an unpaired surrogate as a `string8` keeps it,
//@ and a code point past U+10FFFF is written as U+FFFD. A `cstring` is
//@ UTF-8 up to its NUL, so a script's string holding a NUL character raises
//@ where it crosses into one. A script's method answers Dado its `string`
//@ as Dado's own new `ref([]char8)`, whatever width Dado gave it; a field
//@ and a script's package function take a `string8` only.
//@
//@ **A matrix and a `#soa` array are the arrays they are.** A matrix
//@ `[R,C]T` is the shape it tags, `[C][R]T` — its columns, each of its rows,
//@ as Dado writes one column by column — so a script holds a `[2,3]f32` as a
//@ `[3][2]float`, and a script's answer arrives as the plain array, which a
//@ matrix takes as it takes its shape. A `#soa` array is the array of its
//@ elements it lays out, `[N]P`, each element gathered out of the lanes or
//@ scattered into them; one whose element holds a container does not cross.
//@
//@ **A Dado function is a script's function value.** A Dado function a
//@ script calls may answer one, and Dado may pass one to a script's method
//@ taking a function: `i64(i64, i64)` is the script's `int(int, int)`. Each
//@ call of it crosses as a script's call of that Dado function does — every
//@ argument read by its tag, the answer in the script's type, a failure
//@ thrown with its code. It crosses when each parameter and its answer cross
//@ as values, none a function. A value holds one of the functions the
//@ program names as a value; a nil one raises where it crosses. A script's
//@ function reaches Dado through a parameter that takes one, and is kept
//@ past the call only through a `(proc, rawptr data)` pair; a script's
//@ method answering a function is not called from Dado (`ERR1201`).
