#version 400

layout (location = 0) out vec4 FragColor;

in vec3 LightIntensity;

computeDirectionalLight(

void main()
{
	FragColor = vec4(0.5, 0.5, 0.5, 1.0);//vec4(LightIntensity, 1.0);
}