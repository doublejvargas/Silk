#version 450

//layout (location = 0) in vec3 v_fragColor;

layout (location = 0) out vec4 outColor;

//the name of the uniform doesn't have to match the name of the struct
layout (push_constant) uniform Push 
{ 
	mat2 transform;
	vec2 offset; // the order of the fields MUST match the order of the struct we created (see SimplePushConstantData in AppManager.cpp)
	vec3 color;
} push;

void main()
{
	outColor = vec4(push.color, 1.0); 
}