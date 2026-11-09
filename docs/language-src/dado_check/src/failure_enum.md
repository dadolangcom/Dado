//@ procs/failure 10
//@ ### A failure is a code, and never a value
//@
//@ A function whose return type carries `!` can fail: `!i32 measure(string8 s):`
//@ ends either by `return`, which succeeds, or by `fail` with a nonzero `i32`,
//@ which does not. A failure is never stored — no value's type holds one — so a
//@ failable call is made only by `try`, at the start of a statement. With
//@ nothing after it, `try` passes the failure on to the function it is written
//@ in, which must be failable too, and `return` of the call passes it on whole.
//@ `else:` handles it where it happens, and `else code:` binds its code for the
//@ suite.
//@
//@ **`E!T` says which codes.** With an enum in front of the mark,
//@ `Why!i32 measure(string8 s):` fails only with a member of `Why` — an `enum`
//@ or a `distinct enum` backed by `i32`. Every `fail` in it fails with a
//@ `Why`, and every failure it passes on comes from a call failing with the
//@ same enum, so the code a caller binds is a `Why` and one of its members.
//@ That makes it the second thing a `switch` may be exhaustive over: name
//@ every member, and the `else` may be left out. The binding is read-only, so
//@ it stays a member. A member equal to 0 may be declared — an enum that also
//@ names success, `OK = 0` — but is never a failure: no `fail` may use it, no
//@ caller binds it, and a closed `switch` need not name it.
//@
//@ **A constant is judged where it is written; a value is trusted, and
//@ checked.** `fail 7`, `fail Other.A` and `fail Why(9)` are refused: each is
//@ proven not to be a member. A value of `Why` that does not fold — a kept
//@ failure, `fail self.failed`, or a function translating another package's
//@ codes, `fail mine(why)` — is accepted. A `distinct enum` converts from any
//@ `i32`, so such a value need not be a member, but nothing proves it is not,
//@ and the compiler refuses only what it can prove. A code that is 0 at run
//@ time stops the program where it fails, in every build, as any computed
//@ code does; a sanitized build (a dev build, or `--sanitize`) also stops it
//@ where it fails with a code that is no member, rather than letting a
//@ caller's closed `switch` route it into an arm that does not name it.
//@
//@ ```dado
//@ package doc_sample
//@
//@ distinct enum i32 Why: (EMPTY = 1, TOO_LONG)
//@
//@ public Why!i32 measure(string8 s):
//@     if #len(s) == 0:
//@         fail Why.EMPTY
//@     if #len(s) > 8:
//@         fail Why.TOO_LONG
//@     return i32(#len(s))
//@
//@ // Every member is named, so no `else` is written and none is missing.
//@ public i32 size(string8 s):
//@     try i32 n = measure(s) else why:
//@         switch why:
//@             case Why.EMPTY:
//@                 return 0
//@             case Why.TOO_LONG:
//@                 return -1
//@     return n
//@ ```
//@
//@ A plain `!` function passes such a failure on as its `i32` code. An `E!`
//@ function passes on only its own enum's: a failure of another enum, or a plain
//@ one, is handled where it arrives and failed with a member. A `foreign`
//@ function names no enum, because what C answers is not checked here. Across
//@ every boundary the code is the same `i32` either way.
