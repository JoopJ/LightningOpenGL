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
#include "Shader/Shader.h"
#include "Shader/ShaderSetup.h"
#include "FunctionLibrary.h"
#include "CameraControl.h"
#include "Renderer.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using glm::radians;
using glm::lookAt;

// time for the lightning
double strikeStartTime;
// lightning start position
const vec3 startPnt = vec3(400, 8000, 0);
// post processing
int amount = 1;
// lighting options
int attenuationChoice = 3;
int atteunationRadius;
vec3 boltColor = vec3(1.0f, 1.0f, 0.0f);
// Strike Simulation
bool strike = false;
bool hideBolts = false;
double waitDuration = 2;
double flashDuration = 0.5;
double flashFadeDuration = 1.5;
double darknessDuration = 3.5;
int newBoltProb = 5;
// Strike Part, manages the different parts of the strike
enum StrikePart { Wait, Flash, Fade, Darkness };
StrikePart strikePart = Wait;

// Debuging
bool lightBoxesEnable = false;

enum Method { Lines, TrianglesColor };
Method methods[2] = { Lines, TrianglesColor };
const char* methodNames[2] = { "Line", "TriangleColor" };
int methodChoice = 0;

// function prototypes
// MVP Setters
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 porjection);
void SetVPMatricies(Shader shader, mat4 view, mat4 projection);
// Define Bolt
void DefineBoltLines(LineBoltSegment* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DefineTriangleBolt(TriangleBoltSegment* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void PositionBoltPointLights(vec3* lightPositionsPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void NewBolt(LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* lightPositionsPtr,
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
// Drawing
void DrawTriangleBolt(TriangleBoltSegment* tboltPtr);
void DrawLineBolt(LineBoltSegment* lboltPtr);
// Input
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress);
void ProcessLightningControlInput(GLFWwindow* window,
	LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* bplpPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
// Config
void ConfigureWindow();
void RenderImGui();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);
// renderers
void RenderQuad();
void RenderCube();
void RenderFloor();
void RenderWall();

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

	// Bolt Objects
	// ---------------
	// Lines	- Colored line primatives
	LineBoltSegment boltLines[numSegmentsInPattern];
	LineBoltSegment* lboltPtr = &boltLines[0];
	// Triangles - Colored triangle primatives
	TriangleBoltSegment* tboltPtr;
	TriangleBoltSegment tsegments[numSegmentsInPattern];
	tboltPtr = &tsegments[0];
	// Point Lights - Point lights at each point in the lightning pattern
	vec3 boltPointLightPositions[numSegmentsInPattern];
	vec3* boltPointLightPositionsPtr;
	boltPointLightPositionsPtr = &boltPointLightPositions[0];
	// ---------------

	// Lightning Pattern
	// ---------------
	// 2D array of points
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr;
	// initial setting of pattern
	lightningPatternPtr = GenerateLightningPattern(glm::vec3(400, 8000, 0));
	DefineBoltLines(lboltPtr, lightningPatternPtr);
	DefineTriangleBolt(tboltPtr, lightningPatternPtr);
	PositionBoltPointLights(boltPointLightPositionsPtr, lightningPatternPtr);
	// ---------------

	// Shaders
	// ---------------
	std::string projectBase = ProjectBasePath();
	//std::cout << "Project Base Path: " << projectBase << std::endl;

	std::string boltVertexPath = projectBase + "\\Shader\\VertexShaders\\bolt.vs";
	std::string boltFragmentPath = projectBase + "\\Shader\\FragmentShaders\\bolt.fs";

	std::string screenVertexPath = projectBase + "\\Shader\\VertexShaders\\screen.vs";
	std::string screenFragmentPath = projectBase + "\\Shader\\FragmentShaders\\screen.fs";

	std::string blurVertexPath = projectBase + "\\Shader\\VertexShaders\\blur.vs";
	std::string blurFragmentPath = projectBase + "\\Shader\\FragmentShaders\\blur.fs";

	std::string lightVartexPath = projectBase + "\\Shader\\VertexShaders\\light.vs";
	std::string lightFragmentPath = projectBase + "\\Shader\\FragmentShaders\\light.fs";

	std::string objectVertexPath = projectBase + "\\Shader\\VertexShaders\\object.vs";
	std::string objectFragmentPath = projectBase + "\\Shader\\FragmentShaders\\object.fs";

	std::string objectMultipleLightsFragmentPath = projectBase + "\\Shader\\FragmentShaders\\multiple_lights_object.fs";

	// bolts
	Shader boltShader(boltVertexPath.c_str(), boltFragmentPath.c_str());
	// post processing
	Shader screenShader(screenVertexPath.c_str(), screenFragmentPath.c_str());
	Shader blurShader(blurVertexPath.c_str(), blurFragmentPath.c_str());
	// lighting
	Shader lightShader(lightVartexPath.c_str(), lightFragmentPath.c_str());
	Shader objectMultiLightShader(objectVertexPath.c_str(), objectMultipleLightsFragmentPath.c_str());
	// ---------------

	// Lighting
	// -------------------------
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

	// Properties
	// ---------------
	// Point Light
	vec3 plColor = vec3(1, 1, 0);	// point light color (lightning light color)
	// Wall
	vec3 wallColor = vec3(0.3, 0.3, 0.3);	// material color
	// Postprocessing
	// ---------------
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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tcbo[i], 0);
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
	// ----------------

	// Strike Simulation
	// -----------------

	// bolt properties
	// attenuation
	float linear;
	float quadratic;
	double timer = 0;
	double range;
	double fluxTime = 0.05;
	double fluxTimer = fluxTime;
	double flux = 1;
	newBoltProb = 100 - newBoltProb;
	// -----------------

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// pre-frame time logic
		// -----------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		SetDeltaTime(currentFrame - GetLastFrame());
		SetLastFrame(currentFrame);

		// Strike Simulation Logic
		if (strike) {
			timer += GetDeltaTime();
			switch (strikePart) {
			case Wait:
				if (timer > waitDuration) {
					strikePart = Flash;
					timer = 0;
					break;
				}
				hideBolts = true;
				linear = 0.9;
				quadratic = 2;
				break;
			case Flash:
				if (timer > flashDuration) {
					strikePart = Fade;
					timer = 0;
					break;
				}
				hideBolts = false;
				range = 100 + flux * 100 * (flashDuration - timer);
				linear = 4.5 / range;
				quadratic = 75.0 / (range * range);
				amount = 3 + 5 * (flashDuration - timer);
				break;
			case Fade:
				if (timer > flashFadeDuration) {
					strikePart = Darkness;
					timer = 0;
					break;
				}
				range = 100 * flux * (flashFadeDuration - timer);
				linear = 4.5 / range;
				quadratic = 75.0 / (range * range);
				amount = 1 + 3 * (flashFadeDuration - timer);				break;
			case Darkness:
				if (timer > darknessDuration) {
					strikePart = Wait;
					timer = 0;
					strike = false;
					hideBolts = false;
					break;
				}
				hideBolts = true;
				linear = 0.9;
				quadratic = 2;
				amount = 1;
				break;
			}
			if (strikePart == Flash && (rand() % 100 > newBoltProb)) {
				NewBolt(lboltPtr, tboltPtr, boltPointLightPositionsPtr, lightningPatternPtr);
			}
		}
		// -----------------------

		// input
		// -----------------------
		ProcessKeyboardInput(window);
		ProcessMiscInput(window, &firstMouseKeyPress);	// TODO: move GUI stuff into separate file
		ProcessLightningControlInput(window, lboltPtr, tboltPtr, boltPointLightPositionsPtr, lightningPatternPtr);

		// Render Lightning
		// -----------
		// 
		// MVP
		mat4 model;
		mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
		mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// first pass, to framebuffer object (fbo)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// Drawing
		// Bolts
		if (!hideBolts) {	// need to hide bolts when simulating a strike
			boltShader.Use();
			boltShader.SetVec3("color", boltColor);
			switch (methods[methodChoice]) {
			case Lines:
				// Line Bolt
				SetVPMatricies(boltShader, view, projection);
				DrawLineBolt(lboltPtr);
				break;
			case TrianglesColor:
				// Color Triangle Bolt
				SetVPMatricies(boltShader, view, projection);
				DrawTriangleBolt(tboltPtr);
				break;
			}
		}
		// ------------------		
		// Objects
		// ---------------
		// point lights positioned along the bolt
		if (lightBoxesEnable) {
			lightShader.Use();
			for (int i = 0; i < 100; i++) {
				model = mat4(1.0f);
				model = glm::translate(model, boltPointLightPositions[i]);
				// vec3 pos = (boltPointLightPositionsPtr)[i];
				///std::cout << "pos: " << pos.x << "," << pos.y << "," << pos.z << std::endl;
				//std::cout << "Light Pos: " << (boltPointLightPositionsPtr)[i].x << ", " << (boltPointLightPositionsPtr)[i].y << ", " << (boltPointLightPositionsPtr)[i].z << std::endl;
				model = glm::scale(model, vec3(0.1f));
				SetMVPMatricies(lightShader, model, view, projection);
				RenderCube();
			}
		}

		// point lights and walls
		objectMultiLightShader.Use();
		// get attenuation options
		// x = radius, y = linear, z = quadratic

		if (!strike) {
			vec3 attenuation = attenuationOptions[attenuationChoice];
			atteunationRadius = attenuation.x;
			linear = attenuation.y;
			quadratic = attenuation.z;
		}
		// camera
		objectMultiLightShader.SetVec3("viewPos", GetCameraPos());

		// set properties
		// spot lights
		SetShaderPointLightProperties(objectMultiLightShader, numSegmentsInPattern,
			boltPointLightPositionsPtr, linear, quadratic, plColor);
		// material
		SetShaderMaterialProperties(objectMultiLightShader, wallColor, 32.0f);
		// walls - using point lights
		model = mat4(1.0f);
		SetMVPMatricies(objectMultiLightShader, model, view, projection);
		RenderWall();
		RenderFloor();
		// render 2 more walls, to make a square room
		for (int i = 0; i < 2; i++) {
			model = glm::rotate(model, glm::radians(90.0f), vec3(0, 1, 0));
			objectMultiLightShader.SetMat4("model", model);
			RenderWall();
		}
		// ---------------
		
		// Post Processing
		// ---------------
		// Blur
		bool horizontal = true, first_iteration = true;
		blurShader.Use();
		for (int i = 0; i < amount+1; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			blurShader.SetInt("horizontal", horizontal);
			// bind texutre of other framebuffer, or scene if first iteration
			glBindTexture(GL_TEXTURE_2D, first_iteration ? tcbo[1] : pingpongBuffer[!horizontal]);
			// render quad
			RenderQuad();
			// swap buffers
			horizontal = !horizontal;
			if (first_iteration) {
				first_iteration = false;
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// ---------------

		// second pass, to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0, 0.5, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		screenShader.Use();
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		RenderQuad();

		// ---------------
		// 
		// Render GUI
		// ---------------
		RenderImGui();

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

void StartStrike() {
	strike = true;
}

// Bolt segment Setup
// -------------
void DefineBoltLines(LineBoltSegment* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		lboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i + 1]);
	}
}

void DefineTriangleBolt(TriangleBoltSegment* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		tboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i + 1]);
	}
}

void PositionBoltPointLights(vec3* lightPositionsPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < 100; i++) {
		*(lightPositionsPtr + i) = lightningPatternPtr[i];

	}
}

void NewBolt(LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* lightPositionsPtr, 
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	lightningPatternPtr = GenerateLightningPattern(glm::vec3(400, 8000, 0));
	// bolt point light positions
	PositionBoltPointLights(lightPositionsPtr, lightningPatternPtr);
	// bolt pattern positions
	switch (methods[methodChoice]) {
	case Lines:
		DefineBoltLines(lboltPtr, lightningPatternPtr);
		break;
	case TrianglesColor:
		DefineTriangleBolt(tboltPtr, lightningPatternPtr);
		break;
	}
}
// ----------------

// Bolt Draw
// ---------------
void DrawTriangleBolt(TriangleBoltSegment* tboltPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		tboltPtr[i].Draw();
	}
}

void DrawLineBolt(LineBoltSegment* lboltPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		lboltPtr[i].Draw();
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
}

bool spaceHeld = false;
// ProcessLightningControlInput, process inputs relating to control of the lightning
void ProcessLightningControlInput(GLFWwindow* window,
	LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* bplpPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {

	// recalculate lines	TODO: method choices should choose between lines and tbolts
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		std::cout << "New Strike" << std::endl;
		spaceHeld = true;
		lightningPatternPtr = GenerateLightningPattern(glm::vec3(400, 8000, 0));
		// bolt point light positions
		PositionBoltPointLights(bplpPtr, lightningPatternPtr);
		// bolt pattern positions
		switch (methods[methodChoice]) {
		case Lines:
			DefineBoltLines(lboltPtr, lightningPatternPtr);
			break;
		case TrianglesColor:
			DefineTriangleBolt(tboltPtr, lightningPatternPtr);
			break;
		}
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

	//ImGui::Begin("Lightning");
	//GUINaiveApproach();

	// Bolt Control window
	ImGui::Begin("Bolt Options");
	ImGui::Combo("Methods", &methodChoice, methodNames, IM_ARRAYSIZE(methodNames));	// dosen't work
	if (ImGui::Button("Show Light Positions")) {
		lightBoxesEnable = !lightBoxesEnable;
	}
	ImGui::End();

	ImGui::Begin("Post Processing");
	if (!strike) {
		ImGui::SliderInt("Blur Amount", &amount, 0, 20);
	}
	else {
		ImGui::Text("Striking");
	}
	ImGui::End();

	ImGui::Begin("Lighting");
	ImGui::Text("Attenuation");
	ImGui::Text("Radius: %d", atteunationRadius);
	ImGui::SliderInt("##", &attenuationChoice, 0, 11);
	if (ImGui::Button("Strike")) {
		StartStrike();
	}
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

