#version 450 core

in vec2 UV;

layout(location = 1) uniform sampler2D gPositionTexture;
layout(location = 2) uniform sampler2D gNormalTexture;
layout(location = 3) uniform sampler2D gAlbedoTexture;

out vec4 color;

void main() {
    color = texture(gPositionTexture, UV);
}
