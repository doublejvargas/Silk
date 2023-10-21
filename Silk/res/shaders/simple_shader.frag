#version 450

layout (location = 0) in vec3 v_fragColor;
layout (location = 1) in vec3 v_fragPosWorld;
layout (location = 2) in vec3 v_fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionViewMatrix;
	vec4 ambientLightColor; // w is intensity
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

//the name of the uniform doesn't have to match the name of the struct
// the order of the fields MUST match the order of the struct we created (see SimplePushConstantData in SimpleRenderSystem.cpp)
layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main()
{
	vec3 directionToLight = ubo.lightPosition - v_fragPosWorld;
	float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	// it is possible for dot product to be < 0 (i.e., normal is facing away from light source). we wanna clamp this to 0.
	vec3 diffuseLight = lightColor * max(dot(normalize(v_fragNormalWorld), normalize(directionToLight)), 0); // normalize the worldNormals! (interpolation of 2 normalized normals may not be normalized)
	
	outColor = vec4((diffuseLight + ambientLight) * v_fragColor, 1.0); 
}