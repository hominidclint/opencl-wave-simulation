#version 400

layout (location = 0) in vec4 vPos;
layout (location = 1) in vec4 vNormal;
layout (location = 2) in vec4 vTangentX;

uniform mat4 MVP;

out vec4 color;

void main()
{
	color = vec4(0.5, 0.5, 0.5, 1.0);
	//gl_Position = MVP * vec4(vPos, 1.0);
	gl_Position = MVP * vPos;
}