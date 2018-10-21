#version 450 core

in vec2 UV;

layout(location = 0) uniform sampler2D gPosition;
layout(location = 1) uniform sampler2D gNormal;
layout(location = 2) uniform sampler2D texNoise;

layout(location = 3) uniform mat4 projection;

layout(location = 4) uniform float width;
layout(location = 5) uniform float height;

// parameters 
layout(location = 6) uniform int kernelSize;
layout(location = 7) uniform float radius;
layout(location = 8) uniform float bias;

uniform vec3 samples[8];

out float color;

void main()
{
    vec2 noiseScale = vec2(width/4.0, height/4.0);

    // get input for SSAO algorithm
    vec3 fragPos = texture(gPosition, UV).xyz;
    vec3 normal = normalize(texture(gNormal, UV).xyz);
    vec3 randomVec = normalize(texture(texNoise, UV * noiseScale).xyz);
    //vec3 randomVec = fragPos;


    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    color = occlusion;
}
