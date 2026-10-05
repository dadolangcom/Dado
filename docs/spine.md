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

<!--@ names/hash -->

The second is the set of attributes, and the thing to notice is that it is
**registered** rather than open. An open attribute namespace is the shape that
grows into a macro system, one convenience at a time, and each addition is
individually reasonable.

<!--@ names/at -->

The third, `$`, binds a generic parameter. It can only ever *bind* — it never
begins a name — which is what lets the parser tell a binder from a literal
without lookahead.

## Everything is a tuple

This is the axiom, and almost every other decision in the type system is a
consequence of it rather than a separate choice. Read it before reading about
any particular type, because otherwise each one looks like a special case and
none of them are.

<!--@ types/tuples -->

The immediate consequence is about **names**, and it is the one that surprises
people: a slot's name is a view over a shape rather than part of the shape's
identity.

<!--@ types/views -->

That buys a great deal and it costs something specific, which is said out loud
where it is described rather than discovered later.

## Where nominality lives

If names are views and shapes are structural, then two declarations that happen
to agree are the same type — and sometimes that is exactly wrong. Money and
metres are both a number and adding one to the other is a bug.

So there is one construct whose entire job is to add an identity, and one that
deliberately does not.

<!--@ types/distinct -->

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

<!--@ generics/where -->

A declaration can take a binder too, and then it is a family of types rather
than one: each argument written after its name is an instantiation, made where
it is written and shared by every later writing of it.

<!--@ generics/types -->

## Methods and traits

A trait is that other construct. Behaviour is keyed on a **declaration**, never
on a shape: a method is written in a `type`'s body and called statically, and a
trait prescribes methods and never a shape, taken on by name with `using`. So
two declarations over one shape still share every function written against the
shape, and differ only in what their own bodies say. The grammar comes first —
what a declaration head may say after its name, where a method is written, and
what its first parameter is — and the checking follows it.

<!--@ types/traits -->

<!--@ types/methods -->

## A procedure is a tuple in and a tuple out

Nothing in this section is a feature. Everything in it is the axiom arriving
somewhere else — which is the test of an axiom, and the reason this section
comes after the type system rather than beside it.

A parameter list is a tuple. A return is a tuple. So multiple returns are not a
feature that had to be built, named arguments are not a second way to call
something, and *how many values may a function return* is a question that was
already answered.

<!--@ procs/returns -->

A procedure that can fail says so in its return type, and what it fails with is
a code rather than a value — which is the one thing about failure that every
other rule follows from.

<!--@ procs/failure -->

Visibility is the one thing here that is **not** downstream of the type system,
and it is worth saying so out loud, because the two look alike and are not.

<!--@ procs/visibility -->

If the parameter list is a view, then naming an argument is naming a slot —
same operation, no new machinery, and the rules that follow are about
ambiguity rather than about types.

<!--@ procs/arguments -->

A default looks like part of a signature and is not, which is the one thing
about it worth knowing before it surprises somebody.

<!--@ procs/defaults -->

The last piece is the one that genuinely is a convention rather than a type,
and it is spelled to look like one so that nobody reaches for it in a place a
type is wanted.

<!--@ procs/packs -->

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

<!--@ flow/conditions -->

The loop is where the refusal is sharpest, and where the axiom shows up for the
last time in this arc.

<!--@ flow/iteration -->

`switch` is where a reader's expectations from other languages are most likely
to be wrong, and the surprise is in the *opposite* direction from the usual
one: the construct people expect to be exhaustive is not, and the one they do
not expect to be is.

<!--@ flow/switch -->

Two constructs are left, and neither is about choosing between values. One
chooses between *programs*, before the program exists.

<!--@ flow/when -->

The other is about leaving — and about making the leaving say so at the point
the thing was acquired, rather than at each of the four exits.

<!--@ flow/defer -->

## Signals: handing a value to another thread

A frame loop, a worker pool, an input backend and a network thread all need the
same thing — hand a value to another thread without a lock and without losing
it — and a program that answers it four times answers it four ways. A signal
answers it once, typed, with what may travel checked, and with one layout the
other side of any boundary can read.

<!--@ flow/signals -->

## Threads on the web

A web build is one thread unless it asks for more, because a thread on the web
is a worker sharing the module's memory, and a page gets a shared memory only
when it is served for one.

<!--@ web/threads -->

## The page

A web build writes the page that runs it in a browser, from a template the
author owns: ordinary HTML with comments the build fills in.

<!--@ web/page -->

## The manifest

What a build publishes — Dado's exports, the shapes they reach, what the scripts
export — is one JSON document the compiler writes, and every other format a
caller wants is a separate program that reads it.

<!--@ build/manifest -->

## Shaders: DadoGL, reserved

DadoGL is the shader language a later increment builds. Its spellings are
reserved now, so a program and a graphics backend written today do not change
when it lands.

<!--@ gpu/reserved -->

## DadoScript: one tree, two dialects

DadoScript is the scripting dialect a Dado host loads. It shares Dado's front end, so
the two parse into one tree, and which dialect a piece of code is written in is
decided by the declaration around it, never by a comment or a flag.

<!--@ toys/dialect -->

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
