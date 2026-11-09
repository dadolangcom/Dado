//@ gpu/reserved 10
//@ ### DadoGL is reserved
//@
//@ **A `.dadogl` file is a DadoGL shader unit.** DadoGL is not built yet, so a
//@ package directory holding one is refused, `ERR0001`, naming the file: the
//@ compiler recognises the file by its name and reads nothing in it.
//@
//@ **`#shader_load(Lit)` will load one** — `Lit.dadogl` in this package, or
//@ `#shader_load(pkg.Lit)` for one in an imported package — and answer a
//@ `Shader`, the trait `core:shader` declares whole today: `to_string()` for
//@ the text formats (WGSL, GLSL, GLSL ES, HLSL, MSL), `to_bytes()` for the
//@ binary ones (SPIR-V, DXIL, metallib), `format()` saying which, and the
//@ reflection a backend lays out its pipeline from — `entry_points()` and
//@ their stages, `bindings()` by group and slot, and the vertex layout,
//@ `vertex_attributes()` and `vertex_buffers()`. Until DadoGL is built the call
//@ is refused `ERR0001`, after its argument is checked to name a unit.
//@
//@ **A backend can be written now.** It takes a `Shader` — as a template
//@ parameter, or a trait value held in a slot — and nothing about it changes
//@ when shaders arrive; a test hands it an implementer of its own.
//@
//@ **`--shader=` picks the formats** a build compiles its shaders to, a list
//@ from the closed set — all eight written out,
//@ `--shader=wgsl,glsl,essl,hlsl,msl,spirv,dxil,metallib` — one bundle may carry several. A name outside the set is
//@ refused, naming the set; a build asking for any of them is told DadoGL is
//@ not built yet.
