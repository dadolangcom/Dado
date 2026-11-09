//@ toys/dialect 35
//@ ### `Platform`, and the `when` a script folds
//@
//@ `Platform.os()` is the operating system the script was compiled for — for
//@ a script a Dado program loads, its host's — as a string, and the members
//@ of `Platform` are the strings it can be: `Platform.LINUX`,
//@ `Platform.DARWIN`, `Platform.WINDOWS`, `Platform.FREEBSD`,
//@ `Platform.OPENBSD`, `Platform.NETBSD`, `Platform.WEB` and
//@ `Platform.UNKNOWN_OS`. Compare against a member rather than a string
//@ written out: a misspelt member is refused, a misspelt string is only
//@ unequal.
//@
//@ All of them are known when the script is compiled, so a script's `when`
//@ selects on them as Dado's `when` selects on its compiler definitions:
//@ `when Platform.os() == Platform.WEB:` keeps one arm and the others are
//@ never checked, at the level of a method's statements or of the class's
//@ members. A script's `when` folds literals, `Platform.os()`, the members
//@ and the operators between them; a field, a local or a call is known only
//@ when the script runs, and is refused naming `if`. The CPU architecture
//@ is not asked in DadoScript — that question belongs to Dado.
