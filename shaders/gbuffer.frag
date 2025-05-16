#version 450

layout(location = 0) out vec4 outNormal;
layout(location = 1) out float outDepth;
layout(location = 2) out float  outMeshID;
layout(location = 3) out vec2 outMotionVec;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in float vViewDepth;
layout(location = 2) flat in float vMeshID;
layout(location = 3) in vec3 vWorldPos;
layout(location = 4) in vec4 vClipCurr;
layout(location = 5) in vec4 vClipPrev;

layout(set = 0, binding = 3) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    mat4 prevViewProj;
    vec3 camPos;
} camera;

void main() {

    vec3 N = normalize(vNormal);
    vec3 viewDir = normalize(camera.camPos - vWorldPos);

    if (dot(N, viewDir) < 0.0)
        N = -N;

    outNormal = vec4(normalize(N), 1.0);
    outDepth = vViewDepth;
    outMeshID = vMeshID;

    vec3 ndcCurr  = vClipCurr.xyz  / vClipCurr.w;
    vec3 ndcPrev  = vClipPrev.xyz  / vClipPrev.w;

    vec2 uvCurr = ndcCurr.xy * 0.5 + 0.5;
    vec2 uvPrev = ndcPrev.xy * 0.5 + 0.5;

    outMotionVec = uvCurr - uvPrev;
}
