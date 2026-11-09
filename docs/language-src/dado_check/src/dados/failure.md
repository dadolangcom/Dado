//@ toys/dialect 28
//@ ### A script's failures
//@
//@ Any method of a script may fail, and nothing in its signature says so: a
//@ call that fails passes the failure on to its caller unless it is handled.
//@ A failure is an `Error` — a `message`, a `code` and a `location`, and a
//@ `trace` the runtime attaches where it is raised, innermost first — made
//@ as `Error("no such item")`, `Error("no such item", 16)` or
//@ `Error("no such item", 16, Location(pick))`. The code is 70 when none is
//@ given, and is never 0 (`ERR0715`). `print` writes one as `error 16: no
//@ such item`.
//@
//@ `fail` takes three forms: `fail "no such item"` is
//@ `Error("no such item")`, `fail 16` is `Error("failed with code 16", 16)`,
//@ and `fail e` raises an `Error` already made. `try` handles a failure as
//@ Dado's `try` does, over one call or one index, and its `else` binds the
//@ `Error`: `try int n = parse(text) else e:`, then the suite, which leaves
//@ the scope when the `try` declares its name. A bare `try` is allowed and
//@ passes the failure on, as an untried call does. `defer` runs its statement
//@ at every exit of its scope, in reverse order, and `defer fail` only when a
//@ failure is leaving it.
//@
//@ Faults are ordinary failures: an index past either end, a missing key, an
//@ integer division by zero and an `assert` that does not hold each raise an
//@ `Error` of code 70 in Dado's words, which a `try` catches. `assert cond,
//@ "why"` raises `assertion failed: why`.
//@
//@ A `Location` is a place in the source: `Location()` is where it is
//@ written, `Location(pick)` where the method it names begins, and
//@ `Location(pick, 2)` two lines into it — the method must be the class's,
//@ and the line inside its body, both checked when the script is compiled.
//@ As a parameter's default, `Location at = Location()` is the caller's line,
//@ because a default is filled where the call is written. Its `file`,
//@ `line`, `column` and `function` are read like fields, and it prints as
//@ `file:line:column`, or `<unknown>` for the zero value.
