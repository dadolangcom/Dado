<!-- dadoc 1.0.0-rc.3 -->
<!-- commit 8e62c891753a -->
# core:geom/halfedge

A half-edge polygon mesh: a box and a prism (a planar contour extruded along its own normal),
traversal queries, moving a vertex, an edge or a face, and a bake to triangle buffers.

    [4][3]f64 square = [[0.0, 0.0, 0.0], [2.0, 0.0, 0.0], [2.0, 2.0, 0.0], [0.0, 2.0, 0.0]]
    try halfedge.Mesh m = halfedge.from_polygon3(square[:], 1.0)
    defer halfedge.destroy(&m)
    try halfedge.Baked b = halfedge.bake_geometry(&m)
    defer halfedge.baked_destroy(&b)
    // 8 vertices and 6 faces; the bake has 24 corners and 36 indices

Faces are counter-clockwise seen from outside. Vertices, half-edges, faces and surfaces are
addressed by their i64 index in the mesh's runs, and every run only grows, so an index stays
valid. A mesh's runs are on the allocator that was `#default` when it was made.

## Declarations

56 declarations, 37 public.

* `type Baked` — A mesh as triangle buffers: parallel per-vertex runs (positions, normals, uvs, and tangents with…
* `void baked_destroy(^Baked b)` — Frees a bake's runs and leaves it empty.
* `!Baked bake_geometry(^const Mesh m)` — The mesh as indexed triangles: each face triangulated (faces may be concave; past four corners…
* `const i32 NO_MEMORY = 1` — The code every failable function here fails with: an allocation failed.
* `type Vertex` — A mesh vertex. UVs are per corner, on the half-edge (`HalfEdge.uv`), so a vertex shared by…
* `type Surface` — A plane (nx, ny, nz, d) with dot(n, p) == d, and the faces it covers.
* `type Face` — A face: its surface, and one of its half-edges; the rest of its loop follows by `next`.
* `type HalfEdge` — One side of an edge, inside one face, pointing to a vertex.
* `type Mesh` — A mesh's four runs, made by `from_box` or `from_polygon3` and ended by `destroy`.
* `void destroy(^Mesh m)` — Frees the mesh's runs and leaves it empty.
* `enum i64 BoxFace: (Front, Right, Back, Left, Top, Bottom)` — The six faces of a `from_box` mesh, in build order.
* `i64 box_face(BoxFace f)` — The face index of `f` in a `from_box` mesh.
* `!Mesh from_box([3]f64 size)` — A box of `size` centred on the origin: 8 vertices, 6 quad faces each on its own surface, 24…
* `i64 prism_wall(i64 edge_index)` — The face index, in a `from_polygon3` prism, of the wall over contour edge `edge_index`.
* `i64 prism_top(i64 n)` — The face index of the top cap of a `from_polygon3` prism over an `n`-point contour.
* `i64 prism_bottom(i64 n)` — The face index of the bottom cap of a `from_polygon3` prism over an `n`-point contour.
* `!Mesh from_polygon3([][3]f64 polygon, f64 length)` — The planar contour `polygon` extruded `length` along its own Newell normal into a closed prism.
* `void calculate_normals(^Mesh m)` — Sets each vertex normal to the unit sum of its corners' cross products, so larger faces weigh…
* `void calculate_uvs(^Mesh m)` — Sets each corner's UV by projecting its face onto the face's own plane, measured from the face's…
* `!ref []i64 face_loop_edges(^const Mesh m, i64 face)` — A face's half-edges in counter-clockwise order from its first, as a new run on `#default`: the…
* `!ref []i64 face_vertices(^const Mesh m, i64 face)` — A face's vertices in counter-clockwise order (vertex k is the one half-edge k points to), as a…
* `[3]f64 face_centroid(^const Mesh m, i64 face)` — The mean of a face's corner positions.
* `[4]f64 face_plane(^const Mesh m, i64 face)` — A face's plane as (nx, ny, nz, d): its unit Newell normal, and d through the mean of its…
* `[3]f64 face_normal(^const Mesh m, i64 face)` — A face's unit Newell normal: the first three of `face_plane`.
* `(i64 from, i64 to) edge_endpoints(^const Mesh m, i64 edge)` — The vertices half-edge `edge` runs from and to.
* `bool is_boundary_edge(^const Mesh m, i64 edge)` — Whether half-edge `edge` has no twin: it lies on an open boundary.
* `i64 vertex_incoming_edge(^const Mesh m, i64 vertex)` — Some half-edge pointing to `vertex`, or -1 when none does. A linear scan of the half-edges.
* `!ref []i64 vertex_incoming_edges(^const Mesh m, i64 vertex)` — The half-edges pointing to `vertex`, one per face around it, found by turning next then twin…
* `!ref []i64 vertex_faces(^const Mesh m, i64 vertex)` — The faces around `vertex`, in `vertex_incoming_edges` order, as a new run on `#default`: the…
* `!ref []i64 vertex_neighbors(^const Mesh m, i64 vertex)` — The vertices one edge from `vertex`, in `vertex_incoming_edges` order, as a new run on…
* `!ref []i64 face_neighbors(^const Mesh m, i64 face)` — The faces across each edge of `face`, open edges skipped, as a new run on `#default`: the…
* `type Triangle` — One fan triangle of a face: its corners' positions, and the face it came from.
* `!ref []Triangle get_triangles(^const Mesh m)` — Every face as a fan of triangles from its first corner, as a new run on `#default`: the caller…
* `void refresh(^Mesh m)` — Recomputes each surface's plane from its first face, then every vertex normal. Call it after…
* `void translate_vertex(^Mesh m, i64 vertex, [3]f64 by)` — Moves `vertex` by `by`, then `refresh`es the mesh.
* `void translate_edge(^Mesh m, i64 edge, [3]f64 by)` — Moves both ends of half-edge `edge` by `by`, then `refresh`es the mesh.
* `void translate_face(^Mesh m, i64 face, [3]f64 by)` — Moves every corner of `face` by `by`, then `refresh`es the mesh.
