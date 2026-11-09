//@ toys/dialect 41
//@ ### A script binds a handler to a channel
//@
//@ `receive(hits, on_hit)` binds a script's function to the channel it
//@ names, one a Dado file of its package declares at package level — a
//@ channel of an imported package is named through the import, as a Dado
//@ function of one is. From then on the script is
//@ the channel's one consumer: every message sent, from any thread, is
//@ delivered to the function, one call a message, in the order they were
//@ sent, on the thread the script's VM belongs to. The function takes the
//@ message's fields as a Dado function's answer reaches a script — an integer
//@ as an `int`, a float as a `float`, a `bool`, a character as a `string` —
//@ and answers nothing: `#channel(16) hits(i32 n, f64 x)` takes a
//@ `void(int, float)`, a method of that type or a lambda,
//@ `void(int n, float x): …`.
//@
//@ A handler never runs in the middle of script code. Messages wait for a
//@ safe point: a Dado function the script called returning to it, a call
//@ into the script returning to Dado, and `#script_pump(vm)`, which a Dado
//@ program writes where its frame ends. A handler runs to completion
//@ before the next message's begins, and a safe point reached inside a
//@ handler delivers nothing. A handler that fails has its failure written
//@ to the error sink, naming the channel, and the next message goes on.
//@
//@ A channel has one consumer. A second VM's `receive` of it fails with
//@ `script.BUSY`, as does a `receive` of a channel Dado walks with `#peek` or
//@ `#drain`; a `receive` again in the same VM replaces the handler. A VM's
//@ receivers end with it. A channel whose message carries a `ref` is never
//@ bound: a script holds no Dado allocation, so the program drains it in
//@ Dado and hands the script what it needs.
