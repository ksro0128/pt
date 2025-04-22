#version 460
#extension GL_EXT_ray_tracing : require

struct ShadowPayload {
    bool visible;
};

layout(location = 0) rayPayloadEXT ShadowPayload payload;

void main() {
    payload.visible = true;
}
