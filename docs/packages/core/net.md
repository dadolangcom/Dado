<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:net

core:net — a thin `Reliable`/`Unreliable` wrapper over `vendor:enet`.

    import "core:net"

    net.init()
    defer net.shutdown()

    net.Host server = net.host_server(17777, 32)
    defer net.host_destroy(server)

    net.Host client = net.host_client(1)
    defer net.host_destroy(client)
    net.Peer peer = net.connect(client, "127.0.0.1", 17777)

    net.Event ev = net.poll(server, 10, #default)
    if ev.kind == net.Kind.Receive:
        // ev.data is the caller's now — #delete(ev.data) when done

This is the mechanism half of the networking layer: it knows nothing about
the app barrier, `#ROLE`, or an RPC surface — that is `app`'s job, and it
is not written yet. What lives here is the class-not-socket surface a
caller is promised: `Reliable` and `Unreliable` delivery classes, which
hide channel IDs and packet flags and are the whole vocabulary a program
needs, over
ENet's per-channel reliability (`vendor:enet`). Above the backend line,
`app` builds on this package, never on `vendor:enet` directly —
mechanism down, opinion up, the same split `core:log`/`app:log` and
`core:jobs`/`app` already follow.

**The class-to-channel mapping** was an open design point and this
package settles it: channel 0 is
reliable-ordered (`PacketFlag.Reliable`), channel 1 is
unreliable-unsequenced (`PacketFlag.Unsequenced`) — the minimum
`channelLimit` a host here needs, and it matches `ENetPacketFlag`'s own
`Reliable`/`Unsequenced` split almost directly, exactly the starting
point the handoff suggested.

**One reusable `ENetEvent` per `Host`, not one per `poll` call** — the
same call-reuse `vendor:enet`'s own test demonstrates (`event_create`
once, `enet_host_service` fills the same buffer every time), just wrapped
so a `Host` owns it instead of the caller.

**A `Receive` event's payload is a caller-owned `ref []u8`, copied out of
ENet's own packet before the packet is destroyed.** An ENet packet is
only valid until it is explicitly destroyed or the next service call
reuses its slot; this package copies its bytes into Dado memory and
destroys it immediately, so nothing a caller holds ever outlives its
owner. `poll` takes an explicit `mem.Allocator` rather than defaulting to
`#default` — the same convention `core:cfg`/`core:geom` use and for the
same reason: a parameter default must fold at compile time, so
the ambient allocator can't be one, and writing it is `ERR0901`); pass
`#default` for the ordinary case. Every event kind other than `Receive` carries a
nil `data` and nothing to free; `delete` is a no-op on nil regardless.

**Single-threaded by design, not by accident.** `enet_host_service` is
not documented thread-safe and nothing in `vendor:enet` claims otherwise
— so `poll` is meant to be called
from one place, the same contract `app:input`'s backend-driven queue
already has. `send`/`broadcast` are a thin `enet_peer_send`/
`enet_host_broadcast` call with no queue of their own (unlike
`app:log`'s per-worker rings and the channels `app:events` became, which
exist because *sending* — a log call, an `#send` — happens from arbitrary
job-worker code): the
same single-thread caveat that applies to `poll` applies to these too,
until/unless a future task (`app`'s own RPC surface, Step 3) adds
buffering in front of them.

## Declarations

25 declarations, 22 public.

* `enum i32 Class: (Reliable, Unreliable)`
* `const u8 CHANNEL_RELIABLE = 0`
* `const u8 CHANNEL_UNRELIABLE = 1`
* `const u64 CHANNEL_COUNT = 2`
* `type Host: (^enet.ENetHost h, ^enet.ENetEvent ev)` — A host owns one ENet host and the one `ENetEvent` buffer every `poll`
* `type Peer: ^enet.ENetPeer` — The vendor peer handle, re-exported under this package's own name so a…
* `enum i32 Kind: (None, Connect, Disconnect, Timeout, Receive)`
* `type Event: (Kind kind, Peer peer, ref []u8 data)` — `data` is non-nil only for `Kind.Receive` — see package doc comment on…
* `bool init()` — `false` on failure — nothing else in this package will work.
* `void shutdown()`
* `Host host_server(u16 port, u64 max_peers)` — A server bound to `port` on every local interface, accepting up to…
* `Host host_client(u64 max_outbound)` — A client host with no bound address — it can only `connect` out, never…
* `bool host_valid(Host host)`
* `void host_destroy(Host host)`
* `u32 host_peer_count(Host host)`
* `void flush(Host host)` — Push queued outgoing traffic now rather than waiting for the next…
* `Peer connect(Host host, string8 address, u16 port)` — Connect `host` (built with `host_client`) to `address:port`. `address` is…
* `void disconnect(Peer peer)` — A graceful disconnect — queues a disconnect notification the peer will…
* `void reset(Peer peer)` — An immediate, unacknowledged drop — no `Kind.Disconnect` event follows.
* `bool send(Peer peer, Class cls, []u8 data)` — Send `data` to one peer over the given delivery class. `false` if ENet…
* `void broadcast(Host host, Class cls, []u8 data)` — Send `data` to every peer connected to `host` over the given delivery…
* `Event poll(Host host, u32 timeout_ms, (rawptr(rawptr, AllocatorMode, u64, u64, rawptr, u64) proc, rawptr data) alloc)` — One step of `host`'s event loop, waiting up to `timeout_ms` for…
