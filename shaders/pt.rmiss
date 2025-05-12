#version 460
#extension GL_EXT_ray_tracing : require

struct RayPayload {
    vec3 L_direct;
    vec3 L_indirect;
    vec3 beta;
    vec3 nextOrigin;
    vec3 nextDir;

    int bounce;
    uint seed;
    int terminated;
    float lastPdf;

	vec3 normal;
	vec3 albedo;
	float depth;
	int meshID;
};


layout(location = 0) rayPayloadInEXT RayPayload payload;


void main() {
    payload.terminated = 1;
    payload.normal = vec3(0.0);
    payload.albedo = vec3(0.0);
    payload.depth = 0.0;
    payload.meshID = -1;
}
