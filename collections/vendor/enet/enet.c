/* enet implementation TU (single-header lib, compiled once) — the
 * sokol_time.c pattern (collections/vendor/sokol/sokol_time/sokol_time.c):
 * one .c file defines the IMPL macro before including the header so the
 * function bodies exist exactly once in the final link, and every other
 * translation unit (enet_shim.c, and TOY's own emitted C) includes enet.h
 * without the macro and sees declarations only.
 *
 * `enet.h` assumes an ambient POSIX feature level it never asks for itself
 * (unlike sokol_time.h, whose own comment names this exact problem and is
 * why sokol_time.c carries the identical guard below): under this project's
 * `-std=c99` build, glibc hides `clock_gettime`/`CLOCK_MONOTONIC`/
 * `struct timespec` and `getaddrinfo`/`struct addrinfo`/`AI_NUMERICHOST`
 * behind `_POSIX_C_SOURCE`, and without it `enet_time_get`/
 * `enet_address_set_host_new`'s DNS path fail to compile at all (undeclared
 * identifiers, incomplete `struct timespec`/`addrinfo`). Defined only on
 * non-Apple/non-Windows for the same reason sokol_time.c scopes it: macOS's
 * libc exposes these unconditionally and Windows has no POSIX feature-test
 * macros to set. */
#if !defined(__APPLE__) && !defined(_WIN32)
#  ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 200809L
#  endif
#  ifndef _DEFAULT_SOURCE
#    define _DEFAULT_SOURCE 1
#  endif
#endif

/* IPv4-only, deliberately — this project's own vendoring choice, not
 * upstream's default (zpl-c/enet defaults to dual-stack IPv6, `in6_addr`
 * throughout). Verified directly against raw BSD sockets, not just ENet:
 * `socket(AF_INET6, SOCK_DGRAM, 0)` itself fails with `EAFNOSUPPORT` in the
 * sandbox this project builds and tests in, which is not an ENet bug or a
 * one-off — minimal/CI containers commonly ship without IPv6 at all, and a
 * networking layer that cannot bind without it is not portable to them.
 * ENet's own history backs this too: the original lsalzman/enet was
 * IPv4-only for its entire life; zpl-c/enet's IPv6 support is an addition,
 * not a thing this project's networking baseline — no automatic replication,
 * no encryption — needs yet. One macro reverts this if dual-stack is ever
 * required, and it has to be flipped in **two** places that must agree: here,
 * and on `enet.toy`'s `foreign "enet.h"` line, which carries
 * `define "ENET_IPV4_ONLY=1"`. Both translation units read and write
 * `ENetAddress`'s actual bytes and its `sizeof`/layout differs between the two
 * modes — 8 bytes IPv4-only, 20 bytes dual-stack.
 *
 * TOY-emitted consumer C `#include`s this header too, via every
 * `foreign "enet.h":` block, and it is in **this** mode rather than the
 * header's default. A `define` clause on a `foreign` block is emitted above
 * every `#include` in the emitted translation unit, which is the channel this
 * comment used to say did not exist; it is the same clause `core:time` and
 * `core:os/proc`'s `drain` already stand on. `enet.toy` restates
 * `ENetAddress` transparently over that clause, so the layout is checked by
 * `dado_assert__sizeof__ENetAddress` and `dado_assert__offsetof__ENetAddress__*`
 * on every build rather than left to two translation units agreeing by
 * accident — which is what deleted the hand-written accessors that used to
 * stand between TOY and this struct.
 *
 * Note what the clause costs: it is **program-global**, so a program linking
 * this package defines `ENET_IPV4_ONLY` for every other package in it. Nothing
 * outside this directory names the macro, so today that is free. */
#define ENET_IPV4_ONLY
#define ENET_IMPLEMENTATION
#include "enet.h"
