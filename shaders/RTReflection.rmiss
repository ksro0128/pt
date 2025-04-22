#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 color;
	vec3 beta;
    int bounce;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;


void main() {
    payload.color = vec3(0.0);
	payload.beta = vec3(0.0);
}
