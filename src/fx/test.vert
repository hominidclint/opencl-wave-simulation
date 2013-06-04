#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;

out vec4 Color;
uniform mat4 MVP;

void main()
{
	Color = vColor;
	gl_Position = MVP * vec4(vPos, 1.0);
}