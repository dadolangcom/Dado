<!-- dadoc 1.0.0-rc.6 -->
<!-- commit 4de070662e29 -->
# core:net/enet

core:net/enet: game sessions over ENet's reliable UDP, as two delivery
classes, `Reliable` and `Unreliable`, over `vendor:enet`.

    import "core:net/enet"

    try enet.init() else why:
        fmt.println(enet.message(why))
        return 1
    defer enet.shutdown()

    try enet.Host server = enet.host_server(17777, 32) else why:
        fmt.println(enet.message(why))
        return 1
    defer enet.host_destroy(server)

    try enet.Event ev = enet.poll(server, 10) else why:
        fmt.println(enet.message(why))
        return 1
    switch ev:
        case enet.Received r:
            // r.data is the caller's: #delete it when done
            #delete(r.data)
        case enet.Connected _: fmt.println("a peer came")
        case enet.Disconnected _: fmt.println("a peer left")
        case enet.TimedOut _: fmt.println("a peer timed out")
        case enet.Idle _: fmt.println("nothing yet")

This package is the session half of networking. Sockets, addresses and DNS
are `core:net`; this one is what a game wants on top of UDP: peers that
connect and time out, and two delivery classes that hide ENet's channel
numbers and packet flags. `app:net` builds its RPC layer on it, and a
program never needs `vendor:enet` directly.

**Tier W H, sessions H only.** Linux, macOS, the BSDs and Windows run ENet
(Windows links `ws2_32` and `winmm`). On `#WEB` and `#NONE` the package
builds and `init` fails `Error.UNSUPPORTED`: a page has no UDP socket to
give ENet, and `vendor:enet` compiles its C only where there is a C
library. No host can be made there, so nothing else is reached.

**Errors.** `init`, the host constructors, `connect`, `send` and `poll`
fail with a member of `Error` (codes 2000 to 2099), and `message(why)` is
a sentence for each.

**The class-to-channel mapping.** Channel 0 is reliable and ordered
(`PacketFlag.Reliable`) and channel 1 is unreliable and unsequenced
(`PacketFlag.Unsequenced`). Two is the smallest `channelLimit` a host here
is created with, and the split follows `ENetPacketFlag`'s own.

**One reusable `ENetEvent` per `Host`, not one per `poll` call.** ENet's
service call fills the same buffer every time, and a `Host` owns that
buffer instead of the caller.

**What `poll` answers is an `Event`**, a union with one member for each
thing that can happen, so a `switch` over it names them all and needs no
`else`. `Idle` is "nothing happened in the time allowed", which is no
failure; a failure is ENet's service call itself failing. **A `Received`
event's `data` is a caller-owned `ref []u8`** on `#default`, copied out of
ENet's packet before the packet is destroyed (an ENet packet lives only
until the next service call); the caller `#delete`s it, or polls inside a
`using` of an arena that reclaims it.

**Single-threaded.** ENet's service call is not documented as thread-safe,
so `poll`, `send`, `broadcast` and `flush` are meant to be called from one
thread, the same contract `app:input`'s queue has. There is no send queue
here; one belongs above this package if a caller needs to send from
several threads.

The package is named `netenet` and imported as `enet` (the qualifier is the
directory's name): `vendor:enet` is already a package named `enet`, and
`dadoc query inspect`, which `dado doc` and `dado api` read, lists no
declarations for a second package of the same name.

**What raises into a script.** `message`, and the private mapping from a
class to its channel, each promised by the `#assert_raises`
above it. Everything else calls ENet through a `foreign` declaration or
holds an ENet pointer, which is the C boundary a script does not cross.

## Declarations

31 declarations, 27 public.

* `distinct enum i32 Error` — Why a call failed.
* `string8 message(Error why)` — A sentence for `why`, for a person reading a tool's output.
* `enum i32 Class` — How a packet is delivered: `Reliable` is resent until acknowledged and…
* `const u8 CHANNEL_RELIABLE = 0` — The ENet channel `Class.Reliable` packets travel on.
* `const u8 CHANNEL_UNRELIABLE = 1` — The ENet channel `Class.Unreliable` packets travel on.
* `const u64 CHANNEL_COUNT = 2` — How many channels every `Host` here is created with: one per class.
* `type Host: (^venet.ENetHost h, ^venet.ENetEvent ev)` — One ENet host and the `ENetEvent` buffer every `poll` on it reuses. A…
* `type Peer: ^venet.ENetPeer` — A connected peer: ENet's handle, under this package's name so a caller…
* `distinct type Idle: (u32 timeout_ms)` — Nothing happened within `timeout_ms`, the wait `poll` was given.
* `distinct type Connected: (Peer peer)` — `peer` finished connecting: on a server, a new peer; on a client, the…
* `distinct type Disconnected: (Peer peer)` — `peer` disconnected cleanly.
* `distinct type TimedOut: (Peer peer)` — `peer` stopped answering and was dropped.
* `type Received: (Peer peer, ref []u8 data)` — A packet from `peer` arrived: its bytes, on `#default`, which the caller…
* `union Event: (Idle, Connected, Disconnected, TimedOut, Received)` — One `poll` result: nothing, or one of the four things that happen to a…
* `Error!void init()` — Start ENet for this process; nothing else in this package works until it…
* `void shutdown()` — Stop ENet for this process, after every host is destroyed. Pairs with a…
* `Error!Host host_server(u16 port, u64 max_peers)` — A server bound to `port` on every local interface, accepting up to…
* `Error!Host host_client(u64 max_outbound)` — A client host with no bound address: it can only `connect` out, never…
* `void host_destroy(Host host)` — Destroy a host and its event buffer, dropping every peer it holds without…
* `u32 host_peer_count(Host host)` — How many peer slots the host has, as ENet counts them: connected,…
* `void flush(Host host)` — Push queued outgoing traffic now rather than at the next `poll`: useful…
* `Error!Peer connect(Host host, string8 address, u16 port)` — Connect `host` (built with `host_client`) to `address:port`. `address` is…
* `void disconnect(Peer peer)` — A graceful disconnect: queues a notification the peer acknowledges. Keep…
* `void reset(Peer peer)` — An immediate, unacknowledged drop: no `Disconnected` event follows. For an…
* `Error!void send(Peer peer, Class cls, []u8 data)` — Send `data` to one peer over the given delivery class; a packet is queued…
* `void broadcast(Host host, Class cls, []u8 data)` — Send `data` to every peer connected to `host` over the given delivery…
* `Error!Event poll(Host host, u32 timeout_ms)` — One step of `host`'s event loop, waiting up to `timeout_ms` for something…
