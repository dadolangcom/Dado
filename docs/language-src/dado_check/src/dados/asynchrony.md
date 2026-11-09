//@ toys/dialect 29
//@ ### Async
//@
//@ A method marked `async` — `async int load(string path):` — returns its
//@ value later. Its callers get an `async int`, a value of its own type: it
//@ can be held, `async int pending = load("a")`, kept in a field or an
//@ array and passed on, and nothing else is done with it until a statement
//@ headed by `await` waits for it and reads the `int`: `await int size =
//@ load("a")` declares, `await size = pending` assigns, and `await save(size)`
//@ waits for a call whose value is not wanted.
//@
//@ `await` heads a declaration, an assignment or a call, as `try` does, and
//@ only at the start of a statement: `f(await g())` is refused, and the value
//@ is awaited on a line of its own. It is written only inside an `async`
//@ method or lambda (`async int(): …`), so a method that waits is itself
//@ waited for. A call that answers an `async` value is never written alone
//@ on a line, where nothing would wait for it — await it, or hold what it
//@ answers. `init` and a generic method are never `async` (`ERR1330`).
//@
//@ An `async` value is not printed, alone or in a tuple, an array or a map:
//@ await it and print what it answers. An instance renders whatever its
//@ fields hold, and a pending value among them prints as `<async>`.
//@
//@ A failure inside an `async` method raises at the `await` that waits for
//@ it, with its `Error` unchanged, so `try await int size = load("a") else
//@ e:` handles it as `try` handles a call. A host's `#script_run` of an
//@ `async` method runs the engine until the method's value is ready and
//@ answers it, and every `#script_run` runs to the end whatever work its
//@ method started.
