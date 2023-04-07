/*
LightningOpenGL
Creates a lightning bolt using OpenGL and C++
Currently uses a simple array of lines that follow on from each other to a random position to create a lightning pat
tern.
ImGui is used for the GUI which allows editting of various variables used to genereate the lightning pattern.
*/

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#include <iostream>
#include <vector>
#include <math.h>
#include <memory>
#include <random>
#include <functional>
#include <iterator>
#include <string>

#define PI 3.14159265

#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// other files
#include "BoltGeneration/LineBoltSegment.h"
#include "BoltGeneration/TextureBolt.h"
#include "BoltGeneration/TriangleBoltSegment.h"
#include "BoltGeneration/LightningPatterns.h"
#include "BoltGeneration/BoltSetup.h"
#include "Shader/Shader.h"
#include "Shader/ShaderSetup.h"
#include "FunctionLibrary.h"
#include "CameraControl.h"
#include "Renderer.h"
#include "Performance.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using glm::radians;
using glm::lookAt;

// lightning start position
const vec3 startPnt = vec3(400, 30000, 0);
// post processing
int amount = 0;
float exposure = 1.0f;
// lighting options
int attenuationChoice = 9;
int atteunationRadius;
float boltAlpha = 1.0f;
vec3 boltColor = vec3(1.0f, 1.0f, 0.0f);
// toggles
bool shadowsEnabled = true;
bool shadowKeyPressed = false;
bool bloom = true;
bool bloomKeyPressed = false;
// Array Type
bool DYNAMIC_BOLT = true;
// Method Choice
int methodChoice = 0; // 0 = random, 1 = particle system
const char* methodNames[2] = { "Random Positions", "Particle System" };
// Debuging
bool lightBoxesEnable = false;

// function prototypes
// MVP Setters
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 porjection);
void SetVPMatricies(Shader shader, mat4 view, mat4 projection);
// Drawing
void DrawLineBolt(LineBoltSegment* lboltPtr);
void DrawLineBolt(vector<LineBoltSegment>* lboltPtr);
// Input
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress);
bool ProcessLightningControlInput(GLFWwindow* window);
// Config
void ConfigureWindow();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);
// GUI
void RenderImGui();
void BoltControlGUI();
void PostProcessingGUI();
void LightingGUI();

vector<mat4> GenerateShadowTransforms(vec3 lightPos, mat4 shadowProj);

int main() {
	// Initial Configurations and Window Creation
	// -------------------------
	std::srand(std::time(NULL));
	SetWidthAndHeight(SCR_WIDTH, SCR_HEIGHT);
	ConfigureWindow();
	GLFWwindow* window = CreateWindow();
	if (window == NULL) {	// exit if window creation fails
		return -1;
	}
	InitImGui(window);
	// Global OpenGL state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	// -------------------------
	// Load Textures
	// ---------------
	unsigned int metalWallTexture = LoadTexture("\\Textures\\metal_wall.png");
	// ---------------

	// Input
	// ---------------
	// callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// mouse capture
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	bool firstMouseKeyPress = true;
	// ---------------

	// Bolt Objects Setup
	// ---------------
	// Line Segments
	// static
	LineBoltSegment boltLinesStatic[numSegmentsInPattern+1];
	LineBoltSegment* staticLineSegmentsPtr = &boltLinesStatic[0];
	// dynamic
	vector<LineBoltSegment> boltLinesDynamic;
	vector<LineBoltSegment>* dynamicLineSegmentsPtr = &boltLinesDynamic;

	// Point Lights
	const int maxNumPointLights = 100;
	// static
	vec3 pointLightPositionsStatic[maxNumPointLights];
	vec3* staticPointLightPositionsPtr;
	staticPointLightPositionsPtr = &pointLightPositionsStatic[0];
	// dynamic
	vector<vec3> pointLightPositionsDynamic;
	vector<vec3>* dynamicPointLightPositionsPtr;
	dynamicPointLightPositionsPtr = &pointLightPositionsDynamic;

	// Pattern
	// static
	std::shared_ptr<vec3[numSegmentsInPattern]> lightningStaticPatternPtr;
	lightningStaticPatternPtr = std::make_shared<vec3[numSegmentsInPattern]>();
	// dynamic
	vector<pair<vec3, vec3>> lightningDynamicPattern;
	vector<pair<vec3, vec3>>* lightningDynamicPatternPtr = &lightningDynamicPattern;
	// ---------------

	// Specific options for light attenuation
	vec3 attenuationOptions[12] = {
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

	// Testing ---------
	// Lighting Info
	// ---------------
	// Options
	float linear = attenuationOptions[attenuationChoice].y;
	float quadratic = attenuationOptions[attenuationChoice].z;
	float near_plane = 1.0f, far_plane = 50.0f;
	int NUM_LIGHTS = 3;	// must be less than or equal to MAX_POINT_LIGHTS
	// Constants
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	const float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
	const mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);
	const unsigned int MAX_POINT_LIGHTS = 10; // used to set the size of the cubemap array, should be the same
											// as 
	// ---------------
	// mat4 lightRotateMat4 = glm::rotate(mat4(2.0), glm::radians(0.01f), vec3(0, 1, 0)); // used for rotating the lights
	vector<vec3> lightPositions;
	vec3 lightPos = vec3(0, 2, 0);
	vec3 newPos;
	// set light positions
	for (int i = 0; i < NUM_LIGHTS; i++) {
		// random variation
		newPos = lightPos + vec3((rand() % 10) - 5, - 3 * i, (rand() % 10) - 5);
		lightPositions.push_back(newPos);
	}
	// -----------------

	// Bolt Objects Definition
	// ---------------
	// Set Method Properties:

	// Generation Method 0: Random Positions
	// None

	// Generation Method 1: Particle System
	vec3 seed = vec3(6, -1000, -4);
	SetParticleSystemSeedSegment(seed);

	// Generation Method 2: L-System
	// None

	// Set the pattern generation method
	SetMethod(methodChoice);

	// Generate the pattern and set the line segment positions and point light positions
	if (DYNAMIC_BOLT) {
		// Dynamic Bolt
		NewBolt(dynamicLineSegmentsPtr, dynamicPointLightPositionsPtr,
			startPnt, lightningDynamicPatternPtr);
	}
	else {
		// Static Bolt
		NewBolt(staticLineSegmentsPtr, staticPointLightPositionsPtr,
			startPnt, lightningStaticPatternPtr);
	}
	// ---------------

	// Shaders
	// ---------------
	// Deferred Shadring
	Shader lightCubeShader = LoadShader("light.vert", "light.frag");
	Shader geometryPassShader = LoadShader("g_buffer.vert", "g_buffer.frag");
	Shader lightingPassShader = LoadShader("lighting_pass.vert", "lighting_pass.frag");
	Shader depthSingleCubemapShader = LoadShader("depth.vert", "depth.frag", "depth_single_cubemap.geom");
	Shader depthMultipleCubemapShader = LoadShader("depth.vert", "depth.frag", "depth_multiple_cubemap.geom");

	lightingPassShader.Use();
	lightingPassShader.SetInt("gPosition", 0);
	lightingPassShader.SetInt("gNormal", 1);
	lightingPassShader.SetInt("gAlbedoSpec", 2);
	lightingPassShader.SetInt("depthMap", 3);
	lightingPassShader.SetInt("depthMapArray", 4);
	// -------------------------

	// ------------------------
	// Shadows ----------------
	// Depth Cubemap FBO ------
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);	
	// Depth Cubemap ---------
	// ### 1 depth cubemap by itself
	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	// assign each face a 2D depth-valued texture
	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}

	// set texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach the cubemap as the framebuffers depth attachment
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ### An array of depth cubemaps
	// requires an appropriate fbo
	unsigned int depthMapArrayFBO;
	glGenFramebuffers(1, &depthMapArrayFBO);
	// Depth Cubemap Array texture
	unsigned int depthCubemapArray;
	glGenTextures(1, &depthCubemapArray);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubemapArray);
	// assign the texture 
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0 , GL_DEPTH_COMPONENT32, SHADOW_WIDTH, 
		SHADOW_HEIGHT, 6 * MAX_POINT_LIGHTS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	// set texture parameters
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glClear(GL_DEPTH_BUFFER_BIT); float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapArrayFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemapArray, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// -----------------------------

	// G-Buffer --------------
	glEnable(GL_DEPTH_TEST);

	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gAlbedoSpec;

	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// -----------------

	// Performance Constructors
	// -----------------
	Performance mainLoop("Main Loop", 10.0);
	mainLoop.SetPerSecondOutput(false);
	mainLoop.SetAverageOutput(false);
	mainLoop.Start();
	// -----------------

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// pre-frame time logic
		// -----------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		SetDeltaTime(currentFrame - GetLastFrame());
		SetLastFrame(currentFrame);
		// -----------------------
		
		// Performance
		// -----------------------
		mainLoop.Update();
		// -----------------------

		// input
		// -----------------------
		ProcessKeyboardInput(window);
		ProcessMiscInput(window, &firstMouseKeyPress);	// TODO: move GUI stuff into separate file
		if (ProcessLightningControlInput(window)) {
			// New Bolt
			if (DYNAMIC_BOLT) {
				// Dynamic Bolt
				NewBolt(dynamicLineSegmentsPtr, dynamicPointLightPositionsPtr,
					startPnt, lightningDynamicPatternPtr);
			} 
			else {
				// Static Bolt
				NewBolt(staticLineSegmentsPtr, staticPointLightPositionsPtr,
					startPnt, lightningStaticPatternPtr);
			}
		}
		// -----------------------

		// Update Light Positions ---
		for (int i = 0; i < lightPositions.size(); i++) {
			vec3 pos = lightPositions[i];
			// rotate point around y axis
			pos = vec3(pos.x * cos(GetDeltaTime()) - pos.z * sin(GetDeltaTime()),
								pos.y,
								pos.x * sin(GetDeltaTime()) + pos.z * cos(GetDeltaTime()));
			lightPositions[i] = pos;
		}
		// --------------------------

		// Deferred Rendering
		// -----------------------

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. Geometry Pass: render all geometric/color data to g-buffer
		// ---------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 model = mat4(1.0f);
		mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
		mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		geometryPassShader.Use();
		SetVPMatricies(geometryPassShader, view, projection);

		// bind diffuse texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, metalWallTexture);
		RenderScene(geometryPassShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Lighting Pass
		// ----------------------------------------------------------------------------------------
		// Update Lighting Options
		linear = attenuationOptions[attenuationChoice].y;
		quadratic = attenuationOptions[attenuationChoice].z;
		// far_plane = ;

		// ~ Calculate shadow maps for the lights		
		// 0.1 Render scene of all lights to depth cubemap array --------------------------------
		// Prepare each light for rendering
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapArrayFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		depthMultipleCubemapShader.Use();
		depthMultipleCubemapShader.SetFloat("far_plane", far_plane); // far_plane is constant for all lights
		vector<mat4> shadowTransforms;
		for (unsigned int light = 0; light < NUM_LIGHTS; light++) {
			// For each light...
			shadowTransforms = GenerateShadowTransforms(lightPositions[light], shadowProj);
			for (unsigned int i = 0; i < 6; i++) {
				// For each face of the cubemap...
				depthMultipleCubemapShader.SetMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
			}
			depthMultipleCubemapShader.SetInt("index", light);
			depthMultipleCubemapShader.SetVec3("lightPos", lightPositions[light]);
			RenderScene(depthMultipleCubemapShader);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// ----------------------------------------------------------------------------------------

		// 2. Lighting Pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the g-buffer's content.
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingPassShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		// attach depth map for shadows
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubemapArray);
		// also send light relevant uniforms
		lightingPassShader.SetFloat("Linear", linear);
		lightingPassShader.SetFloat("Quadratic", quadratic);
		lightingPassShader.SetVec3("lightColor", vec3(1.0f, 1.0f, 1.0f));
		lightingPassShader.SetVec3("viewPos", GetCameraPos());
		lightingPassShader.SetFloat("far_plane", far_plane);
		lightingPassShader.SetInt("numLightsActive", NUM_LIGHTS);
		lightingPassShader.SetBool("shadows", shadowsEnabled);
	
		// add light positions
		for (int i = 0; i < NUM_LIGHTS; i++) {
			lightingPassShader.SetVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
		}
		RenderQuad();

		// 2.5 copy contents of geometry's depth buffer to default framebuffer's depth buffer
		// -----------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// blit to default framebuffer.
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindBuffer(GL_FRAMEBUFFER, 0);

		// -----------------------

		lightCubeShader.Use();
		SetVPMatricies(lightCubeShader, view, projection);
		for (unsigned int i = 0; i < lightPositions.size(); i++) {
			// For each light...
			model = mat4(1);
			model = glm::translate(model, lightPositions[i]);
			model = glm::scale(model, vec3(0.5f));
			lightCubeShader.SetMat4("model", model);
			RenderCube();
		}


		// Render GUI
		// ---------------
		RenderImGui();
		// ---------------

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// deleter buffers
	/*
	glDeleteBuffers(1, &fbo);
	glDeleteBuffers(1, &rbo);
	glDeleteBuffers(1, &tcbo[0]);
	glDeleteBuffers(1, &tcbo[1]);
	glDeleteBuffers(1, &pingpongBuffer[0]);
	glDeleteBuffers(1, &pingpongBuffer[1]);
	*/

	// imgui: shutdown and cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	//glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// Bolt Draw
// ---------------
// STATIC
void DrawLineBolt(LineBoltSegment* lboltPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		lboltPtr[i].Draw();
	}
}
// DYNAMIC
void DrawLineBolt(vector<LineBoltSegment>* lboltPtr) {
	for (int i = 0; i < lboltPtr->size(); i++) {
		lboltPtr->at(i).Draw();
	}
}
// ---------------

// Input Processing:
// -------------------
// ProcessMiscInput, process inputs relating to control of the application
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {

		if (*firstButtonPress) {
			*firstButtonPress = false;

			if (GetMouseEngaged()) {
				SetMouseEngaged(false);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				SetMouseEngaged(true);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}

		}
	}
	else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
		*firstButtonPress = true;

	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bloomKeyPressed)
	{
		bloom = !bloom;
		bloomKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
	{
		bloomKeyPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !shadowKeyPressed)
	{
		shadowsEnabled = !shadowsEnabled;
		shadowKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE)
	{
		shadowKeyPressed = false;
	}
}

bool spaceHeld = false;
// ProcessLightningControlInput, process inputs relating to control of the lightning.
// returns true when a new strike is initiated, false otherwise.
bool ProcessLightningControlInput(GLFWwindow* window) {
	// recalculate lines	TODO: method choices should choose between lines and tbolts
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		// std::cout << "New Strike" << std::endl;
		spaceHeld = true;
		return true;
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
		spaceHeld = false;
	}
	return false;
}
// -------------------

// GLFW
// initialize and configure glfw
void ConfigureWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // REMOVE IF ISSUE WITH MACS

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

// GUI:
std::string dynamicText = "Dynamic";
void RenderImGui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//BoltControlGUI();

	//PostProcessingGUI();

	LightingGUI();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void BoltControlGUI() {
	ImGui::Begin("Bolt Options");
	if (ImGui::Combo("Methods", &methodChoice, methodNames, 2)) {
		SetMethod(methodChoice);
	}

	if (ImGui::Button(DYNAMIC_BOLT ? "Dynamic" : "Static")) {
		DYNAMIC_BOLT = !DYNAMIC_BOLT;
	}
	if (DYNAMIC_BOLT) {
		ImGui::Text("DYNAMIC INFO HERE");
	}
	else {
		ImGui::Text("STATIC INFO HERE");
	}

	if (ImGui::Button("Show Light Positions")) {
		lightBoxesEnable = !lightBoxesEnable;
	}
	ImGui::SliderFloat("Bolt Alpha", &boltAlpha, 0, 1);

	ImGui::End();
}

void PostProcessingGUI() {
	ImGui::Begin("Post Processing");
	ImGui::SliderInt("Blur Amount", &amount, 0, 15);
	ImGui::SliderFloat("Exposure", &exposure, 0.1, 100);
	ImGui::End();
}

void LightingGUI() {
	ImGui::Begin("Lighting");
	ImGui::Text("Attenuation");
	ImGui::Text("Radius: %d", atteunationRadius);
	ImGui::SliderInt("##", &attenuationChoice, 0, 11);

	ImGui::End();
}

GLFWwindow* CreateWindow() {
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Deferred Point Shadow Mapping", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}

	// turn off Vsyinc
	glfwSwapInterval(0);

	return window;
}

void InitImGui(GLFWwindow* window) {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

vector<mat4> GenerateShadowTransforms(vec3 lightPos, mat4 shadowProj) {
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

