#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 3) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    mat4 prevViewProj;
} camera;

layout(push_constant) uniform PushConst {
    mat4 model;
    uint meshID;
} pc;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out float vViewDepth;
layout(location = 2) out float vMeshID;
layout(location = 3) out vec2 vMotionVec;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));
    vNormal = normalize(normalMatrix * inNormal);

    vec4 viewPos = camera.view * worldPos;
    vViewDepth = -viewPos.z;

    vMeshID = float(pc.meshID);

    vec4 clipCurr = camera.viewProj * worldPos;
    vec4 clipPrev = camera.prevViewProj * worldPos;

    vec2 uvCurr = clipCurr.xy / clipCurr.w * 0.5 + 0.5;

    vec2 uvPrev;
    if (abs(clipPrev.w) > 1e-5) {
        uvPrev = clipPrev.xy / clipPrev.w * 0.5 + 0.5;
    } else {
        // 완전한 불일치 유도
        uvPrev = vec2(-1.0); 
    }

    vMotionVec = uvCurr - uvPrev;

    gl_Position = clipCurr;
}
