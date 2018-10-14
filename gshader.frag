#version 450 core

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outAlbedo;

in vec2 uv;
in vec4 vertexMV;
in vec3 normal;
in float z;

vec3 hue2hsv(float h)
{
    // range is 0 -> 0.02
    return vec3((h+0.015)*20, 0.7f, 0.7f);
}

vec4 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    vec3 hsv = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    return vec4(hsv,1.0f);
}

void main()
{
    outPosition = vertexMV.xyz;
    outNormal = normalize(normal);
    //outAlbedo = vec3(hsv2rgb(hue2hsv(z)));
    outAlbedo = vec3(0.95);
}
