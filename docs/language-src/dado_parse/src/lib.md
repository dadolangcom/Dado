//@ names/at 30
//@ ### What earns a place in the set
//@
//@ A member is added when a property of the emitted C cannot be stated in
//@ Dado's own types and the C compiler has to be told it directly — and when
//@ its absence is a **defect** rather than a missed optimisation. That is
//@ the bar `@symbol` was admitted under: a function reporting its own frame
//@ address is wrong if it is inlined, and no Dado type says so.
//@
//@ A convenience, a hint, or a switch a program could have written as
//@ ordinary code earns nothing. `@align` is a member because C11's
//@ alignment specifier stands on declarations Dado has no other way to
//@ describe. `@incomplete` is one because Dado never reads the header, so it
//@ cannot derive whether a bodiless `foreign` `type` is one C can size; the
//@ fact has to be declared, and a declared fact needs a spelling.
//@
//@ **None of these words is reserved.** The `@` makes the position
//@ unambiguous by itself, so a declaration spelled `i32 align = 8` and a
//@ call spelled `align(x)` both stay ordinary code, and the whole set costs
//@ the language not one identifier. That is what the sigil is for, and it
//@ is why growing the set by one is cheap for every program and expensive
//@ for the compiler — which is the direction this trade was deliberately
//@ taken in.

//@ names/at 20
//@ ### A `@c` position is not a family member
//@
//@ `@c("…")` is one member of the set, and it is written in four places: a
//@ `foreign` slot's, between its type and its name; a `foreign` `type`'s
//@ own, after the keyword; a `foreign` enumerator's, before its name; and a
//@ `foreign` function's, between its return type and its name.
//@
//@ Those four are **positions**, and positions are not members. The family
//@ is listed once, where the refusal is raised, and each acceptor reads the
//@ one spelling out of that list. The separation is what lets a fifth place
//@ be opened without opening the set: adding a position is an acceptor,
//@ adding a member is a registration, and only the second widens what a
//@ program may write. The count in the refusal stays ten either way.
//@
//@ Every `@` an acceptor does not claim is handed **back**, unconsumed, to
//@ the arm that owns the list. So `@foo` in a slot, and a `@c` written in a
//@ native `type` body where no C name belongs, both arrive at the one
//@ message that names the ten. One `@` is answered once, and no position
//@ grows a second opinion about the family.
//@
//@ What each position *admits* still differs, because C spells a member and
//@ a type differently: an enumerator or a slot takes a plain C identifier,
//@ where a type may also take a tag word and a name. A spelling C could not
//@ have given is `ERR1014`, reported at the string itself, since the
//@ alternative is a C error on a line nobody typed.
