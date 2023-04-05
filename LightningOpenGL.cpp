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
int attenuationChoice = 3;
int atteunationRadius;
float boltAlpha = 1.0f;
vec3 boltColor = vec3(1.0f, 1.0f, 0.0f);
// toggles
bool shadows = true;
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
void RenderImGui();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);

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
	// Bolt
	Shader boltShader = LoadShader("bolt.vs", "bolt.fs");
	// Post Processing
	Shader screenShader = LoadShader("screen.vs", "screen.fs");
	Shader blurShader = LoadShader("blur.vs", "blur.fs");
	// Lighting
	Shader lightShader = LoadShader("light.vs", "light.fs");
	Shader depthShader = LoadShader("depth.vs", "depth.fs", "depth.gs");
	// Object
	Shader objectShader = LoadShader("object.vs", "object.fs");

	// Shader Configs
	// set the location of texture uniforms for shaders
	screenShader.Use();
	screenShader.SetInt("screenTexture", 0);
	screenShader.SetInt("bloomBlur", 1);

	objectShader.Use();
	objectShader.SetInt("diffuseTexture", 0);
	objectShader.SetInt("depthMap", 1);
	// -------------------------

	// Lighting
	// -------------------------
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
	// ------------------------

	// Shadows Objects
	// ------------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	// depth map FBO
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// depth cubematp texture
	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	// asign each of the cubemap faces a 2D depth-valued texture image (shadow map)
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	// set texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	// tell OpenGL we don't want to render any color data
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// ------------------------

	// Rendering Objects
	// ------------------------
	// framebuffer object
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// texture color buffer objects
	unsigned int tcbo[2];
	// genereate and attach to framebuffer object (fbo)
	glGenTextures(2, tcbo);
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, tcbo[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tcbo[i], 0);
	}

	// depth and stencil buffer object
	unsigned int rbo; // can be a renderbuffer object as we don't need to sample from it
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// allocate storage and unbind
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// attach to the framebuffer object (fbo)
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	// tell opengl to render to multiple colorbuffers
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	// check fbo is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ping pong buffers
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);

	for (unsigned int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// ----------------

	// Performance Constructors
	// -----------------
	Performance mainLoop("Main Loop", 10.0);
	mainLoop.SetPerSecondOutput(false);
	mainLoop.SetAverageOutput(false);
	mainLoop.Start();
	// -----------------

	// Testing ---------
	// Rotate the Point light
	mat4 lightRotateMat4 = glm::rotate(mat4(1.0), glm::radians(0.01f), vec3(0, 1, 0));
	vec3 lightPos = vec3(0.0f, 10.0f, 18.0f);
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

		// Rendering
		// -----------------------
		lightPos = vec3(lightRotateMat4 * glm::vec4(lightPos, 1.0));
		float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
		// 0. create depth cubemap transformation matrices
		// -----------
		float near_plane = 0.1f, far_plane = 200.0f;
		mat4 shadowProj = glm::perspective(radians(90.0f), aspect, near_plane, far_plane);
		// create a transform matrix for each side of the cubemap
		vector<mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, 
			lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // right
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, 
			lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));// left
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, 
			lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))); // top
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, 
			lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))); // bottom
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, 
			lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // near
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, 
			lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))); // far
		
		
		// 1. render scene from light's point of view to depth cubemap
		// ---------------------
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		depthShader.Use();
		for (unsigned int i = 0; i < 6; i++) {
			depthShader.SetMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		}
		depthShader.SetFloat("far_plane", far_plane);
		depthShader.SetVec3("lightPos", lightPos);
		// depth shader dosen't need view or projection
		RenderScene(depthShader);

		// 2. render scene as normal to frame buffer object
		// -------------------------
		// First Pass, to framebuffer object (fbo)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glEnable(GL_DEPTH_TEST);

		// MVP
		mat4 model;
		// TODO Move to camera class
		mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
		mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


		// Render Lightning Bolt
		// -----------------
		boltShader.Use();
		boltShader.SetVec3("color", boltColor);
		boltShader.SetFloat("alpha", boltAlpha);
		SetVPMatricies(boltShader, view, projection);
		if (DYNAMIC_BOLT)
			DrawLineBolt(dynamicLineSegmentsPtr);
		else
			DrawLineBolt(staticLineSegmentsPtr);

		// ------------------	
	
		// Objects
		// ------------------
		// Point Lights positioned along the bolt:
		if (lightBoxesEnable) {
			lightShader.Use();
			SetVPMatricies(lightShader, view, projection);
			if (DYNAMIC_BOLT) {
				for (int i = 0; i < GetLightPerSegment() * dynamicLineSegmentsPtr->size(); i++) {
					model = mat4(1.0f);
					model = glm::translate(model, dynamicPointLightPositionsPtr->at(i));
					model = glm::scale(model, vec3(0.3f));
					lightShader.SetMat4("model", model);
					RenderCube();
				}
			} else {
				for (int i = 0; i < GetLightPerSegment() * numSegmentsInPattern; i++) {
					model = mat4(1.0f);
					model = glm::translate(model, staticPointLightPositionsPtr[i]);
					model = glm::scale(model, vec3(0.3f));
					lightShader.SetMat4("model", model);
					RenderCube();
				}
			}
			
		}
		// Shadow Point Light TODO: make dynamic and position along bolt
		lightShader.Use();
		model = mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, vec3(0.3f));
		SetMVPMatricies(lightShader, model, view, projection);
		RenderCube();

		// Light Affected Objects:
		objectShader.Use();
		objectShader.SetVec3("viewPos", GetCameraPos());
		objectShader.SetVec3("lightPos", lightPos);
		objectShader.SetFloat("far_plane", far_plane);
		objectShader.SetBool("shadows", shadows);

		model = mat4(1.0f);
		SetMVPMatricies(objectShader, model, view, projection);
		// bind diffuse texture and depth cube map
		glActiveTexture(GL_TEXTURE0);	// diffuse texture
		glBindTexture(GL_TEXTURE_2D, metalWallTexture);
		glActiveTexture(GL_TEXTURE1);	// depth cubemap
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		RenderScene(objectShader);
		// ---------------
		
		// Post Processing
		// ---------------
		// Blur
		bool horizontal = true, first_iteration = true;
		blurShader.Use();
		// blur the texture
		for (int i = 0; i < amount+1; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			blurShader.SetInt("horizontal", horizontal);
			// bind texutre of other framebuffer, or the texture to blur if first iteration
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? tcbo[1] : pingpongBuffer[!horizontal]);
			// render quad
			RenderQuad();
			// swap buffers
			horizontal = !horizontal;
			if (first_iteration) {
				first_iteration = false;
			}
		}
		// ---------------
		
		// Second Pass, to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		screenShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tcbo[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		screenShader.SetBool("bloom", bloom);
		//screenShader.SetFloat("exposure", exposure);
		RenderQuad();
		// ---------------

		// Render GUI
		// ---------------
		RenderImGui();
		// ---------------

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// deleter buffers
	glDeleteBuffers(1, &fbo);
	glDeleteBuffers(1, &rbo);
	glDeleteBuffers(1, &tcbo[0]);
	glDeleteBuffers(1, &tcbo[1]);
	glDeleteBuffers(1, &pingpongBuffer[0]);
	glDeleteBuffers(1, &pingpongBuffer[1]);

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
		shadows = !shadows;
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

	//ImGui::Begin("Lightning");
	//GUINaiveApproach();

	// Bolt Control window
	ImGui::Begin("Bolt Options");
	if (ImGui::Combo("Methods", &methodChoice, methodNames, 2)) {
		SetMethod(methodChoice);
	}

	if (ImGui::Button(DYNAMIC_BOLT ? "Dynamic" : "Static")) {
			DYNAMIC_BOLT = !DYNAMIC_BOLT;
	}
	if (DYNAMIC_BOLT) {
		ImGui::Text("DYNAMIC INFO HERE");
	} else {
		ImGui::Text("STATIC INFO HERE");
	}

	if (ImGui::Button("Show Light Positions")) {
		lightBoxesEnable = !lightBoxesEnable;
	}
	ImGui::SliderFloat("Bolt Alpha", &boltAlpha, 0, 1);

	ImGui::End();

	ImGui::Begin("Post Processing");
	ImGui::SliderInt("Blur Amount", &amount, 0, 15);
	ImGui::SliderFloat("Exposure", &exposure, 0.1, 100);

	ImGui::End();

	ImGui::Begin("Lighting");
	ImGui::Text("Attenuation");
	ImGui::Text("Radius: %d", atteunationRadius);
	ImGui::SliderInt("##", &attenuationChoice, 0, 11);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GLFWwindow* CreateWindow() {
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "drawing lines", NULL, NULL);
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

