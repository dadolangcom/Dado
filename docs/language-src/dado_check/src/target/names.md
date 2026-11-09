//@ names/hash 10
//@ ### The `#` sigil splits by case, and the split is one-way
//@
//@ One sigil, two populations, told apart by the first ASCII letter after
//@ the `#` and by nothing else at all.
//@
//@ An upper-case name — `#OS`, `#ARCH`, `#TRIPLE`, `#X86_64` — is
//@ compile-time-known, and that guarantee has two visible edges. It is
//@ eligible in a `when`, and it is refused as a `switch` scrutinee
//@ (`ERR0708`), because a `switch` compares at run time whatever it selects
//@ on, and selecting on something the compiler already knew is a mistake
//@ worth naming rather than code worth emitting. The promise is about
//@ *when* an answer is known, not about which construct may ask for it:
//@ `#LEN` is upper-case, and a `when` folds before any type exists, so
//@ `#LEN` has nothing to answer a `when` with. What it answers everywhere
//@ else is a number with no type of its own, which takes the type its
//@ position wants exactly as an integer literal does.
//@
//@ A lower-case name — `#default`, `#soa`, `#no_bounds` — says only that
//@ the compiler owns the spelling. It promises nothing about compile time,
//@ which is exactly why a lower-case name in a `when` condition is
//@ `ERR0917`.
//@
//@ **The promise runs one way.** Upper case carries it; lower case carries
//@ none, and nothing is carved out for a layout tag, an allocation verb or
//@ a run-time name. An exception admitted on day one is an exception
//@ forever, and what refusing one buys is a rule a reader holds in one go.
//@
//@ The **first** letter decides, so `#UNKNOWN_OS` and `#X86_64` are not
//@ questions about underscores and digits, and a name carrying no letter
//@ falls to the lower half — the upper half is the half that makes a claim.
//@
//@ Neither half is overridable by a build define. `-D` replaces the
//@ initializer of a declaration a program wrote, and no program writes a
//@ `#` name. The override is `--target=`, and it changes the *answer* the
//@ toolchain gave rather than the name: cross-compiling moves what `#OS`
//@ says without moving what `#OS` is.

//@ names/hash 20
//@ ### A name the compiler owns is read in one place
//@
//@ The lower half of the sigil is not a namespace a program can reach into.
//@ Every such name is registered with the one grammatical position it may
//@ be written in, and a position is a claim about **where the parser reads
//@ the name**, not about what the name means.
//@
//@ So `#default` is written where a value goes; `#len` is written with its
//@ arguments; `#soa` is written in front of the type it qualifies;
//@ `#no_bounds` is written in front of the operand whose check it drops;
//@ `#any` is written after a parameter's `..`, where the element type goes.
//@ Each of those is a different production, and a name registered for one
//@ is refused by name in the others. `#soa` where a value goes resolves to
//@ nothing whatever — and that is the position's meaning rather than a gap
//@ in it.
//@
//@ The cost is stated: two shapes a reader would call "the same kind of
//@ name" need two registrations, and a name written somewhere no `#` name
//@ could be written before costs a grammar production as well as a row.
//@ What is bought is that no name can be accepted in a place nobody agreed
//@ to, which is what an open position plus a lookup that decides afterwards
//@ gives away.
//@
//@ **There is no second resolution path.** The `where` predicate words —
//@ `#has_eq`, `#is_key`, `#is_declared` and the rest — are ordinary rows of
//@ the same roster with a position of their own, not a separate vocabulary
//@ some second lookup would have to know about.

//@ names/hash 30
//@ ### One roster, keyed by spelling
//@
//@ Every name the compiler owns resolves through a single table, and that
//@ table is keyed by spelling and says nothing about case. `#LEN` sits on
//@ it beside `#len`; whoever wants one population asks the case rule
//@ separately. The count is the table's length and is written in no
//@ sentence anywhere, here included, because a sentence with a number in it
//@ is a thing that goes stale quietly.
//@
//@ Adding a name is three steps and there is no fourth: say what it means,
//@ add the row carrying its spelling and its position, then wire the arm
//@ the compiler now refuses to build without. The middle step is what makes
//@ the third unskippable — a row that introduces a name stops the build
//@ until something says what the name does — so there is no way to register
//@ a spelling and forget to implement it.
//@
//@ **A sigil is a promise that a name stays the compiler's**, which is why
//@ names on their way *out* of the compiler must never be sigilled on the
//@ way. The two printing verbs are absent from this table for that reason:
//@ `core:fmt` declares them, and a row here would have claimed them for the
//@ compiler permanently. A positional statement word is absent for the
//@ other reason — no name ever reaches the lookup.
