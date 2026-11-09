//@ toys/dialect 34
//@ ### Reloading a script while the program runs
//@
//@ `try #script_reload(vm, Goblin)` swaps new code in under a class the VM
//@ has loaded, without stopping it. The new code is the module `dadoc
//@ --emit=reload` writes for the class's file — `reload/<package>/<Class>.js`
//@ under the working directory, or under the directory written third,
//@ `#script_reload(vm, Goblin, dir)`. Before anything changes it is checked against
//@ what this build bound: every Dado function the new code calls must be one
//@ the build binds, with the same signature; every method the program's Dado
//@ runs through a handle must still be there with its signature; every
//@ field the program's Dado reads or writes through a handle, `g.hp`, must
//@ still be there with its type; and every field the new code keeps must
//@ keep its type. The class may be any the build carries, and the VM must
//@ have made an instance of it, which loads it: a VM that has not is
//@ refused. Any miss fails the reload,
//@ the code running now stays, and `#script_error()` names every miss, one
//@ per line — `entities.shake` has no binding in this build; rebuild to call
//@ it from a script.
//@
//@ A reload that checks replaces every method of the class at once, under
//@ every instance there is. Each instance keeps its fields' values; a field
//@ the new code adds starts at its initializer when the instance first
//@ touches it; a field it drops is no longer named; `init` does not run
//@ again. Then each instance a handle holds runs its `reloaded()` method, if
//@ the class has one. Called while the script is running — from a Dado
//@ function the script called — the new code is checked at once and takes
//@ over when that run returns.
//@
//@ A release build, `--release`, refuses `#script_reload` unless it is
//@ built with `--reload` too.
