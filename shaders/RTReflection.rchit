#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require

struct MaterialGPU {
	int type;
	int index;
	float pad0;
	float pad1;
};

struct UberGPU {
	vec3 Kd;
	float pad0;
    
	vec3 Ks;
	float pad1;

    vec3 Kr;
	float pad2;

    vec3 Kt;
	float pad3;

    vec3 opacity;
	float pad4;

    int KdIdx;
    int KsIdx;
    int KrIdx;
    int KtIdx;

    int opacityIdx;
    float eta;
	int etaIdx;
    float uroughness;

	int uroughnessIdx;
	float vroughness;
	int vroughnessIdx;
    int remaproughness;
};

struct MatteGPU {
	vec3 Kd;
	float pad0;

	int KdIdx;
	float sigma;
	int sigmaIdx;
	float pad1;
};

struct MetalGPU {
	vec3 eta;
	float pad0;
	
	vec3 k;
	float pad1;
	
	int etaIdx;
	int kIdx;
	float uroughness;
	int uroughnessIdx;

	float vroughness;
	int vroughnessIdx;
	int remaproughness;
	float pad2;
};

struct GlassGPU {
	vec3 Kr;
	float pad0;

	vec3 Kt;
	float pad1;


	int KrIdx;
	int KtIdx;
	float eta;
	int etaIdx;


	float uroughness;
	int uroughnessIdx;
	float vroughness;
	int vroughnessIdx;

	int remaproughness;
	float pad2;
	float pad3;
	float pad4;
};

struct MirrorGPU {
	vec3 Kr;
	float pad0;

	int KrIdx;
	float pad1;
	float pad2;
	float pad3;
};

struct SubstrateGPU {
	vec3 Kd;
	float pad0;

	vec3 Ks;
	float pad1;

	int KdIdx;
	int KsIdx;
	float uroughness;
	int uroughnessIdx;

	float vroughness;
	int vroughnessIdx;
	int remaproughness;
	float pad2;
};

struct PlasticGPU {
	vec3 Kd;
	float pad0;
	vec3 Ks;
	float pad1;

	
	int KdIdx;
	int KsIdx;
	float roughness;
	int roughnessIdx;

	
	int remaproughness;
	float pad2;
	float pad3;
	float pad4;
};

struct ShapeGPU {
	mat4 modelMatrix;
	
	uint64_t vertexAddress;
	uint64_t indexAddress;
	
	int materialIdx;
	int areaLightIdx;
	int alphaIdx;
	int shadowAlphaIdx;
	
	int reverseOrientation;
	float pad0;
	float pad1;
	float pad2;
};

struct AreaLightGPU {
	vec3 scale;
	float pad0;
	vec3 L;
	float pad1;

	int twosided;
	int samples;
	float pad2;
	float pad3;

	vec4 pad4;
};

layout(set = 0, binding = 0) uniform CameraGPU {
    vec3 camPos;
    float pad0;
    vec3 camDir;
    float pad1;
    vec3 camUp;
    float pad2;
    vec3 camRight;
    float fovY;
} camera;

layout(set = 0, binding = 1) uniform OptionsGPU {
    int frameCount;
    float pad0;
    float pad1;
    float pad2;
} options;

layout(set = 1, binding = 0) readonly buffer instanceBuffer {
    ShapeGPU instances[];
};

layout(set = 2, binding = 0) readonly buffer AreaLightBuffer { 
    AreaLightGPU lights[]; 
};
layout(set = 2, binding = 1) readonly buffer MaterialBuffer { 
    MaterialGPU materials[]; 
};
layout(set = 2, binding = 2) readonly buffer UberBuffer { 
    UberGPU ubers[];
};
layout(set = 2, binding = 3) readonly buffer MatteBuffer { 
    MatteGPU mattes[];
};
layout(set = 2, binding = 4) readonly buffer MetalBuffer { 
    MetalGPU metals[]; 
};
layout(set = 2, binding = 5) readonly buffer GlassBuffer { 
    GlassGPU glasses[]; 
};
layout(set = 2, binding = 6) readonly buffer MirrorBuffer { 
    MirrorGPU mirrors[]; 
};
layout(set = 2, binding = 7) readonly buffer SubstrateBuffer { 
    SubstrateGPU substrates[]; 
};
layout(set = 2, binding = 8) readonly buffer PlasticBuffer { 
    PlasticGPU plastics[]; 
};

layout(set = 3, binding = 0) uniform sampler2D textures[];

layout(set = 4, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 4, binding = 1, rgba16f) uniform image2D pingImage;
layout(set = 4, binding = 2, rgba16f) uniform image2D pongImage;


struct RayPayload {
    vec3 color;
};

layout(location = 0) rayPayloadEXT RayPayload payload;


void main() {
    payload.color = vec3(1.0, 0.0, 0.0);
}