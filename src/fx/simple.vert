#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

uniform mat4 MVP;
uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;

uniform vec3 LightPosition;
uniform vec3 LightDir;
uniform vec3 LightAmbient;
uniform vec3 LightDiffuse;
uniform vec3 LightSpecular;

uniform vec3 Ka;
uniform vec3 Kd;
uniform vec4 Ks;

out vec3 LightIntensity;

void main()
{
	// Convert normal and position to eye coords
	vec3 tnorm = normalize(NormalMatrix * vNormal);
	vec4 eyeCoords = ModelViewMatrix * vec4(vPos, 1.0);

	vec3 s = normalize(LightPosition - eyeCoords.xyz);
	vec3 v = normalize(-eyeCoords.xyz);
	
	vec3 r = reflect(-s, tnorm);
	vec3 ambient = LightAmbient * Ka;
	float sDotN = max(dot(s,tnorm), 0.0);
	vec3 diffuse = LightDiffuse * Kd * sDotN;
	vec3 spec = vec3(0.0);
	if(sDotN > 0.0)
	{
		spec = LightSpecular * Ks.xyz * pow(max(dot(r,v), 0.0), Ks.w);
	}
	LightIntensity = ambient + diffuse + spec;

	gl_Position = MVP * vec4(vPos, 1.0);
}