#version 400
in vec3 vPos;
out vec3 Color;

void main()
{
	Color = vec3(1.0, 0.0, 0.0);
	gl_Position = vec4(vPos, 1.0);
}