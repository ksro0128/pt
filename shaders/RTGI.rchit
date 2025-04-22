#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex {
    vec3 pos; float pad0;
    vec3 normal; float pad1;
    vec2 texCoord; vec2 pad2;
    vec3 tangent; float pad3;
};

layout(buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices  { uvec3 i[]; };

struct RayPayload {
    vec3 color;
    int bounce;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

struct Light {
    int type;
    int shadowMapIndex;
    int castsShadow;
    float intensity;

    vec3 color;
    float range;

    vec3 position;
    float spotInnerAngle;

    vec3 direction;
    float spotOuterAngle;
};

layout(set = 0, binding = 1) readonly buffer LightBuffer {
    Light lights[64];
    vec3 ambientColor;
    int lightCount;
}lightInfo;

layout(set = 1, binding = 1) uniform accelerationStructureEXT topLevelAS;

struct ObjectInstance {
    uint64_t vertexIndex;
    uint64_t indexIndex;
    int modelMatrixIndex;
    int materialIndex;
    int meshIndex;
    int pad;
};

layout(std430, set = 2, binding = 0) readonly buffer ObjectInstances {
    ObjectInstance instances[];
};

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

layout(std430, set = 3, binding = 0) readonly buffer ModelBuffer {
    mat4 modelMatrices[];
};

layout(std430, set = 3, binding = 1) readonly buffer MaterialBuffer {
    Material materials[];
};

layout(set = 3, binding = 2) uniform sampler2D textures[];

hitAttributeEXT vec2 attribs;

void main() {
    ObjectInstance inst = instances[gl_InstanceCustomIndexEXT];
    Vertices vertices = Vertices(inst.vertexIndex);
    Indices indices = Indices(inst.indexIndex);

    uvec3 idx = indices.i[gl_PrimitiveID];
    Vertex v0 = vertices.v[idx.x];
    Vertex v1 = vertices.v[idx.y];
    Vertex v2 = vertices.v[idx.z];

    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;
    vec2 uv = v0.texCoord * w + v1.texCoord * u + v2.texCoord * v;

    Material mat = materials[inst.materialIndex];

	vec3 normal;
	if (mat.normalTexIndex != -1) {
		vec3 tangent = normalize(v0.tangent);
		vec3 bitangent = normalize(cross(tangent, v0.normal));
		mat3 TBN = mat3(tangent, bitangent, v0.normal);

		vec3 sampledNormal = texture(textures[nonuniformEXT(mat.normalTexIndex)], uv).rgb;
		sampledNormal = normalize(sampledNormal * 2.0 - 1.0);
		normal = normalize(TBN * sampledNormal);
	} else {
		normal = normalize(cross(v1.pos - v0.pos, v2.pos - v0.pos));
	}
	vec3 albedo = (mat.albedoTexIndex == -1) ? mat.baseColor.rgb : texture(textures[nonuniformEXT(mat.albedoTexIndex)], uv).rgb;

    vec3 Li = vec3(0.0);

    vec3 emissive = (mat.emissiveTexIndex == -1)
        ? mat.emissiveFactor
        : texture(textures[nonuniformEXT(mat.emissiveTexIndex)], uv).rgb * mat.emissiveFactor;
    Li += emissive;

    Li += lightInfo.ambientColor * albedo * mat.ao;
    payload.color = Li;
}
