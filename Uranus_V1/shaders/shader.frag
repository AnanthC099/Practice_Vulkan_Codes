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

layout(set=0, binding=1) uniform sampler2D albedoTex; // created as VK_FORMAT_R8G8B8A8_SRGB
layout(set=0, binding=2) uniform sampler2D normalTex; // optional (not used when unlit)

layout(location=0) in vec3 vNormalWS;
layout(location=1) in vec2 vUV;

layout(location=0) out vec4 outColor;

vec3 desaturate(vec3 c, float s) {
    // Luminance in linear space
    float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
    return mix(vec3(l), c, clamp(s, 0.0, 1.0));
}

void main()
{
    // Albedo: SRGB texture -> sampled to linear automatically by Vulkan
    vec3 albedo = texture(albedoTex, vUV).rgb;

    // Global saturation control (Mercury looks best ~0.18–0.25)
    vec3 baseCol = desaturate(albedo, ubo.params.z);

    // Flat/unlit path (recommended for planet visualization)
    if (ubo.params.y >= 0.5) {
        outColor = vec4(baseCol, 1.0);
        return;
    }

    // Simple diffuse if lighting enabled
    vec3 N = normalize(vNormalWS);
    vec3 L = normalize(ubo.lightDir.xyz);
    float ndotl = max(dot(N, L), 0.0);

    vec3 lit = baseCol * (ubo.ambientColor.rgb + ubo.lightColor.rgb * ndotl);
    outColor = vec4(lit, 1.0);
}
