//@ toys/dialect 28
//@ ### A type's names, and an instance's slots
//@
//@ `Hero.names()` answers the slot names of the class `Hero` as a `[]string`:
//@ every field, an inherited one first, each class's in the order it is
//@ written, which is the order an instance is built in and `print` writes
//@ it. Asked of an enum, `Mood.names()` answers its members in order, and
//@ asked of a Dado declaration a script names, `Vec2.names()`, the names its
//@ slots are declared with. It is a question about a type, so the receiver
//@ is a type written by its name; the names are known when the script is
//@ compiled, and each call hands over an array of its own. A method an
//@ instance has, `held.names()`, is the instance's.
//@
//@ `for name, value in slots(v):` walks the slots of an instance, in the
//@ same order, binding each slot's name, a `string`, and what it holds, an
//@ `any`, since one class's slots hold values of many types.
//@ `for value in slots(v):` walks the values alone. What is walked may be
//@ an instance, a trait's value or an `any`, and the slots walked are those
//@ of the class the instance is, so a `Lion` held as a `Beast` walks the
//@ lion's; an `any` that holds no instance raises a fault, an `Error` of
//@ code 70. Nothing but declared fields is walked — no method and nothing
//@ the engine keeps. This is how a script writes code over any of its
//@ objects — a serializer, for one:
//@
//@ ```
//@ string text(any v):
//@     string out = ""
//@     for name, value in slots(v):
//@         out += name + "=" + string(value) + " "
//@     return out
//@ ```
//@
//@ `slots` is a built-in function like `print`, and it walks only as the
//@ iterable of a `for` header; written anywhere else, or where the class's
//@ own member or the package's Dado function named `slots` could be meant,
//@ it is refused (`ERR1350`), and so is `Hero.names()` where `Hero` has an
//@ inner class of that name.
