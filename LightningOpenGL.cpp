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

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

// other files
#include "BoltGeneration/LineBoltSegment.h"
#include "BoltGeneration/LightningPatterns.h"
#include "BoltGeneration/BoltSetup.h"
#include "Shader/Shader.h"
#include "Shader/ShaderSetup.h"
#include "Managers/LightManager.h"
#include "Managers/G_Buffer.h"
#include "Managers/FboManager.h"
#include "Managers/PerformanceManager.h"
#include "FunctionLibrary.h"
#include "CameraControl.h"
#include "Timer.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using glm::radians;
using glm::lookAt;

// bolt generation settings
const vec3 startPnt = vec3(25, 90, 0);

// post processing
int amount = 8;
float exposure = 0.5f;
float gamma = 2.2f;

// lightning options
float boltAlpha = 1.0f;
vec3 boltColor = vec3(1.0f, 1.0f, 0.0f);	// Yellow
vec3 cubeLightColor = vec3(1);				// White
bool newBolt = true; // signals to generate a new bolt

// toggles
bool shadowsEnabled = true;
bool shadowKeyPressed = false;
bool bloom = true;
bool bloomKeyPressed = false;
bool gammaCorrectionEnabled = true;
bool exposureEnabled = true;

// Array Type
bool DYNAMIC_BOLT = true;

// Method Choice
int methodChoice = 2; // 0 = random, 1 = particle system, 2 = l-system
const char* methodNames[3] = { "Random Positions", "Particle System", "L-System"};

// function prototypes
// MVP Setters
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 porjection);
void SetVPMatricies(Shader shader, mat4 view, mat4 projection);
// Drawing
void DrawLineBolt(LineBoltSegment* lboltPtr);
void DrawLineBolt(vector<LineBoltSegment>* lboltPtr);
void DrawLightBoxes(Shader shader, vector<vec3>* lightPositions);
void DrawLightBoxes(Shader shader, vec3* lightPositions);
// Input
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress);
void ProcessLightningControlInput(GLFWwindow* window);
// Config
void ConfigureWindow();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);
// GUI
void RenderImGui(LightManager* lm, PerformanceManager* pm);
void PostProcessingGUI();
void BoltControlGUI(PerformanceManager* pm);



int main() {
	std::cout << "LightningOpenGL" << std::endl;
	// Initial Configurations and Window Creation
	// -------------------------
	std::srand(time(0));
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

	// Load Textures & Models
	// -------------------------
	unsigned int crate0Diffuse = LoadTexture("\\Textures\\crate0_diffuse.png");;
	unsigned int crate1Diffuse = LoadTexture("\\Textures\\crate1_diffuse.png");
	LoadModels();
	// -------------------------

	// Input
	// -------------------------
	// callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// mouse capture
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	bool firstMouseKeyPress = true;
	// -------------------------

	// Bolt Objects Setup
	// -------------------------
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
	// -------------------------

	// Shaders
	// -------------------------
	// Bolt
	Shader boltShader = LoadShader("bolt.vert", "bolt.frag");
	// Post Processing
	Shader blurShader = LoadShader("blur.vert", "blur.frag");
	Shader screenShader = LoadShader("screen.vert", "screen.frag");
	// Deferred Shadring
	Shader lightCubeShader = LoadShader("light.vert", "light.frag");
	Shader geometryPassShader = LoadShader("g_buffer.vert", "g_buffer.frag");
	Shader lightingPassShader = LoadShader("lighting_pass.vert", "lighting_pass.frag");
	Shader depthShader = LoadShader("depth.vert", "depth.frag", "depth_multiple_cubemap.geom");

	// Shader Setup
	lightingPassShader.Use();
	lightingPassShader.SetInt("gPosition", 0);
	lightingPassShader.SetInt("gNormal", 1);
	lightingPassShader.SetInt("gAlbedoSpec", 2);
	lightingPassShader.SetInt("depthMapArray", 3);

	blurShader.Use();
	blurShader.SetInt("image", 3);

	lightCubeShader.Use();
	lightCubeShader.SetVec3("lightColor", cubeLightColor);

	screenShader.Use();
	screenShader.SetInt("sceneTexture", 0);
	screenShader.SetInt("bloomTexture", 1);

	geometryPassShader.Use();
	geometryPassShader.SetInt("texture1_diffuse", 0);
	geometryPassShader.SetInt("texture2_diffuse", 1);
	geometryPassShader.SetVec3("color", vec3(0.5f));
	// -------------------------

	// Light Manager Setup -----
	LightManager lightManager;
	lightManager.Init(&depthShader);
	// -------------------------

	// Performance Manager Setup
	PerformanceManager performanceManager;
	//performanceManager.SetTimerPerSecondOutput(FRAME, true);
	//performanceManager.SetTimerAvgOutput(FRAME, true);
	//performanceManager.SetTimerAvgTimeInterval(FRAME, 1.0);
	// -------------------------

	// Testing ---------
	// mat4 lightRotateMat4 = glm::rotate(mat4(2.0), glm::radians(0.01f), vec3(0, 1, 0)); // used for rotating the lights
	vector<vec3> lightPositions;
	vec3 lightPos = vec3(0, 10, 0);
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

	/*
	// Generate the pattern and set the line segment positions and point light positions
	if (DYNAMIC_BOLT) {
		// Dynamic Bolt
		NewBolt(dynamicLineSegmentsPtr, dynamicPointLightPositionsPtr,
			startPnt, lightningDynamicPatternPtr);
		
		lightManager.SetLightPositions(dynamicPointLightPositionsPtr);
	}
	else {
		// Static Bolt
		NewBolt(staticLineSegmentsPtr, staticPointLightPositionsPtr,
			startPnt, lightningStaticPatternPtr);

		lightManager.SetLightPositions(staticPointLightPositionsPtr);
	}
	// ---------------
	*/

	// G-Buffer -------
	G_Buffer gBuffer(SCR_WIDTH, SCR_HEIGHT);
	// ----------------

	// FBO ------------
	FboManager fboManager(SCR_WIDTH, SCR_HEIGHT);
	// ----------------

	std::cout << "Starting Render Loop" << std::endl;
	// render loop
	while (!glfwWindowShouldClose(window)) {
		performanceManager.StartTimer(FRAME);
		// pre-frame time logic
		// -----------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		SetDeltaTime(currentFrame - GetLastFrame());
		SetLastFrame(currentFrame);
		// -----------------------
		
		// Performance updates
		// -----------------------
		performanceManager.DynamicPatternInfo(lightningDynamicPatternPtr);
		performanceManager.StaticPatternInfo(lightningStaticPatternPtr);
		// -----------------------

		// Input
		// -----------------------
		ProcessKeyboardInput(window);
		ProcessMiscInput(window, &firstMouseKeyPress);
		ProcessLightningControlInput(window);
		// --------------------------

		// New Bolt
		if (newBolt) {
			if (DYNAMIC_BOLT) {
				// Dynamic Bolt
				NewBolt(dynamicLineSegmentsPtr, dynamicPointLightPositionsPtr,
					startPnt, lightningDynamicPatternPtr);

				lightManager.SetLightPositions(dynamicPointLightPositionsPtr);
			}
			else {
				// Static Bolt
				NewBolt(staticLineSegmentsPtr, staticPointLightPositionsPtr,
					startPnt, lightningStaticPatternPtr);

				lightManager.SetLightPositions(staticPointLightPositionsPtr);
			}
			newBolt = false;
		}

		// MVP
		mat4 model = mat4(1.0f);
		mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
		mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// Rendering
		// --------------------------
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. Geometry Pass: render all geometric/color data to g-buffer
		// -----------------
		performanceManager.StartTimer(GEOMETRY_PASS);

		gBuffer.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		geometryPassShader.Use();
		SetVPMatricies(geometryPassShader, view, projection);

		// bind diffuse and normal textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, crate0Diffuse);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, crate1Diffuse);

		geometryPassShader.SetInt("useTexture", 1);
		gBuffer.GeometryPass(geometryPassShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Generate Shadow Maps
		performanceManager.StartTimer(RENDER_SHADOWS);
		lightManager.RenderDepthMaps();
		performanceManager.UpdateTimer(RENDER_SHADOWS);

		performanceManager.UpdateTimer(GEOMETRY_PASS);

		// 2. Lighting Pass: calculate lighting by iterating over a screen filled quad 
		//					 pixel-by-pixel using the g-buffer's content.
		// -----------------
		performanceManager.StartTimer(LIGHTING_PASS);

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		fboManager.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingPassShader.Use();

		gBuffer.BindTextures();
		lightManager.BindCubeMapArray();

		lightManager.SetLightingPassUniforms(&lightingPassShader);
		lightingPassShader.SetVec3("viewPos", GetCameraPos());
		lightingPassShader.SetBool("shadows", shadowsEnabled);
		lightingPassShader.SetBool("blurEnabled", bloom);

		RenderQuad();

		performanceManager.UpdateTimer(LIGHTING_PASS);

		// 2.5. copy contents of geometry buffer to fbo
		// -----------------
		performanceManager.StartTimer(G_BUFFER_COPY);

		gBuffer.BindRead();
		fboManager.BindDraw();
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		performanceManager.UpdateTimer(G_BUFFER_COPY);

		// 3. Render Lightning Bolt
		// -----------------
		// rendered to fbo so bolt can be blurred and the scene and bolt can be blended together
		performanceManager.StartTimer(RENDER_BOLT);
		
		glEnable(GL_DEPTH_TEST);

		boltShader.Use();
		boltShader.SetVec3("color", boltColor);
		boltShader.SetFloat("alpha", boltAlpha);
		SetVPMatricies(boltShader, view, projection);

		if (DYNAMIC_BOLT) {
			// Dynamic Bolt
			DrawLineBolt(dynamicLineSegmentsPtr);

			if (lightManager.GetLightBoxesEnabled()) {
				// Draw Point Light boxes
				lightCubeShader.Use();
				SetVPMatricies(lightCubeShader, view, projection);
				DrawLightBoxes(lightCubeShader, dynamicPointLightPositionsPtr);
			}
		}
		else {
			// Static Bolt
			DrawLineBolt(staticLineSegmentsPtr);

			if (lightManager.GetLightBoxesEnabled()) {
				// Draw Point Light boxes
				lightCubeShader.Use();
				SetVPMatricies(lightCubeShader, view, projection);
				DrawLightBoxes(lightCubeShader, staticPointLightPositionsPtr);
			}
		}

		performanceManager.UpdateTimer(RENDER_BOLT);

		// 4. Bloom
		// -----------------
		performanceManager.StartTimer(BLOOM);

		bool horizontal = fboManager.ApplyBloom(blurShader, amount);

		performanceManager.UpdateTimer(BLOOM);

		// 5. Blend Scene and Blurred Bolt to default framebuffer
		// -----------------
		performanceManager.StartTimer(BLEND);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.Use();
		fboManager.BindSceneAndBloom();
		screenShader.SetBool("bloomEnabled", bloom);
		screenShader.SetBool("gammaEnabled", gammaCorrectionEnabled);
		screenShader.SetFloat("exposure", exposure);
		screenShader.SetBool("exposureEnabled", exposureEnabled);
		screenShader.SetFloat("gamma", gamma);
		RenderQuad();

		performanceManager.UpdateTimer(BLEND);

		performanceManager.UpdateTimer(FRAME);
		// 6. GUI
		// -----------------
		RenderImGui(&lightManager, &performanceManager);
		// --------------------------

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
	for (int i = 0; i < GetNumActiveSegments() - 1; i++) {
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
void DrawLightBoxes(Shader shader, vector<vec3>* lightPositions) {

	for (int i = 0; i < GetNumActiveLights(); i++) {
		mat4 model = mat4(1);
		model = glm::translate(model, lightPositions->at(i));
		model = glm::scale(model, vec3(0.5f));
		shader.SetMat4("model", model);
		RenderCube();
	}

}
// ARRAY
void DrawLightBoxes(Shader shader, vec3* lightPositions) {
	for (int i = 0; i < GetNumActiveLights(); i++) {
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
void ProcessLightningControlInput(GLFWwindow* window) {
	// recalculate lines
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		// std::cout << "New Strike" << std::endl;
		spaceHeld = true;
		newBolt = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
		spaceHeld = false;
	}
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

// Window Toggles
bool toggleBoltGenWindow = false;
bool toggleLightingWindow = false;
bool togglePostProcessingWindow = false;
bool toggleSceneWindow = false;
bool toggleBoltControlWindow = false;
bool toggleTimersWindw = false;
// GUI:
void RenderImGui(LightManager *lm, PerformanceManager *pm) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Window Toggle Menu
	ImGui::Begin("Window Menu", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::Button("Bolt Generation")) {
		toggleBoltGenWindow = !toggleBoltGenWindow;
	}
	if (ImGui::Button("Bolt Settings")) {
		toggleBoltControlWindow = !toggleBoltControlWindow;
	}
	if (ImGui::Button("Lighting")) {
		toggleLightingWindow = !toggleLightingWindow;
	}
	if (ImGui::Button("Post Processing")) {
		togglePostProcessingWindow = !togglePostProcessingWindow;
	}
	/*
	if (ImGui::Button("Scene")) {
		toggleSceneWindow = !toggleSceneWindow;
	}
	if (ImGui::Button("Timers")) {
		toggleTimersWindw = !toggleTimersWindw;
	}
	*/

	ImGui::End();

	// Windows
	if (toggleBoltControlWindow)
		BoltControlGUI(pm);

	if (toggleBoltGenWindow)
		BoltGenerationGUI(methodChoice);

	if (toggleLightingWindow)
		lm->LightingGUI();

	if (togglePostProcessingWindow)
		PostProcessingGUI();

	if (toggleTimersWindw)
		pm->TimersGUI();

	// Performance
	pm->PerformanceGUI();

	RenderGUI();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void BoltControlGUI(PerformanceManager* pm) {
	const ImVec2 startPos = ImVec2(5, 183);
	ImGui::SetNextWindowPos(startPos, ImGuiCond_Once);

	ImGui::Begin("Bolt Control", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Methods:");
	if (ImGui::Combo("##", &methodChoice, methodNames, 3)) {
		SetMethod(methodChoice);
	}

	ImGui::Text("Storage Type:");
	if (ImGui::Button(DYNAMIC_BOLT ? "Dynamic" : "Static")) {
		DYNAMIC_BOLT = !DYNAMIC_BOLT;
		newBolt = true;
	}
	ImGui::Text("Pattern Info:");
	if (DYNAMIC_BOLT) {
		pm->DynamicPatternGUI();
	}
	else {
		pm->StaticPatternGUI();
	}

	ImGui::End();
}

void PostProcessingGUI() {
	const ImVec2 startPos = ImVec2(575, 119);
	ImGui::SetNextWindowPos(startPos, ImGuiCond_Once);
	ImGui::Begin("Post Processing", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::CollapsingHeader("Bloom")) {
		ImGui::Text("Amount:");
		ImGui::SliderInt("", &amount, 0, 20);
		ImGui::Checkbox("Enabled", &bloom);
	}

	if (ImGui::CollapsingHeader("Gamma Correction")) {
		ImGui::Text("Gamma:");
		ImGui::SliderFloat("##", &gamma, 0.1f, 5.0f);
		ImGui::Checkbox("##Enabled", &gammaCorrectionEnabled);
	}

	if (ImGui::CollapsingHeader("Exposre")) {
		ImGui::Text("Exposure:");
		ImGui::SliderFloat("###", &exposure, 0.1f, 30);
		ImGui::Checkbox("###Enabled", &exposureEnabled);
	}
	
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

	// turn off Vsync
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

