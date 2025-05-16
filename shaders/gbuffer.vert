#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 3) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    mat4 prevViewProj;
    vec3 camPos;
} camera;

layout(push_constant) uniform PushConst {
    mat4 model;
    uint meshID;
} pc;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out float vViewDepth;
layout(location = 2) flat out float vMeshID;
layout(location = 3) out vec3 vWorldPos;
layout(location = 4) out vec4 vClipCurr;
layout(location = 5) out vec4 vClipPrev;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    vWorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));
    vNormal = normalize(normalMatrix * inNormal);

    vec4 viewPos = camera.view * worldPos;
    vViewDepth = -viewPos.z;

    vMeshID = float(pc.meshID);

    vClipCurr = camera.viewProj     * worldPos;
    vClipPrev = camera.prevViewProj * worldPos;

    gl_Position = vClipCurr;
}
