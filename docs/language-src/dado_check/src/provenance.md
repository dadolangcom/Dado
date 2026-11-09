//@ types/views 30
//@ ### What a view may not outlive
//@
//@ A pointer, a window and a trait value are **views**: Dado did not allocate
//@ what they name and does not keep it alive. So a view is refused wherever
//@ it could outlive its storage, and the compiler follows it to find out —
//@ through names, tuple slots, either arm of a conditional, and calls.
//@
//@ **Storage on a frame** — a local, a by-value parameter, a loop or block
//@ binding — is not returned, not stored through a parameter or into a
//@ package variable, not appended into a handle either reaches, not handed
//@ to a function that keeps it, and not read after the block that declared it
//@ ends (ERR0826; `#stack` storage the same, ERR0821). A function that keeps
//@ a copy of what a pointer points at (`kept = f^`) keeps the copy's views
//@ and not the pointer, so handing it `&b` is refused only when what `b`
//@ holds points at this frame, and so is handing it a pointer local bound
//@ to `&b` and neither assigned nor addressed since (`^const F p = &b;
//@ keep(p)`); what `b` holds counts what was stored into it through
//@ another pointer to it, a pointer to that pointer (`^^F qq = &q;
//@ qq^.rows = local[:]`) included, and what the function itself stores
//@ through its pointer before keeping the copy (`f.rows = r` ahead of
//@ `kept = f^`); through any other pointer, what was stored through that
//@ pointer counts too. Where `b` may have come to point at this frame
//@ through a write the compiler cannot follow — `&b` handed beside a view of
//@ the frame to a function value or a `foreign` function, or a view of the
//@ frame stored through a pointer converted from `rawptr` — handing `&b`
//@ (or such a pointer local) to a function that keeps a copy is warned of (WRN0826),
//@ not refused. Storing through a pointer to package
//@ storage, `put(&f, &SLOT)`, is storing in package storage. **A block behind a
//@ `ref` or a map** is not read through a view after anything that may move
//@ or free it — `#append`, `#resize`, `#reserve`, `#delete`, an insert,
//@ rebinding the handle, or a call whose body grows or frees it — and a
//@ growth whose answer goes to another place (`ys = #append(xs, v)`) leaves
//@ the run it was handed stale as well, as below; a
//@ `defer` counts where it runs, at the scope's exit. **A read the compiler
//@ can prove bad is refused** (ERR0803), and only one that meets every one
//@ of these: **the read touches the block** — a subscript of the view,
//@ pointer or second handle, `^` of it, a slot read through it, a `for` over
//@ it or `#copy` of it when it holds an element (constant bounds that select
//@ one, `a[1..<3]`, or the whole of a `#make` of a constant count above zero,
//@ `a[:]`; a window that may hold none reads nothing), a write through it, or
//@ a second `#delete` — and not
//@ handing it to a call, comparing it, measuring it (`#len`, `#cap`) or
//@ copying it into another binding, which read nothing of the block; **the
//@ value was minted from a `ref` this function holds** (a local or a
//@ by-value parameter, or a slot of a record held so), directly or through
//@ copies, **and the function takes the address of none of them anywhere** —
//@ not of the handle, not of the value read, not of a binding it was copied
//@ through, not even to lend it to a call (`&v`, `&h` of a record holding
//@ one, `&e` handed to a function, a `^self` method called on it) — and
//@ assigns none of them but as a plain `name = value`; **on every path to
//@ the read the block was freed after the mint** — by `#delete`, by a
//@ `defer` that ran, or by a call whose body frees the `ref` it is handed by
//@ value on every way out — with nothing between that could have put another
//@ block there; and **the free gave the block back to an allocator that
//@ frees**. `#delete` frees through the allocator the block was minted on,
//@ so that is a block minted on `#heap` (written on the `#make`, or a `using
//@ #heap` around it), or on the default allocator in the program's `main`
//@ outside every `using`, where `main` is not written `public` or `export`
//@ and nothing in the program calls or names it — and in either case only on
//@ a target with a heap: on `#NONE` no allocator frees (`#make` answers an
//@ empty `ref` and `#delete` of it does nothing). In an arena or the scratch
//@ `#delete` frees nothing, and the default allocator of any other function,
//@ or of a library's `main`, is its caller's, who may have opened a `using`
//@ of one around the call. **Every other such read is a warning** (WRN0803),
//@ `--strict` included, because it is possible and not proven: a stale value
//@ handed to a call, compared or measured, a binding whose address is taken
//@ (through which it may have been rebound), a free on some paths only (a
//@ branch, a loop that may not run, the right side of `&&` or `||`, one
//@ arm of `x if c else y`), a free
//@ through an allocator the compiler cannot show frees, a read in a branch
//@ a constant rules out (`if DEBUG:` of `const bool DEBUG = false`), a
//@ growth, which may have stayed in place, a callee that only may free, a
//@ function value, a free through another copy of the handle, a place
//@ handed as `&a` to a call that answers the pointer back (`q = id(&a)`,
//@ through which another block may be put there), a walk, a walk or
//@ `#copy` of a window that may hold no element, a view returned
//@ past a `defer` that frees it (the caller may only measure it), or
//@ anything below the compiler follows only approximately. The order is the
//@ language's: a subscript's brackets run before its element is read, and
//@ an assignment's place before its value, so `v[consume(a)]` and
//@ `b[consume(a)] = v[1]` read after the free, and `b[v[0]] = consume(a)`
//@ before it. The warning
//@ lets the program build, and **a dev build traps on the read if it
//@ happens**: Dado's sanitizer, on in every build but `--release` (which
//@ `--sanitize` keeps it in) and dropped by `--no-sanitize`, keeps a
//@ witness beside each warned view, pointer or handle copy declared in the
//@ body (`[]i32 v = a[:]`, not a parameter) whose address is never taken. A
//@ `#delete`, a growth that moved the block (not one that stayed in place)
//@ or an `arena.reset` or `arena.destroy` of the arena that minted it
//@ records itself there when the block it frees holds the address the
//@ binding points at, and a read through the binding (an element, a field,
//@ a walk) then stops the program with the warning's message, the read's
//@ place and the free's. A view or pointer minted from such a binding after
//@ the free (`w = v[1..<2]`, `p = &v[0]`, `w = v`) carries its record on. A
//@ `#delete` or a growth records only when it gave the block back to C's
//@ heap, which the run checks of the handle's allocator: in an arena or a
//@ bump `#delete` frees nothing and a growth leaves the old block there, so
//@ neither traps, and on `#NONE` no free records. A block still alive never
//@ traps: a free on a path the run did not take, a handover, a `#send` or a
//@ rebinding leaves the read alone, and so does handing the binding on,
//@ measuring it or comparing it, which reads no byte of the block. What it
//@ does not follow — a callee's free, a scratch frame's rollback,
//@ `arena.end` of a mark, a free through an allocator other than C's heap, a
//@ map's growth, a parameter — is left to C's AddressSanitizer (`--sanitize`
//@ with a C compiler that has one).
//@ Everything below that
//@ this section says of a stale view or handle is that warning, or that
//@ error where the free is proven. A view a
//@ call answers is minted at the call: when `items()` answers `store[:]` of a
//@ package-level `ref`, its result is not read after a call that grows
//@ that `ref`, as `store[:]` written in place is not. A copy of a handle
//@ shares its block: when either moves, the other is stale, and rebinding
//@ the place it was copied from hands the block to the copy. A `ref` a
//@ call answers after filling it with views of an argument — `split(b[:])`
//@ appending `b`'s sections, in place or through a `^ref []T` — is a block
//@ of its own: reading through it, a copy of it or a view of it after `b`
//@ moves is warned about, and measuring or freeing it is not. A handle a call
//@ answers may be a copy of a handle its argument reaches — the package
//@ `ref` it hands back, `p^` for `grab(&b)` — and is stale when that one
//@ moves. A pointer or window a call answers out of one handle slot of a
//@ record (`&e.pool[i]`, `e.keys[0..<n]`, `&e.polys[i].pts[j]`, or another
//@ call's such answer handed straight back, `return at(E, i)`) is tied to
//@ that slot, as the same access written in place is, and not to the
//@ record's other slots, while no two of those slots may hold one block.
//@ Once two may — one written from another, here or in a callee, before or
//@ after the answer was taken, or a record a function answers without
//@ building it from distinct blocks (`return (x, x)`; one handle beside
//@ fresh blocks, `return (pool, #make([]i32, 0))`, is distinct) — a move of
//@ any of them reaches the answer; a function that relies on a record handed
//@ in holding its slots apart is warned of at a call that hands it one whose
//@ slots may share, a call through a function value included, which is
//@ checked against every function the program takes as a value. The tie is
//@ made only for a record the function sees every write to: a local held by
//@ value, a parameter, or a pointer local bound from a pointer parameter by
//@ name (`^Eng F = E`) and neither assigned nor addressed after, which names
//@ the parameter. Through any other pointer local (`F = id(E)`, `F = E` on one
//@ branch, `me = self` in a method that does not spell `self`), a subscript
//@ or package storage, the answer is tied to every handle the record holds,
//@ and a callee that may write the record through one may write any record.
//@ One answered from several slots, from a pointer slot,
//@ or from a slot the callee rebinds is tied to every handle the record
//@ holds. A record written from `ref` locals (`Two t = (a, b)`) holds the
//@ handles: a move through its slot (`t.a = #append(t.a, 2)`, `#delete(t.a)`)
//@ leaves the local stale, not the record, unless another slot of the
//@ record views, points into or holds that block (`R t = (a, a[0..<2])`),
//@ or a value stored in the record since, or a call handed it, may, or the
//@ function takes the local's address (`&a`); a move of the local leaves the
//@ record stale. A binding declared in a loop's body is a new block each pass,
//@ so a copy kept from the last pass is not the next pass's. A `ref`
//@ passed by value to a function that frees it is gone after the call, and
//@ one passed by value to a function that grows it is stale after the call
//@ (`zs = push(ys, 1)`; `ys = push(ys, 1)` is not). A growth written into
//@ another place than the run it grows (`ys = #append(xs, v)`, `ref []T ys =
//@ #append(xs, v)`, `return #append(xs, v)`, a growth handed to a call) moves
//@ the run into its answer: the run handed in is stale after it, an error to read on every
//@ path and a warning on some, until it is rebound — a slot, an element or
//@ `p^` as well as a name (`x = #append(s.items, 1)` leaves `s.items` stale
//@ until `s.items` is written); a callee that grows a slot of a record handed
//@ by value, or a function value any function the program takes as a value
//@ may be, leaves the caller's copy stale the same way; `xs = #append(xs, v)`, the
//@ same place on both sides, grows it where it lives. One
//@ handed by value, or as `&x` to a function that leaves `x^` `nil`, and kept
//@ in a container (`b.items = #append(b.items, x)`) is handed over:
//@ the container holds the handle, a later move of a copy the caller still
//@ has leaves it stale, and a callee that keeps the block and moves or frees
//@ it on one path is warned of (WRN0803).
//@ And one call is not handed a view into a block together with the handle
//@ to it, when the callee may grow the block and then use the view (ERR0827).
//@
//@ ```
//@ ^Vec2 make(f32 a):
//@     Vec2 v = (a, a)
//@     return &v                 // ERR0826: v's frame is gone
//@
//@ void touch(ref []Cell r):
//@     r[0].grow(&r)             // ERR0827: grow may move r, then write self
//@ ```
//@
//@ What each call does is read from the called function's body, so nothing is
//@ written in a signature, and a call through a trait value is read through
//@ every implementer's method; a call through a local bound to one function by
//@ name (`auto f = step`) and never assigned is read through that function. What
//@ is not followed is said out loud: a `foreign` function or any other function
//@ value is taken to keep nothing (a handle it answers may be a copy of any of
//@ that type its arguments reach; a pointer or window a function value answers
//@ may point into the block of any handle an argument reaches, and one a
//@ `foreign` function answers is tied only to an argument of its own type), and
//@ a conversion to `rawptr` drops what it pointed at (a frame escape through
//@ either is warned of, WRN0826, as above). A function that reads a
//@ pointer or window it took through one parameter after it moves or frees a
//@ block through another relies on the two reaching distinct blocks, and a
//@ call that hands it two that may reach one — one place twice (`use2(&e,
//@ &e)`, `use2(&x, &x)`), a record and one of its slots (`use3(&e, e.pool)`,
//@ `use4(&e, &e.pool)`), records holding one block (`use2(&e, &f)` with `Eng f
//@ = (e.pool, …)`) — is warned of (WRN0803); an argument the caller
//@ cannot name (a call's answer) is not checked. A pointer local that is no
//@ other name for a parameter but may point at its record (`^Eng F = G`, then
//@ `F = E`) reaches what a move through the parameter moves. A function the
//@ program exports, or one a script calls, may be handed records or pairs of
//@ arguments that share a block from outside Dado, and what it relies on of
//@ them is checked only at the calls Dado sees: a warning at the export
//@ itself would stand at every exported function that reads a pointer across
//@ a move, whatever its callers hand it. A copy of a handle stored
//@ in a block or in package storage (`hs[0] = b`, `KEEP = b`) is not followed
//@ there, and neither are views of a package `ref`'s own block stored in package
//@ storage. Views of a block stored in package storage warn at a later move of
//@ it in the function that stored them, and not once the block has left that
//@ function, returned or stored away. A record a call keeps pointers into its
//@ own blocks in is taken to manage them itself. Two subscripts are one
//@ place, so a `ref` copied out of one element (`k = hs[1]`, or the last one
//@ a walk bound) is stale once any element is freed. A slot handed over under a
//@ subscript (`ws[i].finish()`) is not recognised, so the answer stays a copy
//@ of what the subscripted array holds. A callee that leaves a slot `nil` on every way out
//@ (`take(&c)` answering `c.tris` after `c.tris = nil`, or `defer self.out =
//@ nil` beside `return self.out`) hands that slot's block over, as `b^ = nil` on
//@ every way out hands over the block `b^` held; the answer is still taken for
//@ a copy of any other handle of its type `c` holds, and warned about after one
//@ of those moves. A slot written from a fresh block (`#make`, `nil`), here or
//@ in a callee, holds neither a block handed over before nor another slot's;
//@ two handle types that differ only by a `distinct` tag (`ref []u32` and
//@ `distinct type Owned: ref []u32`) are one type here, since a conversion
//@ between them is no copy. A `return take(&c)` written directly is not checked
//@ against a `defer` that frees what `c` holds. A pointer into a record's slot taken in place
//@ (`&m.pos[1]` of `M m = (pos, nor)`) is not stale when the local the slot
//@ was copied from grows, nor, in a record of a generic type declared with
//@ no value (`Two(u32) t`), when another slot holding the same block is
//@ freed. A record copied whole shares its blocks with the copy only as
//@ far as the copy is followed: one made through a pointer (`f = q^`), out
//@ of a call's answer (`same(e)`), from another copy (`g = f`), or into a
//@ tuple (`(e, 1)`) or another record's slot (`b.e = e`) is not, so freeing
//@ the copy's block leaves what was taken out of the original live; and
//@ records made to share a block by assignment after they are declared (`f = e`,
//@ or `f.pool = e.pool`) and handed to a function that relies on its
//@ parameters reaching distinct blocks (`use2(&e, &f)`) are not warned of.
//@ An answer out of a handle inside a slot's block
//@ (`&e.polys[i].pts[0]`) is not stale when that inner handle is freed or
//@ grown. A `ref` handed into a container is followed as one copy in one
//@ place: a callee that copies `x^` into a container and leaves `x^` holding
//@ it, or stores a handed `ref` in two slots, makes owners the caller does not
//@ see, and a free through one leaves the others live; once a callee has kept
//@ it in a container a local record holds, that container freeing or growing
//@ the block leaves the caller's name, a copy of it or a view of it stale, and
//@ package storage doing so does not; and a callee
//@ that keeps a `ref` and frees it through a local copy leaves what it kept
//@ live. Swapping two
//@ `ref` locals through a temporary (`t = a; a = b; b = t`) warns of a view of
//@ the old block, as any rebinding does; `(a, b) = (b, a)` hands each block to
//@ its new name.
