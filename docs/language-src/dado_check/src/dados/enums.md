//@ toys/dialect 28
//@ ### Enums and `switch`
//@
//@ An enum is written in a class, as a trait is — `enum Mood: (Calm, Angry)`
//@ in `Goblin` is `Goblin.Mood` in the package and `Mood` inside `Goblin` —
//@ and its members are names only, with no backing type and no values. It is
//@ its own type, and a value of it is always one of its members, written
//@ `Mood.Calm`. Two compare with `==` and `!=`, an enum may key a map, and a
//@ member prints as its name. A field or local of an enum written without a
//@ value holds the first member.
//@
//@ `switch` takes an enum, an `int`, a `string` or a union — a union read
//@ by Dado's narrowing arms (see *Types and unions*). Each `case` names members
//@ or constants, `case Mood.Calm, Mood.Sleepy:` being one arm over both, and
//@ an arm ends where its body ends, so `break` is refused in one. Over an
//@ `int` or a `string` the `else` arm is required. Over an enum the script
//@ declares it is not, since every value is a member: a `switch` naming
//@ every member covers them all. A Dado enum the script names is open, as
//@ Dado holds it — a value of one need not be a member — so a `switch`
//@ over one needs `else`, as Dado's does (see *Enums, `distinct` types and
//@ unions cross the seam*). A member no arm names is reported as a warning
//@ — an error under `--strict` — so a member added later shows every
//@ `switch` it reaches.
