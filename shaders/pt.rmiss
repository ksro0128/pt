#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 L;
    vec3 beta;
    vec3 nextOrigin;
    vec3 nextDir;

    int depth;
    uint seed;
    int terminated;
    int bounce;
    float pdf;
};


layout(location = 0) rayPayloadInEXT RayPayload payload;


void main() {
    // payload.L += payload.beta * vec3(0.1); // 임시 배경색
    payload.terminated = 1;

}
