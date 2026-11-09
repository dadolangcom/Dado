//@ toys/dialect 31
//@ ### An instance crosses into Dado
//@
//@ An instance keeps its methods when it crosses into Dado. A method that
//@ answers an instance of a class — `Goblin spawn()` — answers Dado a handle
//@ of that class's type, `npc.Goblin`, which the program holds and passes on
//@ like any other value; a handle passed back as an argument hands the script
//@ the instance itself. One object is one handle, however often it crosses,
//@ so `==` on two handles asks whether they hold the same instance.
//@ `#delete(g)` lets go of Dado's hold on the instance, and every copy of the
//@ handle — in a local, a field, a slice — is stale from then on: using one
//@ fails with `script.STALE`, never reaches freed memory, and never reaches
//@ another instance. The script's own references to the instance are
//@ untouched, and it lives while the script holds it.
//@
//@ Through a handle Dado reads and writes the class's public fields and calls
//@ its methods by name: `g.hp`, `g.hp = 3`, `g.hit(3)`. A field crosses as
//@ an argument does, a field of a class's type as a handle, and one of any
//@ other type is refused, naming the method to write instead. `g.hit(3)` is
//@ failable under `try`, and otherwise its failure goes where its VM's
//@ failure policy says. A field read or
//@ written is never under `try`, and follows the policy. A Dado function may
//@ take or answer a handle, so a script calls it with its own instances: the
//@ function is handed the handle, and its answer is the instance again. An
//@ instance belongs to the VM that made it: handed to a script in another
//@ VM, it is refused with `script.OTHER_VM`.
