//@ procs/visibility 30
//@ ### An import may make its qualifier optional
//@
//@ `import . "core:fmt"` binds the package under its last element, as `import
//@ "core:fmt"` does, and also spells each of its public names bare in that
//@ file: `println("hi")` and `fmt.println("hi")` are the same call. That is
//@ how a Dado program writes a bare `println`: no name is built in. A script
//@ writes the same line, for a Dado package or a package of scripts, and a
//@ class imported this way is named bare (`Goblin`) as well as
//@ qualified (`npc.Goblin`).
//@
//@ A bare name means one thing, and nothing decides between two by
//@ precedence or shadowing. A bare name that two unqualified imports both
//@ supply — or one of them and a declaration of the file's package, or in a
//@ script one of them and a built-in such as `println` — is refused where
//@ it is written (`ERR0309`), naming each spelling it could mean; the
//@ qualified spelling is always there to write. So two packages that
//@ overlap may both be imported unqualified, and nothing is refused until
//@ an overlapping name is written bare. A declaration in the importing file
//@ that takes a name an unqualified import supplies — a function, a type, a
//@ constant, a class, or a member of a class the file holds — is refused at
//@ the declaration (`ERR0310`). A local or a parameter is a function's own
//@ and shadows a bare import as it shadows the package's own names. An
//@ import belongs to its file, so another file of the package is not
//@ affected.
