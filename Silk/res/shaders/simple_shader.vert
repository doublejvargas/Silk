#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_UV;

layout(location = 0) out vec3 o_fragColor;


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
	vec4 positionWorld = push.modelMatrix * vec4(a_Position, 1.0);
	gl_Position = ubo.projectionViewMatrix * positionWorld;

	// temporary: this is only correct in certain situations!
	// mat3(push.modelMatrix) converts modelMatrix from a mat4 to a mat3 by truncating last column and row.
	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * a_Normal);
	
	vec3 directionToLight = ubo.lightPosition - positionWorld.xyz;
	float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	// it is possible for dot product to be < 0 (i.e., normal is facing away from light source). we wanna clamp this to 0.
	vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

	o_fragColor = (diffuseLight + ambientLight) * a_Color;
}