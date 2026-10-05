# `vendor:enet`

Dado bindings for [ENet](http://enet.bespin.org/) — the reliable-UDP library
`core:net`/P2-NET wraps behind its `Reliable`/`Unreliable` delivery-class
surface. Vendored from the
[zpl-c/enet](https://github.com/zpl-c/enet) single-header fork (MIT; see
LICENSE), not the original two-file `lsalzman/enet` distribution — one file
to vendor, IPv6-capable upstream (this build turns that back off; see
below), and no `.c`/`.h` split to keep in sync.

```dado
import "vendor:enet"

enet.enet_initialize()
defer enet.enet_deinitialize()

// server
^enet.ENetAddress addr = enet.address_create()
defer enet.address_destroy(addr)
enet.address_set_any(addr)
enet.address_set_port(addr, 7777)
^enet.ENetHost server = enet.enet_host_create(addr, 32, 2, 0, 0)
defer enet.enet_host_destroy(server)

^enet.ENetEvent ev = enet.event_create()
defer enet.event_destroy(ev)
i32 rc = enet.enet_host_service(server, ev, 0)
if rc > 0 && enet.event_type(ev) == u32(enet.EventType.Receive):
    ^enet.ENetPacket pkt = enet.event_packet(ev)
    [256]char8 buf
    u64 n = enet.packet_copy(pkt, rawptr(&buf[0]), 256)
    enet.enet_packet_destroy(pkt)
```

## Layout

```
vendor/enet/
  LICENSE               upstream MIT text + vendoring note
  enet.h                the vendored single header, unmodified except for
                         enet-c99-pedantic.patch's four sites and
                         enet-timeout-shift-ub.patch's one (see below)
  enet.c                implementation TU: #define ENET_IMPLEMENTATION,
                         #include "enet.h" — the sokol_time.c pattern
  enet.dado              the binding: a hand-curated pass over a
                         binding-generator draft, not the raw output
                         — and, since 2026-09-13, the whole of the Dado side.
                         `enet_shim.h`/`.c` USED TO SIT HERE and are deleted;
                         see "The shim is gone" below
  enet_test.dado         @test: a real client/server loopback exchange,
                         not just a compile check
  enet-c99-pedantic.patch  the four-site strictness patch, as a diff for
                         reference (already applied to enet.h)
  enet-timeout-shift-ub.patch  the one-site correctness patch, likewise
                         already applied — an unbounded left shift in
                         enet_protocol_check_timeouts (see below)
  link                  no link flags on Linux/macOS
```

One `package enet` — no per-header split the way `vendor:sokol` has one
package per `sokol_*.h`, since ENet ships as a single header to begin with.

## The shim is gone (2026-09-13)

**`enet_shim.h` and `enet_shim.c` are deleted.** Fifteen hand-written C
functions gave `ENetAddress` and `ENetEvent` a heap constructor and one
accessor per member, because neither struct could be restated in Dado. **Both
are restated now**, so the accessors are Dado one-liners over a member read and
the C had nothing left to do. `enet.dado`'s own doc comment carries the
accounting, and `enet.h` is the only C left in this directory besides `enet.c`.

**The blocker that was written down for years was real and was not this
package's.** glibc's dual-stack `struct in6_addr` reaches its bytes through
`#define s6_addr __in6_u.__u6_addr8` — a macro that eats a member name the
emitter has to write into a declaration, which Dado genuinely cannot say and
which `@c("s6_addr")` cannot rescue, because `@c` is precisely the thing that
tells the layout-checking shadow struct which C name to write. **But that is
the dual-stack shape, and this build is not dual-stack.** The `foreign` line in
`enet.dado` carries `define "ENET_IPV4_ONLY=1"`, which puts the Dado translation
unit in the same mode `enet.c` compiles in, under which `ENetAddress.host` is a
`struct in_addr` whose `s_addr` is an ordinary member. **The two sides of this
package had been compiled against different structs all along; the fix was to
stop doing that.**

## Not a raw `bindgen` dump

Dado's binding generator ran against the vendored `enet.h` first and got
189 of 212 declarations across clean (package `enet`, into a scratch directory).
`enet.dado` keeps only the ~30-function **public API** (`enet_initialize` on)
plus five opaque handles; it drops ENet's internal protocol/command/list
machinery bindgen dutifully translated (dead surface no caller outside
`enet.c`'s own translation unit can reach) and the raw socket primitives
(`core:net` drives ENet through `ENetHost`, never the BSD socket
underneath). `enet.dado`'s own doc comment has the full accounting of what
bindgen skipped and why each skip was the *right* answer here rather than a
`dadoc` bug to chase — see it before touching this package.

## IPv4-only — a deliberate vendoring choice

Upstream defaults to dual-stack IPv6. This build turns that off
(`ENET_IPV4_ONLY`): verified
directly against raw BSD sockets that this project's own build sandbox has
no IPv6 support at all (`socket(AF_INET6, …)` itself fails,
`EAFNOSUPPORT`), which minimal/CI containers commonly do, not just this one.
`enet.c`'s own doc comment has the full reasoning.

~~Set identically in `enet.c` and `enet_shim.c`… safe for every consumer
*except* `enet.c`/`enet_shim.c` themselves to disagree about (`ENetAddress` is
fully opaque in `enet.dado` — no Dado-emitted code ever computes its size). One
macro, in both `.c` files, reverts to dual-stack.~~ **`enet_shim.c` is deleted
and `ENetAddress` is no longer opaque**, so the arrangement is different now and
the difference is the point: it is set in **`enet.c` and on `enet.dado`'s
`foreign` line**, as `define "ENET_IPV4_ONLY=1"`, and **the Dado side now has to
agree rather than being free to disagree** — it restates `ENetAddress` and
therefore computes its size. Two places, still one macro, and reverting to
dual-stack means flipping both together. **A `define` clause is program-global**
— it is emitted above every `#include` in the single translation unit a Dado
program is — so linking this package defines `ENET_IPV4_ONLY` for every package
beside it. Nothing outside this directory names that macro, so it costs nothing
today; it is the property to check before adding a second `define` here.

## The `enet-c99-pedantic.patch` sites

~~This project builds `-std=c99 -Wpedantic -Wextra -Werror`~~ — **`-std=c11`
since 2026-09-11** (`CFLAGS_BASE` (`dado`) is the live answer and reads
`-std=c11 -Wall -Wextra -pedantic`; `-Werror` is on by default from
`CFLAGS_WERROR` (`dado`) and `--no-werror` is what turns it off). The
patch's name keeps the older standard's name and that is fine: **it is the
name of a patch file, not a claim about the build.** The vendored
header, as shipped, does not compile clean under those flags (it assumes at least
gnu99-level laxness in a few spots). Four sites, each the smallest possible
edit, each commented in place in `enet.h` with `Dado project patch` and a
one-line reason:

1. `ENetPacket`'s forward-then-full `typedef` (redefining the same typedef
   name to an equivalent type is a C11 feature) — the full definition now
   completes the already-forward-declared `struct _ENetPacket` tag instead
   of re-typedef'ing.
2. `enet_time_get()` → `enet_time_get(void)` (`-Wstrict-prototypes`: `()` is
   an unspecified parameter list in C, not "takes none").
3. `enet_peer_reset_outgoing_commands`'s unused `peer` parameter — silenced
   with `(void) peer;`, the standard idiom, after confirming it really is
   unused.
4. `enet_packet_set_free_callback`'s object-pointer-to-function-pointer
   cast (ISO C does not guarantee they're interconvertible; POSIX does, and
   this whole codebase already assumes POSIX) — scoped
   `#pragma GCC diagnostic ignored "-Wpedantic"` around the one cast, logic
   unchanged.

None of the four touch ENet's actual logic — every one is a compiler-strictness
accommodation, verified by an exhaustive standalone `-Wpedantic -Werror`
compile (`-ferror-limit=0`/`-fmax-errors=0`, so nothing past the first error
was hiding) under both `clang` and `gcc` before and after.

## The `enet-timeout-shift-ub.patch` site

**This one is not a strictness accommodation — it is a correctness fix**, and
it is kept in its own patch file for that reason. One site, in
`enet_protocol_check_timeouts`.

The retransmit test read

```c
((1u << (outgoingCommand->sendAttempts - 1)) >= peer->timeoutLimit && …)
```

and the shift count is unbounded. `sendAttempts` is an `enet_uint16` that this
file increments once per retransmit and never caps, so at 33 attempts the count
reaches 32, and shifting a 32-bit `unsigned int` by its own width is undefined
(C99 §6.5.7p3). At 0 attempts the count is `-1`, undefined for the same reason.

**Reachable in this fork specifically, and that is the part worth reading.**
Upstream ENet doubles `roundTripTimeout` on every timeout, so 33 retries would
take 2³³ ms — about 272 years, and unreachable in practice. This fork replaced
that with a constant, five lines below the patched site:

```c
/* Replaced exponential backoff time with something more linear */
outgoingCommand->roundTripTimeout = peer->roundTripTime + 4 * peer->roundTripTimeVariance;
```

With no doubling, retries fire at a fixed interval of `roundTripTime + 4 ×
variance` — single-digit milliseconds on loopback — so the count passes 32 in
well under a second, long before the five-second `timeoutMinimum` gate can open
and disconnect the peer. **Any peer that stops acknowledging under a live
reliable send reaches it**: a crash, a `kill -9`, a pulled cable, a closed lid.
`example/net` met it on loopback the first time it ran an exchange long enough,
and the graceful-disconnect handling added there is worth having on its own
merits but does not fix this — it only avoids the trigger for a client that
exits cleanly, which is the one failure mode a reliability layer is not for.

**Measured, because "undefined" understates it.** The same expression, over
attempts 30…40 with the default `timeoutLimit` of 32 — `1` means *disconnect
this peer*:

```
gcc   -O0  BEFORE: 1 1 1 0 0 0 0 0 1 1 1
clang -O0  BEFORE: 1 1 1 0 0 0 0 0 1 1 1
gcc   -O1  BEFORE: 1 1 1 0 0 0 0 0 1 1 1
clang -O1  BEFORE: 1 1 1 1 1 1 1 1 1 1 1
gcc   -O2  BEFORE: 1 1 1 1 1 1 1 1 1 1 1
clang -O2  BEFORE: 1 1 1 1 1 1 1 1 1 1 1
             AFTER: 1 1 1 1 1 1 1 1 1 1 1   (every compiler, every level)
```

At `-O0` the count is masked to five bits, so the test **goes false again for
33–37 attempts and true at 38**: the disconnect flaps instead of latching. At
`-O2` the optimiser assumes the count is in range and folds the comparison to a
constant. Both are wrong and they are wrong differently, so a debug build and a
release build disagree about when a dead peer is dropped — which is a bug you
cannot reproduce under a debugger. Under `-fsanitize=undefined
-fno-sanitize-recover=undefined`, the unpatched form aborts with `runtime error:
shift exponent 32 is too large for 32-bit type 'unsigned int'`.

**The edit is a clamp and nothing else.** For 1–32 attempts the value is
bit-identical to what the line always computed; past that it saturates at
`1u << 31`, which restores the property the undefined version lost — the test
is monotone in the retry count, so more retries never un-disconnects a peer —
and 0 attempts now answers false rather than shifting by `-1`. **The linear
backoff itself is deliberately left alone**: it is this fork's own choice,
changing it is a behavioural decision rather than a defect fix, and it is
recorded here so whoever weighs it has the argument. Worth knowing if you do:
without doubling, one dead peer generates thousands of retransmits across the
five-second window instead of a handful, which on a server losing many clients
at once is an amplification at exactly the wrong moment.

**What is and is not verified.** The arithmetic is verified directly, by the
before-and-after table above and by the UBSan abort. The *end-to-end* path — a
real peer going silent under a live reliable send until the counter passes 32 —
is **not** automated: it needs two processes, a killed one, and several seconds
of wall clock, which is not a `@test`. `enet_test.dado`, `core:net`'s and
`app:net`'s suites all pass unchanged, sanitized and not, which is a
no-regression result and not a witness for the fix.

## Verification

`enet_test.dado`'s `@test` opens a real server host and a real client host on
loopback, connects, sends one `Reliable` packet, and asserts the received
bytes match exactly — not a struct round-trip, an actual send across two
independent `ENetHost`s. Green under both `clang` (unsanitized — no
sanitizer runtime in the build sandbox) and `gcc --cc gcc --sanitize`
(ASan/UBSan clean), run repeatedly with no flakiness observed.
