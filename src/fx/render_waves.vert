#version 400

layout (location = 0) in vec4 vPos;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec4 vTangentX; // TODO

out vec4 posW;
out vec3 normalW;

// transformation matrices
uniform mat4 MVP;
uniform mat4 WorldMatrix;
uniform mat3 WorldInvTranspose;

void main()
{
	gl_Position = MVP * vPos;

	posW = WorldMatrix * vPos;
	normalW = WorldInvTranspose * vNormal.xyz;
}