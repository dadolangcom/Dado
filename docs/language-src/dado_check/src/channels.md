//@ flow/channels 10
//@ ### A channel is a mailbox: many producers, one consumer
//@
//@ `#channel(N) name(params)` declares one, at package level or as a member of
//@ a `type`: a typed, bounded, lock-free mailbox of `N` messages, each one
//@ the parameters' fields. `#channel(latest)` keeps only the newest message
//@ instead. Its storage is static or inline in the value that holds it, and
//@ its zero bytes are the empty channel, so it needs no initializer and
//@ nothing runs before the program starts — which is also why it works on a
//@ target with no operating system at all.
//@
//@ `#send` hands a message to it from any thread and never blocks, its
//@ arguments checked as a call's are. **A queue's `#send` fails when the queue
//@ is full, and the program says what that means**: it is written under
//@ `try`, failing with a code or handling it in a suite, exactly as a map
//@ read that finds nothing is. A `latest` channel's `#send` replaces the value
//@ nobody took and cannot fail, so it takes no `try`.
//@
//@ The consumer walks what is queued with `for`: `#peek` looks and consumes
//@ nothing, `#drain` takes, and `#drain(ch, count)` takes at most that
//@ many. A walk is a snapshot — it sees what was queued when it began and
//@ always ends — and a `#drain` **commits when its loop ends, by any exit**:
//@ running off the end, `break`, `return`, or a failure passing out of the
//@ body. An entry whose body began is consumed. `#drain(ch)` alone on a line
//@ is the discard. `#len` counts, from any thread.
//@
//@ ```dado
//@ package doc_sample
//@
//@ const i32 FULL = 1
//@
//@ #channel(32) pressed(i32 id, bool down)
//@ #channel(latest) resized(i32 w, i32 h)
//@
//@ !i32 input(i32 id):
//@     try #send(pressed, id, true) else FULL
//@     #send(resized, 800, 600)
//@     return 0
//@
//@ i32 frame():
//@     i32 held = 0
//@     for id, d in #drain(pressed):
//@         if d && id != 0:
//@             held += 1
//@     i32 area = 0
//@     for w, h in #drain(resized):
//@         area = w * h
//@     return held + area
//@ ```
//@
//@ **What a message may hold is what can outlive the sender.** Plain data is
//@ copied into the slot. A window, a pointer, a string, a map or another channel
//@ is refused, because each is a view of storage the sender keeps. A `ref`
//@ written as a parameter of its own **moves**: a successful `#send` writes
//@ the sender's place nil and reading it afterwards is refused, while on the
//@ path where the queue was full it is still the sender's. A `#drain` binds it
//@ as the owning `ref` and a `#peek` as the view it lends; a `latest` channel,
//@ which overwrites, takes none.
//@
//@ **A value holding a channel is never copied** — it is one mailbox at one
//@ address, and a copy would be a second — so it is reached through a
//@ pointer. And a channel has one consumer: a debug build traps when a second
//@ thread walks one, naming the channel and both threads, and two thread roots
//@ that both walk one package-level channel are a warning, an error under
//@ `--strict`. A consumer that is done hands the channel on with
//@ `#release(ch)`, a statement of its own: the next thread to walk it
//@ becomes the consumer, and a channel the program releases is not linted.
