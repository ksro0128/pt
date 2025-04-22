#version 460

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

struct ObjectInstance {
    uint64_t vectexIndex;
    uint64_t indexIndex;
    int modelMatrixIndex;
    int materialIndex;
    int meshIndex;
    int pad;
};

layout(std430, set = 0, binding = 0) readonly buffer ObjectInstances {
    ObjectInstance instances[];
};

layout(set = 1, binding = 0) readonly buffer ModelBuffer {
    mat4 modelMatrices[];
};

layout(push_constant) uniform PushConstants {
    mat4 lightViewProj;
} pc;

void main() {
    uint instanceIndex = gl_InstanceIndex;
    int modelIdx = instances[instanceIndex].modelMatrixIndex;

    gl_Position = pc.lightViewProj * modelMatrices[modelIdx] * vec4(inPosition, 1.0);
}
