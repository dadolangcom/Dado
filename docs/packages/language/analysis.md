<!-- dadoc 1.0.0-rc.10.2 -->
<!-- self-hosted (tools/dadoc) -->
# language:analysis

LNT0825: a package-level channel walked from two thread roots (the entry
point, and each function handed to `threads.spawn`), unless released.

## Declarations

284 declarations, 27 public.

* `void consumers(^check.World w)`
* `type Fails`
* `void fail_codes(^check.World w)`
* `type Flow`
* `Flow flow(^ir.Fn f, i64 bits, rawptr ctx, void(rawptr, ^Flow, u32, ^ref []u64) step)`
* `void done(^Flow fl)` — Gives back everything `fl` holds.
* `bool has(^ref []u64 s, i64 i)` — Bit `i` of a state (fact bits start at 1).
* `void set(^ref []u64 s, i64 i, bool on)`
* `void must_bit(^Flow fl, i64 i)`
* `bool reached(^ref []u64 s)`
* `ref []u64 entry(^Flow fl)` — The entry state: reached, no fact.
* `void join(^Flow fl, ^ref []u64 into, ^ref []u64 s)` — `into` joined with `s`.
* `bool walk(^Flow fl, ^ref []u64 s)` — The whole body from `s`; answers whether its end is reached.
* `type Own`
* `type Sum` — `fi` moves the block at `j^` + fields `sfx[lo..<hi]` (`verb`); with `i`
* `type GSum` — Body `fi`: G_MOVE package handle `key` (`verb`; `ty` its type), G_ANS a…
* `void own(^check.World w)` — Every body's findings, after the summaries settle.
* `void raises(^check.World w)`
* `void prune(^check.World w)`
* `bool falls(^ir.Fn f)` — Whether control reaches the end of `f`'s body.
* `type Settle`
* `Settle settler(^check.World w)`
* `u32 next(^Settle q)` — The next body to visit, or NONE when every summary has settled.
* `void changed(^Settle q, u32 fi)`
* `void hosted_headers(^check.World w)`
* `void no_arm_for_target(^check.World w)` — WRN0923 (Rust's `no_arm_for_target`): on `#WINDOWS`, each live package with a…
* `void web_spawns(^check.World w)` — ERR1033 on each `threads.spawn` a live body writes, or one outside every…
