#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform FrameUniforms {
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
} frame;

layout(set = 0, binding = 1) uniform LightingUniforms {
    vec4 lightDir;
    vec4 ambientColor;
    vec4 lightColor;
    float intensity;
} lighting;

layout(set = 1, binding = 2) uniform sampler2D albedoTexture;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDirNorm = normalize(lighting.lightDir.xyz);
    
    float diff = max(dot(normal, lightDirNorm), 0.0);
    
    vec4 texColor = texture(albedoTexture, fragTexCoord);
    vec3 albedo = texColor.rgb;
    if (texColor.a < 0.1) {
        albedo = fragColor;
    }
    
    vec3 ambient = lighting.ambientColor.xyz * albedo;
    vec3 diffuse = lighting.lightColor.xyz * diff * albedo * lighting.intensity;
    
    vec3 result = ambient + diffuse;
    outColor = vec4(result, 1.0);
}