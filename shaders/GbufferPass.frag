#version 460
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in mat3 fragTBN;
layout(location = 6) flat in int materialIndex;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAlbedo;
layout(location = 3) out vec4 outPBR;
layout(location = 4) out vec4 outEmissive;

struct Material {
    vec4 baseColor;
    vec3 emissiveFactor;
    float roughness;
    float metallic;
    float ao;
    int albedoTexIndex;
    int normalTexIndex;
    int metallicTexIndex;
    int roughnessTexIndex;
    int aoTexIndex;
    int emissiveTexIndex;
};

layout(std430, set = 2, binding = 1) readonly buffer MaterialBuffer {
    Material materials[];
};

layout(set = 2, binding = 2) uniform sampler2D textures[];

void main() {
    Material mat = materials[materialIndex];

    vec3 albedo = mat.baseColor.rgb;
    if (mat.albedoTexIndex >= 0) {
        albedo *= texture(textures[nonuniformEXT(mat.albedoTexIndex)], fragTexCoord).rgb;
    }

    float metallic = mat.metallic;
    if (mat.metallicTexIndex >= 0) {
        metallic *= texture(textures[nonuniformEXT(mat.metallicTexIndex)], fragTexCoord).b;
    }

    float roughness = mat.roughness;
    if (mat.roughnessTexIndex >= 0) {
        roughness *= texture(textures[nonuniformEXT(mat.roughnessTexIndex)], fragTexCoord).g;
    }

    float ao = mat.ao;
    if (mat.aoTexIndex >= 0) {
        ao *= texture(textures[nonuniformEXT(mat.aoTexIndex)], fragTexCoord).r;
    }

    vec3 normal = normalize(fragNormal);
    if (mat.normalTexIndex >= 0) {
        vec3 tangentNormal = texture(textures[nonuniformEXT(mat.normalTexIndex)], fragTexCoord).xyz * 2.0 - 1.0;
        normal = normalize(fragTBN * tangentNormal);
    }

    vec3 emissive = mat.emissiveFactor;
    if (mat.emissiveTexIndex >= 0) {
        emissive *= texture(textures[nonuniformEXT(mat.emissiveTexIndex)], fragTexCoord).rgb;
    }

    // G-buffer outputs
    outPosition = vec4(fragWorldPos, 1.0);
    // outNormal   = vec4(normal, float(materialIndex));
	outNormal   = vec4(normal, 1.0);
    outAlbedo   = vec4(albedo, 1.0);
    outPBR      = vec4(ao, roughness, metallic, 1.0);
    outEmissive = vec4(emissive, 1.0);
}
