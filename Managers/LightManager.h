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

using std::vector;
using glm::vec3;
using glm::mat4;

class LightManager {

public:
	LightManager();
	void Init(Shader* _depthShader);
	void SetLightPositions(vector<vec3>* _lightPositions, int _numLights);
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
	int numLights;

	// Constants
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	const float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;

	// MAX_POINT_LIGHTS is used to set the size of the depth cubemap array texture, 
	// should be equal to MAX_NUM_LIGHTS in lighting_pass.frag, which sets the size 
	// of the array of light positions.
	const unsigned int MAX_POINT_LIGHTS = 100;

	// Specific options for light attenuation
	const vec3 attenuationOptions[12] = {
		vec3(7, 0.7, 1.8),
		vec3(13, 0.35, 0.44),
		vec3(20, 0.22, 0.20),
		vec3(32, 0.14, 0.07),
		vec3(50, 0.09, 0.032),
		vec3(65, 0.07, 0.017),
		vec3(100, 0.045, 0.0075),
		vec3(160, 0.027, 0.0028),
		vec3(200, 0.022, 0.0019),
		vec3(325, 0.014, 0.0007),
		vec3(600, 0.007, 0.0002),
		vec3(3250, 0.0014, 0.000007)
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