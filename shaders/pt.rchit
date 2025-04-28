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

layout(set = 3, binding = 0) uniform sampler2D textures[];

layout(set = 4, binding = 0) uniform accelerationStructureEXT topLevelAS;
layout(set = 4, binding = 1, rgba16f) uniform image2D pingImage;
layout(set = 4, binding = 2, rgba16f) uniform image2D pongImage;

struct RayPayload {
    vec3 L;
    vec3 beta;
    vec3 nextOrigin;
    vec3 nextDir;
    int depth;
    uint seed;
    bool terminated;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

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

float rand(inout uint seed) {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return float(seed & 0x00ffffffu) / float(0x01000000u);
}

vec2 sample2D(inout uint seed)
{
    float u = rand(seed);
    float v = rand(seed);
    return vec2(u, v);
}

vec3 cosineSampleHemisphere(inout uint seed) {
    float u1 = rand(seed);
    float u2 = rand(seed);
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
    vec2 uv = v0.texCoord * w + v1.texCoord * u + v2.texCoord * v;

	mat4 model = shape.modelMatrix;

	vec3 localNormal = normalize(v0.normal * w + v1.normal * u + v2.normal * v);
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	N = normalize(normalMatrix * localNormal);

	if (shape.reverseOrientation == 1) {
		N = -N;
	}

    // compute hit position
	pos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
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
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha2 - 1.0) * Xi.y));
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
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265359 * denominator * denominator);
}

float ggxPdf(vec3 N, vec3 H, vec3 V, float roughness) {
    float D = distributionGGX(N, H, roughness);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    return D * NdotH / (4.0 * VdotH + 1e-5); // pdf = D(h) * (N⋅H) / (4 * (V⋅H))
}

// Fresnel-Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Geometry Function
float geometrySchlickGGX(float NdotV, float roughness) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
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
    vec3 a2plusb2 = sqrt(t0 * t0 + 4.0 * eta2 * k2);
    vec3 t1 = a2plusb2 + vec3(cos2Theta);
    vec3 t2 = (a2plusb2 * cos2Theta) + vec3(1.0);

    vec3 Rs = (t1 - 2.0 * eta * cosTheta) / (t1 + 2.0 * eta * cosTheta);
    vec3 Rp = (t2 - 2.0 * eta * cosTheta) / (t2 + 2.0 * eta * cosTheta);

    return 0.5 * (Rs * Rs + Rp * Rp);
}

float fresnelDielectric(float cosThetaI, float etaI, float etaT) {
    cosThetaI = clamp(cosThetaI, -1.0, 1.0);

    bool entering = cosThetaI > 0.0;
    if (!entering) {
        float temp = etaI;
        etaI = etaT;
        etaT = temp;
        cosThetaI = abs(cosThetaI);
    }

    float sinThetaI = sqrt(max(0.0, 1.0 - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    if (sinThetaT >= 1.0) {
        return 1.0;
    }

    float cosThetaT = sqrt(max(0.0, 1.0 - sinThetaT * sinThetaT));
    float rParl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    float rPerp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (rParl * rParl + rPerp * rPerp) * 0.5;
}

void evaluateMatte(
	in MatteGPU matte, in vec3 wo, in vec3 wi, in vec3 N, 
	out vec3 f, out float pdf) {
     vec3 Kd = matte.Kd;
    if (matte.KdIdx >= 0) {
        vec2 uv = getUV();
        Kd = texture(textures[matte.KdIdx], uv).rgb;
    }

    f = Kd / PI;

    pdf = max(dot(N, wi), 0.0) / PI;
}

void evaluateMetal(
	in MetalGPU metal, in vec3 wo, in vec3 wi, in vec3 N, in vec3 H,
	out vec3 f, out float pdf) {
    vec3 eta = metal.eta;
    vec3 k = metal.k;

	vec2 uv = getUV();
    if (metal.etaIdx != -1) {
        eta = texture(textures[metal.etaIdx], uv).rgb;
    }
    if (metal.kIdx != -1) {
        k = texture(textures[metal.kIdx], uv).rgb;
    }

    float roughness = metal.uroughness;
    if (metal.remaproughness != 0) {
        roughness = max(roughness, 0.001);
    }

    float NdotL = max(dot(N, wi), 0.05);
    float NdotV = max(dot(N, wo), 0.05);
    float NdotH = max(dot(N, H), 0.05);
    float VdotH = max(dot(wo, H), 0.05);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
	vec3 F = fresnelConductor(VdotH, eta, k);

    f = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);

    pdf = D * NdotH / (4.0 * VdotH + 0.01);
}

void evaluateGlass(
    in GlassGPU glass, in vec3 wo, in vec3 wi, in vec3 N, in vec3 H,
    out vec3 f, out float pdf)
{
    vec3 Kr = glass.Kr;
    vec3 Kt = glass.Kt;
    float eta = glass.eta;

	vec2 uv = getUV();
    if (glass.KrIdx != -1) {
        Kr = texture(textures[glass.KrIdx], uv).rgb;
    }
    if (glass.KtIdx != -1) {
        Kt = texture(textures[glass.KtIdx], uv).rgb;
    }
    if (glass.etaIdx != -1) {
        eta = texture(textures[glass.etaIdx], uv).r;
    }

    float roughness = glass.uroughness;
    if (glass.remaproughness != 0)
        roughness = max(roughness, 0.001);

    float NdotL = max(dot(N, wi), 0.05);
    float NdotV = max(dot(N, wo), 0.05);
    float NdotH = max(dot(N, H), 0.05);
    float VdotH = max(dot(wo, H), 0.05);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    float F = fresnelDielectric(VdotH, 1.0, eta);

    vec3 spec = (D * G * Kr * F) / max(4.0 * NdotV * NdotL, 0.01);

    f = spec;
    pdf = D * NdotH / (4.0 * VdotH + 0.01);
}


void evaluateMirror(in MirrorGPU mirror, in vec3 wo, in vec3 wi, in vec3 N, out vec3 f, out float pdf) {
    vec3 Kr = mirror.Kr;

    if (mirror.KrIdx != -1) {
        vec2 uv = getUV();
        Kr = texture(textures[mirror.KrIdx], uv).rgb;
    }

    float NdotL = max(dot(N, wi), 0.0);

    if (NdotL > 0.0) {
        f = Kr / NdotL;
        pdf = 1.0;
    } else {
        f = vec3(0.0);
        pdf = 0.0;
    }
}

void evaluatePlastic(
    in PlasticGPU plastic, in vec3 wo, in vec3 wi, in vec3 N, in vec3 H,
    out vec3 f, out float pdf, int sampleType) 
{
    vec3 Kd = plastic.Kd;
    vec3 Ks = plastic.Ks;

	vec2 uv = getUV();
    if (plastic.KdIdx != -1) {
        Kd = texture(textures[plastic.KdIdx], uv).rgb;
    }
    if (plastic.KsIdx != -1) {
        Ks = texture(textures[plastic.KsIdx], uv).rgb;
    }

    float roughness = plastic.roughness;
    if (plastic.remaproughness != 0) {
        roughness = max(roughness, 0.001);
    }

    float NdotL = max(dot(N, wi), 0.05);
    float NdotV = max(dot(N, wo), 0.05);
    float NdotH = max(dot(N, H), 0.05);
    float VdotH = max(dot(wo, H), 0.05);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH, Ks);

    float pdfDiffuse = max(dot(N, wi), 0.0) / PI;
    float pdfSpecular = D * NdotH / max(4.0 * VdotH, 0.01);
    pdf = 0.5 * (pdfDiffuse + pdfSpecular);


    if (sampleType == SAMPLE_DIFFUSE) {
        f = Kd / PI;
    } 
    else if (sampleType == SAMPLE_SPECULAR) {
        f = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);
    } 
    else {
        f = vec3(0.0);
        pdf = 0.0;
    }
}

void evaluateSubstrate(
    in SubstrateGPU substrate, in vec3 wo, in vec3 wi, in vec3 N, in vec3 H,
    out vec3 f, out float pdf, int sampleType) 
{
    vec3 Kd = substrate.Kd;
    vec3 Ks = substrate.Ks;

    vec2 uv = getUV();
    if (substrate.KdIdx != -1) {
        Kd = texture(textures[substrate.KdIdx], uv).rgb;
    }
    if (substrate.KsIdx != -1) {
        Ks = texture(textures[substrate.KsIdx], uv).rgb;
    }

    float roughness = substrate.uroughness;
    if (substrate.remaproughness != 0) {
        roughness = max(roughness, 0.001);
    }

    float NdotL = max(dot(N, wi), 0.05);
    float NdotV = max(dot(N, wo), 0.05);
    float NdotH = max(dot(N, H), 0.05);
    float VdotH = max(dot(wo, H), 0.05);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH, Ks);

    float pdfDiffuse = max(dot(N, wi), 0.0) / PI;
    float pdfSpecular = D * NdotH / max(4.0 * VdotH, 0.01);
    pdf = 0.5 * (pdfDiffuse + pdfSpecular);


    if (sampleType == SAMPLE_DIFFUSE) {
        f = Kd / PI;
    } 
    else if (sampleType == SAMPLE_SPECULAR) {
        f = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);
    } 
    else {
        f = vec3(0.0);
        pdf = 0.0;
    }
}

void evaluateUber(
    in UberGPU uber, in vec3 wo, in vec3 wi, in vec3 N, in vec3 H,
    out vec3 f, out float pdf, int sampleType) 
{
    vec3 Kd = uber.Kd;
    vec3 Ks = uber.Ks;

    vec2 uv = getUV();
    if (uber.KdIdx != -1) {
        Kd = texture(textures[uber.KdIdx], uv).rgb;
    }
    if (uber.KsIdx != -1) {
        Ks = texture(textures[uber.KsIdx], uv).rgb;
    }

    float roughness = uber.uroughness;
    if (uber.remaproughness != 0) {
        roughness = max(roughness, 0.001);
    }

    float NdotL = max(dot(N, wi), 0.05);
    float NdotV = max(dot(N, wo), 0.05);
    float NdotH = max(dot(N, H), 0.05);
    float VdotH = max(dot(wo, H), 0.05);

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F = fresnelSchlick(VdotH, Ks);

    float pdfDiffuse = max(dot(N, wi), 0.0) / PI;
    float pdfSpecular = D * NdotH / max(4.0 * VdotH, 0.01);
    pdf = 0.5 * (pdfDiffuse + pdfSpecular);


    if (sampleType == SAMPLE_DIFFUSE) {
        f = Kd / PI;
    } 
    else if (sampleType == SAMPLE_SPECULAR) {
        f = (D * G * F) / max(4.0 * NdotV * NdotL, 0.01);
    } 
    else {
        f = vec3(0.0);
        pdf = 0.0;
    }
}

float getRoughness(int matType, int matIndex) {
    float roughness = 0.5;
    if (matType == MATERIAL_METAL) {
        MetalGPU metal = metals[matIndex];
        roughness = metal.uroughness;
        if (metal.remaproughness != 0)
            roughness = max(roughness, 0.001);
    }
    else if (matType == MATERIAL_GLASS) {
        GlassGPU glass = glasses[matIndex];
        roughness = glass.uroughness;
        if (glass.remaproughness != 0)
            roughness = max(roughness, 0.001);
    }
    else if (matType == MATERIAL_PLASTIC) {
        PlasticGPU plastic = plastics[matIndex];
        roughness = plastic.roughness;
        if (plastic.remaproughness != 0)
            roughness = max(roughness, 0.001);
    }
    else if (matType == MATERIAL_SUBSTRATE) {
        SubstrateGPU sub = substrates[matIndex];
        roughness = sub.uroughness;
        if (sub.remaproughness != 0)
            roughness = max(roughness, 0.001);
    }
    else if (matType == MATERIAL_UBER) {
        UberGPU uber = ubers[matIndex];
        roughness = uber.uroughness;
        if (uber.remaproughness != 0)
            roughness = max(roughness, 0.001);
    }
    return roughness;
}

void sampleDirection(
    int matType, float roughness, inout uint seed, vec3 N, vec3 wo,
    out vec3 H, out vec3 wi, out int sampleType) 
{
    vec3 localWi;

    if (matType == MATERIAL_MATTE) {
        localWi = cosineSampleHemisphere(seed);
        wi = toWorld(localWi, N);
        sampleType = SAMPLE_DIFFUSE;
    }
    else if (matType == MATERIAL_METAL || matType == MATERIAL_GLASS) {
        vec2 Xi = sample2D(seed);
        H = importanceSampleGGX(Xi, N, roughness);
        wi = normalize(reflect(-wo, H));
        sampleType = SAMPLE_SPECULAR;
    }
    else if (matType == MATERIAL_MIRROR) {
        wi = reflect(-wo, N);
        sampleType = SAMPLE_SPECULAR;
    }
    else if (matType == MATERIAL_UBER || matType == MATERIAL_SUBSTRATE || matType == MATERIAL_PLASTIC) {
        float select = rand(seed);
        if (select < 0.5) {
            localWi = cosineSampleHemisphere(seed);
            wi = toWorld(localWi, N);
            sampleType = SAMPLE_DIFFUSE;
        } else {
            vec2 Xi = sample2D(seed);
            H = importanceSampleGGX(Xi, N, roughness);
            wi = normalize(reflect(-wo, H));
            sampleType = SAMPLE_SPECULAR;
        }
    }
    else {
        localWi = cosineSampleHemisphere(seed);
        wi = toWorld(localWi, N);
        sampleType = SAMPLE_DIFFUSE;
    }
}


void main() {
	vec3 N, P;
	computeHitNormal(N, P);
	vec3 wo = -normalize(gl_WorldRayDirectionEXT);

    const uint instanceID = gl_InstanceCustomIndexEXT;
    ShapeGPU shape = instances[instanceID];

	if (shape.areaLightIdx >= 0) {
		vec3 L = lights[shape.areaLightIdx].L;
		if (any(greaterThan(payload.beta, vec3(0.0)))) {
            payload.L += payload.beta * L;
        }
        // payload.L += payload.beta * L;
		payload.terminated = true;
		return;
	}

    int matIdx = shape.materialIdx;
    if (matIdx < 0) {
		payload.terminated = true;
        return;
    }

	MaterialGPU mat = materials[matIdx];
    int matType = mat.type;
    float roughness = getRoughness(matType, mat.index);
    vec3 wi = vec3(0.0);
    int sampleType = SAMPLE_DIFFUSE;
    vec3 H = vec3(0.0);
    sampleDirection(matType, roughness, payload.seed, N, wo, H, wi, sampleType); // out H, wi, sampleType

	vec3 f = vec3(0.0);
	float pdf = 1.0;

    // get f, pdf
	if (matType == MATERIAL_MATTE){
		evaluateMatte(mattes[mat.index], wo, wi, N, f, pdf);
	}
	else if (matType == MATERIAL_METAL){
		evaluateMetal(metals[mat.index], wo, wi, N, H, f, pdf);
	}
	else if (matType == MATERIAL_GLASS){
		evaluateGlass(glasses[mat.index], wo, wi, N, H, f, pdf);
	}
	else if (matType == MATERIAL_MIRROR){
		wi = reflect(-wo, N);
		evaluateMirror(mirrors[mat.index], wo, wi, N, f, pdf);
	}
	else if (matType == MATERIAL_PLASTIC){
		evaluatePlastic(plastics[mat.index], wo, wi, N, H, f, pdf, sampleType);
	}
	else if (matType == MATERIAL_SUBSTRATE){
		evaluateSubstrate(substrates[mat.index], wo, wi, N, H, f, pdf, sampleType);
	}
	else if (matType == MATERIAL_UBER){
		evaluateUber(ubers[mat.index], wo, wi, N, H, f, pdf, sampleType);
	}

	float cosTheta = max(dot(wi, N), 0.0);
	payload.beta *= f * cosTheta / max(pdf, 0.01);

	payload.nextOrigin = P + wi * 0.001;
	payload.nextDir = wi;
	payload.terminated = false;
	payload.depth += 1;
}