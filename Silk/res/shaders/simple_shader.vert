#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_UV;

layout(location = 0) out vec3 o_fragColor;


layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionViewMatrix;
	vec3 directionToLight;
} ubo;

//the name of the uniform doesn't have to match the name of the struct
// the order of the fields MUST match the order of the struct we created (see SimplePushConstantData in SimpleRenderSystem.cpp)
layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

// directional lighting: assumes all light rays that reach object are basically parallel and therefore have the same 
//   direction relative to all vertices in the model being lit (think of sunrays to earth).
//   alternative would be: point lighting, where unit direction is computed per vertex basis (this usually is the case when lightsource is much closer to object)
//const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02;

void main()
{
	gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(a_Position, 1.0);

	// temporary: this is only correct in certain situations!
	// mat3(push.modelMatrix) converts modelMatrix from a mat4 to a mat3 by truncating last column and row.
	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * a_Normal);

	// it is possible for dot product to be < 0 (i.e., normal is facing away from light source). we wanna clamp this to 0.
	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);

	o_fragColor = lightIntensity * a_Color;
}