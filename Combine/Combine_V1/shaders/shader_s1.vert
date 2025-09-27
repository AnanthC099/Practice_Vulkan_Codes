#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 0) out vec2 out_TexCoord;

layout(set = 0, binding = 0) uniform MVPMatrix 
{
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 fade;
} uMVP;

void main()
{
    out_TexCoord = vec2(vTexCoord.x, 1.0 - vTexCoord.y);
    gl_Position = uMVP.projectionMatrix * uMVP.viewMatrix * uMVP.modelMatrix * vec4(vPosition, 1.0);
}
