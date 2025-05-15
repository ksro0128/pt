#version 450

layout(location = 0) out vec4 outNormal;
layout(location = 1) out float outDepth;
layout(location = 2) out float  outMeshID;
layout(location = 3) out vec2 outMotionVec;

layout(location = 0) in vec3 vNormal;
layout(location = 1) in float vViewDepth;
layout(location = 2) in float vMeshID;
layout(location = 3) in vec2 vMotionVec;

void main() {
    outNormal = vec4(normalize(vNormal), 1.0);
    outDepth = vViewDepth;
    outMeshID = vMeshID;
    outMotionVec = vMotionVec;
}
