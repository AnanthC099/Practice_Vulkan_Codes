/* ---------- triangle.frag ---------- */
#version 460

layout(push_constant) uniform PushConstants
{
    mat4 u_projection;   // unused in this stage
    vec4 u_color;        // the colour we will output
} pc;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = pc.u_color;    // <- comes from push constants
}
