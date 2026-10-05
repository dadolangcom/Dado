<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# vendor:enet

vendor:enet — Dado bindings for the zpl-c/enet single-header fork of ENet
(MIT; see LICENSE), the reliable-UDP library `core:net` wraps.

    import "vendor:enet"

    enet.enet_initialize()
    defer enet.enet_deinitialize()

    ^enet.ENetAddress addr = enet.address_create()
    defer enet.address_destroy(addr)
    enet.address_set_any(addr)
    enet.address_set_port(addr, 7777)
    ^enet.ENetHost host = enet.enet_host_create(addr, 32, 2, 0, 0)
    defer enet.enet_host_destroy(host)

**Vendored IPv4-only — a deliberate choice, not upstream's default.**
zpl-c/enet defaults to dual-stack IPv6 (`in6_addr` throughout); `enet.c`
`#define`s `ENET_IPV4_ONLY` before including the header and the `foreign`
block below asks for the same mode with `define "ENET_IPV4_ONLY=1"` (see
`enet.c`'s own doc comment for the full reasoning — verified directly
against raw BSD sockets that this project's own sandbox has no IPv6 at all,
`EAFNOSUPPORT` on `socket(AF_INET6, …)`, which is common for minimal
containers generally, not a one-off). `address_set_host` resolves IPv4
literals and DNS names that resolve to one; an IPv6 literal (`"::1"`) will
not resolve under this build. One macro, flipped in both places together,
reverts to dual-stack if that is ever needed.

**A `define` clause is program-global**: it is emitted above every
`#include` in the single translation unit a Dado program is, so linking this
package defines `ENET_IPV4_ONLY` for every package beside it. Nothing
outside this directory names that macro, so it costs nothing today — but it
is the property to check before adding a second `define` here.

**Head start, hand-finished — not a raw binding-generator dump.** A first
pass ran Dado's binding generator against the vendored `enet.h` (package
`enet`, written to a scratch directory); it emitted 189 declarations clean
and skipped 23. Reviewed against that draft (`bindgen: emitted 189
declarations (20 enum, 35 type, 35 const, 99 fn); skipped 23`), this file
keeps only the **public API** — the functions under `enet.h`'s own
"Public API" banner (`enet_initialize` on) plus the five types a caller
actually touches — and drops everything bindgen dutifully translated from
ENet's *internal* protocol/command/list machinery (`ENetProtocol*`,
`ENetChannel`, `ENetList`, `_ENetOutgoingCommand`, …): none of it is
reachable from outside `enet.c`'s own translation unit, so a binding for
it is dead surface a caller could misuse. The raw low-level socket
primitives (`enet_socket_*`, `ENetSocketSet`/`fd_set`) are dropped for the
same reason from the other direction — `core:net` drives ENet through
`ENetHost`, never the BSD socket underneath it.

**Why bindgen skipped what it skipped, and why hand-authoring (not fixing
`dadoc`) is the right response here.** Two `dadoc` limitations a
transparent-struct restatement could hit were in play when this was
written, and neither stands now:

  * `ENetHost`, `ENetPeer`, `ENetChannel` all transitively reach
    `ENetList`'s intrusive sentinel node — bindgen's note then: "depends on
    unspellable type `_ENetList`" (`ENetList`'s own one-field
    `sentinel: ENetListNode` hit the one-member collapse; a restatement
    keeps its member list whole now). Opaque is still right: **every one of
    these three is used only as an opaque handle by the public API** —
    nothing outside `enet.c` walks a `ENetHost`'s peer list or a
    `ENetChannel`'s window state directly, every value a caller needs is
    already behind a getter (`enet_host_get_peers_count`,
    `enet_peer_get_state`, …), and ENet's own header comment says as much
    ("No fields should be modified unless otherwise specified" — `ENetPeer`).
    `type ENetHost`/`ENetPeer` with no body — a foreign type declared with
    no body is opaque, and only ever crosses by pointer — is
    not a workaround for a `dadoc` bug here; it is the *correct* binding —
    these were never meant to be poked at by value.
  * `ENetEvent.type` and `ENetCompressor.context` were fields named with
    Dado keywords. `context` is not one any more, and `@c("type") type_`
    spells the other, as `sapp_event.type` and `sg_image_desc.type` are
    spelled in `vendor:sokol`. `ENetCompressor` (custom
    packet compression) is out of scope for this vendoring pass entirely —
    it is not part of the networking baseline. `ENetEvent` **is** needed —
    it is what `enet_host_service` fills in — and it is restated member for
    member below, which is what `@c("type") type_` bought.
  * `ENetAddress` carries a system address field `dadoc` had no declaration
    for. It has one: the member is a `struct in_addr` under this binding's
    own `ENET_IPV4_ONLY` mode, `InAddr` restates it above the generated
    region, and the dual-stack `struct in6_addr` whose `s6_addr` glibc
    `#define`s away is not the arm this package compiles.

**`enet_shim.h`/`.c` are gone.** Fifteen hand-written C functions gave
`ENetAddress` and `ENetEvent` a heap constructor and one accessor per member
because neither could be restated; both are restated now, so the accessors
are Dado one-liners over a member read. The accounting is in the block below
the generated region.

**`size_t` parameters** (`enet_packet_create`'s `dataLength`,
`enet_host_create`'s `peerCount`/`channelLimit`, …) are `#c.size_t`, C's own
type, and not `u64`: the width is the same on every target Dado supports and
the type is not — `u64` is `uint64_t`, which Darwin makes `unsigned long
long` — and the emitter asserts each declaration's C type. A caller passes a
literal as it is and converts a `u64` with `#c.size_t(n)`.

## Declarations

65 declarations, 62 public.

* `type @c("struct in_addr") InAddr` — ── hand-written, above the marker, and it stays across a regeneration ──…
* `enum @c("ENetPacketFlag") PacketFlag` — bindgen:begin declarations — regenerated; edits between the markers are lost…
* `enum @c("ENetPeerState") PeerState`
* `enum @c("ENetEventType") EventType`
* `type ENetPacket` — ── Types ─────────────────────────────────────────────────────────────…
* `type ENetAddress`
* `type ENetPeer`
* `type ENetHost`
* `type ENetEvent`
* `^ENetPacket enet_packet_create(const rawptr data, #c.size_t dataLength, u32 flags)` — ── Functions ─────────────────────────────────────────────────────────…
* `void enet_packet_destroy(^ENetPacket packet)`
* `i32 enet_initialize()`
* `void enet_deinitialize()`
* `u32 enet_linked_version()`
* `u32 enet_time_get()`
* `i32 enet_address_set_host_new(^ENetAddress address, cstring hostName)`
* `i32 enet_address_get_host_new(^const ENetAddress address, ^#c.char hostName, #c.size_t nameLength)`
* `u32 enet_host_get_peers_count(^ENetHost host)`
* `u32 enet_host_get_packets_sent(^ENetHost host)`
* `u32 enet_host_get_packets_received(^ENetHost host)`
* `u32 enet_host_get_bytes_sent(^ENetHost host)`
* `u32 enet_host_get_bytes_received(^ENetHost host)`
* `u32 enet_peer_get_rtt(^ENetPeer peer)`
* `PeerState enet_peer_get_state(^ENetPeer peer)`
* `rawptr enet_peer_get_data(^ENetPeer peer)`
* `void enet_peer_set_data(^ENetPeer peer, const rawptr data)`
* `rawptr enet_packet_get_data(^ENetPacket packet)`
* `u32 enet_packet_get_length(^ENetPacket packet)`
* `^ENetHost enet_host_create(^const ENetAddress address, #c.size_t peerCount, #c.size_t channelLimit, u32 incomingBandwidth, u32 outgoingBandwidth)`
* `void enet_host_destroy(^ENetHost host)`
* `^ENetPeer enet_host_connect(^ENetHost host, ^const ENetAddress address, #c.size_t channelCount, u32 data)`
* `i32 enet_host_check_events(^ENetHost host, ^ENetEvent event)`
* `i32 enet_host_service(^ENetHost host, ^ENetEvent event, u32 timeout)`
* `void enet_host_flush(^ENetHost host)`
* `void enet_host_broadcast(^ENetHost host, u8 channelID, ^ENetPacket packet)`
* `void enet_host_channel_limit(^ENetHost host, #c.size_t channelLimit)`
* `void enet_host_bandwidth_limit(^ENetHost host, u32 incomingBandwidth, u32 outgoingBandwidth)`
* `i32 enet_peer_send(^ENetPeer peer, u8 channelID, ^ENetPacket packet)`
* `^ENetPacket enet_peer_receive(^ENetPeer peer, ^u8 channelID)`
* `void enet_peer_ping(^ENetPeer peer)`
* `void enet_peer_reset(^ENetPeer peer)`
* `void enet_peer_disconnect(^ENetPeer peer, u32 data)`
* `void enet_peer_disconnect_now(^ENetPeer peer, u32 data)`
* `void enet_peer_disconnect_later(^ENetPeer peer, u32 data)`
* `rawptr calloc(#c.size_t count, #c.size_t size)`
* `void free(rawptr ptr)`
* `@macro rawptr memcpy(rawptr dst, const rawptr src, #c.size_t n)`
* `^ENetAddress address_create()` — Dado cannot stack-allocate a value and hand back a pointer to it that outlives…
* `void address_destroy(^ENetAddress addr)`
* `bool address_set_host(^ENetAddress addr, cstring hostname)`
* `void address_set_any(^ENetAddress addr)` — "Bind to every interface." `enet.h` spells this `ENET_HOST_ANY`, which under…
* `void address_set_port(^ENetAddress addr, u16 port)`
* `u16 address_get_port(^ENetAddress addr)`
* `bool address_get_host(^ENetAddress addr, rawptr out, u64 outLen)`
* `^ENetEvent event_create()`
* `void event_destroy(^ENetEvent ev)`
* `u32 event_type(^ENetEvent ev)` — `u32` and not `EventType`, because every caller in this tree compares it…
* `^ENetPeer event_peer(^ENetEvent ev)`
* `u8 event_channel_id(^ENetEvent ev)`
* `u32 event_data(^ENetEvent ev)`
* `^ENetPacket event_packet(^ENetEvent ev)`
* `u64 packet_copy(^ENetPacket packet, rawptr out, u64 outCap)` — Copy a received packet's bytes into `out` (up to `outCap` bytes); returns…
