#include "LightManager.h"

LightManager::LightManager()
{
	SetupFBOandTexture();

	// default values
	lightColor = vec3(1.0f, 1.0f, 1.0f);
	near_plane = 1;
	far_plane = 150;
	numLights = 0;
	attenuationChoice = 8;
	vec3 atten = attenuationOptions[attenuationChoice];
	attenuationRadius = atten.x;
	linear = atten.y;
	quadratic = atten.z;

	UpdateShadowProjection();
}

void LightManager::Init(Shader* _depthShader)
{
	depthShader = _depthShader;
}

void LightManager::SetupFBOandTexture() {
	glGenFramebuffers(1, &depthCubemapArrayFBO);
	// Depth Cubemap Array texture
	glGenTextures(1, &depthCubemapArray);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubemapArray);
	// assign the texture 
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32, SHADOW_WIDTH,
		SHADOW_HEIGHT, 6 * MAX_POINT_LIGHTS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// set texture parameters
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glClear(GL_DEPTH_BUFFER_BIT); float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapArrayFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemapArray, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// VECTOR
void LightManager::SetLightPositions(vector<vec3>* _lightPositions, int _numLights)
{
	if (_numLights > MAX_POINT_LIGHTS) {
		std::cout << "ERROR: LightManager::SetLightPositions: _numLights > MAX_POINT_LIGHTS" << std::endl;
		return;
	}
	numLights = _numLights;
	lightPositions.clear();
	for (int i = 0; i < numLights; i++) {
		lightPositions.push_back(_lightPositions->at(i));
	}
}

// ARRAY
void LightManager::SetLightPositions(vec3* _lightPositions) {
	if (numSegmentsInPattern > MAX_POINT_LIGHTS) {
		std::cout << "ERROR: LightManager::SetLightPositions: _numLights > MAX_POINT_LIGHTS" << std::endl;
		return;
	}
	numLights = numSegmentsInPattern;
	lightPositions.clear();
	for (int i = 0; i < numLights; i++) {
		lightPositions.push_back(_lightPositions[i]);
	}
}

void LightManager::RenderDepthMaps() {
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapArrayFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	depthShader->Use();
	depthShader->SetFloat("far_plane", far_plane); // far_plane is constant for all lights

	vector<mat4> shadowTransforms;
	for (unsigned int light = 0; light < numLights; light++) {
		// For each light...
		shadowTransforms = GenerateShadowTransforms(lightPositions[light]);
		for (unsigned int i = 0; i < 6; i++) {
			// For each face of the cubemap...
			depthShader->SetMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}
		depthShader->SetInt("index", light);
		depthShader->SetVec3("lightPos", lightPositions[light]);
		RenderScene(*depthShader);
	}
}

void LightManager::BindCubeMapArray() {
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubemapArray);
}

void LightManager::SetLightingPassUniforms(Shader* shader) {
	shader->SetFloat("Linear", linear);
	shader->SetFloat("Quadratic", quadratic);
	shader->SetFloat("far_plane", far_plane);
	shader->SetVec3("lightColor", lightColor);
	shader->SetInt("numLightsActive", numLights);
	// Set the light positions
	for (int i = 0; i < numLights; i++) {
		shader->SetVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
	}
}

vector<mat4> LightManager::GenerateShadowTransforms(vec3 lightPos) {
	// Create 6 transformation matrices, one for each face of the cube
	vector<mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(-1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0)));
	shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0, -1.0, 0.0), vec3(0.0, 0.0, -1.0)));
	shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0, 0.0, 1.0), vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj * lookAt(lightPos, lightPos + vec3(0.0, 0.0, -1.0), vec3(0.0, -1.0, 0.0)));

	return shadowTransforms;
}

void LightManager::UpdateShadowProjection() {
	shadowProj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);
}

void LightManager::LightingGUI() {
	ImGui::Begin("Lighting");

	ImGui::Text("Attenuation");
	ImGui::Text("Radius: %d", attenuationRadius);
	if (ImGui::SliderInt("##", &attenuationChoice, 0, 11)) {
		vec3 atten = attenuationOptions[attenuationChoice];
		attenuationRadius = atten.x;
		linear = atten.y;
		quadratic = atten.z;
	}

	ImGui::Text("Near Plane: %f", far_plane);
	ImGui::SliderFloat("###", &far_plane, 1, 200);

	ImGui::End();
}