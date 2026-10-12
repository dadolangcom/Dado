<!-- dadoc 1.0.0-rc.4 -->
<!-- commit fa29e0260488 -->
# core:hash

core:hash — hashes over bytes: the SHA family and HMAC for digests,
and FNV-1a, CRC-32, SipHash-2-4, xxHash64 and hash-combine for tables,
formats and fingerprints.

**Tier N W H. Script class S.** Pure Dado: no `foreign` block, no
`#include` beyond what the compiler emits for itself, no allocation except
`hex`'s text. It builds and runs freestanding, on the web and
hosted (POSIX and Windows) unchanged, and answers the same bits on each.

**What raises into a script.** Every package-level function, each
promised by the `#assert_raises` on the line above it; the generic
digest verbs are checked at each instantiation a program makes, and the
tests make one at each digest size. The methods of
`Sha1`, `Sha256` and `Sha512` cannot carry the promise themselves, so
`raises_test.dado` makes it for them through a private function that
calls each one. Nothing is left out.

**Thread-safety.** Nothing here is shared: there is no package state. A
`Sha256` (or `Sha1`, `Sha512`) is a plain value; one thread at a time may
feed a given one, and any number of threads may each feed their own.

**Error codes: none.** Hashing cannot fail. `hex` answers `nil` when its
one allocation fails, as every create verb does.

**Which to reach for.**

* Checking that bytes are the bytes someone published: `sha256` (or
  `sha512`; `sha1` only where a format already names it, as git does).
* A tag that proves a message came from someone holding a key:
  `hmac_sha256` or `hmac_sha512`.
* A hash table whose keys an attacker chooses: `siphash24` with a secret
  key.
* A hash table, cache key or fingerprint over keys nobody attacks:
  `xxh64`, or `fnv1a64` for short keys and the smallest code.
* The checksum zip, gzip and PNG store: `crc32`.
* A record's hash from its fields' hashes: `combine64`.

Each is named for its algorithm, so they sit side by side without a shared
state type or `update` that would have to mean two things. The digests
share one set of verbs, `hex`, `digest_equal` and `equal_hex`, which take
any of the three. Text has a `_text` form of each one-shot function that
hashes the string's bytes. `_init` answers a fresh state.

**None of it is a cryptographic promise.** The digests and HMACs answer
the standard's bits, but nothing here compares in constant time, wipes a
state's memory, or was written against side channels. Use them to notice
that bytes changed and to check a tag a protocol hands you; keep secrets
with a library audited for it. SHA-1 is broken for collisions: use it to
interoperate, never to decide that two documents are the same.

## SHA-1, SHA-256, SHA-512

FIPS 180-4. Streaming, when the bytes arrive in pieces:

    hash.Sha256 s = hash.sha256_init()
    s.update(first)
    s.update(second)
    hash.Sha256Digest d = s.finish()
    ref []char8 text = hash.hex(d)
    defer #delete(text)

One shot, when they are all at hand: `hash.sha256(bytes)` or
`hash.sha256_text(s)`. Checking a published sum:
`hash.equal_hex(d, digits)`, which reads the hex digits in either case.
`sha1` and `sha512` have the same verbs, and their digests the same
`hex`, `digest_equal` and `equal_hex`.

The compression functions' additions are `u32` (`u64` for SHA-512)
additions, which wrap in Dado as in C's unsigned arithmetic — never a
trap, with or without the sanitizer — and their rotations are two shifts
and an or.

## HMAC

RFC 2104 over SHA-256 and SHA-512, tested against RFC 4231's vectors:
`hmac_sha256(key, message)` answers a `Sha256Digest`.

## The table hashes

`fnv1a32` and `fnv1a64` (with `_update` forms to carry a hash across
pieces), `crc32` (with `crc32_update`, zlib's convention),
`siphash24(key, bytes)` (the reference implementation's vectors),
`xxh64(seed, bytes)` (the reference `XXH64`), and `combine64(seed, h)`.
A key, seed or running hash always comes first.

## Declarations

86 declarations, 47 public.

* `const u32 FNV1A32_OFFSET = 0x811c9dc5` — FNV-1a's 32-bit offset basis: the hash of no bytes, and where a running…
* `const u64 FNV1A64_OFFSET = 0xcbf29ce484222325` — FNV-1a's 64-bit offset basis: the hash of no bytes, and where a running…
* `u32 fnv1a32([]u8 data)` — The 32-bit FNV-1a of `data`.
* `u32 fnv1a32_text(string8 text)` — The 32-bit FNV-1a of the bytes of `text`.
* `u32 fnv1a32_update(u32 h, []u8 data)` — `h` carried on over `data`: `fnv1a32_update(fnv1a32(a), b)` is the hash…
* `u64 fnv1a64([]u8 data)` — The 64-bit FNV-1a of `data`.
* `u64 fnv1a64_text(string8 text)` — The 64-bit FNV-1a of the bytes of `text`.
* `u64 fnv1a64_update(u64 h, []u8 data)` — `h` carried on over `data`: `fnv1a64_update(fnv1a64(a), b)` is the hash…
* `u32 crc32([]u8 data)` — The CRC-32 of `data`: the reflected polynomial `0xEDB88320` that zlib,…
* `u32 crc32_text(string8 text)` — The CRC-32 of the bytes of `text`.
* `u32 crc32_update(u32 crc, []u8 data)` — `crc` carried on over `data`, as zlib's `crc32(crc, buf, len)`: start…
* `u64 combine64(u64 seed, u64 value)` — `seed` mixed with `value`, for hashing a record from its fields' hashes:
* `u64 siphash24([16]u8 key, []u8 data)` — SipHash-2-4 of `data` under a 16-byte `key`, as the reference…
* `u64 siphash24_text([16]u8 key, string8 text)` — SipHash-2-4 of the bytes of `text` under `key`.
* `u64 xxh64(u64 seed, []u8 data)` — xxHash's XXH64 of `data` with `seed` (0 for the plain hash): fast,…
* `u64 xxh64_text(u64 seed, string8 text)` — XXH64 of the bytes of `text` with `seed`.
* `ref []char8 hex([$N]u8 digest)` — The digest as lowercase hex, two digits a byte — the form `sha256sum`,…
* `bool digest_equal([$N]u8 a, [N]u8 b)` — Whether two digests are the same bytes. Not constant-time, which a…
* `bool equal_hex([$N]u8 digest, string8 text)` — Whether `text` is `digest` in hex: exactly two digits a byte, upper or…
* `type Sha256Digest: [32]u8` — The 32 bytes of a SHA-256 digest, in the order FIPS 180-4 writes them.
* `type Sha256` — A SHA-256 in progress. Start one with `sha256_init`, feed it with…
* `void update(^self, []u8 data)` — Feed `data`. Any split of a message across calls gives the digest the…
* `void update_text(^self, string8 text)` — Feed the bytes of `text`.
* `Sha256Digest finish(^self)` — Pad, compress the last block and answer the digest. The state is…
* `Sha256 sha256_init()` — A fresh SHA-256: FIPS 180-4's initial hash value, nothing fed.
* `Sha256Digest sha256([]u8 data)` — The SHA-256 of `data`, in one call.
* `Sha256Digest sha256_text(string8 text)` — The SHA-256 of the bytes of `text`, in one call.
* `type Sha1Digest: [20]u8` — The 20 bytes of a SHA-1 digest, in the order FIPS 180-4 writes them.
* `type Sha1` — A SHA-1 in progress: `sha1_init`, then `update` as the bytes arrive,…
* `void update(^self, []u8 data)` — Feed `data`. Any split of a message across calls gives the digest…
* `void update_text(^self, string8 text)` — Feed the bytes of `text`.
* `Sha1Digest finish(^self)` — Pad, compress the last block and answer the digest. The state is…
* `Sha1 sha1_init()` — A fresh SHA-1: FIPS 180-4's initial hash value, nothing fed.
* `Sha1Digest sha1([]u8 data)` — The SHA-1 of `data`, in one call.
* `Sha1Digest sha1_text(string8 text)` — The SHA-1 of the bytes of `text`, in one call.
* `type Sha512Digest: [64]u8` — The 64 bytes of a SHA-512 digest, in the order FIPS 180-4 writes them.
* `type Sha512` — A SHA-512 in progress: `sha512_init`, then `update` as the bytes…
* `void update(^self, []u8 data)` — Feed `data`. Any split of a message across calls gives the digest…
* `void update_text(^self, string8 text)` — Feed the bytes of `text`.
* `Sha512Digest finish(^self)` — Pad, compress the last block and answer the digest. The state is…
* `Sha512 sha512_init()` — A fresh SHA-512: FIPS 180-4's initial hash value, nothing fed.
* `Sha512Digest sha512([]u8 data)` — The SHA-512 of `data`, in one call.
* `Sha512Digest sha512_text(string8 text)` — The SHA-512 of the bytes of `text`, in one call.
* `Sha256Digest hmac_sha256([]u8 key, []u8 data)` — HMAC-SHA-256 of `data` under `key`.
* `Sha256Digest hmac_sha256_text(string8 key, string8 text)` — HMAC-SHA-256 of the bytes of `text` under the bytes of `key`.
* `Sha512Digest hmac_sha512([]u8 key, []u8 data)` — HMAC-SHA-512 of `data` under `key`.
* `Sha512Digest hmac_sha512_text(string8 key, string8 text)` — HMAC-SHA-512 of the bytes of `text` under the bytes of `key`.
