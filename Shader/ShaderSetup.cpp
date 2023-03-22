#include "ShaderSetup.h"

void SetShaderPointLightProperties(Shader shader, int numLights, vec3* lightPosPtr, 
	float linear, float quadratic, vec3 color) {

	shader.Use();
	vec3 diffuse = color * vec3(0.5f);
	vec3 ambient = diffuse * vec3(0.2f);
	vec3 specular = vec3(1.0f);
	
	std::string str;
	// set each spot light properties
	for (int i = 0; i < numLights; i++) {
		str = "pointLights[" + std::to_string(i) + "].";
		shader.SetVec3(str + "position", lightPosPtr[i]);
	}
	str = "pointLightProperties.";
	shader.SetVec3(str + "ambient", ambient);
	shader.SetVec3(str + "diffuse", diffuse);
	shader.SetVec3(str + "specular", specular);
	shader.SetFloat(str + "constant", 1.0f);
	shader.SetFloat(str + "linear", linear);
	shader.SetFloat(str + "quadratic", quadratic);
}

void SetShaderMaterialProperties(Shader shader, vec3 color, float shininess) {

	vec3 diffuse = color * vec3(0.5f);
	vec3 ambient = diffuse * vec3(0.2f);
	vec3 specular = vec3(1.0f);

	shader.Use();
	shader.SetVec3("material.ambient", ambient);
	shader.SetVec3("material.diffuse", diffuse);
	shader.SetVec3("material.specular", specular);
	shader.SetFloat("material.shininess",  shininess);
}