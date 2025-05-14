#version 460
#extension GL_EXT_ray_tracing : require


struct RayPayload {
    vec3 L_direct;
    int bounce;
    vec3 L_indirect;
    uint seed;
    vec3 beta;
	vec3 normal;
    int terminated;
    vec3 nextOrigin;
	float depth;
    vec3 nextDir;
	int meshID;
	vec3 albedo;
    vec3 indirAlbedo;
    float pdf;
};


layout(location = 0) rayPayloadInEXT RayPayload payload;


void main() {
    payload.terminated = 1;
    payload.normal = vec3(0.0);
    payload.albedo = vec3(0.0);
    payload.indirAlbedo = vec3(0.0);
    payload.depth = 0.0;
    payload.meshID = -1;
}
