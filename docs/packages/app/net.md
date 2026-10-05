<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:net

app:net — the RPC-and-packets escape hatch on top of `core:net`: framed
remote calls, and peers named by a small integer.

    import "app:net"    // binds `net` — the last path segment

    void ready():
        net.listen(17777, 32)                    // server role
        // — or —
        i32 srv = net.dial("127.0.0.1", 17777)   // client role; srv is the
                                                   // server's peer id

    void process(f32 dt):
        [16]net.NetEvent evs
        i32 n = net.poll(evs[:])
        for i in 0..<n:
            if evs[i].kind == net.NetEventKind.Connect:
                net.rpc(evs[i].peer, MSG_HELLO, []u8{}, net.Class.Reliable)
            if evs[i].kind == net.NetEventKind.Rpc:
                handle(evs[i].id, evs[i].peer, evs[i].data)

**Named `NetEvent`/`NetEventKind`, not the bare `Event`/`EventKind` a
reader might expect from `app:input`'s precedent.** The names are kept for
readability — a program that polls both this package and `app:input` in one
`process` reads better when the two vocabularies are distinguishable at the
call site — and that is now the *whole* of the reason.

It was not the original reason, and the correction is worth keeping. This
package first chose these names to escape a refusal: with `core:net`'s own
`Event` and a local `Event` both visible, every field access on either was
`ERR0518`, and the comment here explained it as view resolution keying
"off the plain name, not full structural identity." That was a
compiler defect, not a rule — what is refused is *two visible spellings of
**one shape***, and these were two shapes. It is fixed;
`example/net`'s client now imports `core:net` and `app:input` into one file,
which is the case that could not be written. **Reverting the names is a
live option and deliberately not taken here**: it is an API change to a
landed package with a test and an example against it, and the names earn
their keep on their own merits now. Whoever weighs it should know it is a
preference and no longer a constraint.

**The mechanism.** `core:net` is the `Reliable`/
`Unreliable` wrapper over `vendor:enet` — a `Host`, a `Peer`, `send`/
`broadcast`, and a `poll` that drains ENet's event loop. This is the
opinion on top, the same split `core:log`/`app:log` already follows:
**peer identity as a small `i32`**
instead of a raw pointer, and **`rpc`'s `id` framed onto the wire** so a
receiver's `poll` hands back `(id, peer, bytes)` exactly as an
`app.rpc`/`event(id, peer, bytes)` pair reads, without the
program having to invent its own message-id convention.

**One `Host` per process, matching the app singleton** ("there
is one terminal, one window, one app" — extended here to "one network
role"). `listen` makes this process a server; `dial` (on a process with no
host yet) makes it a client and immediately starts connecting to one peer.
A server cannot also `dial` out in this baseline — a program that needs
both roles in one process is out of scope here (the target shape is a
headless server *and* a separate TUI client, two processes).

**Peer ids, not pointers.** `core:net.Peer` is a raw `^ENetPeer`; this
package keeps a fixed `MAX_PEERS`-slot table and hands back a small `i32`
instead — the surface names a `peer_id`, not a pointer, and a
small int is what a program can put in an array index or send back over
the wire itself. A server's peers are registered as `core:net`'s own
`Kind.Connect` events arrive (their id is not known before then); a client's one peer is
registered synchronously by `dial`, at `connect`'s return, before the
handshake even completes — the same event fires again once it does, and
`poll` recognises the peer by pointer identity rather than re-registering
it under a second id.

**`poll` is drained at the barrier, exactly like `input.poll`** —
three hooks, and everything else is polled: call it from `process`, not
anywhere else. Unlike `input`, there is no separate backend-driven fill
step — `core:net.poll`'s own `timeout_ms = 0` call is non-blocking and
ENet already buffers internally, so calling it directly, in a loop, from
inside `process` *is* the drain; no intermediate ring is needed. That was
the open question this package settles — a ring turned out to be
unnecessary, not just avoidable.

**A `Rpc` event's `data` is a window, not an owning `ref`, and needs no
`delete`.** Every payload — the wire-framed outgoing bytes `rpc` builds
and the incoming bytes `poll` decodes — is allocated from `app.temp()`,
the per-frame arena every other transient value in this framework already
uses (`arena.Free` is a documented no-op, `FreeAll` only runs at the next
frame's barrier — see `core:mem/arena`'s own doc comment) — so a `Rpc`
event's `data` stays valid for the rest of this frame and is reclaimed
automatically at the next one — the lifetime every scratch-backed value in
this framework gets, and the reason a `poll` result must be read or copied
within the frame that produced it.

**The wire format `rpc` adds over a raw `core:net` packet is one `i32`,
little-endian, prefixed to the payload** — `id`, so `poll` can hand it
back without the program inventing its own header. This is the one piece
of framing `app:net` owns; the *payload* bytes are still "the program
serialises itself." **`core:net`'s own `send`/`broadcast`
are not re-exposed here** — the "raw packets, one level lower" surface
(`app.send`/`app.receive`, unframed) is deliberately deferred: sharing one
`Host`/channel pair between framed `rpc` traffic and unframed raw traffic
needs its own disambiguation design (`poll` consumes exactly one
`core:net.poll` event per call and cannot peek its bytes before deciding
how to decode them), and the Definition of done here is "exchange RPCs,"
not both surfaces at once. A future task can add it once a real caller
needs it.

## Declarations

0 declarations, 0 public.

