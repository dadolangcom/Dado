//@ toys/dialect 45
//@ ### Functions are values
//@
//@ A function type is written as Dado writes a function reference: the
//@ return type, then the parameter types in brackets — `bool(int)`,
//@ `void()`, `int(int, int)`. It is a type like any other, so a local, a
//@ field, a parameter, an answer or an element of an array may hold a
//@ function. A **lambda** makes one: a function declaration without its
//@ name, `bool(int x): return x > 3`, whose value has the type its head
//@ spells, `bool(int)`. Its body is one statement on the line of its `:`,
//@ or, when the `:` ends the line, an indented block under it — at
//@ statement level and inside brackets alike, where the body ends at the
//@ `,` or the bracket that comes after it:
//@
//@ ```text
//@ []int big = xs.filter(bool(int v):
//@     int w = v * 10
//@     return w > 25
//@ )
//@ ```
//@
//@ A lambda reads and writes the locals, the parameters and the fields
//@ around it **by reference**: a counter it adds to is the method's
//@ counter, and a value the method changes after the lambda was made is
//@ the value the lambda sees. A loop's variable is a new one each turn, so
//@ lambdas made in three turns hold three different values.
//@
//@ A method is a value too: its bare name or `self.hit`, or `g.hit` for
//@ another instance's, read without calling it, is a function bound to its
//@ instance. A value of function type is called as a method is,
//@ `f(3)`, and a call through one passes every argument — a default is no
//@ part of a function's type. Calling a value that holds no function is
//@ refused, naming its type. Two functions are never compared, since a
//@ method read twice is two functions.
//@
//@ Where a `Location` is written — a local, a field, a parameter or an
//@ answer declared `Location` — a method's name stands for where the method
//@ begins, as `Location(hit)` does, and a lambda for where it is written.
//@ Only there: an `any` holds no function and is given no place, and a
//@ comparison converts nothing.
//@
//@ A script passes a function to a Dado function that takes one, which Dado
//@ calls back (below, *A script calls Dado*), and Dado hands a script its
//@ own functions as values (below, *Maps, strings of every width, matrices
//@ and functions cross the seam*). A script's method answering a function
//@ is not called from Dado.
