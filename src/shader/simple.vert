#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;

uniform mat4 MVP;

out vec4 color;

void main()
{
	color = vColor;
	gl_Position = MVP * vec4(vPos, 1.0);
}