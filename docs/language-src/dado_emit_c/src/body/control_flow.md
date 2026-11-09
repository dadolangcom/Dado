//@ flow/defer 50
//@ ### `defer` runs at scope exit, not at function exit
//@
//@ A deferred statement runs when the block it was written in ends — so one
//@ inside a loop body runs at the end of **each** iteration, and every
//@ iteration starts from the same state the caller's did. Within a scope
//@ they run in reverse order of registration, which is the order that makes
//@ a pair of acquisitions unwind correctly without anybody thinking about
//@ it.
//@
//@ There are four exits and all four unwind: the end of the block, a
//@ `return`, a `break` and a `continue`. A `return` unwinds every frame up
//@ to the function; a `break` or `continue` unwinds up to the loop. What a
//@ deferred statement may **not** be is anything that leaves the scope it is
//@ unwinding — a `return`, a `break`, a `continue`, another `defer`.
//@
//@ An exit that leaves several scopes leaves them **one at a time**,
//@ innermost first: each scope's deferred statements, then the allocator
//@ and the script sinks its `using` displaced put back, then the next
//@ scope out. So a deferred statement runs under the allocator and sinks
//@ that were in force where it was written, whichever exit reaches it — a
//@ `defer` at the top of a function is not served by a `using` suite a
//@ `return` happens to leave from.
//@
//@ `quit` is the exception and it is preservation rather than a decision: it
//@ ends the process, and pending deferred statements do not run. Covering
//@ that would be a language change rather than a fix.
//@
//@ A deferred statement may be marked to run only when a **failure** leaves
//@ the scope. That is one stack with the ordinary ones, in the same reverse
//@ order, and every non-failing exit skips it — which is how a release runs
//@ on the way out of a function that gave up, without the success path
//@ paying for a flag.
//@
//@ A cleanup that can itself fail — a flush, a close, a commit — is
//@ deferred with `defer!`: `defer! flush(h)` holds one failable call made
//@ alone, runs at every exit like a plain `defer`, and passes its failure
//@ on. If it fails while the scope is otherwise succeeding, the function
//@ now fails with its code, running the rest of the unwinding as a
//@ failure; if a failure is already leaving, the call still runs and the
//@ first code is the one kept. It sits on the same stack as the others, so
//@ a `defer fail` registered *before* it runs after it and sees its
//@ failure. It stands only in a failable function (`ERR0714`); a plain
//@ `defer` of a failable call is `ERR0611` and a deferred `try` is
//@ `ERR0707`, and both name `defer!` as the repair. The success path pays
//@ one `int32_t` slot in the function's frame, tested where each exit ends.
