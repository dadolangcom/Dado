//@ procs/failure 20
//@ ### Where the program is: `#trace()`
//@
//@ `#trace()` answers where the program is, as a `ref []Location`
//@ innermost first: the place it is written, then the place of each call
//@ the run is inside, out to `main` (or a thread's first function). A
//@ `Location` is `(string8 file, i32 line, i32 column, string8 function)`,
//@ DadoScript's four fields, and the function is named with its package,
//@ `main.update`, a method with its type's, `main.Box.get`. The run is
//@ minted on the ambient allocator and the caller frees it, `#delete(t)`.
//@
//@ What the calls are is known only in a build that keeps the full call
//@ stack: `--debug`, `dado test`, and any build given `--full-trace`
//@ (`--no-full-trace` drops it from those). There each function's entry
//@ pushes a frame on a stack of the thread's own, its every exit pops it,
//@ and each call notes its line and column in the caller's frame first.
//@ Past 4,096 calls deep a call is counted and not kept. Any other build —
//@ a plain `dado build` or `dado run`, `--release` — has no such stack, and
//@ `#trace()` there answers one element, the place it is written: a
//@ constant, and the first element of the full answer, so code that reads
//@ `t[0]` or prints the run means the same in both builds. The same stack
//@ is what a trap prints: a bounds check, an `#assert`, a stale read the
//@ sanitizer caught, or the first report of a narrowing that changed a
//@ value writes the place as `at main.dado:12:5 in main.update`, then one
//@ such line for each call outward, 32 at most and `… N more` past them.
//@ Without the stack the trap writes its own `at main.dado:12:5` line
//@ alone.

//@ procs/failure 25
//@ ### Where it is written: `#location()`
//@
//@ `#location()` answers the place it is written as one `Location`, by
//@ value: `Location here = #location()`. Its file, line and column are
//@ spelled as `#trace()` spells its own first element, and its function
//@ is the one it is written in, `main.update`. It is a constant of static
//@ strings, the same in every build (dev, `--debug`, `--release`,
//@ `--sanitize`, `--full-trace`) and on every target, the web and `#NONE` included:
//@ nothing is allocated, so there is nothing to `#delete`, and it needs no
//@ allocator. It goes wherever a `Location` does — a local, an argument,
//@ a return, a slot. A `#trace()` with no shadow stack to read answers
//@ one element, equal in content to `#location()` written at the same
//@ place; where the stack is kept, that is its first element. DadoScript
//@ writes it `Location()`.
//@
//@ **As a parameter's default it is the caller's place.** A parameter
//@ written `Location at = #location()` is given, by each call that leaves
//@ it out, the place of that call — its file, line and column, and the
//@ function the call is written in — as DadoScript's
//@ `Location at = Location()` is, and as Odin's caller location is. So a
//@ logging function says where it was called from with nothing written at
//@ the call:
//@
//@ ```dado
//@ package doc_sample
//@
//@ import "core:fmt"
//@
//@ public void log(string8 msg, Location at = #location()):
//@     fmt.println(at.file, ":", at.line, ": ", msg)
//@
//@ public void start():
//@     log("starting")
//@ ```
//@
//@ A named call that omits it, a method's default and a generic function's
//@ are the same rule, and a call that passes a `Location` passes its own.
//@ Only the direct form is a caller's default: a `#location()` inside a
//@ larger default value is refused, naming it. A call through a function
//@ value applies no default, since a default is not part of the function's
//@ type, and a DadoScript call of a Dado function passes every argument.
//@
//@ **In a `const` or package variable's initializer it folds** to the place
//@ it is written: `const Location here = #location()` is that line wherever
//@ the constant is read, and names no function (a block's `const` names the
//@ one it is in). Every other compile-time position — an array length, a
//@ `when` — takes it as the call it is, and refuses it there, as it
//@ refuses `#trace()` in all of them.
