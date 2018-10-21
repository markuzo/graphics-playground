#version 450 core

in vec2 UV;

layout(location = 0) uniform sampler2D gPositionTexture;
layout(location = 1) uniform sampler2D gNormalTexture;
layout(location = 2) uniform sampler2D gAlbedoTexture;
layout(location = 3) uniform sampler2D ssaoBlurTexture;

out vec4 color;

void main() {

    vec3 cameraPos = vec3(0,0,0);

    vec3 lightPos = vec3(2,1,2);
    vec3 lightColor = vec3(1,1,1);
    float lightLinear = 0.09f;
    float lightQuadratic = 0.032f;

    vec3 fragPos = texture(gPositionTexture, UV).xyz;
    vec3 normal = texture(gNormalTexture, UV).xyz;
    vec3 albedo = texture(gAlbedoTexture, UV).xyz;
    float ambientOcclusion = texture(ssaoBlurTexture, UV).r;

    vec3 ambient = vec3(0.5 * albedo * ambientOcclusion);
    vec3 viewDir  = normalize(cameraPos - fragPos);
    vec3 lightDir = normalize(lightPos - fragPos);

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * lightColor;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    vec3 specular = lightColor * spec;

    // attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + lightLinear * distance + lightQuadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;

    vec3 lighting  = vec3(0,0,0);
    lighting += ambient + diffuse + specular;

    color = vec4(lighting, 1);
    //color = vec4(ambientOcclusion, ambientOcclusion, ambientOcclusion, 1);
}
