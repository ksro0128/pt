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

struct AreaLightTriangleGPU {
	vec3 worldPos0;
    float pad0;
	vec3 worldPos1;
    float pad1;
	vec3 worldPos2;
    float pad2;
	vec3 worldNormal;
	float area;
    vec3 L;
    float pad3;
};

layout(set = 0, binding = 0) uniform CameraGPU {
    vec3 camPos;
    int areaLightTriangleCount;
    vec3 camDir;
    float pad1;
    vec3 camUp;
    float pad2;
    vec3 camRight;
    float fovY;
} camera;

layout(set = 0, binding = 1) uniform OptionsGPU {
    int frameCount;
    int sampleCount;
	int maxSampleCount;
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
layout(set = 2, binding = 9) readonly buffer AreaLightTriangleBuffer { 
    AreaLightTriangleGPU areaLightTriangles[]; 
};

layout(set = 3, binding = 0) uniform sampler2D textures[];

layout(set = 4, binding = 0) uniform accelerationStructureEXT topLevelAS;

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

layout(location = 1) rayPayloadEXT bool isShadowed;

const int MATERIAL_DISNEY     = 0;
const int MATERIAL_FOURIER    = 1;
const int MATERIAL_GLASS      = 2;
const int MATERIAL_HAIR       = 3;
const int MATERIAL_KDSUBSURF  = 4;
const int MATERIAL_MATTE      = 5;
const int MATERIAL_METAL      = 6;
const int MATERIAL_MIRROR     = 7;
const int MATERIAL_MIX        = 8;
const int MATERIAL_NONE       = 9;
const int MATERIAL_PLASTIC    = 10;
const int MATERIAL_SUBSTRATE  = 11;
const int MATERIAL_SUBSURFACE = 12;
const int MATERIAL_TRANSLUCENT = 13;
const int MATERIAL_UBER       = 14;


const int SAMPLE_DIFFUSE = 0;
const int SAMPLE_SPECULAR = 1;
const int SAMPLE_REFLECT = 2;

const float PI = 3.1415926535;

float radicalInverse_VdC(uint bits) {
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555u) << 1) | ((bits & 0xAAAAAAAAu) >> 1);
    bits = ((bits & 0x33333333u) << 2) | ((bits & 0xCCCCCCCCu) >> 2);
    bits = ((bits & 0x0F0F0F0Fu) << 4) | ((bits & 0xF0F0F0F0u) >> 4);
    bits = ((bits & 0x00FF00FFu) << 8) | ((bits & 0xFF00FF00u) >> 8);
    return float(bits) * 2.3283064365386963e-10; // 1/2^32
}

float radicalInverseBase3(uint n) {
    float invBase = 1.0 / 3.0;
    float result = 0.0;
    float f = invBase;
    while (n > 0u) {
        result += float(n % 3u) * f;
        n /= 3u;
        f *= invBase;
    }
    return result;
}

vec2 halton(uint index) {
    return vec2(
        radicalInverse_VdC(index),       // base 2
        radicalInverseBase3(index)       // base 3
    );
}

float rand(inout uint seed) {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return float(seed) / 4294967296.0;
}

// float rand(inout uint seed)
// {
//     seed = seed * 747796405u + 2891336453u;
//     uint word = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737u;
//     return float((word >> 22) ^ word) / 4294967296.0;
// }

vec2 sample2D(inout uint seed)
{
    // float u = rand(seed);
    // float v = rand(seed);
    // return vec2(u, v);

    return halton(seed);
}

vec3 cosineSampleHemisphere(inout uint seed) {
    // float u1 = rand(seed);
    // float u2 = rand(seed);

    vec2 xy = halton(seed);
    float u1 = xy.x;
    float u2 = xy.y;

    float r = sqrt(u1);
    float theta = 2.0 * 3.141592 * u2;
    return vec3(r * cos(theta), r * sin(theta), sqrt(max(0.0, 1.0 - u1)));
}

vec3 toWorld(vec3 localDir, vec3 N) {
    vec3 T, B;
    if (abs(N.y) < 0.999)
        T = normalize(cross(N, vec3(0.0, 1.0, 0.0)));
    else
        T = normalize(cross(N, vec3(1.0, 0.0, 0.0)));
    B = cross(T, N);
    return localDir.x * T + localDir.y * B + localDir.z * N;
}

float maxComponent(vec3 v) {
    return max(v.x, max(v.y, v.z));
}

struct Vertex {
    vec3 pos; float pad0;
    vec3 normal; float pad1;
    vec2 texCoord; vec2 pad2;
    vec3 tangent; float pad3;
};

layout(buffer_reference, scalar) buffer Vertices {
    Vertex v[];
};
layout(buffer_reference, scalar) buffer Indices {
    uvec3 i[];
};
hitAttributeEXT vec2 attribs;

void computeHitNormal(inout vec3 N, out vec3 pos) {
    uint instanceID = gl_InstanceCustomIndexEXT;
    ShapeGPU shape = instances[instanceID];

    Vertices vertices = Vertices(shape.vertexAddress);
    Indices indices = Indices(shape.indexAddress);

	uvec3 idx = indices.i[gl_PrimitiveID];
    Vertex v0 = vertices.v[idx.x];
    Vertex v1 = vertices.v[idx.y];
    Vertex v2 = vertices.v[idx.z];

	float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;

	mat4 model = shape.modelMatrix;
    // model[0][0] *= -1;

	vec3 localNormal = normalize(v0.normal * w + v1.normal * u + v2.normal * v);
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	N = normalize(normalMatrix * localNormal);

    // compute hit position
	pos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

    if (dot(N, gl_WorldRayDirectionEXT) > 0.0) {
        N = normalize(-N);
    }
}


vec2 getUV() {
	uint instanceID = gl_InstanceCustomIndexEXT;
	ShapeGPU shape = instances[instanceID];
	Vertices vertices = Vertices(shape.vertexAddress);
	Indices indices = Indices(shape.indexAddress);

	uvec3 idx = indices.i[gl_PrimitiveID];
    Vertex v0 = vertices.v[idx.x];
    Vertex v1 = vertices.v[idx.y];
    Vertex v2 = vertices.v[idx.z];

    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;
    vec2 uv = v0.texCoord * w + v1.texCoord * u + v2.texCoord * v;

    // return uv;
	return vec2(uv.x, 1.0 - uv.y);
}

vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
	
    float phi = 2.0 * 3.1415926535 * Xi.x;
    float y = clamp(Xi.y, 0.0, 1.0 - 1e-6);
    float cosTheta = sqrt((1.0 - y) / (1.0 + (alpha2 - 1.0) * y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}


float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = max(roughness * roughness, 0.01);
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.01);
    float NdotH2 = NdotH * NdotH;
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265359 * denominator * denominator);
}

float ggxPdf(vec3 N, vec3 H, vec3 V, float roughness) {
    float D = distributionGGX(N, H, roughness);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH = max(dot(V, H), 0.01);
    return D * NdotH / (4.0 * VdotH + 1e-5); // pdf = D(h) * (N⋅H) / (4 * (V⋅H))
}

// Fresnel-Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    cosTheta = clamp(cosTheta, 0.0, 1.0);
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelSchlickScalar(float cosTheta, float F0) {
	cosTheta = clamp(cosTheta, 0.0, 1.0);
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Geometry Function
float geometrySchlickGGX(float NdotV, float roughness) {
    roughness = max(roughness, 0.01);
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.01);
    float NdotL = max(dot(N, L), 0.01);
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

float luminance(vec3 c) {
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

uint tea(in uint val0, in uint val1)
{
  uint v0 = val0;
  uint v1 = val1;
  uint s0 = 0;

  for(uint n = 0; n < 16; n++)
  {
    s0 += 0x9e3779b9;
    v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
    v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
  }

  return v0;
}

vec3 fresnelConductor(float cosTheta, vec3 eta, vec3 k) {
    cosTheta = clamp(cosTheta, 0.0, 1.0);

    vec3 eta2 = eta * eta;
    vec3 k2 = k * k;

    float cos2Theta = cosTheta * cosTheta;

    vec3 t0 = eta2 - k2 - vec3(1.0);
    vec3 a2plusb2 = sqrt(max(t0 * t0 + 4.0 * eta2 * k2, vec3(1e-2)));
    vec3 t1 = a2plusb2 + vec3(cos2Theta);
    vec3 t2 = (a2plusb2 * cos2Theta) + vec3(1.0);

    vec3 Rs = (t1 - 2.0 * eta * cosTheta) / (t1 + 2.0 * eta * cosTheta);
    vec3 Rp = (t2 - 2.0 * eta * cosTheta) / (t2 + 2.0 * eta * cosTheta);

    return 0.5 * (Rs * Rs + Rp * Rp);
}

float fresnelDielectric(float cosThetaI, float etaI, float etaT) {
    cosThetaI = clamp(cosThetaI, -1.0, 1.0);
    bool entering = cosThetaI > 0.0;
    float ei = entering ? etaI : etaT;
    float et = entering ? etaT : etaI;

    if (abs(ei - et) < 1e-5) return 0.0;

    float sinThetaI = sqrt(max(0.0, 1.0 - cosThetaI * cosThetaI));
    float sinThetaT = ei / et * sinThetaI;

    if (sinThetaT >= 1.0) return 1.0; // 전반사

    float cosThetaT = sqrt(max(0.0, 1.0 - sinThetaT * sinThetaT));

    float Rparl = ((et * cosThetaI) - (ei * cosThetaT)) /
                  ((et * cosThetaI) + (ei * cosThetaT));
    float Rperp = ((ei * cosThetaI) - (et * cosThetaT)) /
                  ((ei * cosThetaI) + (et * cosThetaT));

    return 0.5 * (Rparl * Rparl + Rperp * Rperp);
}

void sampleMatte(
    in MatteGPU matte, in vec3 wo, in vec3 N, inout uint seed,
    out vec3 wi, out vec3 f, out float pdf)
{
    vec3 localWi = cosineSampleHemisphere(seed);
    wi = toWorld(localWi, N);

    vec3 Kd = matte.Kd;
    if (matte.KdIdx >= 0) {
        vec2 uv = getUV();
        Kd = texture(textures[matte.KdIdx], uv).rgb;
    }
    payload.albedo = Kd;
    f = Kd / PI;
    pdf = max(dot(N, wi), 0.0) / PI;
}

void sampleMetal(
    in MetalGPU metal, in vec3 wo, in vec3 N, inout uint seed,
    out vec3 wi, out vec3 f, out float pdf)
{
    float roughness = metal.uroughness;
    if (metal.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec2 Xi = sample2D(seed);
    vec3 H = importanceSampleGGX(Xi, N, roughness);
    wi = normalize(reflect(-wo, H));

    vec3 eta = metal.eta;
    vec3 k = metal.k;

    if (metal.etaIdx != -1) {
        vec2 uv = getUV();
        eta = texture(textures[metal.etaIdx], uv).rgb;
    }
    if (metal.kIdx != -1) {
        vec2 uv = getUV();
        k = texture(textures[metal.kIdx], uv).rgb;
    }

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
	vec3 F = fresnelConductor(VdotH, eta, k);
    payload.albedo = fresnelConductor(1.0, eta, k);

    f = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);
    pdf = D * NdotH / (4.0 * VdotH + 0.01);
}


void sampleGlass(
    in GlassGPU glass, in vec3 wo, in vec3 N, inout uint seed,
    out vec3 wi, out vec3 f, out float pdf)
{
    float roughness = glass.uroughness;
    if (glass.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec2 Xi = sample2D(seed);
    vec3 H = importanceSampleGGX(Xi, N, roughness);
    wi = normalize(reflect(-wo, H));

    vec3 Kr = glass.Kr;
    vec3 Kt = glass.Kt;
    float eta = glass.eta;

    if (glass.KrIdx != -1) {
        vec2 uv = getUV();
        Kr = texture(textures[glass.KrIdx], uv).rgb;
    }
    if (glass.KtIdx != -1) {
        vec2 uv = getUV();
        Kt = texture(textures[glass.KtIdx], uv).rgb;
    }
    if (glass.etaIdx != -1) {
        vec2 uv = getUV();
        eta = texture(textures[glass.etaIdx], uv).r;
    }

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    float F = fresnelDielectric(VdotH, 1.0, eta);
    payload.albedo = vec3(1.0);

    vec3 spec = (D * G * Kr * F) / max(4.0 * NdotV * NdotL, 0.01);

    f = spec;
    pdf = D * NdotH / (4.0 * VdotH + 0.01);
}


void sampleMirror(
    in MirrorGPU mirror, in vec3 wo, in vec3 N, inout uint seed,
    out vec3 wi, out vec3 f, out float pdf)
{
    wi = reflect(-wo, N);

    vec3 Kr = mirror.Kr;

    if (mirror.KrIdx != -1) {
        vec2 uv = getUV();
        Kr = texture(textures[mirror.KrIdx], uv).rgb;
    }

    float NdotL = max(dot(N, wi), 0.0);

    payload.albedo = vec3(1.0);
    if (NdotL > 0.0) {
        f = Kr / NdotL;
        pdf = 1.0;
    } else {
        f = vec3(0.0);
        pdf = 0.0;
    }
}


void samplePlastic(
    in PlasticGPU plastic, in vec3 wo, in vec3 N, inout uint seed,
    out vec3 wi, out vec3 f, out float pdf)
{
    float roughness = plastic.roughness;
    if (plastic.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec3 Kd = plastic.Kd;
    vec3 Ks = plastic.Ks;

    if (plastic.KdIdx != -1) {
        vec2 uv = getUV();
        Kd = texture(textures[plastic.KdIdx], uv).rgb;
    }
    if (plastic.KsIdx != -1) {
        vec2 uv = getUV();
        Ks = texture(textures[plastic.KsIdx], uv).rgb;
    }
    payload.albedo = Kd;

    vec3 F0 = Ks;
    float F_spec = luminance(F0);

    int sampledSpecular = 0;
    vec3 H;
    if (rand(seed) < F_spec) {
        sampledSpecular = 1;
        vec2 Xi = sample2D(seed);
        H = importanceSampleGGX(Xi, N, roughness);
        wi = normalize(reflect(-wo, H));
    } else {
        sampledSpecular = 0;
        vec3 localWi = cosineSampleHemisphere(seed);
        wi = toWorld(localWi, N);
        H = normalize(wo + wi);
    }

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH_final = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH_final, F0);

    vec3 diffusef = Kd / PI;
    vec3 specularf = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);

    float pdf_diff = NdotL / PI;
    float pdf_spec = D * NdotH / (4.0 * VdotH_final + 0.01);

    float weight_diff = pdf_diff * pdf_diff;
    float weight_spec = pdf_spec * pdf_spec;

    float denom = weight_diff + weight_spec;
    float misWeight = (sampledSpecular != 0) ? (weight_spec / denom) : (weight_diff / denom);

    if (sampledSpecular != 0) {
        f = specularf * misWeight;
        pdf = pdf_spec;
    } else {
        f = diffusef * misWeight;
        pdf = pdf_diff;
    }
}


void sampleSubstrate(
    in SubstrateGPU substrate, in vec3 wo, in vec3 N, inout uint seed,
    out vec3 wi, out vec3 f, out float pdf)
{
	float roughness = substrate.uroughness;
    if (substrate.remaproughness != 0)
        roughness = max(roughness, 0.01);

	vec3 Kd = substrate.Kd;
    vec3 Ks = substrate.Ks;

    if (substrate.KdIdx != -1) {
        vec2 uv = getUV();
        Kd = texture(textures[substrate.KdIdx], uv).rgb;
    }
	if (substrate.KsIdx != -1) {
        vec2 uv = getUV();
        Ks = texture(textures[substrate.KsIdx], uv).rgb;
    }
    payload.albedo = Kd;

	vec3 F0 = Ks;
    float F_spec = luminance(F0);

    int sampledSpecular = 0;
    vec3 H;
    if (rand(seed) < F_spec) {
        sampledSpecular = 1;
        vec2 Xi = sample2D(seed);
        H = importanceSampleGGX(Xi, N, roughness);
        wi = normalize(reflect(-wo, H));
    } else {
        sampledSpecular = 0;
        vec3 localWi = cosineSampleHemisphere(seed);
        wi = toWorld(localWi, N);
        H = normalize(wo + wi);
    }

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH_final = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH_final, F0);

    vec3 diffusef = Kd / PI;
    vec3 specularf = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);

    float pdf_diff = NdotL / PI;
    float pdf_spec = D * NdotH / (4.0 * VdotH_final + 0.01);

    float weight_diff = pdf_diff * pdf_diff;
    float weight_spec = pdf_spec * pdf_spec;

    float denom = weight_diff + weight_spec;
    float misWeight = (sampledSpecular != 0) ? (weight_spec / denom) : (weight_diff / denom);

    if (sampledSpecular != 0) {
        f = specularf * misWeight;
        pdf = pdf_spec;
    } else {
        f = diffusef * misWeight;
        pdf = pdf_diff;
    }
}


void sampleUber(
    in UberGPU uber, in vec3 wo, in vec3 N, inout uint seed,
    out vec3 wi, out vec3 f, out float pdf)
{
	float roughness = uber.uroughness;
    if (uber.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec3 Kd = uber.Kd;
    vec3 Ks = uber.Ks;
    float eta = uber.eta;

    if (uber.KdIdx != -1) {
        vec2 uv = getUV();
        Kd = texture(textures[uber.KdIdx], uv).rgb;
    }
    if (uber.KsIdx != -1) {
        vec2 uv = getUV();
        Ks = texture(textures[uber.KsIdx], uv).rgb;
    }
    payload.albedo = Kd;

    float F0_scalar = pow((eta - 1.0) / (eta + 1.0), 2.0);
    vec3 F0 = Ks * F0_scalar;

    float F_spec = luminance(F0);

    int sampledSpecular = 0;
    vec3 H;
    if (rand(seed) < F_spec) {
        sampledSpecular = 1;
        vec2 Xi = sample2D(seed);
        H = importanceSampleGGX(Xi, N, roughness);
        wi = normalize(reflect(-wo, H));
    } else {
        sampledSpecular = 0;
        vec3 localWi = cosineSampleHemisphere(seed);
        wi = toWorld(localWi, N);
        H = normalize(wo + wi);
    }

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH_final = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH_final, F0);

    vec3 diffusef = Kd / PI;
    vec3 specularf = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);

    float pdf_diff = NdotL / PI;
    float pdf_spec = D * NdotH / (4.0 * VdotH_final + 0.01);

    float weight_diff = pdf_diff * pdf_diff;
    float weight_spec = pdf_spec * pdf_spec;

    float denom = weight_diff + weight_spec;
    float misWeight = (sampledSpecular != 0) ? (weight_spec / denom) : (weight_diff / denom);

    if (sampledSpecular != 0) {
        f = specularf * misWeight;
        pdf = pdf_spec;
    } else {
        f = diffusef * misWeight;
        pdf = pdf_diff;
    }
}


void sampleLight(vec3 P, vec3 N, inout uint seed, out vec3 light_wi, out vec3 le, out float light_pdf) 
{
    int lightIdx = int(mod(rand(seed) * float(camera.areaLightTriangleCount), float(camera.areaLightTriangleCount)));
    AreaLightTriangleGPU lightTriangle = areaLightTriangles[lightIdx];

    float u = rand(seed);
    float v = rand(seed);
    if (u + v > 1.0) {
        u = 1.0 - u;
        v = 1.0 - v;
    }

    float w = 1.0f - u - v;
    vec3 sampledPoint = w * lightTriangle.worldPos0 + u * lightTriangle.worldPos1 + v * lightTriangle.worldPos2;

    light_wi = sampledPoint - P;
    le = lightTriangle.L;
    float area = lightTriangle.area;

    float pickPdf = 1.0 / float(camera.areaLightTriangleCount);
    float samplePdf = 1.0 / area;
    float pdf_area = pickPdf * samplePdf;

    float dist2 = dot(sampledPoint - P, sampledPoint - P);
    float cosTheta_l = max(dot(lightTriangle.worldNormal, -normalize(light_wi)), 0.001);

    float pdf_solid_angle = (dist2 / cosTheta_l) * pdf_area;
    

    light_pdf = pdf_solid_angle;
}

void evalMatte(
    in MatteGPU matte, in vec3 wo, in vec3 N, in vec3 wi,
    out vec3 f, out float pdf
) {
    vec3 Kd = matte.Kd;
    if (matte.KdIdx >= 0) {
        vec2 uv = getUV();
        Kd = texture(textures[matte.KdIdx], uv).rgb;
    }
    f = Kd / PI;
    pdf = max(dot(N, wi), 0.0) / PI;
}

void evalMetal(
    in MetalGPU metal, in vec3 wo, in vec3 N, in vec3 wi,
    out vec3 f, out float pdf
) {
    float roughness = metal.uroughness;
    if (metal.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec3 eta = metal.eta;
    vec3 k = metal.k;

    if (metal.etaIdx != -1) {
        vec2 uv = getUV();
        eta = texture(textures[metal.etaIdx], uv).rgb;
    }
    if (metal.kIdx != -1) {
        vec2 uv = getUV();
        k = texture(textures[metal.kIdx], uv).rgb;
    }

    vec3 H = normalize(wo + wi);

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
	vec3 F = fresnelConductor(VdotH, eta, k);

    f = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);
    pdf = (D * NdotH) / (4.0 * VdotH + 0.01);

}

void evalGlass(
    in GlassGPU glass, in vec3 wo, in vec3 N, in vec3 wi,
    out vec3 f, out float pdf
) {
    float roughness = glass.uroughness;
    if (glass.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec3 H = normalize(wo + wi);

    vec3 Kr = glass.Kr;
    vec3 Kt = glass.Kt;
    float eta = glass.eta;

    if (glass.KrIdx != -1) {
        vec2 uv = getUV();
        Kr = texture(textures[glass.KrIdx], uv).rgb;
    }
    if (glass.KtIdx != -1) {
        vec2 uv = getUV();
        Kt = texture(textures[glass.KtIdx], uv).rgb;
    }
    if (glass.etaIdx != -1) {
        vec2 uv = getUV();
        eta = texture(textures[glass.etaIdx], uv).r;
    }

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    float F = fresnelDielectric(VdotH, 1.0, eta);

    vec3 spec = (D * G * Kr * F) / max(4.0 * NdotV * NdotL, 0.01);

    f = spec;
    pdf = (D * NdotH) / (4.0 * VdotH + 0.01);

}

void evalMirror(
    in MirrorGPU mirror, in vec3 wo, in vec3 N, in vec3 wi,
    out vec3 f, out float pdf
) {
    vec3 Kr = mirror.Kr;
    if (mirror.KrIdx != -1) {
        vec2 uv = getUV();
        Kr = texture(textures[mirror.KrIdx], uv).rgb;
    }

    vec3 reflected = reflect(-wo, N);

    if (all(lessThan(abs(reflected - wi), vec3(1e-4)))) {
        f = Kr / max(abs(dot(N, wi)), 1e-4);
        pdf = 1.0;
    } else {
        f = vec3(0.0);
        pdf = 0.0;
    }
}

void evalPlastic(
    in PlasticGPU plastic, in vec3 wo, in vec3 N, in vec3 wi,
    out vec3 f, out float pdf
) {
    float roughness = plastic.roughness;
    if (plastic.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec3 Kd = plastic.Kd;
    vec3 Ks = plastic.Ks;

    if (plastic.KdIdx != -1) {
        vec2 uv = getUV();
        Kd = texture(textures[plastic.KdIdx], uv).rgb;
    }
    if (plastic.KsIdx != -1) {
        vec2 uv = getUV();
        Ks = texture(textures[plastic.KsIdx], uv).rgb;
    }

    vec3 F0 = Ks;
    vec3 H = normalize(wo + wi);

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH_final = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH_final, F0);

    vec3 diffusef = Kd / PI * (vec3(1.0) - F);
    vec3 specularf = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);

    f = diffusef + specularf;

    float diffusePdf = max(dot(N, wi), 0.0) / PI;
    float specularPdf = (D * NdotH) / (4.0 * VdotH_final + 0.01);

    float wspec = (F.r + F.g + F.b) / 3.0;
    pdf = diffusePdf * (1.0 - wspec) + specularPdf * wspec;
}

void evalSubstrate(
    in SubstrateGPU substrate, in vec3 wo, in vec3 N, in vec3 wi,
    out vec3 f, out float pdf
) {
    float roughness = substrate.uroughness;
    if (substrate.remaproughness != 0)
        roughness = max(roughness, 0.01);

	vec3 Kd = substrate.Kd;
    vec3 Ks = substrate.Ks;

    if (substrate.KdIdx != -1) {
        vec2 uv = getUV();
        Kd = texture(textures[substrate.KdIdx], uv).rgb;
    }
	if (substrate.KsIdx != -1) {
        vec2 uv = getUV();
        Ks = texture(textures[substrate.KsIdx], uv).rgb;
    }

	vec3 F0 = Ks;
    vec3 H = normalize(wo + wi);

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH_final = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH_final, F0);

    vec3 diffusef = Kd / PI * (vec3(1.0) - F);
    vec3 specularf = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);

    f = diffusef + specularf;

    float diffusePdf = max(dot(N, wi), 0.0) / PI;
    float specularPdf = (D * NdotH) / (4.0 * VdotH_final + 0.01);

    float wspec = (F.r + F.g + F.b) / 3.0;
    pdf = diffusePdf * (1.0 - wspec) + specularPdf * wspec;
}

void evalUber(
    in UberGPU uber, in vec3 wo, in vec3 N, in vec3 wi,
    out vec3 f, out float pdf
) {
    float roughness = uber.uroughness;
    if (uber.remaproughness != 0)
        roughness = max(roughness, 0.01);

    vec3 Kd = uber.Kd;
    vec3 Ks = uber.Ks;
    float eta = uber.eta;

    if (uber.KdIdx != -1) {
        vec2 uv = getUV();
        Kd = texture(textures[uber.KdIdx], uv).rgb;
    }
    if (uber.KsIdx != -1) {
        vec2 uv = getUV();
        Ks = texture(textures[uber.KsIdx], uv).rgb;
    }

    float F0_scalar = pow((eta - 1.0) / (eta + 1.0), 2.0);
    vec3 F0 = Ks * F0_scalar;

    float F_spec = luminance(F0);

    vec3 H = normalize(wo + wi);

    float NdotL = max(dot(N, wi), 0.01);
    float NdotV = max(dot(N, wo), 0.01);
    float NdotH = max(dot(N, H), 0.01);
    float VdotH_final = max(dot(wo, H), 0.01);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH_final, F0);

    vec3 diffusef = Kd / PI * (1.0 - F_spec);
    vec3 specularf = (D * G * F0) / max(4.0 * NdotV * NdotL, 0.01);

    f = diffusef + specularf;

    float diffusePdf = max(dot(N, wi), 0.0) / PI;
    float specularPdf = (D * NdotH) / (4.0 * VdotH_final + 0.01);

    pdf = diffusePdf * (1.0 - F_spec) + specularPdf * F_spec;
}

void main() {
	vec3 N, P;
	computeHitNormal(N, P);
	vec3 wo = -normalize(gl_WorldRayDirectionEXT);
    payload.normal = N;
    payload.depth = length(P - gl_WorldRayOriginEXT);

    const uint instanceID = gl_InstanceCustomIndexEXT;
    payload.meshID = int(instanceID);
    ShapeGPU shape = instances[instanceID];

	if (shape.areaLightIdx >= 0) {
		vec3 L = lights[shape.areaLightIdx].L;

        if (luminance(L) > 30.0) {
            L *= 30.0 / luminance(L);
        }

        payload.L_indirect += payload.beta * L;
		payload.terminated = 1;
		return;
	}

    int matIdx = shape.materialIdx;
    if (matIdx < 0) {
		payload.terminated = 1;
        return;
    }

	MaterialGPU mat = materials[matIdx];
    int matType = mat.type;

	vec3 wi = vec3(0.0);
	vec3 f = vec3(0.0);
	float pdf = 1.0;

	switch (matType) {
		case MATERIAL_MATTE:
			sampleMatte(mattes[mat.index], wo, N, payload.seed, wi, f, pdf);
			break;
		case MATERIAL_METAL:
			sampleMetal(metals[mat.index], wo, N, payload.seed, wi, f, pdf);
			break;
		case MATERIAL_GLASS:
			sampleGlass(glasses[mat.index], wo, N, payload.seed, wi, f, pdf);
			break;
		case MATERIAL_MIRROR:
			sampleMirror(mirrors[mat.index], wo, N, payload.seed, wi, f, pdf);
			break;
		case MATERIAL_PLASTIC:
			samplePlastic(plastics[mat.index], wo, N, payload.seed, wi, f, pdf);
			break;
		case MATERIAL_SUBSTRATE:
			sampleSubstrate(substrates[mat.index], wo, N, payload.seed, wi, f, pdf);
			break;
		case MATERIAL_UBER:
			sampleUber(ubers[mat.index], wo, N, payload.seed, wi, f, pdf);
			break;
		default:
			payload.terminated = 1;
			return;
	}

    float cosTheta = max(dot(wi, N), 0.0);
    vec3 beta_pre = payload.beta;
    payload.beta *= f * cosTheta / max(pdf, 0.01);
    payload.nextOrigin = P + wi * 0.0001;
    payload.nextDir = wi;
    payload.terminated = 0;
    payload.bounce += 1;
    payload.lastPdf = pdf;

    // if (payload.bounce > 1) {
    //     return ;
    // }

    vec3 light_wi = vec3(0.0);
    vec3 le = vec3(0.0);
    vec3 light_f = vec3(0.0);
    float light_pdf = 0.0;

    sampleLight(P, N, payload.seed, light_wi, le, light_pdf);

    isShadowed = true;
    traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
        0xFF, 0, 0, 1, P + N * 0.01, 0.001, normalize(light_wi), length(light_wi) * 0.999, 1);

    if (isShadowed || dot(light_wi, N) < 0.0  || light_pdf <= 0.0) {
        // direct light 
        // payload.terminated = 1;
        return;
    }

    light_wi = normalize(light_wi);
    float bsdfPdf = 0.0;

    switch (matType) {
        case MATERIAL_MATTE:
			evalMatte(mattes[mat.index], wo, N, light_wi, light_f, bsdfPdf);
			break;
		case MATERIAL_METAL:
			evalMetal(metals[mat.index], wo, N, light_wi, light_f, bsdfPdf);
			break;
		case MATERIAL_GLASS:
			evalGlass(glasses[mat.index], wo, N, light_wi, light_f, bsdfPdf);
			break;
		case MATERIAL_PLASTIC:
			evalPlastic(plastics[mat.index], wo, N, light_wi, light_f, bsdfPdf);
			break;
		case MATERIAL_SUBSTRATE:
			evalSubstrate(substrates[mat.index], wo, N, light_wi, light_f, bsdfPdf);
			break;
		case MATERIAL_UBER:
			evalUber(ubers[mat.index], wo, N, light_wi, light_f, bsdfPdf);
			break;
    }
    if (light_f == vec3(0.0) || bsdfPdf <= 0.0 || light_f.r < 0.0 || light_f.g < 0.0 || light_f.b < 0.0) {
        // direct light 
        // payload.terminated = 1;
        return;
    }

    float misWeightLight = light_pdf / (light_pdf + bsdfPdf);

    vec3 direct = (light_f * le * max(dot(N, light_wi), 0.0)) / max(light_pdf, 0.01);

    if (luminance(direct) > 30.0) {
        direct *= 30.0 / luminance(direct);
    }

    payload.L_direct += (beta_pre * direct * misWeightLight);

    // direct light 
    // payload.terminated = 1;
}
