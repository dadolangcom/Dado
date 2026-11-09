//@ toys/dialect 32
//@ ### Where a script's output and failures go
//@
//@ A script's `print` and `println` write to its VM's output sink, and the
//@ text of a failure — the script's `Error` and its trace, or the runtime's
//@ sentence — goes to its error sink. Both are `Sink` values, the two-word
//@ shape `#render` writes into, and both are named, after the allocator:
//@ `#script_vm(out: log, err: problems)` sets them for the VM's life. Left
//@ out, they are standard output and standard error, the streams
//@ `fmt.println` writes to, so a script's lines and the program's appear in
//@ the order they ran. `fmt.to_bytes(&cursor)` is a sink that captures into
//@ memory the program owns.
//@
//@ **For one call or one stretch of code, scope them with `using`**, beside
//@ the allocator. After a call into a script — `c.greet(x) using out: log`,
//@ `#script_run(c) using out: log, err: problems`, `#script_new(vm, Goblin)
//@ using err: problems` — they hold for that call. As a suite, `using out:
//@ log:` over indented statements, they hold for every script call inside:
//@ member calls, field reads and writes, `#script_run`, `#script_new`, and a
//@ script call inside a Dado function called from there, on any VM of the
//@ thread. An allocator comes first and the sinks after it by name, `using
//@ arena, out: log:`. Scopes nest, the innermost winning, and every way out
//@ of one — its end, `return`, `fail`, `break`, `continue`, a failure passed
//@ on — puts back the sinks in force before it; outside every scope a VM
//@ writes to its own. A sink scoped after a Dado function's call is refused:
//@ Dado's own output never goes through a script's sinks, and the suite form
//@ is the one that reaches the scripts a Dado function calls.
