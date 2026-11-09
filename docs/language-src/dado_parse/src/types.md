//@ names/at 10
//@ ### `@` is a registered set, not an open one
//@
//@ There are ten `@` attributes — `@const`, `@test`, `@c`, `@incomplete`,
//@ `@align`, `@symbol`, `@macro`, `@c_body`, `@freestanding` and
//@ `@thread_local` — and a program has no way to write an eleventh. An
//@ unknown `@name` is not an extension point: it is `ERR0418`, refused by
//@ name, with the ten listed back at whoever wrote it.
//@
//@ **An open namespace was available and was not wanted.** A mechanism a
//@ program may extend has to decide what an unrecognised attribute means,
//@ and both answers are bad. Ignore it and a misspelling is silent, so a
//@ program that asked for something and got nothing looks exactly like one
//@ that asked for nothing. Honour it and the compiler has agreed to
//@ something nobody implemented, which is worse, because it will be found
//@ by the C compiler or by nobody.
//@
//@ The second reason is narrower and sharper: an open `@` is the door
//@ arbitrary C walks in through. That is why `@symbol` — the one member
//@ that is itself a mechanism — takes a *registered word* and not text. It
//@ is a second registry one level in rather than a hole in this one, so a
//@ further symbol property is still a deliberate act, just a cheaper one.
//@
//@ The cost is real and is the one chosen: a new attribute is an edit to
//@ the compiler, so nobody can add one from a library. In exchange the set
//@ is enumerated in exactly one place — the table this arm reads — which
//@ is what lets the refusal name the whole family, and an editor describe
//@ each member, with no second list to fall out of step with it.
