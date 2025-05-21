#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require


layout (set = 0, binding = 0) uniform CameraGPU {
    vec3 camPos;
    float pad0;
    vec3 camDir;
    float pad1;
    vec3 camUp;
    float pad2;
    vec3 camRight;
    float fovY;
} camera;

layout (set = 0, binding = 1) uniform OptionsGPU {
    int frameCount;
    int maxSpp;
    int currentSpp;
    int lightCount;
} options;

struct MaterialGPU {
    vec4 baseColor;
    vec3 emissiveFactor;
    float roughness;

    float metallic;
    float ao;
    int albedoTexIndex;
    int normalTexIndex;

    int metallicTexIndex;
    int roughnessTexIndex;
    int aoTexIndex;
    int emissiveTexIndex;
};

layout (set = 1, binding = 0) buffer MaterialBuffer {
    MaterialGPU materials[];
};

layout(set = 2, binding = 0) uniform sampler2D textures[];

struct InstanceGPU {
    mat4 transform;

    uint64_t vertexAddress;
    uint64_t indexAddress;

    int lightIndex;
    int materialIndex;
    int meshIndex;
    float pad0;
};
layout(set = 3, binding = 0) buffer InstanceBuffer {
    InstanceGPU instances[];
};

struct AreaLightGPU {
    vec3 color;
    float intensity;

    vec3 p0;
    float area;
    vec3 p1;
    float pad0;
    vec3 p2;
    float pad1;
    vec3 p3;
    float pad2;

    vec3 normal;
    float pad3;
};
layout(set = 3, binding = 1) buffer AreaLightBuffer {
    AreaLightGPU areaLights[];
};

layout(set = 4, binding = 0) uniform accelerationStructureEXT topLevelAS;

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

layout(location = 1) rayPayloadEXT bool isShadowed;

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


vec2 sample2D(inout uint seed)
{
    float u = rand(seed);
    float v = rand(seed);
    return vec2(u, v);

    // return halton(seed);
}

vec3 cosineSampleHemisphere(inout uint seed) {
    float u1 = rand(seed);
    float u2 = rand(seed);

    // vec2 xy = halton(seed);
    // float u1 = xy.x;
    // float u2 = xy.y;

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
    vec4 tangent;
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
    InstanceGPU instance = instances[instanceID];

    Vertices vertices = Vertices(instance.vertexAddress);
    Indices indices = Indices(instance.indexAddress);

    uvec3 idx = indices.i[gl_PrimitiveID];
    Vertex v0 = vertices.v[idx.x];
    Vertex v1 = vertices.v[idx.y];
    Vertex v2 = vertices.v[idx.z];

    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;

    vec3 localNormal = normalize(v0.normal * w + v1.normal * u + v2.normal * v);
    mat3 normalMatrix = transpose(inverse(mat3(instance.transform)));

    int matIdx = instance.materialIndex;
    if (matIdx < 0) {
        N = normalize(normalMatrix * localNormal);
    } else {
        MaterialGPU mat = materials[matIdx];

        if (mat.normalTexIndex < 0) {
            N = normalize(normalMatrix * localNormal);
        } else {
            // vec3 tangent = normalize(v0.tangent * w + v1.tangent * u + v2.tangent * v);
            // vec2 uv = v0.texCoord * w + v1.texCoord * u + v2.texCoord * v;
            // vec3 bitangent = normalize(cross(localNormal, tangent));

            // vec3 nTex = texture(textures[nonuniformEXT(mat.normalTexIndex)], uv).rgb;
            // vec3 nTS = normalize(nTex * 2.0 - 1.0);

            // mat3 TBN = mat3(tangent, bitangent, localNormal);
            // vec3 normalObject = normalize(TBN * nTS);
            // N = normalize(normalMatrix * normalObject);
            vec4 tangent4 = v0.tangent * w + v1.tangent * u + v2.tangent * v;
            vec3 tangent = normalize(tangent4.xyz);
            float handedness = tangent4.w;

            vec2 uv = v0.texCoord * w + v1.texCoord * u + v2.texCoord * v;
            vec3 bitangent = normalize(cross(localNormal, tangent) * handedness);

            vec3 nTex = texture(textures[nonuniformEXT(mat.normalTexIndex)], uv).rgb;
            vec3 nTS = normalize(nTex * 2.0 - 1.0);

            mat3 TBN = mat3(tangent, bitangent, localNormal);
            vec3 normalObject = normalize(TBN * nTS);
            N = normalize(normalMatrix * normalObject);
        }
    }
    if (dot(N, gl_WorldRayDirectionEXT) > 0.0) {
        N = normalize(-N);
    }
    pos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
}


vec2 getUV() {
    uint instanceID = gl_InstanceCustomIndexEXT;
    InstanceGPU instance = instances[instanceID];

    Vertices vertices = Vertices(instance.vertexAddress);
    Indices indices = Indices(instance.indexAddress);

    uvec3 idx = indices.i[gl_PrimitiveID];
    Vertex v0 = vertices.v[idx.x];
    Vertex v1 = vertices.v[idx.y];
    Vertex v2 = vertices.v[idx.z];

    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;

    vec2 uv = v0.texCoord * w + v1.texCoord * u + v2.texCoord * v;
    // return vec2(uv.x, 1.0 - uv.y);
    return uv;
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
    float a = max(roughness * roughness, 0.001);
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.001);
    float NdotH2 = NdotH * NdotH;
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265359 * denominator * denominator);
}

float ggxPdf(vec3 N, vec3 H, vec3 V, float roughness) {
    float D = distributionGGX(N, H, roughness);
    float NdotH = max(dot(N, H), 0.001);
    float VdotH = max(dot(V, H), 0.001);
    return D * NdotH / (4.0 * VdotH + 0.001); // pdf = D(h) * (N⋅H) / (4 * (V⋅H))
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
    roughness = max(roughness, 0.001);
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.001);
    float NdotL = max(dot(N, L), 0.001);
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

bool isproblem(vec3 v) {
    return any(isnan(v)) || any(isinf(v));
}


void sampleIndirect(in vec3 N, in vec3 P, in vec3 wo, in vec3 Kd, in float roughness, in float metallic, in vec3 F0, in float probSpec)
{
    int sampledSpecular = rand(payload.seed) < probSpec ? 1 : 0;

    vec3 H = vec3(0.0);
    vec3 wi = vec3(0.0);
    if (sampledSpecular != 0) {
        vec2 Xi = sample2D(payload.seed);
        H = importanceSampleGGX(Xi, N, roughness);
        wi = normalize(reflect(-wo, H));
    } else {
        vec3 localWi = cosineSampleHemisphere(payload.seed);
        wi = toWorld(localWi, N);
        wi = normalize(wi);
        H = normalize(wo + wi);
    }

    float NdotL = max(dot(N, wi), 0.001);
    float NdotV = max(dot(N, wo), 0.001);
    float NdotH = max(dot(N, H), 0.001);
    float VdotH = max(dot(wo, H), 0.001);

    if (sampledSpecular == 1 && dot(wo, N) * dot(wi, N) < 0.0) {
        payload.terminated = 1;
        return;
    }

    float D = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, wo, wi, roughness);
    vec3 F  = fresnelSchlick(VdotH, F0);

    vec3 specularf = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);
    vec3 diffusef = Kd / PI;

    float pdf_diff = NdotL / PI * (1 - probSpec);
    float pdf_spec = D * NdotH / (4.0 * VdotH + 0.001) * probSpec;

    float sumPdf = pdf_diff + pdf_spec;
    float misWeight = (sampledSpecular != 0) ? (pdf_spec / sumPdf) : (pdf_diff / sumPdf);

    vec3 f = (sampledSpecular != 0) ? specularf * misWeight : diffusef * misWeight;
    float pdf = (sampledSpecular != 0) ? pdf_spec : pdf_diff;

    payload.beta *= f * NdotL / max(pdf, 0.001);
    payload.nextOrigin = P + wi * 0.0001;
    payload.nextDir = wi;
    payload.terminated = 0;
    payload.pdf = pdf;
}

void sampleDirect(in vec3 N, in vec3 P, in vec3 wo, in vec3 Kd, in float roughness, in float metallic, in vec3 F0, in float probSpec) {
    int lightIdx = int(mod(rand(payload.seed) * float(options.lightCount), float(options.lightCount)));
    AreaLightGPU light = areaLights[lightIdx];

    float u = rand(payload.seed);
    float v = rand(payload.seed);
    vec3 sampledPos;
    if (u + v <= 1.0) {
        sampledPos = light.p0 * (1.0 - u - v)
                   + light.p1 * u
                   + light.p2 * v;
    } else {
        u = 1.0 - u;
        v = 1.0 - v;
        sampledPos = light.p2 * (1.0 - u - v)
                   + light.p3 * u
                   + light.p0 * v;
    }

    vec3 lightNormal = normalize(light.normal);
    vec3 dir = sampledPos - P;
    float dist = length(dir);
    float dist2 = dist * dist;
    vec3 L_wi = normalize(dir);

    float cosTheta = max(dot(lightNormal, -L_wi), 0.001);
    float areaPdf = 1.0 / light.area;
    float solidAnglePdf = dist2 / (cosTheta + 0.001) * areaPdf;
    float L_pdf = solidAnglePdf / float(options.lightCount);

    isShadowed = true;
    traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
        0xFF, 0, 0, 1, P + N * 0.0001, 0.0001, L_wi, dist * 0.99, 1);

    if (isShadowed || dot(L_wi, N) < 0.0  || L_pdf <= 0.0) {
        return ;
    }
    else {
        int sampledSpecular = rand(payload.seed) < probSpec ? 1 : 0;
        vec3 H = normalize(wo + L_wi);

        float NdotL = max(dot(N, L_wi), 0.001);
        float NdotV = max(dot(N, wo), 0.001);
        float NdotH = max(dot(N, H), 0.001);
        float VdotH = max(dot(wo, H), 0.001);

        float D = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, wo, L_wi, roughness);
        vec3 F  = fresnelSchlick(VdotH, F0);

        vec3 specularf = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);
        vec3 diffusef = Kd / PI;

        float pdf_diff = NdotL / PI * (1 - probSpec);
        float pdf_spec = D * NdotH / (4.0 * VdotH + 0.001) * probSpec;

        float sumPdf = pdf_diff + pdf_spec;
        float misWeight = (sampledSpecular != 0) ? (pdf_spec / sumPdf) : (pdf_diff / sumPdf);

        vec3 f = (sampledSpecular != 0) ? specularf * misWeight : diffusef * misWeight;
        float pdfBRDF = (sampledSpecular != 0) ? pdf_spec : pdf_diff;

        vec3 direct = (f * light.color * light.intensity * NdotL) / L_pdf;
        float w = L_pdf / (L_pdf + pdfBRDF);
        payload.L += payload.beta * direct * w; 
    }

}

void main() {
    uint instanceID = gl_InstanceCustomIndexEXT;
    InstanceGPU instance = instances[instanceID];

    vec3 N, P;
    computeHitNormal(N, P);
    vec3 wo = -normalize(gl_WorldRayDirectionEXT);
    
    if (instance.lightIndex >= 0) {
        AreaLightGPU light = areaLights[instance.lightIndex];

        vec3 lightNormal = normalize(light.normal);

        if (dot(lightNormal, wo) < 0) {
            payload.terminated = 1;
            return ;
        }

        if (payload.bounce == 0) {
            payload.L = light.color;
            payload.terminated = 1;
            return ;
        }

        vec3 dir = P - gl_WorldRayOriginEXT;
        float dist = length(dir);
        float dist2 = dist * dist;
        vec3 L_wi = normalize(dir);

        float cosTheta = max(dot(lightNormal, -L_wi), 0.001);
        float areaPdf = 1.0 / light.area;
        float solidAnglePdf = dist2 / (cosTheta + 0.001) * areaPdf;
        float L_pdf = solidAnglePdf / float(options.lightCount);

        float w = L_pdf / (L_pdf + payload.pdf);

        payload.L = light.color * light.intensity * payload.beta * w;
        payload.terminated = 1;
        return;
    }

    if (dot(N, wo) < 0) {
        payload.terminated = 1;
        return ;
    }

    int matIdx = instance.materialIndex;
    MaterialGPU mat = materials[matIdx];


    vec3 Kd = mat.baseColor.rgb;
    if (mat.albedoTexIndex >= 0) {
        vec2 uv = getUV();
        Kd *= texture(textures[nonuniformEXT(mat.albedoTexIndex)], uv).rgb;
    }

    float roughness = mat.roughness;
    if (mat.roughnessTexIndex >= 0) {
        vec2 uv = getUV();
        roughness *= texture(textures[nonuniformEXT(mat.roughnessTexIndex)], uv).g;
    }
    roughness = max(roughness, 0.04);


    float metallic = mat.metallic;
    if (mat.metallicTexIndex >= 0) {
        vec2 uv = getUV();
        metallic *= texture(textures[nonuniformEXT(mat.metallicTexIndex)], uv).b;
    }

    vec3 F0 = mix(vec3(0.04), Kd, metallic);
    float probSpec = max(max(F0.r, F0.g), F0.b);
    
    sampleDirect(N, P, wo, Kd, roughness, metallic, F0, probSpec);
    
    float ao = mat.ao;
    if (mat.aoTexIndex >= 0) {
        vec2 uv = getUV();
        ao *= texture(textures[nonuniformEXT(mat.aoTexIndex)], uv).r;
    }
    payload.beta *= ao;
    
    
    
    sampleIndirect(N, P, wo, Kd, roughness, metallic, F0, probSpec);

}
