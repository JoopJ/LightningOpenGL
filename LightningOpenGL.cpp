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
//#include "Testing.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using glm::radians;
using glm::lookAt;

// post processing
int amount = 4;
float exposure = 8.0f;
float gamma = 2.2f;

// lightning options
float boltAlpha = 1.0f;
vec3 boltColor = vec3(1.0f, 1.0f, 1.0f);	// Yellow
vec3 cubeLightColor = vec3(1);				// White
bool newBolt = true; // signals to generate a new bolt

// toggles
bool shadowsEnabled = true;
bool shadowKeyPressed = false;
bool bloom = false;
bool bloomKeyPressed = false;
bool gammaCorrectionEnabled = true;
bool exposureEnabled = true;

// Array Type
bool DYNAMIC_BOLT = true;

// Method Choice
int methodChoice = 1; // 0 = random, 1 = particle system, 2 = l-system
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
void SceneGUI();



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

	// Load Models
	// -------------------------
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
	// Set timers that aren't updated every frame
	performanceManager.SetTimerUpdateType(NEW_BOLT, true);
	performanceManager.SetTimerUpdateType(SHADOW_MAPS, true);
	// Set timers to output their results
	//performanceManager.SetOutputResults(SHADOW_MAPS, true);
	// -------------------------

	// Initial Bolt Generation Options
	// ----------------
	// Set the Method
	SetMethod(methodChoice);
	// Set Method Properties:
	SetStartPos(vec3(20, 90, 0));

	// Generation Method 0: Random Positions
	SetRandomOptions(true);

	// Generation Method 1: Particle System
	SetParticleOptions(vec3(6, -100, -4));

	// Generation Method 2: L-System
	// End Point, Detail, Max Displacement.
	SetLSystemOptions(vec3(-20, 0, 0), 8, 30.0f);
	// ----------------

	// G-Buffer -------
	G_Buffer gBuffer(SCR_WIDTH, SCR_HEIGHT);
	// ----------------

	// FBO ------------
	FboManager fboManager(SCR_WIDTH, SCR_HEIGHT);
	// ----------------

	// Speed Testing --
	/*
	//BeginTesting();
	//glfwSetWindowShouldClose(window, true);
	const int wait = 0;
	int waitCount = 0;

	const int numTests = 10;
	int numLights = 1;

	double sum = 0;
	double average = 0;
	int count = 0;

	SetNumLights(numLights);
	*/
	// ----------------


	std::cout << "Starting Render Loop" << std::endl;
	// render loop
	while (!glfwWindowShouldClose(window)) {
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
		// -----------------------

		// Testing ---------------
		/*
		if (count >= numTests) {
			average = sum / double(count);
			std::cout << average << std::endl;

			count = 0;
			sum = 0;

			numLights++;
			SetNumLights(numLights);
		}

		if (numLights < 101) {
			newBolt = true;
		}
		*/
		// -----------------------

		// New Bolt
		if (newBolt) {
			auto t1 = std::chrono::high_resolution_clock::now();

			if (DYNAMIC_BOLT) {
				// Dynamic Bolt
				NewBolt(dynamicLineSegmentsPtr, dynamicPointLightPositionsPtr,
					lightningDynamicPatternPtr);

				performanceManager.Update(NEW_BOLT, t1, std::chrono::high_resolution_clock::now());

				// Set the LineSegment's Positions based on generated pattern
				DefineBoltLines(dynamicLineSegmentsPtr, lightningDynamicPatternPtr);
				// Set the PointLight's Positions based on generated pattern
				PositionBoltPointLights(dynamicPointLightPositionsPtr, lightningDynamicPatternPtr);
				// Set the LightManager's Light Positions
				lightManager.SetLightPositions(dynamicPointLightPositionsPtr);
			}
			else {
				// Static Bolt
				NewBolt(staticLineSegmentsPtr, staticPointLightPositionsPtr,
					lightningStaticPatternPtr);

				performanceManager.Update(NEW_BOLT, t1, std::chrono::high_resolution_clock::now());

				DefineBoltLines(staticLineSegmentsPtr, lightningStaticPatternPtr);
				PositionBoltPointLights(staticPointLightPositionsPtr, lightningStaticPatternPtr);
				lightManager.SetLightPositions(staticPointLightPositionsPtr);
			}
			/*
			duration<double, std::milli> ms = high_resolution_clock::now() - t1;
			sum += ms.count();
			count++;
			*/
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
		auto t1 = std::chrono::high_resolution_clock::now();

		gBuffer.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		geometryPassShader.Use();
		SetVPMatricies(geometryPassShader, view, projection);

		gBuffer.GeometryPass(geometryPassShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		performanceManager.Update(GEOMETRY_PASS, t1, std::chrono::high_resolution_clock::now());

		// Generate Shadow Maps, for new bolts
		if (newBolt) {
			t1 = std::chrono::high_resolution_clock::now();
			lightManager.RenderDepthMaps();
			performanceManager.Update(SHADOW_MAPS, t1, std::chrono::high_resolution_clock::now());
			newBolt = false;
		}

		// 2. Lighting Pass: calculate lighting by iterating over a screen filled quad 
		//					 pixel-by-pixel using the g-buffer's content.
		// -----------------

		t1 = std::chrono::high_resolution_clock::now();

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

		performanceManager.Update(LIGHTING_PASS, t1, std::chrono::high_resolution_clock::now());

		// 2.5. copy contents of geometry buffer to fbo
		// -----------------
		gBuffer.BindRead();
		fboManager.BindDraw();
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);


		// 3. Render Lightning Bolt
		// -----------------
		// rendered to fbo so bolt can be blurred and the scene and bolt can be blended together
		t1 = std::chrono::high_resolution_clock::now();
		
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

		performanceManager.Update(RENDER_BOLT, t1, std::chrono::high_resolution_clock::now());

		// 4. Bloom
		// -----------------
		t1 = std::chrono::high_resolution_clock::now();
		bool horizontal = fboManager.ApplyBloom(blurShader, amount);
		performanceManager.Update(BLOOM, t1, std::chrono::high_resolution_clock::now());

		// 5. Blend Scene and Blurred Bolt to default framebuffer
		// -----------------
		t1 = std::chrono::high_resolution_clock::now();

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

		performanceManager.Update(BLEND, t1, std::chrono::high_resolution_clock::now());

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

// GUI:
void RenderImGui(LightManager *lm, PerformanceManager *pm) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Window Toggles
	static bool toggleBoltGenWindow = false;
	static bool toggleLightingWindow = false;
	static bool togglePostProcessingWindow = false;
	static bool toggleSceneWindow = false;
	static bool toggleBoltControlWindow = false;
	static bool toggleTimersWindw = false;
	static bool toggleRenderWindow = false;

	// Window Toggle Menu
	ImGui::Begin("Window Menu", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	if (ImGui::Button("Bolt Generation")) {
		toggleBoltGenWindow = !toggleBoltGenWindow;
	}
	if (ImGui::Button("Bolt Method")) {
		toggleBoltControlWindow = !toggleBoltControlWindow;
	}
	if (ImGui::Button("Lighting")) {
		toggleLightingWindow = !toggleLightingWindow;
	}
	if (ImGui::Button("Post Processing")) {
		togglePostProcessingWindow = !togglePostProcessingWindow;
	}
	if (ImGui::Button("Render")) {
		toggleRenderWindow = !toggleRenderWindow;
	}
	if (ImGui::Button("Timers")) {
		toggleTimersWindw = !toggleTimersWindw;
	}
	if (ImGui::Button("Scene")) {
		toggleSceneWindow = !toggleSceneWindow;
	}

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

	if (toggleRenderWindow)
		RenderGUI();

	if (toggleSceneWindow)
		SceneGUI();

	// Performance
	pm->PerformanceGUI();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void BoltControlGUI(PerformanceManager* pm) {
	const ImVec2 startPos = ImVec2(5, 183);
	ImGui::SetNextWindowPos(startPos, ImGuiCond_Once);

	ImGui::Begin("Bolt Method", NULL, ImGuiWindowFlags_AlwaysAutoResize);

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
		ImGui::SliderFloat("##", &gamma, 0.1f, 5.0f, "%.1f");
		ImGui::Checkbox("##Enabled", &gammaCorrectionEnabled);
	}

	if (ImGui::CollapsingHeader("Exposre")) {
		ImGui::Text("Exposure:");
		ImGui::SliderFloat("###", &exposure, 0.1f, 30, "%.1f");
		ImGui::Checkbox("###Enabled", &exposureEnabled);
	}
	
	ImGui::End();
}

void SceneGUI() {
	ImGui::Begin("Scenes");

	if (ImGui::BeginListBox("Scenes")) {
		if (ImGui::Selectable("Empty")) {
			SetScene(0);
		}
		if (ImGui::Selectable("Default")) {
			SetScene(1);
			SetStartPos(vec3(-20, 90, 0));
			SetEndPos(vec3(-20, 0, 0));
		}
		if (ImGui::Selectable("Scene _")) {
			//SetScene(2);
		}
		ImGui::EndListBox();
	}

	ImGui::End();
}

GLFWwindow* CreateWindow() {
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lightning Bolt Generation and Rendering", NULL, NULL);
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

