#version 400

layout (location = 0) out vec4 FragColor;

in vec3 posW;
in vec3 normalW;
in vec4 color;

void main()
{
	normalW = normalize(normalW);
	vec3 toEye = eyePosW - posW;
	
	float distToEye = length(toEye);
	
	// normalize
	toEye /= distToEye
	
	// lighting TODO
	// ...

	FragColor = color;
}