//@ types/distinct 45
//@ `type A: B` over a `foreign` type is `B`, as it is over any `type`:
//@ a transparent `B`'s member names are `A`'s, a function taking `B`
//@ takes an `A`, and `distinct type D: B` keeps the names under a tag of
//@ its own. What `B` refuses — an `@incomplete` type by value, a
//@ `cunion`'s `switch`, a flexible tail's length — `A` refuses too.
