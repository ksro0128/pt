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
    vec3 ldr = pow(hdr, vec3(1.0 / 2.2));
    outColor = vec4(ldr, 1.0);
}