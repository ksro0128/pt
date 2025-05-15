#version 460

layout(set = 0, binding = 0) readonly buffer ExposureBuffer {
    float exposureSum;
};
layout(set = 0, binding = 1) uniform sampler2D pingImage;

layout(push_constant) uniform PushConstants {
    uvec2 imageSize;
} pc;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

vec3 Uncharted2Tonemap(vec3 x) {
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
}

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 toneMapACES(vec3 hdr) {
    const mat3 ACESInputMat = mat3(
         0.59719, 0.35458, 0.04823,
         0.07600, 0.90834, 0.01566,
         0.02840, 0.13383, 0.83777
    );
    const mat3 ACESOutputMat = mat3(
         1.60475, -0.53108, -0.07367,
        -0.10208,  1.10813, -0.00605,
        -0.00327, -0.07276,  1.07602
    );
    hdr = ACESInputMat * hdr;
    hdr = RRTAndODTFit(hdr);
    return clamp(ACESOutputMat * hdr, 0.0, 1.0);
}

vec3 toneMapReinhard(vec3 x) {
    return x / (1.0 + x);
}

void main() {
    vec3 hdr = texture(pingImage, uv).rgb;

    float avgLuminance = exposureSum / float(pc.imageSize.x * pc.imageSize.y);
    float exposure = 0.18 / max(avgLuminance, 1e-4);

    // float logAvgLum = exp(exposureSum / float(pc.imageSize.x * pc.imageSize.y));
    // float exposure = 0.18 / max(logAvgLum, 1e-4);

    // vec3 mapped = hdr * exposure;
    // vec3 ldr = mapped / (vec3(1.0) + mapped);
    // ldr = pow(ldr, vec3(1.0 / 2.2));
    // outColor = vec4(ldr, 1.0);
    // outColor = vec4(mapped, 1.0);

    // Gamma correction (sRGB)
    // hdr = toneMapReinhard(hdr);

    vec3 ldr = pow(hdr, vec3(1.0 / 2.2));
    outColor = vec4(ldr, 1.0);
}