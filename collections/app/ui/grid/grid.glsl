// app:ui/grid — the cell-grid shader: one instance per cell, six vertices per
// instance, no vertex buffer and no index buffer.
//
// The quad corner comes from gl_VertexIndex (two triangles) and the cell's
// column and row from gl_InstanceIndex, so the instance record is only what a
// cell *is* — `cell.Cell`, twelve bytes: the slot and the flag bits as one
// pair of unsigned shorts, then the foreground and background as four
// normalised bytes each. The atlas page is a uniform grid of cell-sized slots,
// so a slot's texels are arithmetic on its index and nothing is looked up.
//
// The page is R8: one byte of coverage per texel, read as `.r`.
//
// `@module ui_grid` prefixes every C name the generated header declares
// (`ui_grid_grid_shader_desc`, `ui_grid_vs_params_t`, the `ATTR_`/`UB_`/
// `VIEW_`/`SMP_` macros and the source arrays), because the header lands at
// file scope in a program's one translation unit and another shader's
// `vs_params_t` would otherwise collide with this one.
//
// Regenerate the committed header by hand, from the repository root, with the
// sokol-shdc build for the machine you are on:
//
//   collections/vendor/sokol/sokol_shdc/bin/<platform>/sokol-shdc \
//       -i collections/app/ui/grid/grid.glsl \
//       -o collections/app/ui/grid/grid.glsl.h \
//       --format sokol_impl \
//       --slang glsl410:glsl300es:hlsl4:metal_macos:metal_ios:wgsl

@module ui_grid

@vs vs
layout(binding=0) uniform vs_params {
    // x, y: framebuffer size in pixels. z, w: the grid's top-left, in pixels.
    vec4 frame;
    // x, y: one cell in pixels (also one atlas slot). z: grid columns.
    // w: atlas slots across one row of the page.
    vec4 cell;
    // x, y: the page's size in pixels. z, w: unused.
    vec4 atlas;
    // x: the underline's first pixel row within a cell. y: the strike line's.
    // z: both lines' thickness in pixels. w: unused.
    vec4 lines;
};

// x: the atlas slot (0 is blank). y: the flag bits.
in uvec2 slot_flags;
in vec4 fg;
in vec4 bg;

out vec2 uv;
out vec2 local_px;
out vec4 v_fg;
out vec4 v_bg;
out float v_flags;
out vec3 v_lines;

void main() {
    int v = gl_VertexIndex;
    // Corners in order: (0,0) (1,0) (0,1)  (1,0) (1,1) (0,1).
    float qx = (v == 1 || v == 3 || v == 4) ? 1.0 : 0.0;
    float qy = (v == 2 || v == 4 || v == 5) ? 1.0 : 0.0;
    vec2 q = vec2(qx, qy);

    int cols = int(cell.z);
    int i = gl_InstanceIndex;
    vec2 at = vec2(float(i % cols), float(i / cols));

    // frame.zw and cell.xy are whole pixels, so every corner lands on a pixel
    // edge and the NEAREST sampler reads each texel exactly once.
    vec2 px = frame.zw + (at + q) * cell.xy;
    vec2 ndc = vec2(px.x / frame.x * 2.0 - 1.0, 1.0 - px.y / frame.y * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);

    int slot = int(slot_flags.x);
    int across = int(cell.w);
    vec2 s = vec2(float(slot % across), float(slot / across));
    uv = (s + q) * cell.xy / atlas.xy;
    local_px = q * cell.xy;
    v_fg = fg;
    v_bg = bg;
    v_flags = float(slot_flags.y);
    v_lines = lines.xyz;
}
@end

@fs fs
layout(binding=0) uniform texture2D atlas_tex;
layout(binding=0) uniform sampler atlas_smp;

in vec2 uv;
in vec2 local_px;
in vec4 v_fg;
in vec4 v_bg;
in float v_flags;
in vec3 v_lines;

out vec4 frag_color;

// Bit `b` of the flags, which arrive as a float: every flag value is an
// integer below 2^16, which a float holds exactly, and all six vertices of a
// cell carry the same one, so interpolation hands it over unchanged.
bool flag(float b) {
    return mod(floor(v_flags / b), 2.0) >= 1.0;
}

void main() {
    // cell.REGION_PUNCH: the grid draws nothing here; what is under it stays.
    if (flag(16.0)) {
        frag_color = vec4(0.0);
        return;
    }
    float cov = texture(sampler2D(atlas_tex, atlas_smp), uv).r;
    float row = floor(local_px.y);
    // cell.UNDERLINE and cell.STRIKE: solid rows of ink at the offsets the
    // renderer derived from the face.
    if (flag(1.0) && row >= v_lines.x && row < v_lines.x + v_lines.z) {
        cov = 1.0;
    }
    if (flag(2.0) && row >= v_lines.y && row < v_lines.y + v_lines.z) {
        cov = 1.0;
    }
    // Premultiplied "glyph over background": a transparent bg (alpha 0) lets
    // whatever was drawn under the grid show through, with no colour fringe.
    vec4 b = vec4(v_bg.rgb * v_bg.a, v_bg.a);
    vec4 f = vec4(v_fg.rgb * v_fg.a, v_fg.a);
    frag_color = mix(b, f, cov);
}
@end

@program grid vs fs
