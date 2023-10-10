#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

layout(location = 0) out vec3 o_fragColor;

//the name of the uniform doesn't have to match the name of the struct
// the order of the fields MUST match the order of the struct we created (see SimplePushConstantData in AppManager.cpp)
layout (push_constant) uniform Push
{ 
	mat4 transform;
	vec3 color;
} push;

void main()
{
	gl_Position = push.transform * vec4(a_Position, 1.0);
	o_fragColor = a_Color;
}