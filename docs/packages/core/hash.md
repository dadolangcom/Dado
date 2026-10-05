<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:hash

core:hash — hashes over bytes. Today: SHA-256.

**Tier N W H.** Pure Dado: no `foreign` block, no `#include` beyond what the
compiler emits for itself, no allocation except `sha256_hex`'s text. It
builds and runs freestanding, on the web and hosted (POSIX and Windows)
unchanged. **Script class S.**

**Thread-safety.** Nothing here is shared: there is no package state. A
`Sha256` is a plain value; one thread at a time may feed a given one, and
any number of threads may each feed their own.

**Error codes: none.** Hashing cannot fail. `sha256_hex` answers `nil` when
its one allocation fails, as every create verb does.

## SHA-256

FIPS 180-4's SHA-256, for checksums — the `SHA256SUMS` an archive is
verified against. **It is not a cryptographic promise**: no constant-time
comparison, no protection of the state's memory, no HMAC. Use it to notice
that bytes changed, not to keep a secret.

Streaming, when the bytes arrive in pieces:

    hash.Sha256 s = hash.sha256_init()
    s.update(first)
    s.update(second)
    hash.Sha256Digest d = s.finish()
    ref []char8 text = hash.sha256_hex(d)
    defer #delete(text)

One shot, when they are all at hand: `hash.sha256(bytes)` or
`hash.sha256_text(s)`. Checking a published sum:
`hash.sha256_equal_hex(d, digits)`, which reads the 64 hex digits in
either case.

The compression function's additions are `u32` additions, which wrap
modulo 2^32 in Dado as in C's unsigned arithmetic — never a trap, with or
without the sanitizer — and its rotations are two shifts and an or.

## Room for the rest

The package is where FNV-1a, a fast 64-bit hash, hash-combine and CRC32
will live beside SHA-256. Each will be named for its algorithm
(`fnv1a64`, `crc32`, …), as `sha256` and its helpers are, so they sit
side by side without a shared state type, `update` or `hex` that would
have to mean two things.

## Declarations

18 declarations, 11 public.

* `type Sha256Digest: [32]u8` — The 32 bytes of a SHA-256 digest, in the order FIPS 180-4 writes them.
* `ref []char8 sha256_hex(Sha256Digest d)` — The digest as 64 lowercase hex digits — the form `sha256sum` prints and…
* `bool sha256_equal(Sha256Digest a, Sha256Digest b)` — Whether two digests are the same 32 bytes. Not constant-time, which a…
* `bool sha256_equal_hex(Sha256Digest d, string8 text)` — Whether `text` is `d` in hex: exactly 64 digits, upper or lower case,…
* `type Sha256` — A SHA-256 in progress. Start one with `sha256_init`, feed it with…
* `void update(^self, []u8 data)` — Feed `data`. Any split of a message across calls gives the digest the…
* `void update_text(^self, string8 text)` — Feed the bytes of `text`.
* `Sha256Digest finish(^self)` — Pad, compress the last block and answer the digest. The state is…
* `Sha256 sha256_init()` — A fresh SHA-256: FIPS 180-4's initial hash value, nothing fed.
* `Sha256Digest sha256([]u8 data)` — The SHA-256 of `data`, in one call.
* `Sha256Digest sha256_text(string8 text)` — The SHA-256 of the bytes of `text`, in one call.
