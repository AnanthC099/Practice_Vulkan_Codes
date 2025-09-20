#version 450

layout(std140, set=0, binding=0) uniform UBO {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec4 lightDir;
    vec4 lightColor;
    vec4 ambientColor;
    vec4 params;     // x=dispScale, y=noShadow, z=saturation, w=unused
    vec4 sh[9];
} ubo;

layout(set=0, binding=3) uniform sampler2D heightTex;

layout(location=0) in vec3 inPos;
layout(location=1) in vec2 inUV;

layout(location=0) out vec3 vNormalWS;
layout(location=1) out vec2 vUV;

void main()
{
    // Unit sphere normal in object space
    vec3 nObj = normalize(inPos);

    // Simple vertex displacement along normal (optional)
    float h = texture(heightTex, inUV).r; // 0..1
    float disp = ubo.params.x * h;        // scale in object units
    vec3 posObj = inPos + nObj * disp;

    // To world
    vec4 posWS = ubo.modelMatrix * vec4(posObj, 1.0);
    vNormalWS  = normalize(mat3(ubo.modelMatrix) * nObj);
    vUV        = inUV;

    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * posWS;
}
