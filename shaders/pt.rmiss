#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec4 dummy;

    vec3 L;
    float pad0;
    vec3 beta;
    float pad1;
    vec3 nextOrigin;
    float pad2;
    vec3 nextDir;
    float pad3;

    int depth;
    uint seed;
    int terminated;
    float pad4;
};


layout(location = 0) rayPayloadInEXT RayPayload payload;


void main() {
    // payload.L += payload.beta * vec3(0.1); // 임시 배경색
    payload.terminated = 1;

}
