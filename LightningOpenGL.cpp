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

// lightning options
float boltAlpha = 1.0f;
vec3 boltColor = vec3(1.0f, 1.0f, 1.0f);	// Yellow
vec3 cubeLightColor = vec3(1);				// White

// toggles
bool shadowsEnabled = true;

// Array Type
bool DYNAMIC_BOLT = true;

// Method Choice
int methodChoice = 2; // 0 = random, 1 = particle system, 2 = l-system

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
void ProcessMiscInput(GLFWwindow* window);
void ProcessLightningControlInput(GLFWwindow* window, bool* newBolt);
// Config
void ConfigureWindow();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);
// GUI
void RenderImGui(LightManager* lm, PerformanceManager* pm, FboManager* fm, bool* newBolt);
void BoltControlGUI(PerformanceManager* pm, bool* newBolt);
void SceneGUI();



int main() {
	std::cout << "LightningOpenGL" << std::endl;
	// Initial Configurations and Window Creation
	// -------------------------
	std::srand(time(0));	// seed random number generator
	bool newBolt = true;	// signals to generate a new bolt

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

	// Load
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
	// -------------------------

	// Bolt Objects Setup
	// -------------------------
	
	// STATIC
	// Uses a fixed sized array. size = numSegmentsInPattern (defined in LightningPatterns.h)

	// Segments
	LineBoltSegment staticSegments[numSegmentsInPattern+1];
	LineBoltSegment* staticSegmentsPtr = &staticSegments[0];
	// Point Lights
	const int maxNumPointLights = 100;
	vec3 staticPointLights[maxNumPointLights];
	vec3* staticPointLightsPtr;
	staticPointLightsPtr = &staticPointLights[0];
	// Bolt Pattern
	std::shared_ptr<vec3[numSegmentsInPattern]> staticBoltPtr;
	staticBoltPtr = std::make_shared<vec3[numSegmentsInPattern]>();

	// DYNAMIC
	// Uses a vector of dyanmic size. Required for branching.

	// Segments
	vector<LineBoltSegment> dynamicSegments;
	vector<LineBoltSegment>* dynamicSegmentsPtr = &dynamicSegments;
	// Point Lights
	vector<vec3> dynamicPointLights;
	vector<vec3>* dynamicPointLightsPtr;
	dynamicPointLightsPtr = &dynamicPointLights;
	// Bolt Pattern
	vector<pair<vec3, vec3>> dynamicBolt;
	vector<pair<vec3, vec3>>* dynamicBoltPtr = &dynamicBolt;

	// -------------------------

	// Shaders
	// -------------------------
	// Post Processing
	Shader blurShader = LoadShader("blur.vert", "blur.frag");
	Shader screenShader = LoadShader("screen.vert", "screen.frag");
	// Deferred Shadring
	Shader geometryPassShader = LoadShader("g_buffer.vert", "g_buffer.frag");
	Shader lightingPassShader = LoadShader("lighting_pass.vert", "lighting_pass.frag");
	// Shadow Mapping
	Shader depthShader = LoadShader("depth.vert", "depth.frag", "depth_multiple_cubemap.geom");
	// Light Cube (forward shading)
	Shader lightCubeShader = LoadShader("light.vert", "light.frag");
	// Bolt (forward shading)
	Shader boltShader = LoadShader("bolt.vert", "bolt.frag");

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
	// -------------------------

	// Initial Bolt Generation Options
	// ----------------
	// Set the Method
	SetMethod(2);

	// Set Method Properties:
	SetStartPos(vec3(20, 60, 0));

	// Generation Method 0: Random Positions
	// Scale dx & dz with dy
	SetRandomOptions(true);

	// Generation Method 1: Particle System
	// Seed Segment
	SetParticleOptions(vec3(6, -100, -4));

	// Generation Method 2: L-System
	// End Point, Detail, Max Displacement.
	SetLSystemOptions(vec3(10, 0, 0), 6, 12.0f);
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
		performanceManager.DynamicPatternInfo(dynamicBoltPtr);
		performanceManager.StaticPatternInfo(staticBoltPtr);
		// -----------------------

		// Input
		// -----------------------
		ProcessKeyboardInput(window);
		ProcessMiscInput(window);
		ProcessLightningControlInput(window, &newBolt);
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

		// Generate Bolt
		// -----------------------
		if (newBolt) {
			auto t1 = std::chrono::high_resolution_clock::now();

			// Dynamic Bolt
			if (DYNAMIC_BOLT) {
				NewBolt(dynamicSegmentsPtr, dynamicPointLightsPtr,
					dynamicBoltPtr);

				performanceManager.Update(NEW_BOLT, t1, std::chrono::high_resolution_clock::now());

				// Set the LineSegment's Positions based on generated pattern
				DefineBoltLines(dynamicSegmentsPtr, dynamicBoltPtr);
				// Set the PointLight's Positions based on generated pattern
				PositionBoltPointLights(dynamicPointLightsPtr, dynamicBoltPtr);
				// Set the LightManager's Light Positions
				lightManager.SetLightPositions(dynamicPointLightsPtr);
			}
			// Static Bolt
			else {
				NewBolt(staticSegmentsPtr, staticPointLightsPtr,
					staticBoltPtr);

				performanceManager.Update(NEW_BOLT, t1, std::chrono::high_resolution_clock::now());

				DefineBoltLines(staticSegmentsPtr, staticBoltPtr);
				PositionBoltPointLights(staticPointLightsPtr, staticBoltPtr);
				lightManager.SetLightPositions(staticPointLightsPtr);
			}
			/*
			duration<double, std::milli> ms = high_resolution_clock::now() - t1;
			sum += ms.count();
			count++;
			*/
		}
		// -----------------------

		// Rendering
		// -----------------------
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// MVP
		//mat4 model = mat4(1.0f);
		mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
		mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

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


		// 1.5. Shadow Maps: render depth maps for each light source
		// -----------------
		if (newBolt) {
			t1 = std::chrono::high_resolution_clock::now();
			lightManager.RenderDepthMaps();
			performanceManager.Update(SHADOW_MAPS, t1, std::chrono::high_resolution_clock::now());
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
		lightingPassShader.SetBool("blurEnabled", fboManager.GetGlowEnabled());

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
			DrawLineBolt(dynamicSegmentsPtr);

			if (lightManager.GetLightBoxesEnabled()) {
				// Draw Point Light boxes
				lightCubeShader.Use();
				SetVPMatricies(lightCubeShader, view, projection);
				DrawLightBoxes(lightCubeShader, dynamicPointLightsPtr);
			}
		}
		else {
			// Static Bolt
			DrawLineBolt(staticSegmentsPtr);

			if (lightManager.GetLightBoxesEnabled()) {
				// Draw Point Light boxes
				lightCubeShader.Use();
				SetVPMatricies(lightCubeShader, view, projection);
				DrawLightBoxes(lightCubeShader, staticPointLightsPtr);
			}
		}

		performanceManager.Update(RENDER_BOLT, t1, std::chrono::high_resolution_clock::now());

		// 4. Glow
		// -----------------
		t1 = std::chrono::high_resolution_clock::now();
		fboManager.ApplyGlow(blurShader);
		performanceManager.Update(GLOW, t1, std::chrono::high_resolution_clock::now());

		// 5. Blend Scene and Blurred Bolt to default framebuffer
		// -----------------
		t1 = std::chrono::high_resolution_clock::now();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.Use();
		// Binds textures and Sets uniforms
		fboManager.PrepareScreenShader(&screenShader);
		RenderQuad();

		performanceManager.Update(BLEND, t1, std::chrono::high_resolution_clock::now());

		// 6. GUI
		// -----------------
		newBolt = false;
		RenderImGui(&lightManager, &performanceManager, &fboManager, &newBolt);
		// -----------------------
		// End of Rendering

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
void ProcessMiscInput(GLFWwindow* window) {

	static bool firstButtonPress = true;
	static bool shadowKeyPressed = false;
	static bool bloomKeyPressed = false;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {

		if (firstButtonPress) {
			firstButtonPress = false;

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
		firstButtonPress = true;
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

// ProcessLightningControlInput, process inputs relating to control of the lightning.
// returns true when a new strike is initiated, false otherwise.
void ProcessLightningControlInput(GLFWwindow* window, bool* newBolt) {
	static bool spaceHeld = false;

	// recalculate lines
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		// std::cout << "New Strike" << std::endl;
		spaceHeld = true;
		*newBolt = true;
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
void RenderImGui(LightManager *lm, PerformanceManager *pm, FboManager *fm, bool* newBolt) {
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
		BoltControlGUI(pm, newBolt);

	if (toggleBoltGenWindow)
		BoltGenerationGUI(methodChoice);

	if (toggleLightingWindow)
		lm->LightingGUI();

	if (togglePostProcessingWindow)
		fm->PostProcessingGUI();

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

void BoltControlGUI(PerformanceManager* pm, bool* newBolt) {
	static const char* methodNames[3] = { "Random Positions", "Particle System", "L-System" };

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
		*newBolt = true;
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

void SceneGUI() {
	ImGui::Begin("Scenes");

	if (ImGui::BeginListBox("Scenes")) {
		if (ImGui::Selectable("Empty")) {
			SetScene(0);
			SetStartPos(vec3(20, 60, 0));
			SetEndPos(vec3(10, 0, 0));
		}
		if (ImGui::Selectable("Default")) {
			SetScene(1);
			SetStartPos(vec3(20, 60, 0));
			SetEndPos(vec3(10, 0, 0));
		}
		if (ImGui::Selectable("------")) {
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

