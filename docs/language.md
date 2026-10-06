<!-- The hand-written half of the language documentation.
     This file is the arc. The sections it declares as slots are filled from
     `//@` blocks beside the code that implements each feature, by
     `dado doc crates --ext .rs --spine docs/spine.md -o docs/language.md`.
     A slot declared here that nothing fills, or a block addressed to a slot
     this file does not declare, is a refusal: nothing is written. -->

# Dado, and why it is shaped this way

This is the arc. It explains **why** the language is as it is, in an order a
person can read from the top, and it is hand-written because no code implements
an argument.

Everything under a heading below that describes a *feature* is gathered out of
the compiler — from a comment block sitting beside the code that implements it.
So the narrative and the details are maintained in different places, on purpose:
an argument is edited when somebody changes their mind, and a detail is edited
when somebody changes the code, and those are almost never the same day.

**The compiler is still the specification.** Where this document and `dadoc`
disagree, `dadoc` is right, and the disagreement is a defect in this document.

## There is no specification here, and that is deliberate

Several existed. A specification, a guide, a surface map, four design analyses
and a milestone board. Each described the compiler accurately at some past
moment. Each drifted. And in at least one case a reader trusted one and changed
the code to match the prose instead of the other way round — which is the
failure this whole arrangement exists to prevent.

So the three places the language is written down are all derived from the
compiler and all gated: `dadoc --explain ERR0504` for the rule behind any
diagnostic, `dadoc --emit=lexicon` for the rosters, and the worked examples under
`tests/cases/` and `corpus/`, every one of which is compiled and run.

This document is a fourth, and it is the one that could drift. Two things hold
it: the feature prose lives beside the code, four lines from the thing it
describes rather than in another directory; and a gate step refuses a block that
names a word, a compiler name or a diagnostic code the compiler does not have,
and compiles every sample in one.

That is not the same as being derived. It is what is available for prose, and
the difference is stated here rather than papered over.

## Names carry sigils so that nothing collides

Three closed namespaces, and each exists so that a program and the compiler can
never be reaching for the same word.

The first is the compiler's own, and it splits by case in a way worth
understanding before any of the rest.

### The `#` sigil splits by case, and the split is one-way

One sigil, two populations, told apart by the first ASCII letter after
the `#` and by nothing else at all.

An upper-case name — `#OS`, `#ARCH`, `#TRIPLE`, `#X86_64` — is
compile-time-known, and that guarantee has two visible edges. It is
eligible in a `when`, and it is refused as a `switch` scrutinee
(`ERR0708`), because a `switch` compares at run time whatever it selects
on, and selecting on something the compiler already knew is a mistake
worth naming rather than code worth emitting. The promise is about
*when* an answer is known, not about which construct may ask for it:
`#LEN` is upper-case, and a `when` folds before any type exists, so
`#LEN` has nothing to answer a `when` with. What it answers everywhere
else is a number with no type of its own, which takes the type its
position wants exactly as an integer literal does.

A lower-case name — `#default`, `#soa`, `#no_bounds` — says only that
the compiler owns the spelling. It promises nothing about compile time,
which is exactly why a lower-case name in a `when` condition is
`ERR0917`.

**The promise runs one way.** Upper case carries it; lower case carries
none, and nothing is carved out for a layout tag, an allocation verb or
a run-time name. An exception admitted on day one is an exception
forever, and what refusing one buys is a rule a reader holds in one go.

The **first** letter decides, so `#UNKNOWN_OS` and `#X86_64` are not
questions about underscores and digits, and a name carrying no letter
falls to the lower half — the upper half is the half that makes a claim.

Neither half is overridable by a build define. `-D` replaces the
initializer of a declaration a program wrote, and no program writes a
`#` name. The override is `--target=`, and it changes the *answer* the
toolchain gave rather than the name: cross-compiling moves what `#OS`
says without moving what `#OS` is.

### A name the compiler owns is read in one place

The lower half of the sigil is not a namespace a program can reach into.
Every such name is registered with the one grammatical position it may
be written in, and a position is a claim about **where the parser reads
the name**, not about what the name means.

So `#default` is written where a value goes; `#len` is written with its
arguments; `#soa` is written in front of the type it qualifies;
`#no_bounds` is written in front of the operand whose check it drops;
`#any` is written after a parameter's `..`, where the element type goes.
Each of those is a different production, and a name registered for one
is refused by name in the others. `#soa` where a value goes resolves to
nothing whatever — and that is the position's meaning rather than a gap
in it.

The cost is stated: two shapes a reader would call "the same kind of
name" need two registrations, and a name written somewhere no `#` name
could be written before costs a grammar production as well as a row.
What is bought is that no name can be accepted in a place nobody agreed
to, which is what an open position plus a lookup that decides afterwards
gives away.

**There is no second resolution path.** The `where` predicate words —
`#has_eq`, `#is_key`, `#is_declared` and the rest — are ordinary rows of
the same roster with a position of their own, not a separate vocabulary
some second lookup would have to know about.

### One roster, keyed by spelling

Every name the compiler owns resolves through a single table, and that
table is keyed by spelling and says nothing about case. `#LEN` sits on
it beside `#len`; whoever wants one population asks the case rule
separately. The count is the table's length and is written in no
sentence anywhere, here included, because a sentence with a number in it
is a thing that goes stale quietly.

Adding a name is three steps and there is no fourth: say what it means,
add the row carrying its spelling and its position, then wire the arm
the compiler now refuses to build without. The middle step is what makes
the third unskippable — a row that introduces a name stops the build
until something says what the name does — so there is no way to register
a spelling and forget to implement it.

**A sigil is a promise that a name stays the compiler's**, which is why
names on their way *out* of the compiler must never be sigilled on the
way. The two printing verbs are absent from this table for that reason:
`core:fmt` declares them, and a row here would have claimed them for the
compiler permanently. A positional statement word is absent for the
other reason — no name ever reaches the lookup.


The second is the set of attributes, and the thing to notice is that it is
**registered** rather than open. An open attribute namespace is the shape that
grows into a macro system, one convenience at a time, and each addition is
individually reasonable.

### `@` is a registered set, not an open one

There are ten `@` attributes — `@const`, `@test`, `@c`, `@incomplete`,
`@align`, `@symbol`, `@macro`, `@c_body`, `@freestanding` and
`@thread_local` — and a program has no way to write an eleventh. An
unknown `@name` is not an extension point: it is `ERR0418`, refused by
name, with the ten listed back at whoever wrote it.

**An open namespace was available and was not wanted.** A mechanism a
program may extend has to decide what an unrecognised attribute means,
and both answers are bad. Ignore it and a misspelling is silent, so a
program that asked for something and got nothing looks exactly like one
that asked for nothing. Honour it and the compiler has agreed to
something nobody implemented, which is worse, because it will be found
by the C compiler or by nobody.

The second reason is narrower and sharper: an open `@` is the door
arbitrary C walks in through. That is why `@symbol` — the one member
that is itself a mechanism — takes a *registered word* and not text. It
is a second registry one level in rather than a hole in this one, so a
further symbol property is still a deliberate act, just a cheaper one.

The cost is real and is the one chosen: a new attribute is an edit to
the compiler, so nobody can add one from a library. In exchange the set
is enumerated in exactly one place — the list this arm carries — which
is what lets the refusal name the whole family with no second list to
fall out of step with it.

### A `@c` position is not a family member

`@c("…")` is one member of the set, and it is written in four places: a
`foreign` slot's, between its type and its name; a `foreign` `type`'s
own, after the keyword; a `foreign` enumerator's, before its name; and a
`foreign` function's, between its return type and its name.

Those four are **positions**, and positions are not members. The family
is listed once, where the refusal is raised, and each acceptor reads the
one spelling out of that list. The separation is what lets a fifth place
be opened without opening the set: adding a position is an acceptor,
adding a member is a registration, and only the second widens what a
program may write. The count in the refusal stays ten either way.

Every `@` an acceptor does not claim is handed **back**, unconsumed, to
the arm that owns the list. So `@foo` in a slot, and a `@c` written in a
native `type` body where no C name belongs, both arrive at the one
message that names the ten. One `@` is answered once, and no position
grows a second opinion about the family.

What each position *admits* still differs, because C spells a member and
a type differently: an enumerator or a slot takes a plain C identifier,
where a type may also take a tag word and a name. A spelling C could not
have given is `ERR1014`, reported at the string itself, since the
alternative is a C error on a line nobody typed.

### What earns a place in the set

A member is added when a property of the emitted C cannot be stated in
Dado's own types and the C compiler has to be told it directly — and when
its absence is a **defect** rather than a missed optimisation. That is
the bar `@symbol` was admitted under: a function reporting its own frame
address is wrong if it is inlined, and no Dado type says so.

A convenience, a hint, or a switch a program could have written as
ordinary code earns nothing. `@align` is a member because C11's
alignment specifier stands on declarations Dado has no other way to
describe. `@incomplete` is one because Dado never reads the header, so it
cannot derive whether a bodiless `foreign` `type` is one C can size; the
fact has to be declared, and a declared fact needs a spelling.

**None of these words is reserved.** The `@` makes the position
unambiguous by itself, so a declaration spelled `i32 align = 8` and a
call spelled `align(x)` both stay ordinary code, and the whole set costs
the language not one identifier. That is what the sigil is for, and it
is why growing the set by one is cheap for every program and expensive
for the compiler — which is the direction this trade was deliberately
taken in.


The third, `$`, binds a generic parameter. It can only ever *bind* — it never
begins a name — which is what lets the parser tell a binder from a literal
without lookahead.

## Everything is a tuple

This is the axiom, and almost every other decision in the type system is a
consequence of it rather than a separate choice. Read it before reading about
any particular type, because otherwise each one looks like a special case and
none of them are.

### Everything is a tuple

Function arguments, structs, arrays, unions, enums and multiple returns
are one construct, and that is the axiom the rest of the model is
derived from. A language carrying six of them owes six answers to
equality, to layout, to destructuring and to what a generic binding may
stand for, and gets them out of step — which is where most of a type
system's special cases come from. There is one answer here because
there is one thing to answer about.

**A one-slot tuple is strictly its element**, collapsed at construction
rather than seen through by every reader afterwards. So no one-slot
wrapper exists to be compared, dumped or mangled two ways, and a
declaration of one named position over an `i32` *is* `i32`. Keeping the
wrapper and teaching each consumer to erase it writes one rule into
dozens of places and gets it wrong in one of them.

**An array is a tuple whose slots share a type.** `TypeArena::array` is
`TypeArena::tuple` over N copies of one id and nothing else, so `[N]T`
is not a second kind of type with rules of its own: `[0]T` is the
empty tuple and `[1]T` is its element, both by the collapse above
rather than by a case written here.

Being subscriptable is then a consequence and not a grant. Every slot
is the same width, so slot k sits k widths from the front and a
computed subscript is arithmetic the layout has already licensed. A
declaration whose slots differ has no such spacing, which is the whole
of why it is not subscriptable — the refusal falls out of the model
instead of sitting beside it as a rule.

```dado
type Rgb: (f32 r, f32 g, f32 b)

// One declaration, both reads: Rgb is [3]f32, so a written name and a
// computed subscript reach the same three slots.
public f32 channel(Rgb c, i32 i):
    return c.r + c[i]
```

**The cost is paid in the emitted name.** A mangled name spells every
slot, so twenty of a six-slot declaration mangles to 612 characters and
has to be cut back to the 63 C guarantees, with a hash on the end to
keep two long names from becoming one.


The immediate consequence is about **names**, and it is the one that surprises
people: a slot's name is a view over a shape rather than part of the shape's
identity.

### A name is a view, not identity

Slot names take no part in type identity. A declaration that gives its
positions letters and an anonymous one of the same slots are **one type**,
one arena entry and one emitted C struct: nothing about the name was ever
stored in the shape, so `TypeArena::equal` has nothing to compare. A dump
under `--emit=types` prints both as the bare shape, which is the honest
rendering rather than a lossy one.

**This is the tuple axiom paying out, not a second decision.** Were naming
a position to make a new type, a declaration of three `f32` positions would
stop being the array of three and stop being subscriptable, and the one
construct would have quietly become two. Names are therefore held beside
the shape and handed to a rendering when one asks — which is what
`SlotNames` is: a borrowed function from a declared name to one string per
slot, owned by the crate that knows which declaration is in hand rather
than copied into the arena, where it would be a second copy to go stale.

**The cost is that Dado → C → Dado is not an identity, and the loss is
real.** A declaration whose slots differ survives: `(i32 id, f32 w, u8 t)`
emits a struct of three named members and comes back spelled as written.
One whose slots share a type does not. `(f32 x, f32 y)` emits
`struct { float s[2]; }` and returns as `([2]f32 s)` — because that shape
*is* the array of two, and a struct of two named members has nothing a
computed subscript could reach.

Two repairs were measured and both fail: an overlay through an anonymous
union compiles and the binding generator cannot read it, and taking the
address of the first member and walking is undefined behaviour. So the
loss is stated instead of patched. **Signatures are an identity** — name,
count, order, widths and parameter spellings all return unchanged — and a
declaration is exactly where the round trip stops being one.

### What a view may not outlive

A pointer, a window and a trait value are **views**: Dado did not allocate
what they name and does not keep it alive. So a view is refused wherever
it could outlive its storage, and the compiler follows it to find out —
through names, tuple slots, either arm of a conditional, and calls.

**Storage on a frame** — a local, a by-value parameter, a loop or block
binding — is not returned, not stored through a parameter or into a
package variable, not appended into a handle either reaches, not handed
to a function that keeps it, and not read after the block that declared it
ends (ERR0826; `#stack` storage the same, ERR0821). **A block behind a
`ref` or a map** is not read through a view after anything that may move
or free it — `#append`, `#resize`, `#reserve`, `#delete`, an insert,
rebinding the handle, or a call whose body grows or frees it — and a
`defer` counts where it runs, at the scope's exit (ERR0803). A copy of a
handle shares its block: when either moves, the other is stale, and
rebinding the place it was copied from hands the block to the copy. A
`ref` passed by value to a function that frees it is gone after the call.
And one call is not handed a view into a block together with the handle
to it, when the callee may grow the block and then use the view (ERR0827).

```
^Vec2 make(f32 a):
    Vec2 v = (a, a)
    return &v                 // ERR0826: v's frame is gone

void touch(ref []Cell r):
    r[0].grow(&r)             // ERR0827: grow may move r, then write self
```

What each call does is read from the called function's body, so nothing is
written in a signature. What is not followed is said out loud: a `foreign`
function or a function value is taken to keep nothing, a conversion to
`rawptr` drops what it pointed at, and a handle copy stored in the heap or
a package variable is not tracked there.

### What a frame takes back

A block minted from an allocator the compiler can **name** lies in a
region: `core:runtime`'s scratch (`runtime.temp()`, `runtime.enter_temp()`,
or a `using` of either), or a `core:mem/arena` arena
(`arena.allocator(&a)`). It lies above the innermost frame or mark the
function holds open on that region when it is minted — a `runtime.enter_temp`, a
`runtime.temp_open`, an `arena.begin` — and growing it moves it above the innermost
one open when it grows. **What ends the region ends the block**: rolling
the frame back (`runtime.close_temp()`, `runtime.temp_close(cp)`,
`runtime.close_temp_hoisting`), ending the mark (`arena.end`, `arena.end_keeping`),
destroying or resetting the arena or the scratch. After that the handle, its
copies and its views are not read, written, walked, grown or handed on
(ERR0803); `#len`, `#cap`, `#delete` and rebinding stay legal. A frame
closed with `runtime.close_temp_keep()`, or opened escaping
(`runtime.enter_temp(true)`), is not rolled back: what it held belongs to the
enclosing frame.

The same holds through calls. A helper that closes the frame its caller
opened (`close_frame()`) ends that frame's blocks at the call. A function
that grows a handle it was handed inside a scratch frame it rolls back takes
the growth back with the frame, so a caller that hands it a handle **it
knows is in the scratch** — or in an arena over it — may not use that
handle after the call. And a function that mints its result on its
caller's allocator inside a frame it rolls back hands a scratch caller
memory that is already gone.

```
void build(^ref []i64 out):
    using runtime.enter_temp():
        _ = #append(out^, 1)      // grown above build's frame
    _ = runtime.close_temp()      // ... and rolled back with it

using runtime.enter_temp():
    ref []i64 small = #make([]i64, 0)
    build(&small)
    i64 v = small[0]              // ERR0803: the call took it back
```

What is not followed is said out loud. A handle whose allocator the
compiler cannot name — `#default` or `#return` a caller chose, a parameter,
a package variable, `core:app`'s own frames — is not checked, and neither is
its growth inside a callee's frame: the callee cannot see its caller's
allocator, and it hands the result out by reserving before the frame,
hoisting it, or keeping the frame when `runtime.temp_allocs()` moved. A
frame some path closes with `runtime.close_temp_keep()` is taken to decide at its
close. A handle kept in a struct's field, a `rawptr` (the bytes
`runtime.close_temp_hoisting` and `arena.end_keeping` hand back among them), and a frame
opened or closed on only some paths are not followed either.

### `#truncate` shortens a run where it lives

`#resize` is exact because you named a number: a `#resize` down hands
the capacity back to the allocator, and the block may move. Where the
point is to keep the room — a stack popped and pushed again — the verb
is `#truncate(r, n)`. It writes the length and nothing else: no
allocator is asked, the block does not move, the capacity is what it
was, and a view taken before it still points into the same block.

It only ever shortens. A count the compiler can see is negative is
refused (`ERR0828`); a count past the run's length is a trap at run
time that names both numbers, on every target, and `--no-assert` does
not remove it. `#truncate` of a nil run to `0` does nothing.

**The elements it drops are not destroyed**, because Dado has no
destructors: a `ref` held in one is not freed, and is lost if nothing
else holds it — `#delete` it first. Their storage is zeroed, so the
room past the length reads as zero, as a fresh `#make` does, when a
later `#resize` grows back into it.

```dado
package doc_sample

// Pop the top of a stack: the length drops by one, the capacity stays.
public i32 pop(^ref []i32 stack):
    i64 top = #len(stack^) - 1
    i32 value = stack^[top]
    #truncate(stack^, top)
    return value
```


That buys a great deal and it costs something specific, which is said out loud
where it is described rather than discovered later.

## Where nominality lives

If names are views and shapes are structural, then two declarations that happen
to agree are the same type — and sometimes that is exactly wrong. Money and
metres are both a number and adding one to the other is a bug.

So there is one construct whose entire job is to add an identity, and one that
deliberately does not.

### Where nominality lives

Two declaration words, and exactly one of them makes a type.

**`type` adds no identity.** It binds a name to a shape and stops
there, so the name is a spelling and the shape is the type. Two
declarations of the same slots are the same type however they were
spelled or wherever they were written; handing one where the other is
required is not a conversion, and there is nothing for a check to ask.

**`distinct` adds one.** The shape is computed exactly as it would be
without the word and a tag goes on afterwards — a modifier, not a
second kind of declaration, so nothing about the shape changes when the
word appears. The tag is the package name joined to the declared name,
so two packages declaring one name are two types and identity does not
depend on the order files were read.

What that buys is the refusal. A temperature and a bare `f32` are the
same float and only the tagged one declines to stand in the other's
position; the diagnostic is `ERR0504`, raised at the argument and
naming both shapes rather than at the declaration.

What it costs is that every crossing is now written out. Nothing
converts on your behalf, so a tagged value reaching a routine that
takes the underlying shape needs a conversion at the call site, and
enough of those is a wrapper nobody wanted to maintain. Spend it where
the mix-up is the bug — two floats measuring different things — and not
on every name.

**The tag sits beside the shape and erases at transpile.** Operators
are inherited within it, the emitted C is the underlying shape's C, and
`TypeArena::underlying` reaches through for everything downstream that
wants bytes rather than a label. A tag that changed the layout would be
a different feature wearing this one's word.

`distinct` over a union is refused with `ERR0515` rather than quietly
ignored: a union is already nominal by its tag set, so the word would
promise a second tag that is not there.

### A C type is not a third kind

A transparent `foreign` type restates a layout a header already
named — `type Vector2: (f32 x, f32 y)` — and it **is** the shape it
restates, as a `type` declaration is: `Vector2`, `(f32, f32)` and
`[2]f32` are one type for checking, for generics, for operators and
for `where`. `v.x + v.y`, `v[i]`, `for a in v` and
`linalg.length(v)` all work on one, and there is no parallel native
type to keep beside it and no conversion to call. A window is the one
exception, for C's reason: `v[0..<2]` is one run of memory, and where
the header named each slot as a member of its own the slots are two
objects, not one run — so the window is refused there and taken where
one member holds every slot (`type Big: ([40]f64 a)`).

What stays two is the **spelling**. The header calls the type
`Vector2` and Dado calls its own `(f32, f32)` something else, and C
never makes two separately declared structs compatible — so wherever a
value moves between the two, the emitted C copies it across member by
member. The layout assertions the emitted C carries for every
restatement are what make that a copy and not a reinterpretation.
One type in the checker, two names in the emitter.

**A value crosses; an address does not.** A `^Vector2` and a
`^(f32, f32)` point at two C struct types, and there is nothing to
copy — `ERR1025` refuses the pointer, the window and the reference
alike. Nor is a restatement that has no Dado shape to be one type with:
a struct laid out in **bits**, and one ending in a flexible array
member, stay types of their own, as a `distinct` does. And a program
that wants a C type kept apart from its shape writes `distinct` over
it, which is the word for that everywhere else.

`type A: B` over a `foreign` type is `B`, as it is over any `type`:
a transparent `B`'s member names are `A`'s, a function taking `B`
takes an `A`, and `distinct type D: B` keeps the names under a tag of
its own. What `B` refuses — an `@incomplete` type by value, a
`cunion`'s `switch`, a flexible tail's length — `A` refuses too.

### A number that does not fit its type

An integer literal takes the type it is written for, and a value that
type does not hold **wraps**, exactly as the conversion does at run
time: `u8 f = 300` holds 44, `i8 c = 200` holds -56, and `u32 x = -1`
holds 4294967295. Because that is rarely what was meant, it is a
lint — a warning while writing, an error under `--strict` — whose repair
is to write the conversion, `u8(300)`, which says the wrap is on
purpose and is accepted. The rule is the same wherever a number meets a
type: a constant's initializer, a value folded from other constants, an
untyped constant at its use, a default argument, an enum member, a
`#LEN`, a slot of an array literal, and a literal meeting no type at
all, which is an `i32`. A character literal that does not fit its
character type keeps its low bits on the same terms, and `char8('é')`
says so on purpose.

What has no value in any type is refused instead: an integer past 64
bits, a constant whose folding overflows the compiler's arithmetic, and
a float that would be infinite or not a number in its type.


## Generics constrain, and never implement

A generic parameter is bound with `$`, and what a function may *require* of one
is written in a predicate before the body. The important word is *require*: the
predicate says what must be true, and never what to do about it.

That line is the whole reason this feature did not become an interface system.
A constraint that could also supply an implementation is a trait, and a trait is
its own construct with its own design, built in a0.12 and described below;
keeping the two apart was a decision rather than an accident of scheduling. A
`where` may ask whether a declaration took a trait on, and the methods then
called are still the declaration's own.

### `where` constrains and never implements

A header `where` is **one** comptime predicate, written on a signature
after the parameter list and before the `:`, stating the domain a
template is for:

```dado
public i32 index_of([]$T xs, T probe) where #has_eq(T):
    i32 i = 0
    for x in xs:
        if x == probe:
            return i
        i += 1
    return 0 - 1
```

It is a gate and nothing more. It selects no body, carries none, names
no implementation and supplies none: a type satisfying the clause is
admitted to the one body the template has, and a type that does not is
refused at the call site (`ERR0919`). There is no spelling of `where`
that means *and here is what to do instead*.

**The selection half was designed and is not built.** A chain of `where`
arms standing in place of a body would want a *list* of predicates
walked by this same evaluator, so it will inherit the one that exists
rather than half of a second one. Until then a signature carries one
predicate and no arms at all, because a field nothing reads cannot be
tested.

One question about a clause needs no call to answer: whether it names
anything the signature binds. A predicate naming no binding is a
constant — true for every call or false for every call — and neither is
a domain, so it is refused once at the declaration (`ERR0920`) instead
of once per call.

### The vocabulary is closed

A predicate is written out of a fixed list, and nothing outside it is
admissible.

**Leaves.** A name the enclosing signature binds, a type spelling, or an
integer literal.

**Operators.** `==`, `!=` and `is` between types; `in` from a type to a
parenthesised list; the six comparisons and the five arithmetic
operators between integers; `&&`, `||` and `!` between the answers.
Nothing else. There is no order on types, so `<` between two of them is
not a false claim but no claim at all, and `is` and `in` both reduce to
the identity `==` already asks.

**`has`.** `T has +, <` holds when every operator listed applies, a
binary one as `T op T`, answered by the same code the operator itself is
decided by — so `has ==` and `#has_eq` cannot give two answers. Written
with a type name in place of an operator, it holds when `T`'s
declaration embeds that type, directly or transitively; it is asked of
the declaration and never of the layout, so a binding that arrived as a
shape written out carries no declaration and is refused rather than
guessed at.

**Predicate words.** One closed roster, spelled with the sigil and
resolved through the same table every other compiler-owned name is. No
second namespace.

The narrowness is the whole design. A wider vocabulary would be a second
constant folder — one walking scopes, raising its own diagnostics and
refusing a binding outright — where this is a walk over about a dozen
node shapes. The cost is paid honestly: a condition the list cannot
express has to arrive as a predicate word or not at all.

### Asked at the instantiation, after the memoisation hit

The predicate is evaluated where the answer is knowable: at the
instantiation, of what a call actually bound. The **call** is what a
refusal underlines, never the template's own line — that line is correct
for every other caller, so blaming it would blame the wrong text.

It is asked at each *new* instantiation, from the point **after** the
memoisation hit. A repeat call binding the same types never re-asks it.
That is not an optimisation: it keeps the number of evaluations
proportional to how many distinct shapes a template is used at and
independent of how often it is called, so no predicate becomes a
compile-time cost by sitting in a hot loop.

The price is the bargain every uncalled template already strikes: a
clause on a template nothing calls is checked only for whether it names
a binding. Nothing evaluates it, exactly as nothing checks the body.

### `#has_eq` is not `#is_key`

`#has_eq`, asked of a binding, is whether `==` applies to it — decided
by the same code that decides the operator, so the word and the
operator give one answer and cannot drift apart.

`#is_key`, asked of the same binding, is whether it may key a map,
which is two further questions: whether `==` is *stable*, and whether
Dado can lay the key out. It is strictly the narrower of the two. `f32`
and `f64` have `==` and are not keys; `#c.int` and its fellow C types
have `==` and are not keys either.

They stay two words because they are two rules, and the narrower one is
asked wherever a type reaches key position anyway — so a binding that a
`where` clause admitted on `#has_eq` is still refused, at that same
instantiation, the moment it is used as a key.


A declaration can take a binder too, and then it is a family of types rather
than one: each argument written after its name is an instantiation, made where
it is written and shared by every later writing of it.

### A generic `type` is instantiated where it is written

A `type` head may bind whole types after its name, and its body writes
them bare. `Pool(i32)` is then a type: the declaration instantiated at
`i32`, made the first time it is written, from the declaration's own
file, and the same type wherever it is written again. A generic
declaration's name alone is not a type, and neither is one written
with the wrong number of arguments; both are refused quoting the head.

```dado
package doc_sample

type Pool($T): ([4]T items, i64 used)

// A recursive container is a pointer away from itself, as any is.
type List($T): (T head, ^List(T) tail)

// `Pool($T)` in a signature reads `T` back out of the argument.
private void put(^Pool($T) p, T v):
    p^.items[p^.used] = v
    p^.used += 1
    return

public i64 filled():
    Pool(i32) ints = ([0, 0, 0, 0], 0)
    put(&ints, 7)
    Pool(f32) reals = ([0.0, 0.0, 0.0, 0.0], 0)
    put(&reals, 0.5)
    List(i32) last = (2, nil)
    List(i32) first = (1, &last)
    return ints.used + reals.used + i64(first.tail^.head)
```

**An instantiation is a declared name, so everything a declared name
does it does.** Its slots are named by the declaration's body, it is
interchangeable with its shape, and its C struct is its shape's, so
`Pool(Vector2)` and `Pool(Size)` over one shape are one struct in the
C. They are still two instantiations — one per argument, each
carrying its argument's declaration exactly as `[]Vector2` and
`[]Size` do — and `--emit=types` lists each as it was spelled,
beside a `generic Pool($T)` row for the declaration itself. A
`distinct` generic is a tag per instantiation.

A `where` on the head is the declaration's **domain**: it is asked of
each instantiation's arguments where the type is written, and an
instantiation outside it is refused there, naming the clause. A body
nobody instantiates is never lowered, which is the bargain a template
has: what is checked is what a program asks for.

A generic body names every slot it has. Embedding splices another
shape's names into this one, which would give each instantiation a
different view, so a member written without a name is refused with
the name to give it; a binder is a whole type and never a count, and a
channel, being one mailbox named after its declaration, belongs in a
declaration that binds nothing.

### Methods and traits on a generic `type`

A method written in a generic body is a template over the
declaration's binders. Each instantiation has its own copy of it,
declared and checked the first time a call or a trait it takes on
reaches it: `Pool(i32)` and `Pool(Vector2)` never share one, and a
method nothing reaches is never checked — so its signature may write
the generic at a bigger argument, made only when it is called. As
with `Vec2.len`, `Pool(i32).push` names one as a function. Inside it,
`T` is the instantiation's argument and `Self` the instantiation;
`self` and `^self` take the value as for any method.

```dado
package doc_sample

trait Sized:
    i64 count(self)

type Stack($T) using Sized:
    [4]T items,
    i64  used,
    void push(^self, T v):
        self.items[self.used] = v
        self.used += 1
    i64 count(self):
        return self.used

public i64 both():
    Stack(i32) ints = ([0, 0, 0, 0], 0)
    ints.push(7)
    Stack(f32) reals = ([0.0, 0.0, 0.0, 0.0], 0)
    reals.push(0.5)
    reals.push(1.5)
    return ints.count() + reals.count()
```

An instantiation is a declaration of its own, named for the generic
and its arguments: its methods, the traits it takes on and its
number as a trait value's implementer are `Stack(i32)`'s, never its
shape's. A `using` on a generic head takes the trait on for **every**
instantiation, and conformance is checked for each at its own
arguments: a method whose signature holds for `Stack(i32)` may not
for `Stack(f32)`, and that one is refused, naming it. There is no
conditional conformance — a trait taken on only where `T` has some
other — so a `using` that needs something of `T` is written with a
`where` that makes it the domain.


## Methods and traits

A trait is that other construct. Behaviour is keyed on a **declaration**, never
on a shape: a method is written in a `type`'s body and called statically, and a
trait prescribes methods and never a shape, taken on by name with `using`. So
two declarations over one shape still share every function written against the
shape, and differ only in what their own bodies say. The grammar comes first —
what a declaration head may say after its name, where a method is written, and
what its first parameter is — and the checking follows it.

### What a declaration head and body may now say

A `type` head reads, after its name and in this order, each part
optional: a binder list `($T, $U)`, the traits it takes on
(`using Drawable, Movable`), a `where` over its binders, and then its
`:`. A `trait` head has the same shape with `extends` where a `type`
has `using`, because a trait builds on traits and a type takes them
on — so each word written on the other head is refused naming the
right one. Nothing else can stand between a declaration's name and
its `:`, so neither word is reserved: `i32 using = 2` and
`i32 extends = 2` stay ordinary declarations.

A method is a line of a `type` body whose name is followed by `(`. A
slot holding a function writes its type first, `R(A) name`, so one
token tells the two apart. Three bodies carry methods: a member list
written one per line, slots and methods mixed; a parenthesized list
on the head line with the methods indented under it; and an alias
with the methods indented under it. Those last two wrote the shape on
the head line, so the block under them holds methods only.

    type Vec2 using Drawable:
        f32 x
        f32 y
        f32 len(self):
            return self.x
        void move(^self, f32 dx):
            self.x += dx

    type Size: (f32 w, f32 h)
        f32 area(self): return self.w * self.h

A method's first parameter is its receiver, written with no type:
`self` for a copy of the value it was called on, `^self` for a pointer
to it. The word is a receiver only there — `self` is an ordinary name
everywhere else, including a typed parameter called `self`.

A `trait` body is methods only, because a trait prescribes methods
and never a shape: a signature ending its line is one every
implementer writes, and one with a `:` and a suite is a default. A
slot line in a trait is refused naming the `type` it belongs in.

    trait Drawable:
        void draw(self)
        void draw_twice(self):
            self.draw()
            self.draw()

**Methods, traits, a head's `using`, a generic declaration's binder
list and `where`, and a trait written as a value's type are built** —
the last is a trait value, below. A generic trait's binders are read
and refused `ERR0001` where they are written, so a program using one
hears that rather than getting half of it.

### Traits

A `trait` names a set of methods and nothing else — never a slot, since
a trait prescribes methods and never a shape. A signature ending its
line is **required**; one with a body is a **default**, which an
implementer that writes no method of that name gets.

```dado
trait Drawable:
    void draw(self)

trait Shape extends Drawable:
    f32 area(self)

type Square using Shape:
    f32 side
    void draw(self):
        return
    f32 area(self):
        return self.side * self.side
```

A trait is in the type namespace, so a trait and a type of one name in
a package are refused. `extends` names traits and never cycles; a
trait's methods are its own and its ancestors'. A child may give a
parent's *required* method a default, and may not replace a parent's
*default* — only an implementer overrides a default, so each pair of
implementer and method has one body. Two parents asking one name with
one signature ask one method; two defaults or two signatures for one
name are refused at the child, naming both.

**Conformance is declared, at the `using`.** A `type` takes a trait on
by writing `using` it (or a trait that extends it) at its head, and
every required method must then be in its body — or promoted into it
by embedding — with the trait's signature, `Self` read as the
implementer, and the same receiver. A missing or different one is
refused there, naming the line to write. Embedding an implementer
does not make the embedder one.

**A trait written as a parameter's whole type is a binder**: `void
render(Drawable v)` takes any declaration that takes `Drawable` on,
and is instantiated once per implementer, in which `v.draw()` is a
direct call to that implementer's method. `^Drawable v` binds a
pointer the same way. `where T has Drawable` asks the same question
of a template's own binding. A literal or a shape written out
implements nothing — `Drawable` is answered by a declaration's
`using` — and a declaration that does not take the trait on is
refused at the call, naming the `using` to add.

### Default bodies

A trait method written with a body is a **default**: a template over
`Self`, the implementer. It is instantiated once per implementer that
takes it, when a call reaches it — `v.draw_twice()` on the implementer,
or on a binder's value, or `self.draw_twice()` in another default — and
never by the `using` alone, so a default no call reaches is never
checked. An instance is the implementer's own method as far as any
caller can tell: it is called, and emitted, exactly as a method of that
name written in the implementer's body would be: embedding promotes
it, and `Box.draw_twice` names it as a function value, which
instantiates it as a call would.

```dado
trait Drawable:
    void draw(self)
    void draw_twice(self):
        self.draw()
        self.draw()
```

Inside a default, `self` has **no view**: a trait prescribes methods
and never a shape, so a default reaches `self` only through its
trait's methods, required or default. `self.x` is refused whatever
the implementer's slots are called, and so is a method only one
implementer writes — make it a required method, and each implementer
answers it. A `^self` default writes through the trait's `^self`
methods the same way.

### Trait values

A trait name written anywhere but a parameter's whole type — a slice
or array element, a field, a local, a return, a map value, a function
type's parameter — is a **trait value**: a view of an implementer
stored somewhere else, the pair of its address and its declaration's
number. `#size(Drawable)` is the pair's: two words, 16 bytes where a
pointer is 8.

```dado
trait Drawable:
    void draw(self)

type Scene:
    []Drawable items
    Drawable focus
```

Zero bytes are the empty value, so `Drawable d` with no initialiser
and `[4]Drawable` are empty until assigned. A trait value copies into
another of the same trait, and is passed and returned like any view.
It has no `==`, is no map key and has no rendering — compare or
render the implementers.

**A trait value is made by writing its implementer where one is
asked** — a declaration, an assignment, an argument, a `return`. The
implementer is a place — a name, an element, a field, `p^` — or a `^T`
or `ref T` pointing at one: the pair views it where it is stored, so a
temporary or a constant is refused with *bind it first*. Its type is a
declaration that takes the trait on; a shape written out implements
nothing. A value of a trait that extends the one asked converts as it
stands. `Drawable(b)` is no conversion: there is one spelling. A
`const` implementer is refused, since a `^self` method reached
through the pair would write it.

**A call through a trait value dispatches at run time**: it reaches
the method of whichever implementer the value views, its own or its
default. `self` gets a copy of the implementer and `^self` the
implementer itself, so a write through a pair writes the original.
A call through an empty value stops the program, naming the trait.
A method whose signature mentions `Self` beyond its receiver, or that
binds a type of its own, cannot be called through a value — the pair
does not say which declaration `Self` is — and is called through a
binder or on the implementer instead. A trait value passed to a
binder parameter binds the pair, which dispatches inside.

**Every rule a view obeys, a trait value obeys.** A pair of an
implementer the function's own frame stores does not outlive it —
returned, stored where it outlives the frame, or handed to a function
that keeps it — as a window over a local array does not; a pair made of an element
of a `ref` run is stale after `#append`, `#resize`, `#reserve` or
`#delete` on it; a pair of `#stack` storage stays in its block; and
no trait value rides in a channel's message or crosses to C through an
`export` or a `foreign` signature — C is handed the implementer's own
type.

### The `Allocator` trait

An allocator is two words — a procedure and its state — and
`core:mem`'s `Allocator` is the trait over them: one required method,
`data()`, answering the two words, and a default, `ok()`, asking
whether they carry a procedure. No type names the two words; code that
holds them writes the shape.

```dado
trait Allocator:
    (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) data(self)
    bool ok(self): return self.data().proc != nil
```

**Wherever an allocator is asked, either answers**: the two words — a
rung such as `#default`, a local, a slot, a call that hands them back —
or a value of a declaration that takes `Allocator` on, whose `data()`
is called there, or an `Allocator` trait value, or a `^` of one,
whose `data()` is called through the pair's dispatch — so a
`[]mem.Allocator` of an arena and the heap can each be opened with
`using d:`. That is a verb's last argument, a `using` subject,
`#format`'s last argument, and every parameter, local, slot or return
written as the shape. `Allocator a` as a parameter's whole type is a
binder, instantiated per implementer, and takes implementers only —
so a parameter that must also take a rung is written as the shape.
The two words' slots read as `proc` and `data` wherever they are held.
A trait never crosses to C: an `export` or `foreign` signature takes
the two words.


### Methods

A `type` declaration's body may hold **methods**: functions written
with the declaration, called on a value of it. A member whose name is
followed by `(` is a method; every other member is a slot.

```dado
type Vec2:
    f32 x
    f32 y
    f32 len2(self):
        return self.x * self.x + self.y * self.y
    void move(^self, f32 dx):
        self.x += dx
```

A method is **keyed on the declaration**, never on the shape: `Vector2`
and `Size` over one `(f32, f32)` each have the methods their own body
writes, and `v.describe()` asks the declaration the value was typed with. A
value's declaration travels wherever its type does, and its methods
with it; a `view` renames slots in its own scope and touches no
method. Inside a method, `self` is viewed by the declaration that wrote
the method, whatever the caller's view called its slots.

Every method takes a receiver, `self` or `^self`, written first with no
type; a function of no receiver is a package-level function. `Self`
names the declaration inside its own body. A slot and a method may not
share a name, because `v.name` answers exactly one of them, and a
method is no more visible than its type — an unmarked method is
exactly as visible. A `union`, `cunion`, `enum`, `foreign` or channel
declaration holds no methods: declare a `type` over it and write them
there.

A method is a function in every other way. `Vec2.len2` names it as a
value — `f32(Vec2)`, or `void(^Vec2, f32)` for `^self` — and it is
emitted as a direct function, dropped when nothing reaches it.

### Embedding promotes methods

A declaration that embeds another by name takes its methods on for
calls: `type Sprite: (Vec2, i32 id)` makes `s.len2()` call `Vec2.len2`
on a `Vec2` rebuilt from the positions `Vec2`'s slots were spliced
into. Embedding splices slots, so there is no `Vec2` object inside a
`Sprite` for a `^self` method to point at: a promoted `^self` method is
refused at the call, naming the member to hold instead. Two embedded
members bringing one method name, or a method and a slot, are refused
at the declaration, as two slots are; a method the embedder writes
itself collides with a promoted one rather than hiding it.

A **redeclaration** `type W: V` has `V`'s slots and view and none of
its methods: `W`'s methods are what `W`'s body writes. `V.len(w)`
still calls `V`'s, since a parameter takes any value of its shape.

### Calling a method

`recv.name(args)` calls the method of that name of the declaration the
receiver was typed with, once no slot of that name answers — a direct call, the same C a
call to a function makes. The receiver is passed first, the way the
method takes it:

| receiver written | `self` | `^self` |
|---|---|---|
| a place: a name, `s[i]`, `p^` | a copy of it | its address |
| a temporary `make()`, a `const` | a copy of it | refused — bind it first |
| a `^T` | a copy of what it points at | passed as is |
| a `ref T` | a copy of what it holds | the pointer it lends |

Through at most one `^` or `ref`: a `^^Vec2` is not a receiver. A
method is called, so `v.len` with no call is refused and names the
function, `Vec2.len`. In a template, calling a method on a value of `$T`
asks `T`'s declaration, so the template is instantiated once per
declaration and each calls its own method.


## A procedure is a tuple in and a tuple out

Nothing in this section is a feature. Everything in it is the axiom arriving
somewhere else — which is the test of an axiom, and the reason this section
comes after the type system rather than beside it.

A parameter list is a tuple. A return is a tuple. So multiple returns are not a
feature that had to be built, named arguments are not a second way to call
something, and *how many values may a function return* is a question that was
already answered.

### A signature is a tuple in and a tuple out

A parameter list is a tuple and so is a return, which is why multiple
returns needed nothing built for them. `(i32 a, f64 b) g()` and a local
written `(i32, f64)` are **one arena entry** — not two shapes that happen
to agree, one interned shape reached by two spellings. A dump under
`--emit=types` shows the single tuple where a reader might expect two.

So *how many values may a function return* is the same question as *how
many slots may a shape have*, and it is answered once.

```dado
package doc_sample

type Pair: (i32, i32)

private (i32 lo, i32 hi) split():
    return (3, 4)

private i32 sum(Pair p):
    return p[0] + p[1]

// `split`'s names are a view over the shape, so its value fills a `Pair`
// with no conversion written anywhere.
public i32 total():
    Pair p = split()
    return sum(p)
```

The names travelling with the view rather than the type is the
consequence people meet first: a caller that does not restate them
reaches the slots by the axes instead, and a name the caller never
declared is not found.


A procedure that can fail says so in its return type, and what it fails with is
a code rather than a value — which is the one thing about failure that every
other rule follows from.

### A failure is a code, and never a value

A function whose return type carries `!` can fail: `!i32 measure(string8 s):`
ends either by `return`, which succeeds, or by `fail` with a nonzero `i32`,
which does not. A failure is never stored — no value's type holds one — so a
failable call is made only by `try`, at the start of a statement. With
nothing after it, `try` passes the failure on to the function it is written
in, which must be failable too, and `return` of the call passes it on whole.
`else:` handles it where it happens, and `else code:` binds its code for the
suite.

**`E!T` says which codes.** With an enum in front of the mark,
`Why!i32 measure(string8 s):` fails only with a member of `Why` — an `enum`
or a `distinct enum` backed by `i32`. Every `fail` in it is known to be a
member, and every failure it passes on comes from a call failing with the
same enum, so the code a caller binds is a `Why` and one of its members by
construction. That makes it the second thing a `switch` may be exhaustive
over: name every member, and the `else` may be left out. The binding is
read-only, so it stays a member. A member equal to 0 may be declared — an
enum that also names success, `OK = 0` — but is never a failure: no `fail`
may use it, no caller binds it, and a closed `switch` need not name it.

```dado
package doc_sample

distinct enum i32 Why: (EMPTY = 1, TOO_LONG)

public Why!i32 measure(string8 s):
    if #len(s) == 0:
        fail Why.EMPTY
    if #len(s) > 8:
        fail Why.TOO_LONG
    return i32(#len(s))

// Every member is named, so no `else` is written and none is missing.
public i32 size(string8 s):
    try i32 n = measure(s) else why:
        switch why:
            case Why.EMPTY:
                return 0
            case Why.TOO_LONG:
                return -1
    return n
```

A plain `!` function passes such a failure on as its `i32` code. An `E!`
function passes on only its own enum's: a failure of another enum, or a plain
one, is handled where it arrives and failed with a member. A `foreign`
function names no enum, because what C answers is not checked here. Across
every boundary the code is the same `i32` either way.


Visibility is the one thing here that is **not** downstream of the type system,
and it is worth saying so out loud, because the two look alike and are not.

### Visibility is a name rule, not a type rule

Four spellings and one slot for them: `public`, `private`, file-scoped
`private`, and `export`. Writing none of them means `public`, and writing
`public` says what leaving it out says — so *meant public* and *forgot*
are distinguishable in the source, which is the only reason the redundant
spelling is allowed. Two modifiers in one slot is refused rather than
resolved. The formatter writes back the word that was written and
invents none, and `dadoc --warn-unmarked` asks for a word on every
package-level declaration (`WRN0419`, a warning and never an error).

**`private` hides a name and never a shape.** A `type` declared `private`
is still the tuple it is, and another package that writes the same slots
gets the same type — because the shape was never the package's to own.
That is structural identity holding, not a rule about visibility.

The file-scoped form narrows a name to one file of a package and is
otherwise `private`: both emit the same C linkage, and the whole
difference is which Dado file may write the name.

`export` is `public` plus a promise outside Dado: the symbol is emitted
under the name as written, unmangled, for something that is not this
compiler to call. Inside Dado it reaches exactly as far as `public`
does.

**An `export` is exported on every target**, whether or not anything in
the program calls it: no compiler or linker drops it. In a native
executable it is a visible symbol in the dynamic symbol table, so
`dlsym` and a host that loads plugins reach it; in an object file or a
library it is an ordinary public symbol, and on `#NONE` an external one
the link keeps. On the web target, `#WEB`, it is a **wasm export** under
the same name, listed by `--emit=build` as a `wasm-export` line, so
JavaScript calls it through the instance's exports:

    export i32 add(i32 a, i32 b):
        return a + b

is `instance.exports.add(2, 3)` on a page. The signature crosses by the
target's C ABI, the same on every target: an integer of 32 bits or fewer,
a `bool` and a pointer are `i32`s, an `i64` a JavaScript `BigInt`, `f32`
and `f64` numbers; a `string`, a slice or a tuple is passed by the address
of its bytes in the module's memory (a `string` is its `(ptr, len)` pair),
and one answered is written to an address the caller passes first. **A
failable export keeps the destination word** it has in C: the address its
value is written to comes first, and it answers an `i32` code, 0 when it
succeeded. A program that wants its host to write into its memory exports
its own allocator, `export rawptr alloc(i64 n)`.

The other direction is not an `export`: Dado reaches JavaScript only
through DadoScript, by calling a script's function.

### An import may make its qualifier optional

`import . "core:fmt"` binds the package under its last element, as `import
"core:fmt"` does, and also spells each of its public names bare in that
file: `println("hi")` and `fmt.println("hi")` are the same call. That is
how a Dado program writes a bare `println`: no name is built in. A script
writes the same line, for a Dado package or a package of scripts, and a
class imported this way is named bare (`Goblin`) as well as
qualified (`npc.Goblin`).

A bare name means one thing, and nothing decides between two by
precedence or shadowing. A bare name that two unqualified imports both
supply — or one of them and a declaration of the file's package, or in a
script one of them and a built-in such as `println` — is refused where
it is written (`ERR0309`), naming each spelling it could mean; the
qualified spelling is always there to write. So two packages that
overlap may both be imported unqualified, and nothing is refused until
an overlapping name is written bare. A declaration in the importing file
that takes a name an unqualified import supplies — a function, a type, a
constant, a class, or a member of a class the file holds — is refused at
the declaration (`ERR0310`). A local or a parameter is a function's own
and shadows a bare import as it shadows the package's own names. An
import belongs to its file, so another file of the package is not
affected.


If the parameter list is a view, then naming an argument is naming a slot —
same operation, no new machinery, and the rules that follow are about
ambiguity rather than about types.

### Named arguments are the parameter list being a view

A parameter list is a view over the argument tuple, so naming an argument
is the same operation as naming a slot — there is no second mechanism and
no extra machinery in the call. Names reorder: `sub(b: 4, a: 30)` is
`sub(30, 4)`.

Three rules hold it together, and each is a refusal rather than a
resolution.

A name must be one the callee declared. A slot may be filled once. And a
call is **all positional or all named**, never a mixture — because in a
mixed call which slot a positional argument fills depends on counting the
names written to the left of it, and that is a rule a reader has to
execute rather than read.

```dado
package doc_sample

private i32 sub(i32 a, i32 b):
    return a - b

public i32 backwards():
    return sub(b: 4, a: 30)
```

A splat fills positions, so it cannot be given a name.


A default looks like part of a signature and is not, which is the one thing
about it worth knowing before it surprises somebody.

### A default is call-site sugar and not part of the type

A parameter may carry a default, and the value is folded at the
declaration: it may name a constant and it may not name a call, nor
another parameter — a parameter is not in scope where its neighbour's
default is written, and that is a scope fact rather than an ordering one.

Defaults are **trailing-only**, and the refusal is at the declaration
rather than at some later call. A positional call fills slots left to
right, so a gap in the middle would be a slot the call could not say it
had skipped. A *named* call may omit any defaulted parameter it likes,
because a named call never had to count.

**A default is not part of the function's type**, which is the part worth
knowing before it surprises somebody: a function declared with two
parameters, one of them defaulted, has a two-parameter type, and binding
it to a one-parameter function reference is refused. The sugar exists at
the call site and nowhere else — the emitted function has no idea.


The last piece is the one that genuinely is a convention rather than a type,
and it is spelled to look like one so that nobody reaches for it in a place a
type is wanted.

### A pack is a calling convention, not a type

A parameter may take the rest of the arguments. Two spellings, and the
difference is what the caller may write.

A homogeneous pack takes any number of one type. `#any` takes whatever
the call writes, each argument keeping its own type — and the function is
instantiated once per *sequence* of argument types, so the body is
compiled knowing exactly what it was handed. Neither one is a runtime
list and neither allocates: the storage is a compound literal in the
caller's own block.

**A pack is not part of a function's type.** It cannot be written in a
function reference, and it cannot cross into a `foreign` signature —
where it would have to mean C's own variadic convention, which is a
different thing that this language does not promise to match.

```dado
package doc_sample

// `#len` over a pack is the count the call wrote, known at compile time.
public i64 count(..#any xs):
    return #len(xs)
```

One parameter may be the pack and it is the last.


## Control flow, and what it refuses to be a concept about

Every construct here could have been an abstraction. A condition could have
been anything coercible to truth; iteration could have been a protocol a type
implements; a `switch` could have been pattern matching. None of them are, and
in each case the same reasoning applies: the construct answers from the shape
in front of it, and there is nothing a program can implement to change the
answer.

That is a real cost and it buys one thing — a reader can tell what a loop does
by looking at what it walks, without finding out what somebody taught the type
to mean.

### A condition is a `bool` and nothing else is read as one

C takes any scalar and calls zero false. This does not, and the reason
is the mistake it prevents: an `if` written over an integer is how a
comparison against zero gets written by accident, and the two read
identically on the page.

One rule, one place: the same check answers for `if` and for the
conditional `for`, so the two can never drift apart.

`if` is a statement and produces no value; a conditional **value** is
`a if c else b`, which is an expression and not a statement `if`.

An `else if` chain is flat rather than nested, and an `else` binds to the
`if` at its own indentation — layout is the block delimiter, so the
dangling `else` the C-family grammars argue about cannot be written.

### The conditional value

`a if c else b` is `a` when `c` holds and `b` when it does not, and only
the one taken is evaluated. It is the loosest expression there is and
right-associative, so `a if c else b if d else e` chains to the right
with no parentheses. A newline ends it, so its `else` is on the same
line — except inside brackets, where a line continues until they
close, as it does for any expression: `(a if c` and `else b)` on the
next line is one value. The condition is a `bool`, as every condition
is.

**The two arms are one type**, and nothing is found in between: no
widening, no common type, no conversion supplied. An untyped literal in
one arm — `#LEN` among them — takes the other arm's type, exactly as it
takes a typed sibling's in an array literal, so `n if n > 0 else 0` is
`n`'s type whatever that is. Two arms of two types are refused, and
the repair is the conversion written on the arm that should change.

```dado
package doc_sample

// A folded constant: package-level, outside every `when`, and so
// visible to everything downstream — a `when` included.
const i32 PAGE = 16384 if #OS == #DARWIN else 4096

public i32 limit(i32 base, bool verbose):
    return base if base > 0 else PAGE if verbose else 256
```

It is an expression and not a statement: written on a line of its own it
is a value thrown away, and refused as one. What runs something only
when a condition holds is an `if`.


The loop is where the refusal is sharpest, and where the axiom shows up for the
last time in this arc.

### Iteration is stride, not a concept

There is no iterator in this language: no trait to implement, no hook to
provide, nothing a program can make walkable. What may be walked is
decided by one closed question — *does this shape have one element type*
— and the answer comes from the layout.

That is the tuple axiom paying out again. An array is a tuple whose slots
share a type, so every element sits the same distance from the one before
it, and a loop variable has a type to have. A shape whose slots differ
has no single element type, so there is nothing for the loop variable to
be; the refusal is a fact about the shape rather than a feature nobody
wrote yet.

A slice walks, a string walks as its code units, a map walks as key and
value, and a value that is not a tuple at all walks once as itself — that
last one falling out of *a one-slot tuple is its element* rather than
being a case. A channel walks as its parameters, destructured, through
`#peek` or `#drain` — any number of them, and one name binds the whole
message as its tuple.

```dado
package doc_sample

// `Vec2` is two slots of one type, so it is `[2]f32` and it walks.
type Vec2: (f32 x, f32 y)

public f32 total(Vec2 v):
    f32 sum = 0.0
    for a in v:
        sum += a
    return sum
```

A range is the exception and is not a value: it is a thing to count
through, and what it binds is what it counts in.

### A value's slots, walked at compile time

`for name, slot in #slots(a):` walks the slots of the declaration `a`'s
type was written with, in the order it declares them. It is not a loop:
the body is written out once per slot and each copy is checked on its
own, so `slot` has that slot's own type — no box, no tag, no dispatch at
run time — and `name` is that slot's name as a `string8` constant, the
text `#names` gives for the same position.

```dado
package doc_sample

type Point: (i32 x, i32 y)

// Written out twice: once with `name` = "x" and `slot` = `p.x`, once
// with "y" and `p.y`.
public i32 sum_except(Point p, string8 skipped):
    i32 total = 0
    for name, slot in #slots(p):
        if name != skipped:
            total += slot
    return total
```

Two bindings are the name and the value, a map's order; one binding is the
value. The value is a copy, as a `for` over an array binds one, and
`for name, &slot in #slots(a)` binds each slot's address instead, which
is how a reader fills a value in. With no loop left in the program there
is nothing for `break` or `continue` to leave, so one aimed at the walk is
refused; guard the rest of the body with an `if`.

The point is generic code. A template walking `#slots(a)` is
instantiated once per declaration, and calling itself on a slot's value
instantiates it at that slot's type. Inside a template a value with no
slots — a number, a string, a slice — walks zero times, so the same body
renders a leaf and walks a declared type, and one function serializes
every declared type there is. Written where the type is fixed, the same
walk is refused, because it could only ever do nothing. So are a pointer
(walk `p^`), a union, whose members are alternatives rather than parts,
and a shape no declaration named, which has no names to give.


`switch` is where a reader's expectations from other languages are most likely
to be wrong, and the surprise is in the *opposite* direction from the usual
one: the construct people expect to be exhaustive is not, and the one they do
not expect to be is.

### Exhaustive over a union or a bound failure, never over an `enum` value

A `switch` compares a scrutinee against arms. The scrutinee is evaluated
once, arm values are constants, and an arm an earlier arm already covers
is refused rather than left unreachable.

The rule worth knowing before it surprises somebody: **a `switch` over an
`enum` always needs an `else`, even when every member is named.** An
enum's members are the values somebody wrote down, not the values the
type can hold — the backing is an integer and a value need not be one of
the members — so promising exhaustiveness there would be promising
something the type does not support.

**A union discriminant is the one thing in this language closed by
construction**, and it is the one place the `else` may be dropped: when
the arms name every member there is nothing left for an `else` to catch.
An arm that binds a name gets that member's type inside its body, which
is narrowing with no flow analysis anywhere — the type comes from the arm
rather than from a deduction about what ran before it.

**The failure an `E!` call answered is the other**, bound by its
`else` as an `E`: every code such a function fails with was checked to be
a member, and the binding is read-only, so a `switch` over it that names
every member needs no `else` either, and one that leaves a member out is
refused naming it.

```dado
package doc_sample

union Shape: (i32, f64)

// Every member is named, so no `else` is written and none is missing.
public i32 size(Shape s):
    switch s:
        case i32 n:
            return n
        case f64 f:
            return i32(f)
```

`fallthrough` hands control to the next arm and is legal only as the last
statement of one, in a value `switch` that has a next arm. It is refused
in a union `switch`, where each arm binds a different narrowed type and
there is no shared body for control to arrive in.

An arm may name several values: `case a, b, c:` is **one** arm. It
matches when the scrutinee is any of them, opens one scope over one
body, and takes part in the first-match rule value by value — a value an
earlier arm, or an earlier place in the same list, already names is
refused as a whole arm would be. A list is written over a value `switch`
and not over a union, where an arm narrows the value to one member's
type and one body cannot see it as two.

**The `else` stays, and it is not where a member goes.** A `switch`
whose arms name members of one `enum` is told about every declared
member no arm names — a warning while writing and an error under
`--strict`, as an unread name is — so adding a member shows every
`switch` it reaches instead of routing it silently into the `else`.
The `else` is for a value that is no member at all — so over a failure
bound from an `E!` call, which is always a member, it may be left out
once every member is named, and one left out is then an error that
names it. Where a member
wants what the `else` does, name it in an arm that says so; one that
wants what another arm does goes in that arm's list, `case a, b:`, and
shares its body. Code that tests a
few members and treats every other alike is an `if`, which claims
nothing about the rest.

A `case` arm ends where its body ends, and `fallthrough` is how one
continues — so **`break` is refused inside an arm**, `else` included.
In C, C++, Java, JavaScript, C# and Go a `break` there leaves the
`switch`; here it would leave the loop around it, the same text with a
different meaning. A `break` meant for the loop goes after the `switch`,
or the arm returns. **`continue` inside an arm is allowed**, and means
what it means in C and Go: the next iteration of the enclosing loop.


Two constructs are left, and neither is about choosing between values. One
chooses between *programs*, before the program exists.

### `when` selects, and the arms it does not select barely exist

`when` is the compile-time branch. Exactly one arm is selected; the
others are lexed and parsed and **nothing else** — not resolved, not
checked, not emitted, and the names they declare do not exist. That is
what lets an arm name a header, a symbol or a type that is absent on this
target: the arm for another platform is never asked to make sense here.

Selection happens before checking, and the condition is folded in an
environment fixed before any arm is taken: compiler definitions,
literals, and package-level constants declared outside every `when`. A
condition may not name something a `when` declares, because selection
decides which names exist and a condition that depended on that would
have no answer to start from.

Only a name known before the program runs may be branched on — `#OS`,
`#ARCH`, `#TRIPLE` and their members. The `#` sigil splits by case and
the split is one-way: the half the compiler owns promises nothing at
compile time, so there is no spelling of it a `when` can select on.

**The admitted cost is that an untaken arm rots.** It parses long after
it has stopped meaning anything, and nothing reports that until somebody
builds for the target it belongs to. Covering the combinations with real
programs is the only thing that finds it.

A `switch` is not an alternative: every arm of a `switch` is checked and
emitted, including the arms a given build can never take.

**A build invariant is written `#static_assert(COND, "message")`**, at
package level. Its condition is folded exactly where and as a `when`
condition is — so it may name `#` definitions and package-level constants
declared outside every `when`, and a `#size` or a function call is
refused with the `when` rule's own reason — and a build in which it folds
to `false` is refused with the message. The refusal says where each
operand's value came from, because the usual culprit is a `-D`:

```dado
package doc_sample

@const i32 RING_CAPACITY = 64

#static_assert(RING_CAPACITY & (RING_CAPACITY - 1) == 0, "RING_CAPACITY must be a power of two")
```

Built with `-D RING_CAPACITY=63`, that is refused, naming the flag. One
inside an arm a `when` did not select is never folded, like everything
else there.


The other is about leaving — and about making the leaving say so at the point
the thing was acquired, rather than at each of the four exits.

### `defer` runs at scope exit, not at function exit

A deferred statement runs when the block it was written in ends — so one
inside a loop body runs at the end of **each** iteration, and every
iteration starts from the same state the caller's did. Within a scope
they run in reverse order of registration, which is the order that makes
a pair of acquisitions unwind correctly without anybody thinking about
it.

There are four exits and all four unwind: the end of the block, a
`return`, a `break` and a `continue`. A `return` unwinds every frame up
to the function; a `break` or `continue` unwinds up to the loop. What a
deferred statement may **not** be is anything that leaves the scope it is
unwinding — a `return`, a `break`, a `continue`, another `defer`.

An exit that leaves several scopes leaves them **one at a time**,
innermost first: each scope's deferred statements, then the allocator
and the script sinks its `using` displaced put back, then the next
scope out. So a deferred statement runs under the allocator and sinks
that were in force where it was written, whichever exit reaches it — a
`defer` at the top of a function is not served by a `using` suite a
`return` happens to leave from.

`quit` is the exception and it is preservation rather than a decision: it
ends the process, and pending deferred statements do not run. Covering
that would be a language change rather than a fix.

A deferred statement may be marked to run only when a **failure** leaves
the scope. That is one stack with the ordinary ones, in the same reverse
order, and every non-failing exit skips it — which is how a release runs
on the way out of a function that gave up, without the success path
paying for a flag.

A cleanup that can itself fail — a flush, a close, a commit — is
deferred with `defer!`: `defer! flush(h)` holds one failable call made
alone, runs at every exit like a plain `defer`, and passes its failure
on. If it fails while the scope is otherwise succeeding, the function
now fails with its code, running the rest of the unwinding as a
failure; if a failure is already leaving, the call still runs and the
first code is the one kept. It sits on the same stack as the others, so
a `defer fail` registered *before* it runs after it and sees its
failure. It stands only in a failable function (`ERR0714`); a plain
`defer` of a failable call is `ERR0611` and a deferred `try` is
`ERR0707`, and both name `defer!` as the repair. The success path pays
one `int32_t` slot in the function's frame, tested where each exit ends.


## Channels: handing a value to another thread

A frame loop, a worker pool, an input backend and a network thread all need the
same thing — hand a value to another thread without a lock and without losing
it — and a program that answers it four times answers it four ways. A channel
answers it once, typed, with what may travel checked, and with one layout the
other side of any boundary can read.

### A channel is a mailbox: many producers, one consumer

`#channel(N) name(params)` declares one, at package level or as a member of
a `type`: a typed, bounded, lock-free mailbox of `N` messages, each one
the parameters' fields. `#channel(latest)` keeps only the newest message
instead. Its storage is static or inline in the value that holds it, and
its zero bytes are the empty channel, so it needs no initializer and
nothing runs before the program starts — which is also why it works on a
target with no operating system at all.

`#send` hands a message to it from any thread and never blocks, its
arguments checked as a call's are. **A queue's `#send` fails when the queue
is full, and the program says what that means**: it is written under
`try`, failing with a code or handling it in a suite, exactly as a map
read that finds nothing is. A `latest` channel's `#send` replaces the value
nobody took and cannot fail, so it takes no `try`.

The consumer walks what is queued with `for`: `#peek` looks and consumes
nothing, `#drain` takes, and `#drain(ch, count)` takes at most that
many. A walk is a snapshot — it sees what was queued when it began and
always ends — and a `#drain` **commits when its loop ends, by any exit**:
running off the end, `break`, `return`, or a failure passing out of the
body. An entry whose body began is consumed. `#drain(ch)` alone on a line
is the discard. `#len` counts, from any thread.

```dado
package doc_sample

const i32 FULL = 1

#channel(32) pressed(i32 id, bool down)
#channel(latest) resized(i32 w, i32 h)

!i32 input(i32 id):
    try #send(pressed, id, true) else FULL
    #send(resized, 800, 600)
    return 0

i32 frame():
    i32 held = 0
    for id, d in #drain(pressed):
        if d && id != 0:
            held += 1
    i32 area = 0
    for w, h in #drain(resized):
        area = w * h
    return held + area
```

**What a message may hold is what can outlive the sender.** Plain data is
copied into the slot. A window, a pointer, a string, a map or another channel
is refused, because each is a view of storage the sender keeps. A `ref`
written as a parameter of its own **moves**: a successful `#send` writes
the sender's place nil and reading it afterwards is refused, while on the
path where the queue was full it is still the sender's. A `#drain` binds it
as the owning `ref` and a `#peek` as the view it lends; a `latest` channel,
which overwrites, takes none.

**A value holding a channel is never copied** — it is one mailbox at one
address, and a copy would be a second — so it is reached through a
pointer. And a channel has one consumer: a debug build traps when a second
thread walks one, naming the channel and both threads, and two thread roots
that both walk one package-level channel are a warning, an error under
`--strict`. A consumer that is done hands the channel on with
`#release(ch)`, a statement of its own: the next thread to walk it
becomes the consumer, and a channel the program releases is not linted.


## Threads on the web

A web build is one thread unless it asks for more, because a thread on the web
is a worker sharing the module's memory, and a page gets a shared memory only
when it is served for one.

### `--web-threads`: a threaded web module

`./dado build --target=web --web-threads` (and the driver's run and test
commands with the same two flags) builds a module whose threads are real. The C is compiled
with wasm's atomics and bulk memory (`-matomics -mbulk-memory`), and the
module imports a **shared** memory of at most 1 GiB, which the loader
creates and hands to every worker it starts. `threads.spawn` becomes a
host call carrying a function-table slot and a pointer; the loader answers
it with a worker — node's worker threads, a `Worker` in a page —
that instantiates the same module on the same memory, points its stack
pointer at a 1 MiB stack the spawner allocated, and runs the thread there.
Each worker gets its own thread-local block before anything runs, so the
ambient allocator and every `@thread_local` variable are per thread, as
they are natively. `#heap` and the console each take a lock.

**`threads.join` waits**, with `memory.atomic.wait32`. Node's main thread may do
that; **a browser's main thread may not** — the engine traps a wait there —
so a join on it is refused: it answers `false` and says why on standard
error. Join from a worker instead, or have the thread set a flag the frame
loop reads. A thread that traps ends the program with status 134, as an
abort on a native thread ends the process.

**A page serving a threaded module must be cross-origin isolated**, or it
has no shared memory at all. The document is served with both headers:

    Cross-Origin-Opener-Policy: same-origin
    Cross-Origin-Embedder-Policy: require-corp

Without `--web-threads` a web build is single-threaded, its atomics are
plain loads and stores, and a `threads.spawn` it reaches is refused at
compile time, naming the flag (`ERR1033`). The flag with any target but
the web is refused too: a native build has its threads already.


## The page

A web build writes the page that runs it in a browser, from a template the
author owns: ordinary HTML with comments the build fills in.

### The page: an HTML template with `dado:` markers

`./dado build --target=web` writes, beside `<package>.wasm`, a page that
runs it in a browser: `<package>.html`, the `loader.js` it loads, and,
for a `--web-threads` build, a headers file named _headers. The page comes from a
template the author owns, given with `--html=<template.html>`; without
one the toolchain's default is used, which holds the markers and a
`<pre id="dado-output">` for the program's output.

A template is ordinary HTML. Where it writes `<!-- dado:NAME -->` — a
comment, so the template opens and validates as a page of its own — the
build writes the marker's output:

* `dado:importmap` — the `<script type="importmap">` mapping the
  JavaScript modules the build writes beside the module to their files,
  with each file's hash: a program that hosts DadoScript scripts has one,
  for the scripts the build writes beside the module.
  Nothing, when the build writes no such module. **Required.**
* `dado:preload` — a `<link rel="modulepreload">` for each of those
  modules.
* `dado:loader` — the `<script type="module" src="loader.js">` that
  fetches the module, instantiates it and runs its entry point. **Required.**
* `dado:title` — a `<title>` naming the package. Write your own `<title>`
  instead to call the page something else.
* `dado:base` — a `<base href>` from `--base=<prefix>` (default `./`), so
  the page works from a subpath or loads its files from another origin.

Every file the page references is a file of its own, never inlined, and
each reference carries `integrity="sha384-…"`, so a page served under a
Content-Security-Policy with no `'unsafe-inline'` runs: such a policy
needs `'wasm-unsafe-eval'` in `script-src` for the module itself, and,
when the import map is not empty, the hash the build prints for it.

A program that hosts DadoScript scripts runs on the page as it runs natively.
The build writes its scripts beside the module, in `<package>.scripts/`,
and names each file in the module's build manifest; the page preloads
every one with its hash, the import map holds every hash, and the
loader's tag names the scripts' `registry.js`, which the loader imports
before the entry point runs — on a threaded page, in the worker that runs it,
and again in each thread's. A script file changed after the build is
refused by the browser, and the program does not start.

A template is refused at build time (`ERR1034`), naming the marker and
the repair, when a required marker is missing, a marker is written
twice, a `dado:` name is not one of the five, or a marker stands where
its output cannot work: inside a `<script>`, `<style>`, `<template>`,
`<textarea>` or `<title>` body; `dado:importmap` after a module script,
after `dado:preload` or after `dado:loader`; `dado:base` after a marker
that writes a URL.

On the page, standard output and standard error go to the console and
into `<pre id="dado-output">` (made at the end of the body when the
template has none), standard error in a `<span class="dado-stderr">`.
When the program ends, its status — what the entry point returned, 134 for a trap, 2
when the loader could not start it — is written into the element whose id is
dado-status (made when absent), set as `data-dado-status` on the `<html>` element and
on `window.dadoPage.status`, and announced with a `dado-exit` event. A
`?dado-seed=N` in the page's address pins the hash seed, as
`DADO_HASH_SEED` does for `./dado run`.

A threaded page (`--web-threads`) must be cross-origin isolated, so the
build writes _headers beside it — the static-host convention — with
`Cross-Origin-Opener-Policy: same-origin` and
`Cross-Origin-Embedder-Policy: require-corp`. A page served without
them does not run the program: it says, on the page and on the console,
which of the two headers is missing. A threaded page runs its entry point in a
worker, where `threads.join` may wait, and each thread in a worker of
its own.

`./dado run --target=web` still runs the module under node; the page is a
build artefact, run by a browser.


## The manifest

What a build publishes — Dado's exports, the shapes they reach, what the scripts
export — is one JSON document the compiler writes, and every other format a
caller wants is a separate program that reads it.

### The manifest: the build's public surface, as JSON

`dadoc --emit=manifest` writes one JSON document describing what a
build publishes — **public declarations only**, from both dialects:

    dadoc --emit=manifest --package game
    ./dado build game --manifest=h,dts

* **Dado's `export`s** (`dado.functions`): each one's Dado signature, its C
  symbol and C prototype, the failure convention a failable one keeps —
  an `int32_t` code, and the destination its value is written through —
  and on the web its wasm signature;
* **the shapes those signatures reach** (`dado.types`): tuples, slices,
  strings, maps, unions, enums and `distinct` tags, each with its C name,
  its members and, where the compiler can say, its size, alignment and
  every member's offset — for the target named at the top
  (`target.pointer_bytes`), never another;
* **what the scripts publish** (`dados.classes`, `dados.functions`): each
  exported class with its constructor, fields and methods, each package
  function, every type as the DadoScript spelling and as what a JavaScript
  caller receives;
* **the public channels** (`dado.channels`): name, kind, capacity and the
  index a reader of the channel directory finds it at.

Every entry carries its `dialect`, its `span` (file, line, column and
byte range) and its `doc`: the `//` comment lines directly above the
declaration, with no blank line between. Private declarations and
function bodies are never in it.

The first field, `"schema": "manifest/1"`, names the format: a reader
checks it, and a change that would break a reader changes the number.
The format is `tools/manifest/schema.json`, a JSON Schema the gate
validates every manifest against.

**Other formats are extensions**: separate programs that read the
manifest on standard input and write their one output to standard
output. Two are official — `h`, a C header declaring every Dado export
with the types it needs, and `dts`, the TypeScript declarations of what
the scripts publish (`--emit=js-package`'s `index.d.ts` is written by
it). `./dado build --manifest=h,dts` runs them beside the build; a path
in the list names a third-party one.


## Shaders: DadoGL, reserved

DadoGL is the shader language a later increment builds. Its spellings are
reserved now, so a program and a graphics backend written today do not change
when it lands.

### DadoGL is reserved

**A `.dadogl` file is a DadoGL shader unit.** DadoGL is not built yet, so a
package directory holding one is refused, `ERR0001`, naming the file: the
compiler recognises the file by its name and reads nothing in it.

**`#shader_load(Lit)` will load one** — `Lit.dadogl` in this package, or
`#shader_load(pkg.Lit)` for one in an imported package — and answer a
`Shader`, the trait `core:shader` declares whole today: `to_string()` for
the text formats (WGSL, GLSL, GLSL ES, HLSL, MSL), `to_bytes()` for the
binary ones (SPIR-V, DXIL, metallib), `format()` saying which, and the
reflection a backend lays out its pipeline from — `entry_points()` and
their stages, `bindings()` by group and slot, and the vertex layout,
`vertex_attributes()` and `vertex_buffers()`. Until DadoGL is built the call
is refused `ERR0001`, after its argument is checked to name a unit.

**A backend can be written now.** It takes a `Shader` — as a template
parameter, or a trait value held in a slot — and nothing about it changes
when shaders arrive; a test hands it an implementer of its own.

**`--shader=` picks the formats** a build compiles its shaders to, a list
from the closed set — all eight written out,
`--shader=wgsl,glsl,essl,hlsl,msl,spirv,dxil,metallib` — one bundle may carry several. A name outside the set is
refused, naming the set; a build asking for any of them is told DadoGL is
not built yet.


## DadoScript: one tree, two dialects

DadoScript is the scripting dialect a Dado host loads. It shares Dado's front end, so
the two parse into one tree, and which dialect a piece of code is written in is
decided by the declaration around it, never by a comment or a flag.

### One tree, two dialects: where each construct may be written

A `.dado` file is Dado and a `.dados` file is DadoScript; a `class` block is DadoScript
wherever it stands, so a `.dado` file may hold one, and everything inside it
is DadoScript. Nothing smaller than a declaration changes dialect: a function
body, a statement and an expression are written in the dialect of the file
or class around them.

Most of the grammar is shared, so most code means the same in both. What
only Dado writes — memory management (`ref`, `^`, `&`, allocators), every
`#` spelling, explicit widths (`u8`, `f32`), the C boundary (`@c`,
`foreign "x.h":`, `cunion`, bitfields), the failable mark `!`, `quit`,
`package` and `export` — is refused in DadoScript, `ERR1301`, naming what DadoScript
writes instead: `#len(xs)` is `xs.len()`, a width such as `u8` is the one
integer type DadoScript has, `!T` is `T`, and `ref []T` is `[]T`. What only DadoScript
writes — lambdas, `async` and `await`, the `assert` statement, `super`, a
script's header line, and a `foreign "x.js":` block — is refused in Dado,
`ERR1302`, naming Dado's spelling where it has one: `assert c, "m"` is
`#assert(c, "m")`.

DadoScript's eight bare built-in names — print, println, any, Error, Location,
Script, Platform and globalThis — exist only in DadoScript. In Dado they are
ordinary names a program may declare.

### A script is a class

A `.dados` file declares one class. Its first line may name it,
`class Goblin`; without that line the class is named by the file's
basename — a file `goblin.dados` declares a class named goblin — and a
basename that is not an identifier is refused (`ERR1303`). A `class Name:` block with an
indented body declares a class inside a `.dado` file. Either way the class
is a member of its file's package, and two classes of one name in one
package — or a class and a Dado declaration of that name — are refused
(`ERR0402`).

A class holds **fields** and **methods**. A field is written like a
variable, `int hp = 10`, and without an initializer starts at its type's
zero. Fields are initialised in the order they are written, so an
initializer reads only the fields above it. Every function written in a
class is a method; writing `self` as its first parameter is allowed and
changes nothing. Inside a method a field is reached by its bare name or as
`self.hp`, and a method is called as `bump(1)` or `self.bump(1)`; a local
or parameter of the same name hides the bare field, and `self.hp` still
reaches it.

Two method names belong to the harness that runs a script: **`init`** runs
once per instance and takes the constructor's arguments, so it returns
nothing; **`ready`** is the entry a host runs with `#script_run`, as a
program's main function is run, so it returns an `int` or nothing
(`ERR1304`), and takes the arguments `#script_run` passes after the handle.

A script's types are `int` (exactly Dado's `i64`, wrapping), `float`
(`f64`), `bool` and `string` — JavaScript's string, which has no width.
An integer literal with nothing to adapt to is an `int`. Everything else
is Dado's: the operators and what they accept, how a literal adapts and
wraps, linted, where it does not fit, conditions that are `bool`, `for`
loops over a range or on a condition, and every conversion written. So a `float` is never made an
`int` by itself — write `int(x)`, which drops the fraction — nor an `int`
a `float` — write `float(x)`. Strings join with `+` and compare with `==`
and `!=`; a value is made a string only by `string(x)`, so
`"total: " + 5` is refused naming `string(5)`. `print` and `println` take
any number of those four types and write them with no separator.

What DadoScript will have and the checker does not check yet is refused
`ERR0001`, naming it. A script may import a Dado package, and calls its
functions as a Dado file does (see *A script calls Dado*).

Each class is emitted as a JavaScript module named `<package>/<Class>`,
which `dadoc --emit=js` prints. There an `int` is still exactly `i64` —
past 2⁵³ and wrapping at 2⁶³ — and an integer division or remainder by
zero raises an error, `integer division by zero`, where Dado stops the
program. A `float` is written as the fewest digits that read back as the
same value, `1.0`, `0.1`, `1e+16`, `1e-05`; not-a-number prints nan, the
infinities inf and -inf, and negative zero `-0.0`. A program holding DadoScript builds as its Dado, carrying the
module of every class it loads and no other: a class nothing loads is
checked and left out. A main package of scripts alone has no Dado to host
them, and its build is refused `ERR0001`.

### Classes are types

A class's name is its type and its constructor: `Goblin g = Goblin(3, 4.0)`
declares a local of the class's type and makes an instance, passing the
arguments to the class's init method, as a method call passes them. An
instance is held by reference: a local, a field, an argument and a return
all hand over the same one. `g.hit(2)` calls a method of it, and `g.hp`
reads or writes a field of it. A field or method is public unless written
`private`, which keeps it to the package, or `private "file"`, which keeps
it to the file. A field or local whose type is a class is written with its
value, since a script has no null to start it at.

A script may hold an inner class: a `class Loot:` block with an indented
body, named `Goblin.Loot` in the package and `Loot` inside `Goblin`. Inner
classes go one level deep, and one is never loaded on its own.

A class extends one other: `class Boss extends Goblin`. It inherits every
field and method, and a method of the same name overrides the inherited
one, taking the same parameters and answering the same type. In a
subclass, `super.hit(n)` calls the parent's method, and `super(…)`,
written once in the init method, runs the parent's — required when the
parent's init takes arguments, and otherwise run first by itself. A class
that extends nothing extends `Script`. A cycle of `extends` is refused
(`ERR1310`), and so is extending a type Dado declares: a Dado base for scripts
is not built yet.

A package of scripts is imported as any package is: `import "game/npc"`,
then `npc.Goblin` is its class, as a type and as a constructor, and
`import foes "game/npc"` binds the package under that other name. Scripts may import each
other in a cycle. A directory holding only scripts is a package named by
the directory, since a script names no package.

### A script's arrays, maps and strings

`[]int` is an array and `{string: int}` a map, written as Dado writes
them — `[1, 2, 3]`, `{"ada": 36}`, and `[]` or `{}` where the type is
declared — and both are **held by reference**: assigning one to another
name, or passing it, hands over the same array, so a change through one
name is seen through the other. A slice, `xs[1..<3]`, is a new array; so
is `copy`. A map keeps its entries in the order they were first put in,
and is keyed by an `int`, a `float`, a `string` or a `bool` — never by a
container (`ERR0801`), whose `==` is not defined (`ERR0607`): which array
or what it holds is a question TypeScript and GDScript answer
differently. `[]byte` is the byte buffer: an element reads as an `int`,
and storing an `int` keeps it modulo 256.

Reading what is not there **raises**, never answers a stand-in: `xs[i]`
past either end, with Dado's own words (`index out of bounds: 4 is not a
position in a run of 4`); `m[k]` when the key is missing (`m.get(k,
default)` is the form that does not); `pop` of an empty array; and a
search that finds nothing — `index_of`, `last_index_of`, `find_if` — so
`contains` asks first. `for x in xs` walks an array (`for x, i in xs`
adds the position), `for k, v in m` a map in its order, and `for c in s`
a string by code point.

A string is JavaScript's: `s.len()` counts UTF-16 code units, `s[i]` is
one of them as a string, and `s[a..<b]` cuts at them; `code_point_at`
reads a code point's number and `bytes` the UTF-8. The methods are the
admitted lists — 26 for an array, 9 for a map, 19 for a string and
`decode_utf8` on a `[]byte` — and another language's name for one is
refused naming DadoScript's (`ERR1305`): `size()` is `len()`, `push(v)` is `append(v)`.
The methods that take a function — `sort`, `filter`, `map`, `reduce`,
`any`, `all`, `find_if` — take a lambda written in place, its types
first, `xs.filter(bool(int v): return v > 2)`, or any function of the
shape, a method's name among them: `xs.filter(is_big)`. `print` and `string(x)`
write a container as Dado does, `[1, 2]` and `{ada: 36}`.

### Traits

A class takes a trait on with `using`, written on its first line after any
`extends`: `class Boss extends Goblin using Drawable`. A trait is written
in a class, as an inner class is — `trait Drawable:` in `Goblin` is
`Goblin.Drawable` in the package and `Drawable` inside `Goblin` — and its
body is methods only: a signature ending its line is required, one with a
body is a default. A trait may extend other traits.

A class that takes a trait on has every method the trait prescribes, with
the trait's signature, `Self` read as the class — its own or one it
inherits — and a method missing or of another shape is refused at the
`using`, naming the line to write (`ERR0536`). A default the class writes
no method for becomes its method. Inside a default, `self` reaches only
the trait's methods, since a trait prescribes methods and never fields.

A trait is a type: `Drawable d = g` holds any instance of a class that
takes it on, and `[]Drawable` holds several of different classes. A call
through one runs the method of whichever class the value holds. A method
taking a `Self` is called on the class's own type, never through a trait
value, which cannot say which class `Self` is. A trait Dado declares is not
taken on by a script: that is the boundary between classes and Dado's
types, not built yet.

A trait method may be `async` — `async int load()` — and the method a
class writes for it is `async` too; a call through a trait value answers
`async int`, awaited as any other. An `async` method answering `Self` is
not built (`ERR1330`).

### Enums and `switch`

An enum is written in a class, as a trait is — `enum Mood: (Calm, Angry)`
in `Goblin` is `Goblin.Mood` in the package and `Mood` inside `Goblin` —
and its members are names only, with no backing type and no values. It is
its own type, and a value of it is always one of its members, written
`Mood.Calm`. Two compare with `==` and `!=`, an enum may key a map, and a
member prints as its name. A field or local of an enum written without a
value holds the first member.

`switch` takes an enum, an `int` or a `string`. Each `case` names members
or constants, `case Mood.Calm, Mood.Sleepy:` being one arm over both, and
an arm ends where its body ends, so `break` is refused in one. Over an
`int` or a `string` the `else` arm is required. Over an enum it is not,
since every value is a member: a `switch` naming every member covers them
all. A member no arm names is reported as a warning — an error under
`--strict` — so a member added later shows every `switch` it reaches.

### A script's failures

Any method of a script may fail, and nothing in its signature says so: a
call that fails passes the failure on to its caller unless it is handled.
A failure is an `Error` — a `message`, a `code` and a `location`, and a
`trace` the runtime attaches where it is raised, innermost first — made
as `Error("no such item")`, `Error("no such item", 16)` or
`Error("no such item", 16, Location(pick))`. The code is 70 when none is
given, and is never 0 (`ERR0715`). `print` writes one as `error 16: no
such item`.

`fail` takes three forms: `fail "no such item"` is
`Error("no such item")`, `fail 16` is `Error("failed with code 16", 16)`,
and `fail e` raises an `Error` already made. `try` handles a failure as
Dado's `try` does, over one call or one index, and its `else` binds the
`Error`: `try int n = parse(text) else e:`, then the suite, which leaves
the scope when the `try` declares its name. A bare `try` is allowed and
passes the failure on, as an untried call does. `defer` runs its statement
at every exit of its scope, in reverse order, and `defer fail` only when a
failure is leaving it.

Faults are ordinary failures: an index past either end, a missing key, an
integer division by zero and an `assert` that does not hold each raise an
`Error` of code 70 in Dado's words, which a `try` catches. `assert cond,
"why"` raises `assertion failed: why`.

A `Location` is a place in the source: `Location()` is where it is
written, `Location(pick)` where the method it names begins, and
`Location(pick, 2)` two lines into it — the method must be the class's,
and the line inside its body, both checked when the script is compiled.
As a parameter's default, `Location at = Location()` is the caller's line,
because a default is filled where the call is written. Its `file`,
`line`, `column` and `function` are read like fields, and it prints as
`file:line:column`, or `<unknown>` for the zero value.

### A type's names, and an instance's slots

`Hero.names()` answers the slot names of the class `Hero` as a `[]string`:
every field, an inherited one first, each class's in the order it is
written, which is the order an instance is built in and `print` writes
it. Asked of an enum, `Mood.names()` answers its members in order, and
asked of a Dado declaration a script names, `Vec2.names()`, the names its
slots are declared with. It is a question about a type, so the receiver
is a type written by its name; the names are known when the script is
compiled, and each call hands over an array of its own. A method an
instance has, `held.names()`, is the instance's.

`for name, value in slots(v):` walks the slots of an instance, in the
same order, binding each slot's name, a `string`, and what it holds, an
`any`, since one class's slots hold values of many types.
`for value in slots(v):` walks the values alone. What is walked may be
an instance, a trait's value or an `any`, and the slots walked are those
of the class the instance is, so a `Lion` held as a `Beast` walks the
lion's; an `any` that holds no instance raises a fault, an `Error` of
code 70. Nothing but declared fields is walked — no method and nothing
the engine keeps. This is how a script writes code over any of its
objects — a serializer, for one:

```
string text(any v):
    string out = ""
    for name, value in slots(v):
        out += name + "=" + string(value) + " "
    return out
```

`slots` is a built-in function like `print`, and it walks only as the
iterable of a `for` header; written anywhere else, or where the class's
own member or the package's Dado function named `slots` could be meant,
it is refused (`ERR1350`), and so is `Hero.names()` where `Hero` has an
inner class of that name.

### Async

A method marked `async` — `async int load(string path):` — returns its
value later. Its callers get an `async int`, a value of its own type: it
can be held, `async int pending = load("a")`, kept in a field or an
array and passed on, and nothing else is done with it until a statement
headed by `await` waits for it and reads the `int`: `await int size =
load("a")` declares, `await size = pending` assigns, and `await save(size)`
waits for a call whose value is not wanted.

`await` heads a declaration, an assignment or a call, as `try` does, and
only at the start of a statement: `f(await g())` is refused, and the value
is awaited on a line of its own. It is written only inside an `async`
method or lambda (`async int(): …`), so a method that waits is itself
waited for. A call that answers an `async` value is never written alone
on a line, where nothing would wait for it — await it, or hold what it
answers. `init` and a generic method are never `async` (`ERR1330`).

An `async` value is not printed, alone or in a tuple, an array or a map:
await it and print what it answers. An instance renders whatever its
fields hold, and a pending value among them prints as `<async>`.

A failure inside an `async` method raises at the `await` that waits for
it, with its `Error` unchanged, so `try await int size = load("a") else
e:` handles it as `try` handles a call. A host's `#script_run` of an
`async` method runs the engine until the method's value is ready and
answers it, and every `#script_run` runs to the end whatever work its
method started.

### Tuples

A tuple holds a fixed number of values of fixed types: `(int, string)` is
its type and `(3, "ann")` a value of it, so a method may answer two
values at once. A tuple is a value, like Dado's: assigning one copies it.
Its slots are read by destructuring it into locals,
`int n, string name = pair()`, an underscore naming a slot not wanted, and a slot
is never written in place — a new tuple replaces the whole. Two tuples do
not compare with `==`: destructure them and compare the slots. A tuple
prints as its slots in parentheses, `(3, ann)`.

### `any`, the untyped value

`any` holds a value whose type is known only when the script runs: a
host object, a JavaScript library's answer, or a script's own value put
where any type will do — `[]any` and `{string: any}` are ordinary
containers. **A value goes into an `any` freely**: `any a = 5`, or
appending a `string` to a `[]any`, writes no conversion. **A value comes
out of one checked**: `int n = a`, returning `a` from a method that
answers an `int`, passing it where a `string` is taken, or writing
`int(a)` or `float(a)`, checks what it holds when the line runs and
raises a fault — an `Error` of code 70, `any is not an int: "abc"` —
when it is something else. An array or a map is checked element by
element as it comes out, and an instance of a script's class, an
`Error` or a `Location` by what it is an instance of (`Goblin g = a`
takes a `Goblin` or an instance of a class that extends it). JavaScript
has one kind of number, so an `any` holding a whole number comes out as
an `int` or a `float` alike.

What an `any` holds is reached the way JavaScript reaches it: a member,
`a.name`; a call through one, `a.name(x)`; a call of the value itself,
`f(x)`; an index by an `int` or a `string`, `a[k]`; and a store through
a member or an index. Each answers another `any`. Nothing computes with
one directly — no operator, no condition, no `for` over it, no map keyed
by it — so the conversion is written first, and that is the one place
its type is checked: `int(a) + 1`, not `a + 1`. `string(a)` and `print`
write whatever it holds.

**`globalThis` is an `any`**: the engine's global object, the door to
what the host provides — `globalThis.Math.sqrt(2.0)`, converted where it
is used, `float r = globalThis.Math.sqrt(2.0)`.

### A Dado program runs a script

A Dado program runs scripts in a VM it makes: `try VM vm = #script_vm()`
makes one on `#heap`, and `#script_vm(alloc)` on the allocator written.
A VM is an engine of its own, with its own heap; it belongs to the thread
that made it, and a thread may make several. `#delete(vm)` ends the
engine and everything in it.

`try npc.Goblin g = #script_new(vm, npc.Goblin, 3)` makes an instance of
a class in the VM — `#script_new(vm, Goblin)` for a class of this
package — passing its init method the arguments after the class, and
holds it by a handle of the class's type. The script is never named by a
path: the compiler checked it and carries it in the program, and the VM
loads the class the first time it makes one. Making one can fail, since
init runs, so it is written under `try`. `#delete(g)` lets go of the
instance.

`#script_run(g)` runs the instance's `ready` method, as a program's
main function is run: it waits for it, and answers an `i64` — what `ready`
returns when it returns an `int`, 0 when it returns nothing, and when it
fails, the status a failing main function exits with, its text already on the
error sink. It never fails itself, so it is not written under `try`. The
arguments after the handle are `ready`'s, as `#script_new` passes `init`
the ones after the class: `#script_run(g, 60, "Ann")` runs
`int ready(int frames, string who)`, checked as a method's call is.

Every other method is called through the handle: `g.hit(3)`. The
compiler checks the method against the class, with the count of its
arguments and the type of each: a script's `int` parameter takes any
integer (a `u64` by its bits), its `float` an `f32` or `f64`, its `bool`
a `bool`, and its `string` a copy of a `string8` or a `ref([]char8)`. A
script's array, `[]int`, takes a Dado slice, a `ref([]T)` or an array
whose elements cross, and a tuple takes a Dado tuple of as many slots. It
answers what the method returns, as Dado holds it: an int as `i64`, a
float as `f64`, a bool as `bool`, a string as a `ref([]char8)`, an array
as a `ref([]T)` of those — each made on the ambient allocator, which the
program ends with `#delete` — and a tuple as a Dado tuple of those.

What a script prints and what `fmt.println` writes go to the same
standard output, in the order they ran, unless the VM is given sinks of
its own (below). A method that throws, called under `try`, fails that
statement, with its text already on the error sink; one called without
`try` ends the program with the failure's status after the same text. A
build that cannot run a script is refused where it makes a VM: a `#NONE`
target has no C library for the engine. On the web a script runs on the
page's own JavaScript engine, and the few features that engine cannot give a
wasm module (new code at run time, waiting on an `async` method, a pause
with no second thread) are refused where they are written.

### Dado calls a script's function

An `export` method of a script that reaches no `self` — no field, no
other method, no `self` — is a **function of its package**, and Dado
calls it by the package, with no VM and no handle:

    // game/Rules.dados
    class Rules

    export int on_hit(int hp, int damage):
        if damage > hp:
            return 0
        return hp - damage

    // main.dado
    import "./game"

    i64 left = game.on_hit(10, 3)

Its arguments and its answer are a script's scalars — an `int`, a
`float`, a `bool`, a `string` — crossing as a method's do through a
handle; anything else is refused (`ERR1226`). Under `try` a failure is the
script's `Error`'s code, its text already on the error sink; with no
`try`, a failure ends the program as an `abort` does, status 134. The
call runs on an engine of the calling thread's own for package
functions, made the first time, so it sees no instance a VM holds. A
function that reads only its parameters, constants and the JavaScript it
imports is called directly — on the web its JavaScript is the wasm
module's import, with nothing in between; one that does more, such as
`print`, goes through the seam, and `dadoc --emit=strategies` says which,
and why.

### Stopping, pausing and asking after a running script

A script runs until it returns, and a Dado program that runs one on a
thread of its own keeps the right to end it, from any thread.
`#script_stop(vm, 100)` asks the script running in the VM to stop,
waits up to 100
milliseconds for its run to leave the engine, and forces it if it has
not. Asked, the script meets an `Error` at its next loop or call — its
deferred lines run, and a `try` around the work it was doing reaches the
stop. Forced, it runs nothing more, not even a deferred line; a force
waits for any Dado function the script is inside to return. Either way
the engine then ends: every value it holds is released and its memory
is freed at once, and a later call through any handle on it fails, with
`core:script`'s code `script.STOPPED` under `try`. The
timeout is milliseconds, and it is always written: `#script_stop(vm, 0)`
forces at once, and `#script_stop(vm)` is refused naming both. An engine
with no run in progress ends at once. `#delete(vm)` still frees the
VM afterwards.

The run a stop interrupts answers `script.STOPPED` where it was written
under `try`; written without one, a stop is not a failure — the program
decided it — so the run answers its type's zero value and the program
goes on.

`#script_pause(vm)` holds the run in progress at its next loop or call
until `#script_resume(vm)`; while it is paused, the VM is busy, and a
call or a `#script_new` on it fails with `script.BUSY`. A VM's runs come
only from the thread that made it: one from another thread fails with
`script.THREAD`, while its stop, pause, resume and queries come from any
thread. A stop resumes a paused
script into its polite phase. `#script_is_running(vm)` answers whether a
run is in progress on the engine, paused or not, and
`#script_is_paused(vm)` whether it is paused.

`i64 n = #script_pump(vm)` is the host's frame boundary for channels: it
delivers every message waiting for the VM's handlers — the functions its
scripts bound to the program's channels — and answers how many. A VM
it cannot enter answers a negative code, `script.THREAD` from another
thread, `script.STALE` once deleted, `script.STOPPED` once stopped, its
text on the error sink. Messages are delivered as a call into the script
returns, too; the pump is for a frame that calls nothing.

### An instance crosses into Dado

An instance keeps its methods when it crosses into Dado. A method that
answers an instance of a class — `Goblin spawn()` — answers Dado a handle
of that class's type, `npc.Goblin`, which the program holds and passes on
like any other value; a handle passed back as an argument hands the script
the instance itself. One object is one handle, however often it crosses,
so `==` on two handles asks whether they hold the same instance.
`#delete(g)` lets go of Dado's hold on the instance, and every copy of the
handle — in a local, a field, a slice — is stale from then on: using one
fails with `script.STALE`, never reaches freed memory, and never reaches
another instance. The script's own references to the instance are
untouched, and it lives while the script holds it.

Through a handle Dado reads and writes the class's public fields and calls
its methods by name: `g.hp`, `g.hp = 3`, `g.hit(3)`. A field crosses as
an argument does, a field of a class's type as a handle, and one of any
other type is refused, naming the method to write instead. `g.hit(3)` is
failable under `try`, and otherwise its failure goes where its VM's
failure policy says. A field read or
written is never under `try`, and follows the policy. A Dado function may
take or answer a handle, so a script calls it with its own instances: the
function is handed the handle, and its answer is the instance again. An
instance belongs to the VM that made it: handed to a script in another
VM, it is refused with `script.OTHER_VM`.

### A script reaches JavaScript

A script imports a JavaScript module as it imports a package, by its
path, and the extension says it is JavaScript: `import "lib/three.js"`
(or `.mjs`). The path is the file's, relative to the script's directory
or under a collection, `core:` and its path. The module's namespace is
bound under the file's name without its extension, three, or under an
alias, `import THREE "lib/three.js"`; a file whose name is no identifier,
`three.module.js`, needs the alias (`ERR1320`). The namespace is an
`any`: its members are read and called as JavaScript reads and calls
them, and each answer is an `any`.

A `foreign "lib.js":` block, written in a class, declares the module's
functions with a script's types, and the class calls them by name. Each
argument is checked as a method's is, and the answer is checked when it
arrives: a declared `int` that comes back `2.5` raises an `Error` of code
70, as an `any` leaving into an `int` does. An exception thrown in
JavaScript is caught as an `Error` by `try … else e:`. A block declares
functions; their parameters and answers are `int`, `float`, `bool`,
`string`, `any`, arrays and maps of those, and `void` for no answer, and
a last parameter `..any xs` takes the rest. If the file is also imported,
the declared functions are typed through its namespace too, and every
other member stays an `any`.

The JavaScript is part of the program: each file, and every file it
imports by a relative path, is embedded in the binary under the name
`js:` and its path, and the script's module imports it by that name.

### Where a script's output and failures go

A script's `print` and `println` write to its VM's output sink, and the
text of a failure — the script's `Error` and its trace, or the runtime's
sentence — goes to its error sink. Both are `Sink` values, the two-word
shape `#render` writes into, and both are named, after the allocator:
`#script_vm(out: log, err: problems)` sets them for the VM's life. Left
out, they are standard output and standard error, the streams
`fmt.println` writes to, so a script's lines and the program's appear in
the order they ran. `fmt.to_bytes(&cursor)` is a sink that captures into
memory the program owns.

**For one call or one stretch of code, scope them with `using`**, beside
the allocator. After a call into a script — `c.greet(x) using out: log`,
`#script_run(c) using out: log, err: problems`, `#script_new(vm, Goblin)
using err: problems` — they hold for that call. As a suite, `using out:
log:` over indented statements, they hold for every script call inside:
member calls, field reads and writes, `#script_run`, `#script_new`, and a
script call inside a Dado function called from there, on any VM of the
thread. An allocator comes first and the sinks after it by name, `using
arena, out: log:`. Scopes nest, the innermost winning, and every way out
of one — its end, `return`, `fail`, `break`, `continue`, a failure passed
on — puts back the sinks in force before it; outside every scope a VM
writes to its own. A sink scoped after a Dado function's call is refused:
Dado's own output never goes through a script's sinks, and the suite form
is the one that reaches the scripts a Dado function calls.

### When a script fails

A script's failure reaches Dado as a code. Under `try … else code:`, the
code is the script's own when the script failed — `fail 16` is 16, a
fault 70 — and otherwise one of the runtime's reserved codes, which a
program names after `import "core:script"`: `script.STALE` for a handle
that reaches no script (deleted, or its VM deleted), `script.TYPE` for an
answer of another type than the method returns, `script.NO_MEMORY` when
the allocator refused or a table is full, `script.STOPPED` and
`script.BUSY` for a stopped or busy VM, `script.THREAD` for a call from a
thread that did not make the VM, `script.OTHER_VM` for an instance handed
to a script in another VM than its own, and `script.ERROR` for a throw
that carries no code, or an `Error` whose code was 0. The reserved codes
are the band from `INT32_MIN`, so no script's own code can mean one.

In that `else`, `#script_error()` answers the failure's text — what its
error sink was given: the script's `Error` as `error 16: …` with the lines
of its trace, or the runtime's sentence. It is a `string8` borrowed until
the `else` ends; `#format(#script_error())` keeps a copy. Anywhere else `#script_error()` is
refused, since there is no failure for it to mean.

A failure that leaves the main function ends the program with the code's status,
whose low eight bits the operating system keeps — and a code whose low
eight bits are 0, like `fail 256` or `script.ERROR`, ends it with 70, so
a failure never reports success.
`#script_run(g)` answers that same status for a failure that leaves the
instance's `ready`, rather than failing itself.

### A JavaScript library reaches Dado's exports

A JavaScript file a script imports may import the program's own `export`s
by the reserved name `dados:exports`:

    import { exports, memory } from "dados:exports";

    export function hit(n) {
        return exports === null ? 0 : exports.damage(n);
    }

On the web, `exports` holds each `export` of the Dado program as a
function of its wasm instance, under its C name, and `memory` the
instance's memory; both are filled before `main` runs, and nothing else
the module exports is in them. A page reaches the same module by that
name, which its import map holds. A native build has no instance, so
both are `null` there.

### When a script fails and no `try` is written

A method called through a handle without `try`, or a field read or
written through one, is not failable: when the script fails, its text
goes to the error sink, and what happens next is the VM's policy, set
when it is made. By default it is `script.QUIT`: the program ends with
the failure's status, as an unhandled failure in Dado does.
`#script_vm(on_error: script.CONTINUE)` makes it the other policy: the
failing call answers its type's zero value and the program goes on, as a
game keeps running past a broken mod. The names are `core:script`'s, so
a program that writes one imports it. `#script_new` is failable and
`#script_run` answers a status, so neither follows the policy.
`try … else code:` still catches a failure under either policy.

### Reloading a script while the program runs

`try #script_reload(vm, Goblin)` swaps new code in under a class the VM
has loaded, without stopping it. The new code is the module `dadoc
--emit=reload` writes for the class's file — `reload/<package>/<Class>.js`
under the working directory, or under the directory written third,
`#script_reload(vm, Goblin, dir)`. Before anything changes it is checked against
what this build bound: every Dado function the new code calls must be one
the build binds, with the same signature; every method the program's Dado
runs through a handle must still be there with its signature; every
field the program's Dado reads or writes through a handle, `g.hp`, must
still be there with its type; and every field the new code keeps must
keep its type. The class may be any the build carries, and the VM must
have made an instance of it, which loads it: a VM that has not is
refused. Any miss fails the reload,
the code running now stays, and `#script_error()` names every miss, one
per line — `entities.shake` has no binding in this build; rebuild to call
it from a script.

A reload that checks replaces every method of the class at once, under
every instance there is. Each instance keeps its fields' values; a field
the new code adds starts at its initializer when the instance first
touches it; a field it drops is no longer named; `init` does not run
again. Then each instance a handle holds runs its `reloaded()` method, if
the class has one. Called while the script is running — from a Dado
function the script called — the new code is checked at once and takes
over when that run returns.

A release build, `--release`, refuses `#script_reload` unless it is
built with `--reload` too.

### Loading a script by its path

A script may load another script that the program was not compiled with
— a mod — by its path, at run time: `any g = Script.load("mods/goblin",
self)` reads `mods/goblin.js` under the working directory, the mod's
JavaScript, which `dadoc --emit=mod` wrote, and constructs its class, its
init method given the arguments after the path. The mod runs in the same
engine as the script that loaded it, and stopping or deleting that
script's engine ends it too. Each load reads the file afresh.

A mod sees only what it is handed. It imports nothing but the runtime
every script imports: a mod that imports Dado — a `dado:` module, or a
package's script — or JavaScript fails at load, and the `Error` names the
import. So the API a mod has is the objects its loader hands its init
method; `dadoc --emit=script-api` writes the classes of a package a mod
is compiled against.

What `Script.load` answers is an `any`. A script calls it as it calls
any untyped value, converting each answer where it is used, or takes it
as a class of its own program — `Enemy e = g` — which checks the mod's
object at that line: every method of `Enemy` but init must be there with
as many parameters, and every field there too. From then on each
method's answer and each field read is checked as it arrives, as any
value leaving an `any` is. A load that fails — no file, a module that
does not compile, an import it may not have, an init method that throws
— raises an `Error` the script may catch.

### `Platform`, and the `when` a script folds

`Platform.os()` is the operating system the script was compiled for — for
a script a Dado program loads, its host's — as a string, and the members
of `Platform` are the strings it can be: `Platform.LINUX`,
`Platform.DARWIN`, `Platform.WINDOWS`, `Platform.FREEBSD`,
`Platform.OPENBSD`, `Platform.NETBSD`, `Platform.WEB` and
`Platform.UNKNOWN_OS`. Compare against a member rather than a string
written out: a misspelt member is refused, a misspelt string is only
unequal.

All of them are known when the script is compiled, so a script's `when`
selects on them as Dado's `when` selects on its compiler definitions:
`when Platform.os() == Platform.WEB:` keeps one arm and the others are
never checked, at the level of a method's statements or of the class's
members. A script's `when` folds literals, `Platform.os()`, the members
and the operators between them; a field, a local or a call is known only
when the script runs, and is refused naming `if`. The CPU architecture
is not asked in DadoScript — that question belongs to Dado.

### Generic methods

A method whose parameters write `$T` is generic, as a Dado function is:
`T first([]$T xs)` takes an array of anything and answers its element
type. Each call binds `T` from what it passes — `first(names)` binds
`string` — and the method is checked for that binding, so a mistake in
its body is reported for the call that made it. `where` limits what a
call may bind: `where T has ==` admits only types with `==`, which is
what `contains` and `index_of` over a `[]T` need, and `where T is int`
or `T is scalar` (an `int` or a `float`) names the types themselves.

Inside a generic method `when T is int:` selects by what `T` is bound
to, so each binding keeps its own arm and the others are never checked.
A generic method whose bindings all come out as the same JavaScript is
emitted once; one whose arms or printing differ by type is emitted once
per binding. A script's numbers are `int` and `float` only, so its
generics are over containers.

### A script calls Dado

A script calls a function of its package's Dado files by its bare name,
as it calls a method of its own class — a method of the class wins over a
Dado function of the same name — and a function of a Dado package it
imports through the import's name, as a Dado file does. It sees what a Dado
file of its package sees. The function takes and answers scalars — a
sized integer, `f32` or `f64`, `bool`, `char8`, `char16`, `char32` or
`string8` — and slices, arrays and tuples of them, and it may answer a new
string or array as `ref([]char8)` or `ref([]T)`. It may take or answer a
handle of a script class, `npc.Goblin`, which the script passes and
receives as its own instance. It may be failable.

The script passes its own values and the compiler converts them. An
int passed to a narrower integer wraps, as Dado's written conversion and
a literal that does not fit do, so 300 passed to a `u8` arrives as 44;
an int passed to an `i64` is exact and one passed to a `u64` goes by its
bits. A float passed to an
`f32` rounds, and an int passed to an `f32` or `f64` becomes the nearest
one. A float is never made an integer by itself: passing one to an
integer parameter is refused, naming `int(x)`. A string crosses as a copy,
read as UTF-8, and a `char32` takes a string of exactly one code point.
A `char8` is a byte, which a script holds as an int: it wraps into one as
a `u8` does. A `char16` takes a string of exactly one UTF-16 code unit —
JavaScript's own character — and any other string raises when the call
is made. What comes back is the script's own type: every integer and a
`char8` an int, `f32` and `f64` a float, a `char32`, a `char16` or a Dado
string a string.

Narrowing is never an error. Under `--strict` it is a warning
(`WRN1207`) wherever the argument is not a constant the parameter holds
exactly. A parameter or a result that cannot cross — a pointer, alone
or as an element, a `ref` other than a new string or array answered, a
map, a `distinct` type — makes the function one a script cannot call
(`ERR1206`), and the refusal names the part and what to write instead.

A failable function's failure is thrown into the script as an error whose
message names the function and the code. Nothing in the script has to
mark it: uncaught, it leaves the method, and the Dado program's
`#script_run` fails.

A Dado function may take a script's function. A parameter of a function
type, `void(i32) f`, takes a script's `void(int)` — a lambda or a
method — which Dado may call while the call runs and not after: one the
Dado function keeps, storing it or handing it to a function that does, is
refused (`ERR1220`). A `(proc, rawptr data)` pair whose procedure takes
its `data` first carries a script's function past the call: a `Sink`
takes a `void(string)`, so `core:fmt` writes into a script, and a pair
of the program's own, `(!void(rawptr, i64) proc, rawptr data)`, takes a
`void(int)`. Dado passes the function scalars as it answers them, and the
function answers a scalar or nothing. Each call enters the script's VM,
so one from another thread fails `script.THREAD`, and one through a
pair whose VM was deleted fails `script.STALE`; a failable procedure
answers the code. A Dado function the script's function calls sees the
`using` allocator in effect when Dado called it. When the script's
function fails during a Dado function the script called, the script's own
error reaches it as that Dado function returns.

### A script binds a handler to a channel

`receive(hits, on_hit)` binds a script's function to the channel it
names, one a Dado file of its package declares at package level — a
channel of an imported package is named through the import, as a Dado
function of one is. From then on the script is
the channel's one consumer: every message sent, from any thread, is
delivered to the function, one call a message, in the order they were
sent, on the thread the script's VM belongs to. The function takes the
message's fields as a Dado function's answer reaches a script — an integer
as an `int`, a float as a `float`, a `bool`, a character as a `string` —
and answers nothing: `#channel(16) hits(i32 n, f64 x)` takes a
`void(int, float)`, a method of that type or a lambda,
`void(int n, float x): …`.

A handler never runs in the middle of script code. Messages wait for a
safe point: a Dado function the script called returning to it, a call
into the script returning to Dado, and `#script_pump(vm)`, which a Dado
program writes where its frame ends. A handler runs to completion
before the next message's begins, and a safe point reached inside a
handler delivers nothing. A handler that fails has its failure written
to the error sink, naming the channel, and the next message goes on.

A channel has one consumer. A second VM's `receive` of it fails with
`script.BUSY`, as does a `receive` of a channel Dado walks with `#peek` or
`#drain`; a `receive` again in the same VM replaces the handler. A VM's
receivers end with it. A channel whose message carries a `ref` is never
bound: a script holds no Dado allocation, so the program drains it in
Dado and hands the script what it needs.

### Functions are values

A function type is written as Dado writes a function reference: the
return type, then the parameter types in brackets — `bool(int)`,
`void()`, `int(int, int)`. It is a type like any other, so a local, a
field, a parameter, an answer or an element of an array may hold a
function. A **lambda** makes one: a function declaration without its
name, `bool(int x): return x > 3`, whose value has the type its head
spells, `bool(int)`. Its body is one statement on the line of its `:`,
or, when the `:` ends the line, an indented block under it — at
statement level and inside brackets alike, where the body ends at the
`,` or the bracket that comes after it:

```text
[]int big = xs.filter(bool(int v):
    int w = v * 10
    return w > 25
)
```

A lambda reads and writes the locals, the parameters and the fields
around it **by reference**: a counter it adds to is the method's
counter, and a value the method changes after the lambda was made is
the value the lambda sees. A loop's variable is a new one each turn, so
lambdas made in three turns hold three different values.

A method is a value too: its bare name or `self.hit`, or `g.hit` for
another instance's, read without calling it, is a function bound to its
instance. A value of function type is called as a method is,
`f(3)`, and a call through one passes every argument — a default is no
part of a function's type. Calling a value that holds no function is
refused, naming its type. Two functions are never compared, since a
method read twice is two functions.

Where a `Location` is written — a local, a field, a parameter or an
answer declared `Location` — a method's name stands for where the method
begins, as `Location(hit)` does, and a lambda for where it is written.
Only there: an `any` holds no function and is given no place, and a
comparison converts nothing.

A script passes a function to a Dado function that takes one, which Dado
calls back (below, *A script calls Dado*); Dado hands a script no function
of its own, and a script's method that takes or answers one is not
called from Dado.

### Arrays and tuples cross the seam

A script's `[]T` is a JavaScript array, and so is its tuple. Both cross
the seam **as a copy, for the call**, element by element, and an element
crosses as it would alone: into a script's `[]int` a Dado program passes a
slice, a `ref([]T)` or an array of integers, and a Dado function a script
calls with a `[]int` may take it as a `[]i32`, each element wrapping as a
lone `int` would. An array, `[4]f32`, is a tuple whose slots share a type,
and so is a declared shape like `type Vec2: (f32 x, f32 y)`: each crosses
as a script's array, and a script's array passed into one must hold
exactly as many elements — any other length raises when the call is made,
naming the function, the parameter and both lengths. A tuple whose slots
differ crosses as a script's tuple of as many slots. A map does not cross.

The glue reads a script's array by each element's tag and never converts
one kind into another: an element of the wrong kind raises, naming the
function, the parameter and where the element is (`at [2][0]`). An array
answered to Dado is a create verb's: a `ref([]T)` on the ambient allocator,
its strings `ref([]char8)`, each Dado's to end with `#delete`.

A run or a construction that crosses a container is a run like any
other: it enters its engine before it builds a single argument, so on an
engine another thread is running, or one paused, it answers
`script.BUSY`, after `#script_stop` `script.STOPPED`, and through a handle
that holds no script `script.STALE`; its `out:` and `err:` are its own,
`#script_error()` reads its failure's text, and without `try` it follows
the script's `on_error:` policy.

**A structure lent across is locked for its call.** While a Dado function
holds a script's array, code it calls back into the script must not write
that array; and while a script runs with a Dado slice, a Dado function the
script calls must not write the slice. A write raises: in the script, an
error naming the function and the parameter; in Dado, the run fails with
the reserved code `script.LENT`. To write freely, pass a copy. What a Dado
function does to its own copy of a script's array is not a write of the
script's. Nothing is checked unless the call reached back across the
seam, so a call that does not pays nothing for the lock.

### A script names Dado's declarations

A script may write a Dado declaration of its package as a type, by its
bare name, and one of a package it imports through the import's name:
`entities.Vec2 p = entities.pos()`. The declaration is a view over the
type it crosses the seam as — a shape whose slots share a type, like
`type Vec2: (f32 x, f32 y)`, is the script's `[]float`, and a shape of
mixed slots its tuple — so the two are interchangeable: `[]float q = p`
checks, and so does passing the value where a `[]float` is taken. What the
name adds is the slots' names: `p.x` reads slot 0 and `p.y` slot 1,
exactly as `p[0]` and `p[1]` read them, so an array shorter than the
declaration raises there as an index does. `entities.Vec2(1.0, 2.0)`
makes one, an array or a tuple of its arguments, one per slot in order.
A slot of an array is written as its element is, `p.x = 3.0`; a tuple
is never written in place, so a slot of a shape held as a tuple is
refused (`ERR1208`) and the refusal names the construction that
replaces it. The names travel with what was written: a local, a
parameter or a field declared with the declaration's name, a Dado
function answering one, a construction, and a slot whose own type is a
declaration (`r.pos.x`). A declaration whose type does not cross is not
named (`ERR1208`).

### DadoScript as a JavaScript library: `export`

A package of scripts can be built as a **JavaScript library** — an ES-module
package that node, a browser page or a bundler imports with no Dado in it:

    dadoc --emit=js-package --package -o dist/ game/Goblin.dados
    ./dado build game --target=js

**`export` says what the package publishes.** `export class Goblin` — a
script's header, or a class block — publishes the class: its constructor
(`init`'s parameters), and every method and field that is not `private`,
inherited ones included. An `export` method that reads no `self` — no
field, no other method — is published as a function of the package too,
whether or not its class is:

    export class Dice

    int sides = 6

    void init(int n):
        sides = n

    int roll(int seed):
        return seed % sides + 1

    export int clamp(int x, int lo, int hi):
        if x < lo:
            return lo
        if x > hi:
            return hi
        return x

A JavaScript caller writes `new Dice(20).roll(7)` and `clamp(9, 0, 5)`.

**What crosses is what a TypeScript declaration can say.** The package
carries `index.d.ts`, written from the checked program: `int` is
`number | bigint`, `float` `number`, `bool` `boolean`, `string`
`string`, `[]T` `T[]`, `{K: V}` `Map<K, V>`, a tuple `readonly [A, B]`, an
enum the union of its members' names, `any` `unknown`, a function type
`(a: A) => R`, an answered `async T` `Promise<T>`, and an exported class
its own declaration. A published signature naming anything else — a trait
value, a class that is not exported — is refused at the type (`ERR1340`),
naming the repair.

**The boundary is checked.** Every argument a JavaScript caller passes is
checked as an `any` leaving into the parameter's type: a wrong one throws.
An `int` takes a `number` holding an integer or a `bigint`, and an `int`
answered past 2⁵³ − 1 arrives as a `bigint`. A failure — a wrong argument,
a `fail`, a fault — reaches JavaScript as a thrown `DadosError`, an `Error`
carrying `code`, `message` and `trace`. Calls between the scripts are not
checked again.

**A library has no Dado host**, so its scripts call no Dado function and load
no script by its path (`ERR1341`); `print` writes to the JavaScript host's
standard output, or its console.


## What is not here yet

This arc is being filled in as the compiler is marked up, feature by feature.
**What is unmarked is simply absent** — there is no placeholder, no stub and no
"to be documented", because a heading with nothing under it is a promise and a
promise in a document is the thing that goes stale first.

The order it is being filled in is roughly the order above: the axioms first,
then the types that follow from them, then procedures, control flow, failure,
memory, compile-time and the C boundary.

A coverage floor records how many of the compiler's own spellings are named by
some block. It may rise and it may not fall.

