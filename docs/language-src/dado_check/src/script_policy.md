//@ toys/dialect 34
//@ ### When a script fails and no `try` is written
//@
//@ A method called through a handle without `try`, or a field read or
//@ written through one, is not failable: when the script fails, its text
//@ goes to the error sink, and what happens next is the VM's policy, set
//@ when it is made. By default it is `script.QUIT`: the program ends with
//@ the failure's status, as an unhandled failure in Dado does.
//@ `#script_vm(on_error: script.CONTINUE)` makes it the other policy: the
//@ failing call answers its type's zero value and the program goes on, as a
//@ game keeps running past a broken mod. The names are `core:script`'s, so
//@ a program that writes one imports it. `#script_new` is failable and
//@ `#script_run` answers a status, so neither follows the policy.
//@ `try … else code:` still catches a failure under either policy.
