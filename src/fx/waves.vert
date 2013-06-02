#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;
layout (location = 3) in vec2 uv;

uniform mat4 WorldViewProj;
uniform mat4 WorldInvTranspose;
uniform float gridSpatialStep;

out vec3 posW;
out vec3 normalW;
out vec4 color;

void main()
{
	// sample displacement map
	vec4 displacedPosition = vec4(vPos, 1.0);
	displacedPosition.y = texture2D(sampler, uv).r;
	
	// finite difference scheme for normal calculation
	float du = imageWidth;
	float dv = imageHeight;
	
	float l = texture2D(sampler, uv - vec2(du, 0.0)).x;
	float r = texture2D(sampler, uv + vec2(du, 0,0)).x;
	float t = texture2D(sampler, uv - vec2(0.0, dv)).x;
	float b = texture2D(sampler, uv + vec2(0.0, dv)).x;
	
	vec3 estimatedNormal = normalize(vec3(-r+l, 2.0 * gridSpatialStep, b-t));
	
	// transform to world space
	posW = (World * displacedPosition).xyz;
	normalW = mat3(WorldInvTranspose) * estimatedNormal; // equivalent to float3x3 ?
	
	// color dummy
	color = vec4(1.0, 1.0, 1.0, 1.0);
	
	// transform to homogeneous clip space
	gl_Position = WorldViewProj * vec4(displacedPosition, 1.0);
}