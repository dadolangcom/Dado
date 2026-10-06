<!-- The hand-written half of the channels guide.
     The pattern sections are slots filled from `//@` blocks in
     `collections/core/threads/channels.dado`, whose samples the gate compiles, by
     `dado doc collections/core/threads --spine docs/channels-spine.md -o CHANNELS.md`.
     Edit this file or the blocks, never CHANNELS.md. -->

# Channels

A **channel** is a typed, bounded, lock-free mailbox: many producers, one
consumer. `#channel(N) name(params)` declares a queue of `N` messages,
`#channel(latest) name(params)` a mailbox that keeps only the newest. `#send`
hands a message in from any thread and never blocks; the one consumer walks what
is queued with `for … in #peek(ch)` (look) or `for … in #drain(ch)` (take),
and a `#drain` commits when its loop ends, by any exit. Storage is static, or
inline in the value holding the channel, and all-zero bytes are an empty channel:
nothing to initialise, nothing to free, and it works with no operating system.

This guide says when to reach for one, what each side may rely on, how the web
reads the same bytes, the patterns, and — in the appendices — what it costs and
what has been proven. The language's own rules are in `docs/language.md`
(*A channel is a mailbox*) and behind `dadoc --explain` for each code
(`ERR0528`–`ERR0532`, `ERR0614`, `ERR0719`–`ERR0721`, `ERR0823`–`LNT0825`).
**Where this guide and `dadoc` disagree, `dadoc` is right.**

## When to use a channel, and when not

| you have | use | because |
|---|---|---|
| one-way traffic between threads, or across the DadoScript boundary, that can wait until the consumer next looks | **a channel** | no lock, no blocking, typed, ownership checked, and the web reads it with no call per message |
| a question whose answer is needed **now, on this thread** | **a call** (a callback, a function value) | a channel is delivered when the consumer walks it — later, maybe on another thread |
| state keyed by identity that many threads read | **a map**, guarded | a channel is consumed by one reader; it is not a table |
| shared state that must be read, modified and written as one step | **a mutex** (`core:threads`) | a channel moves values; it does not make a read-modify-write atomic |
| state where only the newest value matters — a window size, a setting, a target | **`#channel(latest)`** | writers never fail and never wait; the reader sees the newest value once |

A channel is not a faster mutex. Uncontended on one thread a lock and unlock
can be as cheap (appendix A); what a channel buys is that **no producer ever
waits on another party**, so it is safe from a thread that must not block — an
audio callback, an input backend, a worker in a pool — and that the same bytes
are readable from JavaScript.

## The threading contract

**Who may send.** Any thread, any number at once. `#send` never blocks. A
queue's `#send` fails when the queue is full and is written under `try`: the
producer decides what full means (see *back-pressure*). A `latest` `#send`
cannot fail; when two writers overlap, one wins and the other is **superseded**
— it returns at once, its value is never seen, and it takes effect as though
overwritten at the winner's publication, which may come after it returns.

**Who may walk.** One consumer. `#peek`, `#drain`, `#drain(ch, n)` and the
discard statement (`#drain(ch)` alone on a line) are consumer operations;
`#send` and `#len` are not.

* A **debug build** records the consumer in the channel's owner word at its first
  walk and traps when a walk comes from anyone else, naming the channel and both
  consumers. `--no-assert` strips the check.
* **Statically**, two thread roots — `main`, and each function handed to
  `core:threads.spawn` — that both reach a walk of one package-level channel are
  `LNT0825`: a warning, an error under `--strict`.
* A walk of a channel begun inside a walk of the same channel is refused where the
  compiler sees it (`ERR0823`) and traps in a debug build where it does not
  (through a call).
* **The rule as it stands is "one consumer for the life of the program", which
  is stricter than the language needs.** Thread A drains, finishes, is joined,
  and then thread B drains: nothing runs at once, and the hand-off is legal —
  but the owner word still names A, so B's first walk **traps in a debug
  build**. The lint is a warning for exactly this reason (a hand-off is legal
  and the compiler cannot see the join); the runtime guard has no way to see it
  either. **`#release(ch)`** (R11.10, Sam, 2026-09-29) is how a program says
  so: a consumer statement that clears the owner, so the next thread to walk
  becomes the consumer. It compiles to nothing without assertions, and the
  guard stays; a channel the program releases is not `LNT0825`'s (see
  *Handing consumption to another thread* below).

**What `#len` promises.** How many are queued: claimed and not yet consumed,
clamped to `[0, N]` for a queue; 1 or 0 for `latest` (a value has arrived that no
`#drain` has walked). Exact from the consumer when no producer is running,
approximate otherwise. Any thread may ask.

**What a walk sees.** A snapshot: the entries claimed before it began, in claim
order, and nothing sent during it — so **a walk always ends**. It stops early
at an entry that is claimed and not yet written (a producer mid-`#send`); that
entry and everything after it come on a later walk. Nothing is skipped,
reordered, duplicated or lost. Each producer's messages arrive in the order it
sent them; across producers the order is claim order.

**When a walk commits.** A `#drain` commits on every exit of its loop: running
off the end, `break`, `return`, and a failure
passing out of the body. An entry whose body began is consumed — including the
one that ran `break`. A trap commits nothing.

**What full means.** A producer is told FULL only after it has seen `N` entries
claimed and not committed — queued, being written by another producer, **or
walked by a `#drain` whose loop has not ended**. Precisely: FULL means at least
`N` claims past a head no older than any head that producer read before the
call. (The claim and the head are read one after the other, not as one
snapshot, so FULL can be answered a moment after room appeared; making the two
reads consistent cost 2.2× uncontended on the aarch64 device and was not taken.)

**A consumer must never wait for room in the queue it is draining.** Because a
walk's entries count against the capacity until the loop ends, a consumer that
sends into the channel it is draining, from inside that walk, and retries until
the `#send` succeeds, spins forever. Send into another channel, or keep the
message and send it after the loop.

**Holding a channel in a value.** A `type` may hold a channel as a field; the
value is then one mailbox at one address and is never copied (`ERR0531`) —
pass `^T` or `ref T`. A `#delete` of such a value while its queue still holds
`ref` payloads traps in a debug build. **Known limitation:** the struct asks for
128-byte alignment (each party's words on their own cache line), but a value
made by `#new` is aligned only as the allocator aligns — 16 bytes — so on the
heap the line separation the layout promises does not hold, and C calls an
object at an address its type's alignment does not divide undefined. Package
channels and locals are aligned as asked. Prefer a package-level channel, or a
holder that is not heap-allocated, until allocation honours the type's
alignment.

## The payload: what a message may hold, and who owns it

A message outlives its `#send` — it may be read on another thread, later — so
each parameter is something that can outlive the sender (`ERR0529`):

* **Plain data is copied** into the slot: scalars, `enum`s, `distinct`s, fixed
  arrays, tuples and `type`s of plain data, function values, and `rawptr`
  (whose meaning is yours).
* **A view is refused** — `[]T`, `^T`, a string, a `cstring`, a trait value —
  because it points at storage the sender keeps. Send a `ref`, or copy into a
  fixed array.
* **A map and a channel are refused.**
* **A `ref T` or `ref []T` moves.** A successful `#send` writes the sender's place
  nil, and reading it afterwards is refused (`ERR0824`); on the `else` (full)
  path it is still the sender's, to retry or `#delete`. Under `#drain` the field
  binds as the owning `ref` — the body keeps it or `#delete`s it; under `#peek`
  it binds as the view it lends.
* **A `latest` channel takes no `ref`** (`ERR0530`): it overwrites, and an
  overwritten owner would leak or be freed behind the program's back.
* **Nothing leaks by the channel's hand:** a queue never drops; the discard
  statement frees the `ref`s it discards; and at exit a package channel's queued
  `ref`s are the process's, like any live `ref`.

## The web contract

**Single-threaded by default.** A `#WEB` build (`./dado build --target=web`) runs
the same helpers with every atomic lowered to a plain access, which is correct
because nothing is concurrent — and cheap: a `#send` in wasm measured 3–5 ns
(appendix A). A channel program runs under node through `tools/web/load.mjs`
with the stdout and status of its native build; `corpus/channel_loop` is the
pattern, built to do exactly that.

**Threads are opt-in**, and need the page served cross-origin isolated
(`Cross-Origin-Opener-Policy: same-origin`, `Cross-Origin-Embedder-Policy:
require-corp`) so memory can be shared with workers. On shared memory wasm's
atomics are sequentially consistent, stronger than every order the helpers ask
for. Threaded `#WEB` is not built on this base; nothing here depends on it.

**The layout is read from JavaScript with no call per message.** Each channel
is one C struct; the program's **channel directory** is a table of every number
another party needs — each channel's index, kind, capacity, name, address,
`sizeof`, the offsets of its words and slots, the slot stride, and each payload
field's name, offset, size and kind — every one written by C (`offsetof`,
`sizeof`), none by the compiler. On `#WEB` the module exports
`dado_rt__channel_directory`, which answers the table's address; a host calls it
**once**, and after that every send, walk, look and length is loads and stores
on the module's memory. `tools/web/channels.mjs` is the reference reader — the
helper family transliterated line for line over typed-array views — and
`tests/web/channels/` proves it: JavaScript drains a program's queue and
`latest`, sends into two more that the program drains on its next frame, and
the calls into wasm are four whatever the message count (the directory, two
frames, the console's flush), with stdout byte-identical to the native build.

What the reader derives and what it assumes:

* **Derived from the directory:** every number about a channel.
* **Hard-coded:** the directory's own three structs, whose pointer-width fields
  no `offsetof` describes — the wasm32 layout (4-byte pointers, 8-aligned
  `uint64_t`; an entry is 96 bytes, a field row 24), checked against the table
  (the version, each entry's index); and that a queue slot's message sits
  8 bytes in, after its `seq` — true of every message whose alignment is at most
  8, which is every message of scalars, and checked against each field's
  extent. A directory row for the message's offset in a slot would retire the
  second assumption.
* **Which accesses.** Every word another party writes is read and written with
  `Atomics.*`: sequentially consistent on shared memory, and legal and equivalent
  to a plain access on an unshared one — so the reader has one path, right on
  both. 64-bit counters are `BigUint64Array` elements; a walk counts from the
  head as a Number, so a message costs no BigInt arithmetic while positions are
  below 2⁵³. The payload is plain data ordered by `seq`, read and written
  through a `DataView`, little-endian.
* **A `ref` payload does not cross.** JavaScript cannot own memory the
  program's allocator holds, so the reader refuses to send a `ref` and a drain
  from JavaScript of a channel carrying one would leak it: such a channel is the
  program's to consume.

**DadoScript's check points.** The runtime that delivers channels to DadoScript handlers
tests `CHANNEL_ANY` — bit 0 of `dado_rt__flags` — on every return from wasm and at
the frame: one load through a `Uint32Array` made once, **measured at about 1 ns
over no check** in node 22 (appendix A; an `Atomics.load` costs 8–10 ns there).
When it is set, the watcher **looks** — clears `CHANNEL_ANY`, then exchanges each
word of `dado_rt__channel_bits` with zero — and walks every channel whose bit it
took before it looks again. Clear, then walk: the helpers guarantee that at
quiescence an unconsumed entry leaves its bit and `CHANNEL_ANY` set, so nothing is
missed. The watcher runs on the consumer's thread.

**Natively** (a0.15, `core:script`'s `channels.dado`) a script's `receive(ch,
handler)` makes its VM the channel's consumer, and the VM's thread tests
`CHANNEL_ANY` at its safe points: a Dado function the script called returning
into it, a call into the script returning to Dado, and `#script_pump(vm)`. Several
VMs share the bitmap, so a native watch takes only its own VM's bits —
`fetch_and` with its mask, a word per 64 — and sets `CHANNEL_ANY` again when it
saw a bit of a channel bound to another VM still set.

**One consumer across the boundary.** A channel a DadoScript handler is bound to is
consumed by the DadoScript runtime, and a Dado walk of the same channel is a second
consumer. The owner word says which consumer a channel has:

* `0` — no walk yet;
* a **thread**: the address of that thread's 8-byte per-thread guard object,
  which `dado_rt__chan_self` answers — a multiple of 8, and never below 1024
  (natively the page at address 0 is unmapped; on `#WEB` static data and thread
  storage begin at the linker's global base, 1024);
* a **DadoScript engine**: `8 × (k + 1)` for engine `k`, the multiples of 8 below
  1024 — a range no thread's id can be in, room for 127 engines;
* **bit 0** set while one of the consumer's queue walks is open (debug builds
  only; what makes a nested walk through a call trap).

So a debug build catches the conflict at run time from either side: the
engine's walk finds a thread's id and refuses, and a Dado walk finds the
engine's and traps, naming it as the first consumer. The static diagnostic
belongs to the DadoScript runtime's reach scan, which sees both the `receive` and the
walk. `tests/web/channels/` exercises both directions.

## Patterns

Each sample is a package of its own and compiles as written.

<!--@ channels/fan-in -->

<!--@ channels/per-frame -->

<!--@ channels/jobs -->

<!--@ channels/request-response -->

<!--@ channels/latest -->

<!--@ channels/hand-off -->

<!--@ channels/back-pressure -->

The whole shape in one program — workers fanning in through a queue, an input
source through a second, a `latest` setting, a frame loop draining all three,
and a `when #OS == #WEB:` arm that runs the producers a step at a time on the
page's one thread — is `corpus/channel_loop`, which prints the same bytes
natively and under node.

## Appendix A — what it costs

### Native, quoted from `proofs/channels/bench/RESULTS.md`

Measured 2026-09-28 against the helpers at md5 `8470efd63e744121a568119b8ff054be`
(the header as of commit `d3bcc8b`, unchanged in the queue and `latest` helpers
since; only the debug owner guard has changed). Per message, median of three
runs unless marked ¹ (one run). N = 1024.

**Machines.** *aarch64 device — carries the conclusions:* a 4-vCPU Apple-silicon
Linux VM, Ubuntu 22.04, **gcc 11.4 `-O2`** (LSE atomics), Go 1.24.7. *x86-64
sandbox — recorded, no conclusion:* the 2-vCPU agent VM (Xeon @ 2.1 GHz), gcc
13.3, go 1.24.7, rustc 1.95, shared with three other agents.

The bar (Q4): beat a mutex-guarded ring uncontended; match or beat Go channels
contended. **Met on the aarch64 device, with one exception each way.**

Uncontended, one thread sending a batch and then draining it (aarch64, ns):

| payload / batch | channel | mutex + ring | Go channel |
|---|---|---|---|
| 8 B, batch 512 | **3.81** | 4.97 | 28.7 |
| 64 B, batch 512 | **4.73** | 6.10 | — |
| 256 B, batch 512 | **8.89** | 9.74 | — |
| 8 B, batch 16 ¹ | **3.49** | 5.65 | 28.9 |
| 8 B, batch 1 (send, drain, repeat) | 10.33 | **10.05** | 30.0 |

Contended, P producers and one consumer spinning on `#drain`, 8 B (aarch64, ns):

| P | channel | mutex + ring | Go channel |
|---|---|---|---|
| 1 | **40.8** | 59.6 | 62.8 |
| 2 | **39.8** | 60.8 | 87.2 |
| 3 | 61.7 | **56.4** | 67.2 |

Draining once per frame, 8 B, bursts of 16 at 200 000 messages/s per producer
(aarch64, ¹; producer ns per accepted `#send` / consumer ns per message drained):

| frame | P | channel | mutex + ring | Go channel |
|---|---|---|---|---|
| 1 ms | 1 | **6.1** / 1.95 | 10.1 / 1.68 | 19.1 / 46.0 |
| 1 ms | 3 | **9.7** / 1.94 | 48.5 / 1.58 | 38.1 / 277 |
| 16.6 ms | 1 | **9.7** / 1.24 | 27.7 / 0.99 | 25.7 / 29.1 |
| 16.6 ms | 3 | **22.7** / 1.13 | 276.9 / 0.91 | 57.8 / 274 |

Latency, send → walked, p50 / p99 / p99.9 (aarch64, ¹, ns): P = 1 channel
**125** / 417 / 21 042, mutex 334 / 709 / 13 250, Go 208 / 291 / 23 875; P = 3
channel **125** / 584 / 159 791, mutex 583 / 10 000 / 100 000, Go 208 / 25 500 /
1 875 916. `latest` against a mutex-guarded value, writer ns per `#send` with a
reader spinning: P = 1 **39.1** vs 65.2, P = 2 **28.1** vs 93.5, P = 3 **8.9** vs
169.3.

x86-64 sandbox (no conclusion): batch 512, 8 B — channel 35.0, mutex 15.7, Go
126.1, Rust `ArrayQueue` 73.9, crossbeam `bounded` 69.3, `mpsc::sync_channel`
105.1. Uncontended the channel is 2.2× the mutex on that Xeon VM: the claim's
load-then-CAS pays a store-forwarding stall behind the previous send's locked
instruction (35 ns against 21 ns for the same CAS with a known operand); a
≥ 4-core x86 machine is needed before Q4 is judged there.

**Quoted for scale, not measured here** (sources, dates and exact wording in
`proofs/channels/bench/RESULTS.md`; other machines and workloads, so no
conclusion). moodycamel `ConcurrentQueue` (Desrochers, 2014, an 8-core Xeon
E5506 @ 2.13 GHz), multi-producer single-consumer, time per item dequeued: 62.5
ns at 2 threads and 63.9 ns at 4 without tokens, 55.6 and 45.4 ns with them; the
post's mutex-guarded queue 154.6 and 666.2 ns. It is unbounded and allocates on
growth; a channel is bounded and static. Qt: no published per-message number for
a *queued* (cross-thread) connection was found. Qt's documentation puts a
*direct* connection at about ten times a non-virtual call (2 000 000 signals a
second to one receiver on an i586-500, Qt 3); a queued emission copies its
arguments to the heap and posts a `QMetaCallEvent` to the receiver's event queue
(Goffart, 2016), where a channel's `#send` allocates nothing and takes no lock.

### The web path, measured here (row 2.21)

node **v22.22.2** on the x86-64 sandbox (Intel Xeon @ 2.10 GHz, 2 vCPU, shared
with other agents), the module built by `./dado build --target=web` (clang 18,
`-O2`, wasm32, single-threaded) at this guide's commit; `proofs/channels/run.sh
web` reproduces it. Two sessions, load average about 1–3 and about 3–5; medians
of three runs each.

| per return from wasm (10⁸ calls of a trivial export) | session 1 | session 2 |
|---|---|---|
| no check | 3.49 ns | 3.75 ns |
| `CHANNEL_ANY` through a cached `Uint32Array` | 4.80 ns (**+1.32**) | 4.91 ns (**+1.16**) |
| the same load as `Atomics.load` | 11.85 ns (+8.37) | 14.00 ns (+10.25) |

For scale, quoted from a0.16's boundary table: a JavaScript → wasm call costs
6.7–7.4 ns. The check a0.15 plans is about a nanosecond, as it assumed; an
`Atomics.load` in that position costs as much as the call, so on shared memory
a plain load as the hint, followed by the look's atomic read-modify-writes, is
the shape to keep.

| per message: wasm `#send` of 8 bytes, then a JavaScript drain | session 1 | session 2 |
|---|---|---|
| K = 512 per frame, the reference `drain` (one object per message) | 64.2 ns (send 2.9 + drain 65.5) | 120.7 ns (4.8 + 104.5) |
| K = 512 per frame, the same walk inlined, fields from an `Int32Array` | **21.8 ns** (send 3.4 + drain 22.5) | 38.5 ns (3.7 + 31.7) |
| K = 1 per frame, reference | 366 ns | 733 ns |
| K = 1 per frame, inlined | 527 ns (runs 623 / 527 / 365) | 244 ns |

(The send/drain split is timed per frame and includes the clock; the total is a
separate run without it.) A `#send` in single-threaded wasm is 3–5 ns — a claim,
a copy and a store. A JavaScript drain is dominated by per-message work in
JavaScript: the BigInt `seq` load and, for the reference, building an object per
message; a generated drain specialised per channel is the floor to aim at. With
one message per frame the per-walk overhead (the owner word, the begin, the
commit: about ten 64-bit `Atomics` operations) is not amortized. No call into
wasm is made per message in any row.

## Appendix B — what has been proven, and how far

The helpers checked are `crates/dado_emit_c/src/rt/channel.h`'s queue and
`latest` family at md5 `8470efd63e744121a568119b8ff054be`; those functions are
unchanged since. **The debug owner guard was rewritten after the checking**
(the open-walk bit, `dado_rt__chan_open`/`dado_rt__chan_close`); it is debug-only,
touched only by the consumer except for the first walk's CAS, and is covered by
the tests, not by the model checkers. The written proofs are
`proofs/channels/PROOF.md`; how to run each piece is `proofs/channels/README.md`.

| checker | what it checked | bounds | result |
|---|---|---|---|
| **shared trace test** | the Rust transliteration against the C, helper answer and state word after every op | 3 000 random scripts, 166 387 ops | identical — which is what makes loom's results about the C |
| **loom** (0.7.2, mandatory) | queue: payload races, torn reads, FULL, `#len`, `#peek`, no lost wake-up, per-producer exactly-once in order; `latest`: races, tears, per-writer order, lost wake-up, linearizability against a sequential register with superseded writes | exhaustive **under a preemption bound of 2** (3 for `latest`); unbounded did not finish P = 2, M = 1, N = 1 in 10 minutes | P = 2 × M ∈ {1, 2, 3} × N ∈ {1, 2, 3}, every script: **pass**; P = 3 × M = 1 × N = 1 × `#drain`: pass; **the rest of P = 3 did not finish on the loaded VM and was stopped — not run, not a pass**; `latest` W ∈ {2, 3} × M ∈ {1, 2} and `linearizable`: pass |
| **GenMC** (v0.19.0, primary) | the C itself, exhaustively for the bounded threads, under RC11 and IMM (a hardware model covering ARMv8 and POWER); its race detection is the payload-race check | weak CAS made strong for GenMC only (sound) | RC11 queue: P = 2 × M = 1 × N ∈ {1, 2, 3} over seven scripts, P = 2 × M = 2 × N = 1 over three, N = 2 × `#drain`, `latest` W = 2 × M = 1 — **every configuration run passes**; IMM: P = 2 × M = 1 × N = 1 over three scripts pass; **P = 2 × M = 2 × N = 2 × `#peek`-then-drain timed out at 20 minutes — not a pass** |
| **CBMC** (5.95.1) | the same C harnesses under SC; weak memory by instrumentation | `--unwind` | SC, P = 2 × M = 1 × N = 1 over three scripts: pass. **At N = 2 CBMC disagrees with everything else**: on the unmodified header it reports seven violations, including a torn read and a message taken twice, in an execution with two read-modify-writes of `tail` both reading 3 — which no SC execution has; GenMC (SC and RC11) and loom find nothing in the same shape. The likeliest cause, unconfirmed, is CBMC's `<stdatomic.h>` model passing results through temporaries it treats as shared. **CBMC's N = 2 answers are discounted**, catches included. Weak memory: the fence shim is validated on message passing, but `goto-instrument --mm` could not instrument the queue (an invariant failure on a global struct member; the scalarized, inlined harness's cycle search did not finish in 10 minutes under `--mm tso`) |
| **TSan** | P = 3 producers + 2 `latest` writers, every walk kind | N ∈ {1, 2, 3, 7, 64, 1024} × 3 runs | x86-64: gcc 13.3 **18/18**, clang 18.1.3 **18/18** clean (sanitized runtime linked); aarch64 device, gcc 11.4, 2·10⁶ messages × 6 seeds: **36/36** |
| **stress**, unsanitized | per-producer sequences with no gap, duplicate or reorder; FULL; `#len`; no lost wake-up at quiescence; `latest` | **10⁸ messages per N** | x86-64 **6/6**; aarch64 device **6/6** (79.5–174 ns per message) |
| **litmus**, aarch64 | message passing, store buffering, the wake-up shape, IRIW | 5·10⁶ (IRIW 2·10⁶) | store buffering 4 274 484 weak with plain accesses, **0** with the wake-up's RMWs; the rest 0 even relaxed on this hardware — so the release/acquire pairs are not exercised by reordering there, and their evidence is GenMC and loom |
| **mutants** | one weakened order or removed step each, 18 | every checker above | **every mutant that breaks a property is caught by at least one checker**; the two missed by all break nothing (`PROOF.md`). Weakening the raise to relaxed is caught **only by GenMC under IMM** — the reason the IMM pass exists |

**What the proofs found, stated rather than hidden.** FULL is decided on two
reads that are not one snapshot (loom pins an execution with `p − h = 2` at
N = 1), so the guarantee is the weaker one under *what full means* above. The
first-drafted claim — reading `tail` relaxed and `head` after it — answered FULL
on an empty queue; GenMC and the aarch64 stress run both caught it, and the
reload that fixes it is in the helpers. A superseded `latest` `#send` can return
**before** the winning writer publishes, so a reader in between sees neither
value (loom pins it); every other `latest` operation linearizes. The consumer's
take on `latest` is lock-free, not wait-free.

**Tested beyond the model:** `core:threads`' channel `@test`s (many producers, every walk kind, `ref` payloads, the guards) — `./dado test --sanitize=thread --cc gcc collections/core/threads`, 6 passed, sanitized, at this guide's commit; the fan-in and `core:jobs` samples above, given a `main`, TSan-clean under gcc 13;
`corpus/channel_loop` — three worker threads and an input thread against the
frame loop — TSan-clean under gcc 13 (3/3) and clang 18 with its TSan runtime
(3/3), and byte-identical under node; the JavaScript reader against the native
program in `tests/web/channels/`.
