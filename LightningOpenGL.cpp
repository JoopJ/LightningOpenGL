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
#include "Managers/LightManager.h"
#include "Managers/G_Buffer.h"
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
const vec3 startPnt = vec3(0, 90, 0);
// post processing
int amount = 0;
float exposure = 1.0f;
// lightning options
float boltAlpha = 1.0f;
vec3 boltColor = vec3(1.0f, 1.0f, 0.0f);
// toggles
bool shadowsEnabled = true;
bool shadowKeyPressed = false;
bool bloom = true;
bool bloomKeyPressed = false;
// Array Type
bool DYNAMIC_BOLT = false;
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
void DrawLightBoxes(Shader shader, vector<vec3>* lightPositions, int numLights);
void DrawLightBoxes(Shader shader, vec3* lightPositions, int numLights);
// Input
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress);
bool ProcessLightningControlInput(GLFWwindow* window);
// Config
void ConfigureWindow();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);
// GUI
void RenderImGui(LightManager* lm);
void PostProcessingGUI();
void BoltControlGUI();

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

	// Shaders
	// ---------------
	Shader boltShader = LoadShader("bolt.vert", "bolt.frag");
	// Deferred Shadring
	Shader lightCubeShader = LoadShader("light.vert", "light.frag");
	Shader geometryPassShader = LoadShader("g_buffer.vert", "g_buffer.frag");
	Shader lightingPassShader = LoadShader("lighting_pass.vert", "lighting_pass.frag");
	Shader depthShader = LoadShader("depth.vert", "depth.frag", "depth_multiple_cubemap.geom");

	lightingPassShader.Use();
	lightingPassShader.SetInt("gPosition", 0);
	lightingPassShader.SetInt("gNormal", 1);
	lightingPassShader.SetInt("gAlbedoSpec", 2);
	lightingPassShader.SetInt("depthMapArray", 3);

	lightCubeShader.Use();
	lightCubeShader.SetVec3("lightColor", vec3(1));
	// -------------------------

	// Light Manager Setup -----
	LightManager lightManager;
	lightManager.Init(&depthShader);
	// -------------------------

	// Testing ---------
	// mat4 lightRotateMat4 = glm::rotate(mat4(2.0), glm::radians(0.01f), vec3(0, 1, 0)); // used for rotating the lights
	vector<vec3> lightPositions;
	vec3 lightPos = vec3(0, 2, 0);
	vec3 newPos;
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
		lightManager.SetLightPositions(dynamicPointLightPositionsPtr, 
			dynamicPointLightPositionsPtr->size());
	}
	else {
		// Static Bolt
		NewBolt(staticLineSegmentsPtr, staticPointLightPositionsPtr,
			startPnt, lightningStaticPatternPtr);
		lightManager.SetLightPositions(staticPointLightPositionsPtr);
	}
	// ---------------

	// G-Buffer --------------
	G_Buffer gBuffer(SCR_WIDTH, SCR_HEIGHT);
	// -----------------

	// Performance Constructors
	// -----------------
	Performance mainLoop("Main Loop", 10.0);
	mainLoop.SetPerSecondOutput(true);
	mainLoop.SetAverageOutput(true);
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
			// Generate the pattern and set the line segment positions and point light positions
			if (DYNAMIC_BOLT) {
				// Dynamic Bolt
				NewBolt(dynamicLineSegmentsPtr, dynamicPointLightPositionsPtr,
					startPnt, lightningDynamicPatternPtr);
				lightManager.SetLightPositions(dynamicPointLightPositionsPtr, 
					dynamicPointLightPositionsPtr->size());
			}
			else {
				// Static Bolt
				NewBolt(staticLineSegmentsPtr, staticPointLightPositionsPtr,
					startPnt, lightningStaticPatternPtr);
				lightManager.SetLightPositions(staticPointLightPositionsPtr);
			}
		}
		// --------------------------

		// Deferred Rendering
		// --------------------------
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. Geometry Pass: render all geometric/color data to g-buffer
		// ---------------
		gBuffer.Bind();
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

		// Generate Shadow Maps
		lightManager.RenderDepthMaps();


		// 2. Lighting Pass: calculate lighting by iterating over a screen filled quad 
		//					 pixel-by-pixel using the g-buffer's content.
		// -----------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingPassShader.Use();

		gBuffer.BindTextures();
		lightManager.BindCubeMapArray();

		lightManager.SetLightingPassUniforms(&lightingPassShader);
		lightingPassShader.SetVec3("viewPos", GetCameraPos());
		lightingPassShader.SetBool("shadows", shadowsEnabled);
		RenderQuad();

		// 2.5 copy contents of geometry's depth buffer to default framebuffer's depth buffer
		// -----------------
		gBuffer.BindRead();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		// blit to default framebuffer.
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindBuffer(GL_FRAMEBUFFER, 0);

		// -----------------------

		// Render Lightning
		boltShader.Use();
		boltShader.SetVec3("color", vec3(1, 1, 0));
		boltShader.SetFloat("alpha", 1.0f);
		SetVPMatricies(boltShader, view, projection);
		if (DYNAMIC_BOLT) {
			// Dynamic Bolt
			DrawLineBolt(dynamicLineSegmentsPtr);

			if (lightBoxesEnable) {
				// Draw Point Light boxes
				lightCubeShader.Use();
				SetVPMatricies(lightCubeShader, view, projection);
				DrawLightBoxes(lightCubeShader, dynamicPointLightPositionsPtr,
					dynamicPointLightPositionsPtr->size());
			}
		}
		else {
			// Static Bolt
			DrawLineBolt(staticLineSegmentsPtr);

			if (lightBoxesEnable) {
				// Draw Point Light boxes
				lightCubeShader.Use();
				SetVPMatricies(lightCubeShader, view, projection);
				DrawLightBoxes(lightCubeShader, staticPointLightPositionsPtr,
					numSegmentsInPattern);
			}
		}

		// Render GUI
		// ---------------
		RenderImGui(&lightManager);
		// ---------------

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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

// Light Boxes
// VECTOR
void DrawLightBoxes(Shader shader, vector<vec3>* lightPositions, int numLights) {
	for (unsigned int i = 0; i < numLights; i++) {
		mat4 model = mat4(1);
		model = glm::translate(model, lightPositions->at(i));
		model = glm::scale(model, vec3(0.5f));
		shader.SetMat4("model", model);
		RenderCube();
	}
}
// ARRAY
void DrawLightBoxes(Shader shader, vec3* lightPositions, int numLights) {
	for (unsigned int i = 0; i < numLights; i++) {
		mat4 model = mat4(1);
		model = glm::translate(model, lightPositions[i]);
		model = glm::scale(model, vec3(0.5f));
		shader.SetMat4("model", model);
		RenderCube();
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
void RenderImGui(LightManager* lm) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();;

	//PostProcessingGUI();

	lm->LightingGUI();

	BoltControlGUI();
	BoltGenerationGUI(methodChoice);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void BoltControlGUI() {
	ImGui::Begin("Bolt Control");
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

	ImGui::End();
}

void PostProcessingGUI() {
	ImGui::Begin("Post Processing");
	ImGui::SliderInt("Blur Amount", &amount, 0, 15);
	ImGui::SliderFloat("Exposure", &exposure, 0.1, 100);
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

