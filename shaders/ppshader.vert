#version 450 core

layout(location = 0) in vec3 vertexIn;

out vec2 UV;

void main(){
	gl_Position = vec4(vertexIn,1.0f);
	UV = (vertexIn.xy + vec2(1,1)) / 2.0;
}
