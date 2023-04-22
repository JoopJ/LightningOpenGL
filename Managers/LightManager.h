#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad.h>
#include <vector>

#include "../Shader/Shader.h"
#include "../FunctionLibrary.h"
#include "../Renderer.h"
#include "../include/imgui/imgui.h"
#include "../BoltGeneration/LightningPatterns.h"
#include "../BoltGeneration/BoltSetup.h"

using std::vector;
using glm::vec3;
using glm::mat4;

class LightManager {

public:
	LightManager();
	void Init(Shader* _depthShader);
	void SetLightPositions(vector<vec3>* _lightPositions);
	void SetLightPositions(vec3* _lightPositions);
	void RenderDepthMaps();
	void BindCubeMapArray();
	void SetLightingPassUniforms(Shader* shader);
	void LightingGUI(bool* lightBoxesEnable);

private:
	// Variables ---------
	unsigned int depthCubemapArrayFBO;
	unsigned int depthCubemapArray;
	Shader* depthShader;
	mat4 shadowProj;

	vector<vec3> lightPositions;
	int numActiveLights;

	// Constants
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	const float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;

	// MAX_POINT_LIGHTS is used to set the size of the depth cubemap array texture, 
	// should be equal to MAX_NUM_LIGHTS in lighting_pass.frag, which sets the size 
	// of the array of light positions.
	const unsigned int MAX_POINT_LIGHTS = 300;

	// Specific options for light attenuation
	const vec3 attenuationOptions[12] = {
		vec3(7, 0.7f, 1.8),
		vec3(13, 0.35f, 0.44f),
		vec3(20, 0.22f, 0.20f),
		vec3(32, 0.14f, 0.07f),
		vec3(50, 0.09f, 0.032f),
		vec3(65, 0.07f, 0.017f),
		vec3(100, 0.045f, 0.0075f),
		vec3(160, 0.027f, 0.0028f),
		vec3(200, 0.022f, 0.0019f),
		vec3(325, 0.014f, 0.0007f),
		vec3(600, 0.007f, 0.0002f),
		vec3(3250, 0.0014f, 0.000007f)
	};

	// Options
	float linear, quadratic; // Attenuation
	float near_plane, far_plane;
	vec3 lightColor;
	int attenuationChoice;
	int attenuationRadius;

	// Functions ---------
	void SetupFBOandTexture();
	vector<mat4> GenerateShadowTransforms(vec3 lightPos);
	void UpdateShadowProjection();
};