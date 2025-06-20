/* ---------- triangle.vert ---------- */
#version 460            // keep both shaders on the same version
layout(location = 0) in vec3 inPos;

/* 80‑byte push‑constant block: 64 B mat4 + 16 B vec4 */
layout(push_constant) uniform PushConstants
{
    mat4 u_projection;   // used only by the vertex stage
    vec4 u_color;        // declared but NOT used here
} pc;

void main()
{
    gl_Position = pc.u_projection * vec4(inPos, 1.0);
}
