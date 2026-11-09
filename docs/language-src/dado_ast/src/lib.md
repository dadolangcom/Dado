//@ procs/visibility 20
//@ ### Visibility is a name rule, not a type rule
//@
//@ Four spellings and one slot for them: `public`, `private`, file-scoped
//@ `private`, and `export`. Writing none of them means `public`, and writing
//@ `public` says what leaving it out says — so *meant public* and *forgot*
//@ are distinguishable in the source, which is the only reason the redundant
//@ spelling is allowed. Two modifiers in one slot is refused rather than
//@ resolved. The formatter writes back the word that was written and
//@ invents none, and `dadoc --warn-unmarked` asks for a word on every
//@ package-level declaration (`WRN0419`, a warning and never an error).
//@
//@ **`private` hides a name and never a shape.** A `type` declared `private`
//@ is still the tuple it is, and another package that writes the same slots
//@ gets the same type — because the shape was never the package's to own.
//@ That is structural identity holding, not a rule about visibility.
//@ A shape that only `private` types declare is taken, by the compiler's
//@ analyses, to get its values from its own package: they may assume no
//@ other package made or wrote one.
//@ Nor does it hide what a `using` promised: a trait's methods are reached
//@ through the trait wherever the trait is visible, a `private` type's
//@ among them, and the type's other methods stay as private as it is.
//@
//@ The file-scoped form narrows a name to one file of a package and is
//@ otherwise `private`: both emit the same C linkage, and the whole
//@ difference is which Dado file may write the name.
//@
//@ `export` is `public` plus a promise outside Dado: the symbol is emitted
//@ under the name as written, unmangled, for something that is not this
//@ compiler to call. Inside Dado it reaches exactly as far as `public`
//@ does.
//@
//@ **An `export` is exported on every target**, whether or not anything in
//@ the program calls it: no compiler or linker drops it. In a native
//@ executable it is a visible symbol in the dynamic symbol table, so
//@ `dlsym` and a host that loads plugins reach it; in an object file or a
//@ library it is an ordinary public symbol, and on `#NONE` an external one
//@ the link keeps. On the web target, `#WEB`, it is a **wasm export** under
//@ the same name, listed by `--emit=build` as a `wasm-export` line, so
//@ JavaScript calls it through the instance's exports:
//@
//@     export i32 add(i32 a, i32 b):
//@         return a + b
//@
//@ is `instance.exports.add(2, 3)` on a page. The signature crosses by the
//@ target's C ABI, the same on every target: an integer of 32 bits or fewer,
//@ a `bool` and a pointer are `i32`s, an `i64` a JavaScript `BigInt`, `f32`
//@ and `f64` numbers; a `string`, a slice or a tuple is passed by the address
//@ of its bytes in the module's memory (a `string` is its `(ptr, len)` pair),
//@ and one answered is written to an address the caller passes first. **A
//@ failable export keeps the destination word** it has in C: the address its
//@ value is written to comes first, and it answers an `i32` code, 0 when it
//@ succeeded. A program that wants its host to write into its memory exports
//@ its own allocator, `export rawptr alloc(i64 n)`.
//@
//@ The other direction is not an `export`: Dado reaches JavaScript only
//@ through DadoScript, by calling a script's function.
