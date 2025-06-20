#version 460

layout(location = 0) in vec3 inPos;

/* 64‑byte push‑constant block for the projection matrix */
layout(push_constant) uniform PushConstants
{
    mat4 u_projection;
} pc;

void main()
{
    gl_Position = pc.u_projection * vec4(inPos, 1.0);
}
