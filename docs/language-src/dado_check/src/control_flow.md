//@ flow/conditions 10
//@ ### A condition is a `bool` and nothing else is read as one
//@
//@ C takes any scalar and calls zero false. This does not, and the reason
//@ is the mistake it prevents: an `if` written over an integer is how a
//@ comparison against zero gets written by accident, and the two read
//@ identically on the page.
//@
//@ One rule, one place: the same check answers for `if` and for the
//@ conditional `for`, so the two can never drift apart.
//@
//@ `if` is a statement and produces no value; a conditional **value** is
//@ `a if c else b`, which is an expression and not a statement `if`.
//@
//@ An `else if` chain is flat rather than nested, and an `else` binds to the
//@ `if` at its own indentation — layout is the block delimiter, so the
//@ dangling `else` the C-family grammars argue about cannot be written.

//@ flow/switch 31
//@ **The `else` stays, and it is not where a member goes.** A `switch`
//@ whose arms name members of one `enum` is told about every declared
//@ member no arm names — a warning while writing and an error under
//@ `--strict`, as an unread name is — so adding a member shows every
//@ `switch` it reaches instead of routing it silently into the `else`.
//@ The `else` is for a value that is no member at all — so over a failure
//@ bound from an `E!` call, which is always a member, it may be left out
//@ once every member is named, and one left out is then an error that
//@ names it. Where a member
//@ wants what the `else` does, name it in an arm that says so; one that
//@ wants what another arm does goes in that arm's list, `case a, b:`, and
//@ shares its body. Code that tests a
//@ few members and treats every other alike is an `if`, which claims
//@ nothing about the rest.

//@ flow/switch 32
//@ A `case` arm ends where its body ends, and `fallthrough` is how one
//@ continues — so **`break` is refused inside an arm**, `else` included.
//@ In C, C++, Java, JavaScript, C# and Go a `break` there leaves the
//@ `switch`; here it would leave the loop around it, the same text with a
//@ different meaning. A `break` meant for the loop goes after the `switch`,
//@ or the arm returns. **`continue` inside an arm is allowed**, and means
//@ what it means in C and Go: the next iteration of the enclosing loop.

//@ flow/switch 30
//@ ### Exhaustive over a union or a bound failure, never over an `enum` value
//@
//@ A `switch` compares a scrutinee against arms. The scrutinee is evaluated
//@ once, arm values are constants, and an arm an earlier arm already covers
//@ is refused rather than left unreachable.
//@
//@ The rule worth knowing before it surprises somebody: **a `switch` over an
//@ `enum` always needs an `else`, even when every member is named.** An
//@ enum's members are the values somebody wrote down, not the values the
//@ type can hold — the backing is an integer and a value need not be one of
//@ the members — so promising exhaustiveness there would be promising
//@ something the type does not support.
//@
//@ **A union discriminant is the one thing in this language closed by
//@ construction**, and it is the one place the `else` may be dropped: when
//@ the arms name every member there is nothing left for an `else` to catch.
//@ An arm that binds a name gets that member's type inside its body, which
//@ is narrowing with no flow analysis anywhere — the type comes from the arm
//@ rather than from a deduction about what ran before it.
//@
//@ **The failure an `E!` call answered is the other**, bound by its
//@ `else` as an `E`: every code such a function fails with is a member —
//@ a constant checked to be one where it is written, a value of `E` checked
//@ where it fails in a sanitized build — and the binding is read-only, so a
//@ `switch` over it that names every member needs no `else` either, and one
//@ that leaves a member out is refused naming it.
//@
//@ ```dado
//@ package doc_sample
//@
//@ union Shape: (i32, f64)
//@
//@ // Every member is named, so no `else` is written and none is missing.
//@ public i32 size(Shape s):
//@     switch s:
//@         case i32 n:
//@             return n
//@         case f64 f:
//@             return i32(f)
//@ ```
//@
//@ `fallthrough` hands control to the next arm and is legal only as the last
//@ statement of one, in a value `switch` that has a next arm. It is refused
//@ in a union `switch`, where each arm binds a different narrowed type and
//@ there is no shared body for control to arrive in.
//@
//@ An arm may name several values: `case a, b, c:` is **one** arm. It
//@ matches when the scrutinee is any of them, opens one scope over one
//@ body, and takes part in the first-match rule value by value — a value an
//@ earlier arm, or an earlier place in the same list, already names is
//@ refused as a whole arm would be. A list is written over a value `switch`
//@ and not over a union, where an arm narrows the value to one member's
//@ type and one body cannot see it as two.
