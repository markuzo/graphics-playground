#version 450 core

layout(location = 6) uniform vec4 eye;
layout(location = 7) uniform vec4 light;

in vec3 normal;
in vec4 vertexMV;
in float z;

layout(location = 0) out vec4 outColor;

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
    vec3 n = normalize(normal);
    vec3 e = normalize(vec3(eye - vertexMV));
    vec3 l = normalize(vec3(light - vertexMV));

    // double sided lighting
    if (dot(n,e) < 0.0) n = -n;

    vec3 r = reflect(-l, n);

    float diff = max(dot(n,l), 0.0);
    //vec4 diffuse = diff * vec4(0,1,1,1);
    vec4 diffuse = diff * hsv2rgb(hue2hsv(z));

    float spec = pow(max(dot(e, r), 0.0), 32);
    vec4 specular = 0.25f * spec * vec4(1,0,0,1);

    outColor = diffuse + specular;
}
