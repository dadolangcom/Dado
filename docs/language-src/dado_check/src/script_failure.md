//@ toys/dialect 33
//@ ### When a script fails
//@
//@ A script's failure reaches Dado as a code. Under `try … else code:`, the
//@ code is the script's own when the script failed — `fail 16` is 16, a
//@ fault 70 — and otherwise one of the runtime's reserved codes, which a
//@ program names after `import "core:script"`: `script.STALE` for a handle
//@ that reaches no script (deleted, or its VM deleted), `script.TYPE` for an
//@ answer of another type than the method returns, `script.NO_MEMORY` when
//@ the allocator refused or a table is full, `script.STOPPED` and
//@ `script.BUSY` for a stopped or busy VM, `script.THREAD` for a call from a
//@ thread that did not make the VM, `script.OTHER_VM` for an instance handed
//@ to a script in another VM than its own, and `script.ERROR` for a throw
//@ that carries no code, or an `Error` whose code was 0. The reserved codes
//@ are the band from `INT32_MIN`, so no script's own code can mean one.
//@
//@ In that `else`, `#script_error()` answers the failure's text — what its
//@ error sink was given: the script's `Error` as `error 16: …` with the lines
//@ of its trace, or the runtime's sentence. It is a `string8` borrowed until
//@ the `else` ends; `#format(#script_error())` keeps a copy. Anywhere else `#script_error()` is
//@ refused, since there is no failure for it to mean.
//@
//@ A failure that leaves the main function ends the program with the code's status,
//@ whose low eight bits the operating system keeps — and a code whose low
//@ eight bits are 0, like `fail 256` or `script.ERROR`, ends it with 70, so
//@ a failure never reports success.
//@ `#script_run(g)` answers that same status for a failure that leaves the
//@ instance's `ready`, rather than failing itself.
