#version 450 core

layout(location = 0) in vec3 vertexIn;
layout(location = 1) in vec3 normalIn;

layout(location = 2) uniform mat4 modelMat;
layout(location = 3) uniform mat4 viewMat;
layout(location = 4) uniform mat4 projectionMat;
layout(location = 5) uniform mat3 normalMat;

out vec3 normal;
out vec4 vertexMV;
out float z;

void main() {
    vertexMV = viewMat * modelMat * vec4(vertexIn, 1.0);
    normal = normalMat * normalIn;
    z = vertexIn.y;

    gl_Position = projectionMat * viewMat * modelMat * vec4(vertexIn, 1.0);

}
