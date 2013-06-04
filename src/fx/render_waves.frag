#version 400

layout (location = 0) out vec4 FragColor;

in vec4 posW;
in vec3 normalW;

// camera
uniform vec3 eyePosW;

// light
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec3 lightDir;

// material
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular; // w component for shininess

// prototypes
void calculateDirectionalLight(inout vec4 ambient, inout vec4 diffuse, inout vec4 specular, vec3 normal, vec3 toEye);

void main()
{
	vec3 normal = normalize(normalW);
	vec3 toEyeW = normalize(eyePosW - posW.xyz);

	vec4 ambient = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);

	calculateDirectionalLight(ambient, diffuse, specular, normal, toEyeW);

	vec4 lightColor = ambient + diffuse + specular;
	lightColor.a = materialDiffuse.a;

	FragColor = lightColor;
}

void calculateDirectionalLight(inout vec4 ambient, inout vec4 diffuse, inout vec4 specular, vec3 normal, vec3 toEye)
{
	vec3 lightVector = -lightDir;

	ambient = materialAmbient * lightAmbient;
	float diffuseFactor = dot(lightVector, normal);

	if(diffuseFactor > 0.0)
	{
		vec3 v = reflect(-lightVector, normal);
		float specularFactor = pow(max(dot(v, toEye), 0.0), materialSpecular.w);
		diffuse = diffuseFactor * materialDiffuse * lightDiffuse;
		specular = specularFactor * materialSpecular * lightSpecular;
	}
}