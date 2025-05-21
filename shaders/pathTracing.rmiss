#version 460
#extension GL_EXT_ray_tracing : require


struct RayPayload {
    vec3 L;
    vec3 beta;
	vec3 nextOrigin;
	vec3 nextDir;
	int bounce;
	uint seed;
	int terminated;
    float pdf;
};


layout(location = 0) rayPayloadInEXT RayPayload payload;


void main() {
    payload.terminated = 1;
}
