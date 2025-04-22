#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require

struct Vertex {
    vec3 pos; float pad0;
    vec3 normal; float pad1;
    vec2 texCoord; vec2 pad2;
    vec3 tangent; float pad3;
};

layout(buffer_reference, scalar) buffer Vertices { Vertex v[]; };
layout(buffer_reference, scalar) buffer Indices  { uvec3 i[]; };

struct RayPayload {
    vec3 color;
	vec3 beta;
    int bounce;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

layout(location = 1) rayPayloadEXT bool isShadowed;

struct Light {
    int type;
    int shadowMapIndex;
    int castsShadow;
    float intensity;

    vec3 color;
    float range;

    vec3 position;
    float spotInnerAngle;

    vec3 direction;
    float spotOuterAngle;
};

layout(set = 0, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 proj;
    vec3 camPos;
	int frameCount;
} camera;


layout(set = 0, binding = 1) readonly buffer LightBuffer {
    Light lights[64];
    vec3 ambientColor;
    int lightCount;
}lightInfo;

layout(set = 1, binding = 1) uniform accelerationStructureEXT topLevelAS;

struct ObjectInstance {
    uint64_t vertexIndex;
    uint64_t indexIndex;
    int modelMatrixIndex;
    int materialIndex;
    int meshIndex;
    int pad;
};

layout(std430, set = 2, binding = 0) readonly buffer ObjectInstances {
    ObjectInstance instances[];
};

struct Material {
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

layout(std430, set = 3, binding = 0) readonly buffer ModelBuffer {
    mat4 modelMatrices[];
};

layout(std430, set = 3, binding = 1) readonly buffer MaterialBuffer {
    Material materials[];
};

layout(set = 3, binding = 2) uniform sampler2D textures[];

hitAttributeEXT vec2 attribs;


float hash1(vec2 p) {
    p = fract(p * vec2(123.34, 345.45));
    p += dot(p, p + 34.345);
    return fract(p.x * p.y);
}

float vdcSequence(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersleySequence(uint i, uint N)
{
    return vec2(float(i) / float(N), vdcSequence(i));
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

uint initRandom(in uvec2 resolution, in uvec2 screenCoord, in uint frame)
{
  return tea(screenCoord.y * resolution.x + screenCoord.x, frame);
}

void main() {
    ObjectInstance inst = instances[gl_InstanceCustomIndexEXT];
    Vertices vertices = Vertices(inst.vertexIndex);
    Indices indices = Indices(inst.indexIndex);

    uvec3 idx = indices.i[gl_PrimitiveID];
    Vertex v0 = vertices.v[idx.x];
    Vertex v1 = vertices.v[idx.y];
    Vertex v2 = vertices.v[idx.z];

    float u = attribs.x;
    float v = attribs.y;
    float w = 1.0 - u - v;
    vec2 uv = v0.texCoord * w + v1.texCoord * u + v2.texCoord * v;

    Material mat = materials[inst.materialIndex];

	mat4 model = modelMatrices[inst.modelMatrixIndex];

	vec3 normal;
	if (mat.normalTexIndex != -1) {
		vec3 tangent = normalize(v0.tangent);
		vec3 bitangent = normalize(cross(tangent, v0.normal));
		mat3 TBN = mat3(tangent, bitangent, v0.normal);

		vec3 sampledNormal = texture(textures[nonuniformEXT(mat.normalTexIndex)], uv).rgb;
		sampledNormal = normalize(sampledNormal * 2.0 - 1.0);
		normal = normalize(TBN * sampledNormal);
	} else {
		vec3 localNormal = normalize(v0.normal * w + v1.normal * u + v2.normal * v);
		mat3 normalMatrix = transpose(inverse(mat3(model)));
		normal = normalize(normalMatrix * localNormal);
	}
	vec3 albedo = (mat.albedoTexIndex == -1) 
		? mat.baseColor.rgb 
		: mat.baseColor.rgb * texture(textures[nonuniformEXT(mat.albedoTexIndex)], uv).rgb;
    vec3 emissive = (mat.emissiveTexIndex == -1)
        ? mat.emissiveFactor
        : mat.emissiveFactor * texture(textures[nonuniformEXT(mat.emissiveTexIndex)], uv).rgb;

	float roughness = (mat.roughnessTexIndex == -1) ? mat.roughness : mat.roughness * texture(textures[nonuniformEXT(mat.roughnessTexIndex)], uv).g;
	if (roughness < 0.04) {
        roughness = 0.04;
    }
	float metallic = (mat.metallicTexIndex == -1) ? mat.metallic : mat.metallic * texture(textures[nonuniformEXT(mat.metallicTexIndex)], uv).b;
	float ao = (mat.aoTexIndex == -1) ? mat.ao : mat.ao * texture(textures[nonuniformEXT(mat.aoTexIndex)], uv).r;


    vec3 Li = vec3(0.0);
    // Li += emissive;
    // Li += lightInfo.ambientColor * albedo * ao * payload.beta;

    


    {
        Light light = lightInfo.lights[0];
        vec3 L = normalize(-light.direction);
        float attenuation = 1.0;
        vec3 N = normal;
        vec3 worldPos  = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

        isShadowed = true;
        traceRayEXT(topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
        0xFF, 0, 0, 1, worldPos + L * 0.01, 0.001, L, 1e4, 1);

        if (isShadowed) {
            attenuation = 0.0;
        } else {
            attenuation = 1.0;
        }

		vec3 V = normalize(gl_WorldRayOriginEXT - worldPos);

        vec3 ambient = lightInfo.ambientColor * albedo * ao;
        vec3 H = normalize(V + L);
        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);

        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 1e-3) * max(dot(N, L), 1e-3) + 1e-5;
        vec3 specular = numerator / denominator;
        
        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.01);
        vec3 diffuse = kD * albedo / 3.14159265359;
        vec3 radiance = light.intensity * light.color * attenuation * NdotL;

        Li += (ambient + (diffuse + specular) * radiance) * payload.beta;
    }



	if (payload.bounce - 1 > 0) {
		vec3 worldPos  = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
		vec3 worldNormal = normal;
		vec3 viewDir = normalize(gl_WorldRayOriginEXT - worldPos);
		vec3 F0 = mix(vec3(0.04), albedo, metallic);
		
        // uint seed = gl_LaunchIDEXT.x * 1973u ^ gl_LaunchIDEXT.y * 9277u ^ uint(payload.bounce);
		// vec2 Xi = hammersleySequence(seed, 1024);
        uint baseSeed = initRandom(uvec2(gl_LaunchSizeEXT.xy), uvec2(gl_LaunchIDEXT.xy), uint(camera.frameCount));
        vec2 Xi = vec2(
          float(tea(baseSeed, 0)) / float(0xFFFFFFFFu),
          float(tea(baseSeed, 1)) / float(0xFFFFFFFFu)
        );

		vec3 H = importanceSampleGGX(Xi, worldNormal, roughness);
		vec3 L = normalize(reflect(-viewDir, H));
		if (dot(worldNormal, L) > 0.0) {
			float pdf = ggxPdf(worldNormal, H, viewDir, roughness);
			if (pdf < 1e-5) {
				payload.color = Li;
				return;
			}
			float NoV = max(dot(worldNormal, viewDir), 1e-3);
			float NoL = max(dot(worldNormal, L), 1e-3);
			vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);
			float NDF = distributionGGX(worldNormal, H, roughness);
			float G = geometrySmith(worldNormal, viewDir, L, roughness);
			vec3 brdf = (NDF * G * F) / (4.0 * NoV * NoL + 1e-5);
			vec3 beta = payload.beta * brdf * NoL / max(pdf, 1e-5);

			payload.color = vec3(0.0);
			payload.beta = beta;
			payload.bounce = payload.bounce - 1;

			traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0,
				worldPos + L * 0.01, 0.001, L, 1e4, 0);

			Li += payload.color;
		}
	}

	payload.color = Li;
}
