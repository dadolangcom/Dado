<!-- dadoc 1.0.0-rc.4 -->
<!-- commit 549ca2ce6a97 -->
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

0 declarations, 0 public.

