#version 460

/* one vec4 delivered through a uniform buffer                              */
/* set = 0 because we use only one descriptor set in this sample            */
/* binding = 0 because it is the first (and only) resource inside the set   */
layout(set = 0, binding = 0) uniform UBO
{
    vec4 u_color;
} ub;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = ub.u_color;
}
