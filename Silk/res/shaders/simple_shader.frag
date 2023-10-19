#version 450

layout (location = 0) in vec3 v_fragColor;

layout (location = 0) out vec4 outColor;

//the name of the uniform doesn't have to match the name of the struct
// the order of the fields MUST match the order of the struct we created (see SimplePushConstantData in SimpleRenderSystem.cpp)
layout (push_constant) uniform Push
{ 
	mat4 transform; // projection * view * model
	mat4 normalMatrix;
} push;

void main()
{
	outColor = vec4(v_fragColor, 1.0); 
}