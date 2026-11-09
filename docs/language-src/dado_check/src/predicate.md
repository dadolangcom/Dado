//@ generics/where 20
//@ ### The vocabulary is closed
//@
//@ A predicate is written out of a fixed list, and nothing outside it is
//@ admissible.
//@
//@ **Leaves.** A name the enclosing signature binds, a type spelling, or an
//@ integer literal.
//@
//@ **Operators.** `==`, `!=` and `is` between types; `in` from a type to a
//@ parenthesised list; the six comparisons and the five arithmetic
//@ operators between integers; `&&`, `||` and `!` between the answers.
//@ Nothing else. There is no order on types, so `<` between two of them is
//@ not a false claim but no claim at all, and `is` and `in` both reduce to
//@ the identity `==` already asks.
//@
//@ **`has`.** `T has +, <` holds when every operator listed applies, a
//@ binary one as `T op T`, answered by the same code the operator itself is
//@ decided by — so `has ==` and `#has_eq` cannot give two answers. Written
//@ with a type name in place of an operator, it holds when `T`'s
//@ declaration embeds that type, directly or transitively; it is asked of
//@ the declaration and never of the layout, so a binding that arrived as a
//@ shape written out carries no declaration and is refused rather than
//@ guessed at.
//@
//@ **Predicate words.** One closed roster, spelled with the sigil and
//@ resolved through the same table every other compiler-owned name is. No
//@ second namespace.
//@
//@ The narrowness is the whole design. A wider vocabulary would be a second
//@ constant folder — one walking scopes, raising its own diagnostics and
//@ refusing a binding outright — where this is a walk over about a dozen
//@ node shapes. The cost is paid honestly: a condition the list cannot
//@ express has to arrive as a predicate word or not at all.

//@ generics/where 10
//@ ### `where` constrains and never implements
//@
//@ A header `where` is **one** comptime predicate, written on a signature
//@ after the parameter list and before the `:`, stating the domain a
//@ template is for:
//@
//@ ```dado
//@ public i32 index_of([]$T xs, T probe) where #has_eq(T):
//@     i32 i = 0
//@     for x in xs:
//@         if x == probe:
//@             return i
//@         i += 1
//@     return 0 - 1
//@ ```
//@
//@ It is a gate and nothing more. It selects no body, carries none, names
//@ no implementation and supplies none: a type satisfying the clause is
//@ admitted to the one body the template has, and a type that does not is
//@ refused at the call site (`ERR0919`). There is no spelling of `where`
//@ that means *and here is what to do instead*.
//@
//@ **The selection half was designed and is not built.** A chain of `where`
//@ arms standing in place of a body would want a *list* of predicates
//@ walked by this same evaluator, so it will inherit the one that exists
//@ rather than half of a second one. Until then a signature carries one
//@ predicate and no arms at all, because a field nothing reads cannot be
//@ tested.
//@
//@ One question about a clause needs no call to answer: whether it names
//@ anything the signature binds. A predicate naming no binding is a
//@ constant — true for every call or false for every call — and neither is
//@ a domain, so it is refused once at the declaration (`ERR0920`) instead
//@ of once per call.

//@ generics/where 30
//@ ### Asked at the instantiation, after the memoisation hit
//@
//@ The predicate is evaluated where the answer is knowable: at the
//@ instantiation, of what a call actually bound. The **call** is what a
//@ refusal underlines, never the template's own line — that line is correct
//@ for every other caller, so blaming it would blame the wrong text.
//@
//@ It is asked at each *new* instantiation, from the point **after** the
//@ memoisation hit. A repeat call binding the same types never re-asks it.
//@ That is not an optimisation: it keeps the number of evaluations
//@ proportional to how many distinct shapes a template is used at and
//@ independent of how often it is called, so no predicate becomes a
//@ compile-time cost by sitting in a hot loop.
//@
//@ The price is the bargain every uncalled template already strikes: a
//@ clause on a template nothing calls is checked only for whether it names
//@ a binding. Nothing evaluates it, exactly as nothing checks the body.
//@
//@ ### `#has_eq` is not `#is_key`
//@
//@ `#has_eq`, asked of a binding, is whether `==` applies to it — decided
//@ by the same code that decides the operator, so the word and the
//@ operator give one answer and cannot drift apart.
//@
//@ `#is_key`, asked of the same binding, is whether it may key a map,
//@ which is two further questions: whether `==` is *stable*, and whether
//@ Dado can lay the key out. It is strictly the narrower of the two. `f32`
//@ and `f64` have `==` and are not keys; `#c.int` and its fellow C types
//@ have `==` and are not keys either.
//@
//@ They stay two words because they are two rules, and the narrower one is
//@ asked wherever a type reaches key position anyway — so a binding that a
//@ `where` clause admitted on `#has_eq` is still refused, at that same
//@ instantiation, the moment it is used as a key.
