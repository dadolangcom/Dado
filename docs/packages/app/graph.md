<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# app:graph

app:graph — the neutral scene graph: parent/child, composed
Node2D transforms, lifecycle by tree walk, in ordinary program memory.

    import "app:graph"

    appgraph.Graph g = graph.init() using app.allocator()
    appgraph.Node player = graph.create(&g, position: (10.0, 5.0))
    graph.set_on_process(&g, player, on_player_process)
    appgraph.Node weapon = graph.create(&g, position: (1.0, 0.0))
    graph.add_child(&g, appgraph.INVALID, player)   // a root
    graph.add_child(&g, player, weapon)             // rides along with the player

    void process(f32 dt):
        graph.process(&g, dt)                        // walks the tree, fires on_process
        [2]f32 muzzle = graph.to_global(&g, weapon, (0.5, 0.0))

── Neutral, not graphical ─────────────────────────────────────────────────

A headless server builds this same tree, positions nodes and runs their
`process` hook — it just never draws them. Drawing a node is
a presentation layer's job, gated on `VectorDraw`; nothing here imports a
backend or knows one exists.

── An explicit value, not a singleton ─────────────────────────────────────

`app:log`/`app:input`/`app:net` are process-wide singletons because what
they wrap genuinely has one instance per program (one keyboard, one log
stream, one network host). A scene graph does not — it is app-defined
state, the same way `app:ui`'s `UIGraph` is, and the two are drawn as
trees as siblings on purpose. So `Graph` is a value the caller creates
with `init` and passes explicitly to every call here, exactly like
`app:ui`'s `^UIGraph` parameter, not a package-level global.

── Handles, not pointers (`Node` is `(i32 index, u32 generation)`) ────────

Nodes live in one growable `ref []Slot` the `Graph` owns. A `ref` can
move when it grows (`resize`), so a raw `^Slot` a caller held across a
`create`/`add_child`/`remove` call could dangle — the same class of bug
`app:events` hit for a `rawptr` held past its pointee's lifetime, before
it gave way to typed channels.
A `Node` handle is a slot index (stable across a resize) plus a
generation (bumped when a slot is freed), so a handle held past `remove`
reads as invalid at the next call rather than silently reaching whatever
node has since reused that slot. Every function below re-derives its own
`^Slot` from a `Node` immediately before use and never holds one across a
call into a lifecycle hook — a hook is caller code and is free to
`create`/`add_child`/`set_parent`/`remove` on this same `Graph`, including
on the very node it was called for.

── Lifecycle: hooks, not methods — Dado has neither ────────────────────────

Godot's `_enter_tree`/`_ready`/`_process` become function-ref
fields a node carries, the same `fn` + `data` dispatch shape `core:jobs`'
`JobFn` and `app:app`'s `Backend` already use — `userdata` is what lets one
function serve many node instances (Dado has no closures either), and the
leading `^Graph` is what lets that same function read or write the very
node it was called for (there is no other way back to the tree from
inside a hook — no singleton, no implicit self). Only
three of Godot's lifecycle notifications are here: `on_enter_tree` /
`on_exit_tree` fire once, synchronously, inside `add_child`/`remove`;
`on_ready` fires once, synchronously, immediately after `on_enter_tree`
— there is no "end of frame" to defer it to the way Godot does, because
this tree has no frame of its own, only whatever `process` calls it from.
`_physics_process`/`_input`/`_unhandled_input` are left out: input
dispatch onto scene nodes is graphical-backend-shaped work
for a presentation layer, not for this neutral one.

── The zero-node case: lazy, and no synthetic root ────────────────────────

"Mandatory or lazy" was left for the scene-graph design to decide.
Settled: **lazy**. `init` allocates nothing; the node pool (`Graph.slots`)
is only ever `make`d on the first `create`. There is also no single
mandatory root node — a `Node` attached with an invalid parent simply *is*
a root, and a `Graph` may hold any number of them side by side. Forcing one
synthetic root a program never asked for, and would then have to guard
against removing, buys nothing a pure TUI or immediate-mode program (or a
server hosting several independent top-level entities) needs — the
"pay for what you use" applies here exactly as it does to the walking
skeleton itself.

`destroy` is `init`'s counterpart and the only thing that gives the pool
back. `remove` returns a node's slot to the free list, which is reuse and
not release — the run itself is never shrunk — so a `Graph` that outlives
its owner and is merely dropped leaks every byte `create` ever doubled
into. A caller that owns a `Graph` for a while (a canvas, a scene, a level)
needs a verb for the end of that while, and this is it.

── Where the pool lands ───────────────────────────────────────────────────

`init` takes no allocator. It allocates nothing, so there is nothing for one
to place; what it does is **read the ambient** — `#default`, whatever the
caller has arranged at that line — and keep it in `Graph.alloc` for the
`create` that first needs a pool. So the caller decides with one word at the
call site and this package never carries an allocator through a parameter
list:

    Graph g = init()                          // wherever the caller's default is
    Graph g = init() using arena.allocator(&a) // the pool lands in `a`

**`Graph.alloc` survives, and it is not there for `destroy`.** Freeing never
needed it: `#delete` and `#resize` use the allocator the `ref` itself
carries, so `destroy` would work with the slot gone. It is there because the
pool is *lazy* — `alloc_slot` mints it on the first `create`, which is an
arbitrary distance from `init` — and without the slot that `#make` would
read whatever ambient happened to be live at that call. A `create` inside a
scratch frame would then put the node pool in the scratch arena and the
rollback would take the whole tree with it. Capturing the ambient once, at
`init`, is what makes the pool's home the same fact it was when this was a
parameter, and `using` is now how the caller states it.

── Order is insertion order, in both directions ───────────────────────────

**This changed, and the old behaviour was documented.** `add_child` used
to push a child onto the front of its parent's list, so sibling order was
*reverse* insertion order; and the roots were whatever `0..<used` happened
to yield, which is slot-index order. Both were fine while the only thing
a walk produced was a side effect. Neither survives a consumer that paints:
paint order is sibling order, and a program that writes a background and
then a label on top of it means exactly what it wrote.

So a child is **appended**, and each `Slot` carries a `last_child` to make
that O(1) rather than a walk to the end — the same 8 bytes per node
`app:ui`'s own tree already spends for the same reason. And the roots are
an **explicit ordered list** threaded through the same `next_sibling` field
the children use, with `first_root`/`last_root` on the `Graph`: no second
allocation, and the lazy-`init` promise above is kept.

The slot scan had a second defect the order change would not have fixed:
free-list reuse moves a node's slot index, so removing a root and creating
another put the new one wherever the free list happened to point. An
explicit list is stable because nothing but `add_child`/`set_parent`/`remove`
ever touches it.

One behaviour follows from the list and is worth stating: a node that was
`create`d and never attached is **not** walked. Under the slot scan it was,
because "alive with no parent" and "a root" were the same test. They are
not the same thing — `add_child` is what puts a node in the tree, and it is
what fires `on_enter_tree` — so a detached node no longer ticks.

── Presentation properties, carried and never interpreted ─────────────────

`visible`, `modulate`, `self_modulate`, `z_index`, `z_as_relative`, `clip`
with its custom rect, and `payload` are all stored here and read by nothing
in this file. That is the point. A renderer built on this tree needs
somewhere to put them, and the alternatives are a parallel array it keys by
`Node` — which then has to be resized, freed and generation-checked in
lockstep with this one, twice the bookkeeping for the same facts — or a
second tree, which is two trees to keep in step. Neither is worth the
backend-neutrality it would buy back, because carrying a `[4]u8` costs this
package no import and no opinion: `modulate` is four bytes here and a colour
only where somebody multiplies by it.

**`payload` is deliberately not `userdata`.** The four lifecycle hooks
already own `userdata` — it is the only thing standing in for a closure —
so a renderer naming its command list there would be fighting the program's
own hooks for one field. `payload` is a bare `u32` this package never
dereferences, which is what lets it name a handle into something this
package has never heard of.

── The local transform is a matrix; the TRS triple is a view on it ────────

A `Slot` holds both `xform.Transform2D local` and the
`(position, rotation, scale)` triple, and the triple's setters rebuild the
matrix as they write. That is one `from_trs` — one `sin`/`cos` pair — per
*edit* rather than per node per frame, which is what the walk used to pay;
a tree where most nodes hold still between frames now does no trigonometry
at all. `set_transform` writes the matrix directly and decomposes it back
into the triple so `position`/`rotation`/`scale` keep answering.

The decomposition is lossy exactly where `core:geom/xform` says it is: a
matrix carrying shear has no TRS triple, so `set_transform` followed by
`rotation` can answer something the caller did not write. The matrix itself
is kept verbatim and is what composes, so nothing downstream of
`global_transform` loses anything — only the three readers do, and they
were already documented as a decomposition.

── `process` and `update_transforms` are one walk ─────────────────────────

A renderer must not tick game logic in order to find out where things are.
So the composition — parent's global times the node's own local, written
into `Slot.global` — is one traversal with a flag, and `process` and
`update_transforms` are the two ways to enter it. They are not two walks:
a second copy of the composition arithmetic is the sort of duplication that
stays correct for exactly as long as nobody edits one of them.

## Declarations

68 declarations, 55 public.

* `type Node: (i32 index, u32 generation)` — A handle to one node. See the package doc comment for why this is an…
* `const Node INVALID = (-1, 0)` — The handle that names "no node" — an invalid parent (a root), an absent…
* `bool valid(Node n)`
* `type ReadyFn: void(^Graph, Node, rawptr)` — ── lifecycle hooks ─────────────────────────────────────────────────────…
* `type ProcessFn: void(^Graph, Node, f32, rawptr)`
* `type TreeFn: void(^Graph, Node, rawptr)`
* `type Graph` — The tree. See the package doc comment for the ownership model, and for why…
* `Graph init()` — A fresh, empty graph. Allocates nothing until the first `create`, and the…
* `void destroy(^Graph g)` — Tear the tree down, give the node pool back, and leave `g` in exactly the…
* `Node create(^Graph g, [2]f32 position = (0.0, 0.0), f32 rotation = 0.0, [2]f32 scale = (1.0, 1.0))` — Create a detached node — not yet part of the tree, and so not walked by…
* `void set_userdata(^Graph g, Node n, rawptr ud)`
* `void set_on_ready(^Graph g, Node n, ReadyFn fn)`
* `void set_on_process(^Graph g, Node n, ProcessFn fn)`
* `void set_on_enter_tree(^Graph g, Node n, TreeFn fn)`
* `void set_on_exit_tree(^Graph g, Node n, TreeFn fn)`
* `void add_child(^Graph g, Node parent, Node child)` — Attach `child` under `parent`, or as a root when `parent == INVALID`,…
* `bool set_parent(^Graph g, Node child, Node parent)` — Move `child` under `parent`, or into the root list when…
* `void remove(^Graph g, Node n)` — Detach `n` from its parent (or from the root list) and free it and its…
* `void process(^Graph g, f32 dt)` — Walk every root and its descendants in order, recomputing each node's…
* `void update_transforms(^Graph g)` — The same walk with the hooks left alone: every node's global transform is…
* `Node parent_of(^Graph g, Node n)`
* `Node first_child_of(^Graph g, Node n)`
* `Node last_child_of(^Graph g, Node n)`
* `Node next_sibling_of(^Graph g, Node n)`
* `Node first_root(^Graph g)` — The first root, and the root after `n` — the same two readers…
* `Node next_root(^Graph g, Node n)`
* `bool in_tree(^Graph g, Node n)` — Whether `n` is in the tree — attached to a parent or standing in the root…
* `[2]f32 position(^Graph g, Node n)`
* `void set_position(^Graph g, Node n, [2]f32 p)`
* `f32 rotation(^Graph g, Node n)`
* `void set_rotation(^Graph g, Node n, f32 r)`
* `[2]f32 scale(^Graph g, Node n)`
* `void set_scale(^Graph g, Node n, [2]f32 sc)`
* `xform.Transform2D transform(^Graph g, Node n)` — The node's own transform, as the matrix everything here composes with.
* `void set_transform(^Graph g, Node n, xform.Transform2D t)` — Set the local transform from a matrix, for a caller that has one — a…
* `bool visible(^Graph g, Node n)`
* `void set_visible(^Graph g, Node n, bool v)`
* `[4]u8 modulate(^Graph g, Node n)` — The tint a renderer composes down through this node's whole subtree.
* `void set_modulate(^Graph g, Node n, [4]u8 c)`
* `[4]u8 self_modulate(^Graph g, Node n)` — The tint that applies to this node alone and is not handed to its children…
* `void set_self_modulate(^Graph g, Node n, [4]u8 c)`
* `i32 z_index(^Graph g, Node n)`
* `void set_z_index(^Graph g, Node n, i32 z)`
* `bool z_as_relative(^Graph g, Node n)` — Whether `z_index` is read as an offset from the parent's layer (the…
* `void set_z_as_relative(^Graph g, Node n, bool rel)`
* `bool clip(^Graph g, Node n)`
* `void set_clip(^Graph g, Node n, bool on)`
* `[4]f32 custom_rect(^Graph g, Node n)` — The rectangle `clip` clips to, as `(x, y, w, h)`. Stored whether or not…
* `void set_custom_rect(^Graph g, Node n, [4]f32 r)`
* `u32 payload(^Graph g, Node n)` — An opaque `u32` this package stores and never interprets — a renderer's…
* `void set_payload(^Graph g, Node n, u32 p)`
* `xform.Transform2D global_transform(^Graph g, Node n)`
* `[2]f32 global_position(^Graph g, Node n)`
* `[2]f32 to_global(^Graph g, Node n, [2]f32 local_point)` — A point in `n`'s local space, expressed in the tree's root space.
* `[2]f32 to_local(^Graph g, Node n, [2]f32 global_point)` — A point in the tree's root space, expressed in `n`'s local space.
