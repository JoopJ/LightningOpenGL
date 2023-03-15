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
#include <glm/gtc/matrix_transform.hpp>\

// other files
#include "BoltGeneration/Line.h"
#include "BoltGeneration/TextureBolt.h"
#include "BoltGeneration/BoltTriangleColor.h"
#include "BoltGeneration/NaiveApproach.h"
#include "BoltGeneration/LightningPatterns.h"
#include "Shader/Shader.h"
#include "FunctionLibrary.h"
#include "CameraControl.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using glm::radians;
using glm::lookAt;

// time for the lightning
double strikeStartTime;
// lightning start position
const vec3 startPnt = vec3(400,8000, 0);

enum Method { Naive, ParticalSystem };
Method methods[2] = { Naive, ParticalSystem };
std::string methodNames[2] = { "Naive", "Partical System" };
int methodChoice = 0;

// function prototypes
void SetMVPMatricies(Shader shader, mat4 view, mat4 projection);
int DefineBoltLines(Line* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DefineBoltTextures(TextureBolt* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DrawBoltTriangleColor(BoltTriangleColor* cboltPtr);
void ProcessMiscInput(GLFWwindow* window, bool* mouseEngaged, bool* firstButtonPress);
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount,
	Line* lboltPtr, int methodChoice, TextureBolt* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
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
	// ------------------------

	// Input
	// ---------------
	// callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// mouse capture
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	bool mouseEngaged = true;
	bool firstMouseKeyPress = true;
	// ---------------

	// Bolt Objects
	// ---------------
	// Lines	- Colored line primatives
	Line boltLines[2000];
	Line* lboltPtr = &boltLines[0];
	int lineCount = 0;
	// TextureBolts	- Textured Triangle primatives
	TextureBolt* tboltPtr;
	TextureBolt boltTextures[numSegmentsInPattern];
	tboltPtr = &boltTextures[0];
	// Color Bolts	- Colored Triangle primatives
	BoltTriangleColor* cboltPtr;
	BoltTriangleColor boltColors[numSegmentsInPattern];
	cboltPtr = &boltColors[0];
	// ---------------

	// Lightning Pattern
	// ---------------
	// 2D array of points
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr;
	SetLineArraySize(2000);
	// ---------------

	// Shaders
	// ---------------
	std::string projectBase = ProjectBasePath();

	std::string vertexPath = projectBase + "\\Shader\\bolt_triangle_color.vs";
	std::string fragmentPath = projectBase + "\\Shader\\bolt_triangle_color.fs";

	Shader boltShaderColor(vertexPath.c_str(), fragmentPath.c_str());
	// ---------------

	// Textures
	// ---------------
	LoadTexture("C:/_THINGS/Programming/Dissertation/LightningOpenGL/triangle.png");
	// ---------------

	glBindFramebuffer(GL_FRAMEBUFFER,0);
	// render loop
	while (!glfwWindowShouldClose(window)) {
		// set background color
		glClearColor(0.0, 0.5, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// pre-frame time logic
		// -----------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		SetDeltaTime(currentFrame - GetLastFrame());
		SetLastFrame(currentFrame);

		// input
		// -----------------------
		ProcessKeyboardInput(window);
		ProcessMiscInput(window, &mouseEngaged, &firstMouseKeyPress);	// TODO: move GUI stuff into separate file
		ProcessLightningControlInput(window, &lineCount, lboltPtr, methodChoice, tboltPtr, lightningPatternPtr);

		// Render
		// -----------
		// 
		// MVP (no model's atm)
		mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
		mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		// Strike Timing
		/*
		double strikeDuration = 0.1;
		// draw lines one by one over time
		double currentTime = glfwGetTime();
		int numLinesToDraw = std::min((int)((currentTime - strikeStartTime) / (strikeDuration / lineCount)), lineCount);
		if (numLinesToDraw != lineCount) {
			std::cout << numLinesToDraw << std::endl;
		}
		*/

		// Line Bolt
		for (int i = 0; i < lineCount; i++) {
			// alternate color, useful to see structure
			if (i % 2 == 0) { lboltPtr[i].SetColor(vec3(1, 0, 1)); }
			lboltPtr[i].SetProjection(projection);
			lboltPtr[i].SetView(view);
			lboltPtr[i].Draw();
		}

		// Color Triangle Bolt
		boltShaderColor.Use();
		SetMVPMatricies(boltShaderColor, view, projection);
		DrawBoltTriangleColor(cboltPtr);
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

void SetMVPMatricies(Shader shader, mat4 view, mat4 projection) {
	// camera and projection setting
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
}

// Bolt segment Setup - TODO: 
// -------------
int DefineBoltLines(Line* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		lboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i + 1]);
	}
	return numSegmentsInPattern;
}

void DefineBoltTextures(TextureBolt* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		tboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i+1]);
	}
}

void DefineBoltColors(BoltTriangleColor* cboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		cboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i + 1]);
	}
}
// ----------------

// Bolt Draw
// ---------------
void DrawBoltTriangleColor(BoltTriangleColor* cboltPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		cboltPtr[i].Draw();
	}
}
// ---------------

// Input Processing:
// -------------------
// ProcessMiscInput, process inputs relating to control of the application
void ProcessMiscInput(GLFWwindow* window, bool* mouseEngaged, bool* firstButtonPress) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {

		if (*firstButtonPress) {
			*firstButtonPress = false;

			if (&mouseEngaged) {
				*mouseEngaged = false;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				*mouseEngaged = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}

		}
	}
	else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
		*firstButtonPress = true;
	}
}

bool spaceHeld = false;
// ProcessLightningControlInput, process inputs relating to control of the lightning
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount,
	Line* lboltPtr, int methodChoice, TextureBolt* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {

	// recalculate lines	TODO: method choices should choose between lines and tbolts
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		std::cout << "New Strike" << std::endl;
		spaceHeld = true;
		lightningPatternPtr = GenerateLightningPattern(glm::vec3(400, 8000, 0));
		// Lines
		*lineCount = DefineBoltLines(lboltPtr, lightningPatternPtr);
		// Tbolts
		DefineBoltTextures(tboltPtr, lightningPatternPtr);
		strikeStartTime = glfwGetTime();
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
void RenderImGui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Lightning");

	GUINaiveApproach();

	// Camera Control window
	ImGui::Begin("Options");
	ImGui::Text("Lightning Options:");
	ImGui::SliderInt("Bolt Generation Method", &methodChoice, 0, 1);
	ImGui::Text(("Current Method: " + methodNames[methodChoice]).c_str());

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

	return window;
}

void InitImGui(GLFWwindow* window) {
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}