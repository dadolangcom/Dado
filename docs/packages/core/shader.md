<!-- dadoc 1.0.0-dev+1eb4a2567aaf.dirty -->
<!-- commit 1eb4a2567aaf (dirty) -->
# core:shader

core:shader — what a compiled DadoGL shader looks like to the program that
hands it to a graphics backend. **DadoGL is reserved and not built yet**
(a0.13b row H14): nothing produces a `Shader` today, and `#shader_load`,
which will, is refused `ERR0001`. The trait and its shapes are declared
whole now so that a backend written against them today does not change when
DadoGL lands.

    import "core:shader"

    void upload(shader.Shader s):
        switch s.format():
        case shader.Format.Wgsl:
            create_module(s.to_string())
        case shader.Format.Spirv:
            create_module_spirv(s.to_bytes())
        else:
            pass

A shader carries its **interface** as well as its code — which entry points
it has and in which stage, what it binds where, and what vertex input it
reads — so a backend lays out its pipeline from these answers instead of
parsing the shader again.

## Declarations

10 declarations, 10 public.

* `enum Format: (Wgsl, Glsl, Essl, Hlsl, Msl, Spirv, Dxil, Metallib)` — The closed set of formats a shader is compiled to, one per `--shader=`
* `bool is_text(Format f)` — Whether `f` is a text format, answered by `to_string`, rather than a binary…
* `enum Stage: (Vertex, Fragment, Compute)` — The pipeline stage an entry point runs in.
* `type EntryPoint: (string8 name, Stage stage)` — One entry point: its name in the compiled code, and its stage.
* `enum BindingKind: (UniformBuffer, StorageBuffer, ReadOnlyStorageBuffer, Sampler, ComparisonSampler, SampledTexture, StorageTexture)` — What a binding slot holds.
* `type Binding: (u32 group, u32 slot, BindingKind kind, string8 name)` — One resource binding: the group (WebGPU; a descriptor set in Vulkan, a…
* `enum VertexFormat: (F32, F32x2, F32x3, F32x4, I32, I32x2, I32x3, I32x4, U32, U32x2, U32x3, U32x4, U8x4Norm, I8x4Norm, U16x2Norm, U16x4Norm, F16x2, F16x4)` — The type of one vertex attribute as the vertex buffer holds it.
* `type VertexAttribute: (u32 location, u32 buffer, u64 offset, VertexFormat format)` — One vertex attribute: the shader's input location, the vertex buffer it is…
* `type VertexBuffer: (u32 slot, u64 stride, bool per_instance)` — One vertex buffer: its slot, the bytes from one element to the next, and…
* `trait Shader` — A compiled shader, as `#shader_load` will answer one.
