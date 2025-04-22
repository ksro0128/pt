#version 460
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out mat3 fragTBN;
layout(location = 6) flat out int outMaterialIndex;

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    vec3 camPos;
    int frameCount;
} camera;

struct ObjectInstance {
    uint64_t vectexIndex;
    uint64_t indexIndex;
    int modelMatrixIndex;
    int materialIndex;
    int meshIndex;
    int pad;
};

layout(std430, set = 1, binding = 0) readonly buffer ObjectInstances {
    ObjectInstance instances[];
};

layout(std430, set = 2, binding = 0) readonly buffer ModelBuffer {
    mat4 modelMatrices[];
};

void main() {
    uint instanceIndex = gl_InstanceIndex;

    int modelIdx = instances[instanceIndex].modelMatrixIndex;
    int materialIdx = instances[instanceIndex].materialIndex;

    mat4 model = modelMatrices[modelIdx];
    vec4 worldPos = model * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(model)));
    fragNormal = normalMatrix * inNormal;

    vec3 T = normalize(normalMatrix * inTangent);
    vec3 N = normalize(fragNormal);
    vec3 B = normalize(cross(N, T));
    fragTBN = mat3(T, B, N);

    fragTexCoord = inTexCoord;
    outMaterialIndex = materialIdx;

    gl_Position = camera.proj * camera.view * worldPos;
}
