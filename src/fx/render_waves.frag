// Copyright (c) 2013, Hannes Würfel <hannes.wuerfel@student.hpi.uni-potsdam.de>
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

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