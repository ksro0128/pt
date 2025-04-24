#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 color;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;


void main() {
    payload.color = vec3(0.0, 0.0, 1.0);
}
