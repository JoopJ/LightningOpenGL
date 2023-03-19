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
const vec3 startPnt = vec3(400, 8000, 0);
// post processing
int amount = 1;
// light
vec3 lightPos(1.2f, 5.0f, 2.0f);

enum Method { Lines, TrianglesColor };
Method methods[2] = { Lines, TrianglesColor };
const char* methodNames[2] = { "Line", "TriangleColor" };
int methodChoice = 0;

// function prototypes
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 porjection);
void SetVPMatricies(Shader shader, mat4 view, mat4 projection);
int DefineBoltLines(LineBoltSegment* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DefineTriangleBolt(TriangleBoltSegment* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DrawTriangleBolt(TriangleBoltSegment* tboltPtr);
void DrawLineBolt(LineBoltSegment* lboltPtr);
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress);
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount,
	LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
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
	LineBoltSegment boltLines[2000];
	LineBoltSegment* lboltPtr = &boltLines[0];
	int lineCount = 0;
	// Triangles - Colored triangle primatives
	TriangleBoltSegment* tboltPtr;
	TriangleBoltSegment tsegments[numSegmentsInPattern];
	tboltPtr = &tsegments[0];
	// ---------------

	// Lightning Pattern
	// ---------------
	// 2D array of points
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr;
	// initial setting of pattern
	lightningPatternPtr = GenerateLightningPattern(glm::vec3(400, 8000, 0));
	switch (methods[methodChoice]) {
	case Lines:
		lineCount = DefineBoltLines(lboltPtr, lightningPatternPtr);
		break;
	case TrianglesColor:
		DefineTriangleBolt(tboltPtr, lightningPatternPtr);
		break;
	}
	// ---------------

	// Shaders
	// ---------------
	std::string projectBase = ProjectBasePath();
	//std::cout << "Project Base Path: " << projectBase << std::endl;

	std::string boltVertexPath = projectBase + "\\Shader\\bolt.vs";
	std::string boltFragmentPath = projectBase + "\\Shader\\bolt.fs";

	std::string screenVertexPath = projectBase + "\\Shader\\screen.vs";
	std::string screenFragmentPath = projectBase + "\\Shader\\screen.fs";

	std::string blurVertexPath = projectBase + "\\Shader\\blur.vs";
	std::string blurFragmentPath = projectBase + "\\Shader\\blur.fs";

	std::string lightVartexPath = projectBase + "\\Shader\\light.vs";
	std::string lightFragmentPath = projectBase + "\\Shader\\light.fs";

	std::string objectVertexPath = projectBase + "\\Shader\\object.vs";
	std::string objectFragmentPath = projectBase + "\\Shader\\object.fs";

	std::string objectMultipleLightsFragmentPath = projectBase + "\\Shader\\multiple_lights_object.fs";

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
	// multiple point light testing stuff
	vec3 pointLightPositions[4] = {
		vec3(10.7f, 3.2f, 2.0f),
		vec3(2.3f, 3.3f, -4.0f),
		vec3(-4.0f, 3.0f, -12.0f),
		vec3(-11.0f, 3.0f, -3.0f)
	};
	// ------------------------

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

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// pre-frame time logic
		// -----------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		SetDeltaTime(currentFrame - GetLastFrame());
		SetLastFrame(currentFrame);

		// input
		// -----------------------
		ProcessKeyboardInput(window);
		ProcessMiscInput(window, &firstMouseKeyPress);	// TODO: move GUI stuff into separate file
		ProcessLightningControlInput(window, &lineCount, lboltPtr, tboltPtr, lightningPatternPtr);

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
		boltShader.Use();
		boltShader.SetVec3("color", vec3(1, 1, 0));
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
		// ------------------
		
		// Point Lights - for testing, will be moved to bolts later
		// ------------------
		lightShader.Use();
		for (int i = 0; i < 4; i++) {
			model = mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, vec3(0.5f));
			SetMVPMatricies(lightShader, model, view, projection);
			RenderCube();
		}
		// Objects
		// ---------------

		// multiple spot light properties for object shader
		objectMultiLightShader.Use();
		vec3 slColor = vec3(1, 1, 0);	// light color
		vec3 slDiffuse = slColor * vec3(0.5f);
		vec3 slAmbient = slDiffuse * vec3(0.2f);
		vec3 slSpecular = vec3(1.0f, 1.0f, 1.0f);
		float slConstant = 1.0f;
		float slLinear = 0.09f;
		float slQuadratic = 0.032f;
		// camera
		objectMultiLightShader.SetVec3("viewPos", GetCameraPos());
		// spot light properties
		for (int i = 0; i < 4; i++) {
			std::ostringstream stream;
			stream << "pointLights[" << i << "].";
			std::string pointLight = stream.str(); // pointLight = "poingLights[i]."
			objectMultiLightShader.SetVec3(pointLight + "position", pointLightPositions[i]);
			objectMultiLightShader.SetVec3(pointLight + "ambient", slAmbient);
			objectMultiLightShader.SetVec3(pointLight + "diffuse", slDiffuse);
			objectMultiLightShader.SetVec3(pointLight + "specular", slSpecular);
			objectMultiLightShader.SetFloat(pointLight + "constant", slConstant);
			objectMultiLightShader.SetFloat(pointLight + "liner", slLinear);
			objectMultiLightShader.SetFloat(pointLight + "quadratic", slQuadratic);
		}
		// material properties
		vec3 mColor = vec3(0.3, 0.3, 0.3);	// material color
		vec3 mDiffuse = mColor * vec3(0.5f);
		vec3 mAmbient = mDiffuse * vec3(0.2f);
		vec3 mSpecular = vec3(1.0f, 1.0f, 1.0f);
		objectMultiLightShader.SetVec3("material.ambient", mAmbient);
		objectMultiLightShader.SetVec3("material.diffuse", mDiffuse);
		objectMultiLightShader.SetVec3("material.specular", mSpecular); // dosen't really take effect
		objectMultiLightShader.SetFloat("material.shininess", 32.0f);
		// floor
		model = mat4(1.0f);
		SetMVPMatricies(objectMultiLightShader, model, view, projection);
		// wall
		objectMultiLightShader.SetVec3("objectColor", vec3(0, 0, 0));
		RenderWall();
		// ---------------
		
		// Post Processing
		// ---------------
		// Blur
		bool horizontal = true, first_iteration = true;
		blurShader.Use();
		for (int i = 0; i < amount; i++)
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

void SetVPMatricies(Shader shader, mat4 view, mat4 projection) {
	// camera and projection setting
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
}

void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 projection) {
	shader.SetMat4("model", model);
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
}

// Bolt segment Setup - TODO: 
// -------------
int DefineBoltLines(LineBoltSegment* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		lboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i + 1]);
	}
	return numSegmentsInPattern;
}

void DefineTriangleBolt(TriangleBoltSegment* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		tboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i + 1]);
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
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount,
	LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {

	// recalculate lines	TODO: method choices should choose between lines and tbolts
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		std::cout << "New Strike" << std::endl;
		spaceHeld = true;

		lightningPatternPtr = GenerateLightningPattern(glm::vec3(400, 8000, 0));
		switch (methods[methodChoice]) {
		case Lines:
			*lineCount = DefineBoltLines(lboltPtr, lightningPatternPtr);
			break;
		case TrianglesColor:
			DefineTriangleBolt(tboltPtr, lightningPatternPtr);
			break;
		}
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

	//ImGui::Begin("Lightning");
	//GUINaiveApproach();

	// Camera Control window
	ImGui::Begin("Options");
	ImGui::Text("Lightning Options:");
	ImGui::Combo("Methods", &methodChoice, methodNames, IM_ARRAYSIZE(methodNames));	// dosen't work
	ImGui::SliderInt("Blur Amount", &amount, 1, 100);

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

unsigned int quadVAO = 0;
void RenderQuad() {
	if (quadVAO == 0) {	// setup
		float quadVertices[] = {	// fills whole screen in NDC;
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};
		unsigned int quadVBO;
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
void RenderCube() {
	if (cubeVAO == 0) {
		float cubeVertices[] = { // setup
			// position			  // normal
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
		};

		unsigned int cubeVBO;
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		glBindVertexArray(cubeVAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

}

unsigned int floorVAO = 0;
void RenderFloor() {
	if (floorVAO == 0) {
		float floorVertices[48] = {
			// positions          // normals           // texture coords
			25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f,   25.0f, 0.0f,
			-25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

			25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f,   25.0f, 0.0f,
			-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
			25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   25.0f, 25.0f
		};
		unsigned int floorVBO;
		glGenVertexArrays(1, &floorVAO);
		glGenBuffers(1, &floorVBO);
		glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

		glBindVertexArray(floorVAO);
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	// render
	glBindVertexArray(floorVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

unsigned int wallVAO = 0;
void RenderWall() {
	if (wallVAO == 0) {
		float wallVertices[]{
			// positions          // normals
			25.0f,  25.5f, 10.0f,  0.0f, 0.0f, -1.0f,	
			-25.0f,  25.5f, 10.0f,  0.0f, 0.0f, -1.0f,
			-25.0f, -25.5f, 10.0f,  0.0f, 0.0f, -1.0f,

			25.0f,  25.5f, 10.0f,  0.0f, 0.0f, -1.0f,
			-25.0f, -25.5f, 10.0f,  0.0f, 0.0f, -1.0f,
			25.0f, -25.5f, 10.0f,  0.0f, 0.0f, -1.0f,
		};

		unsigned int wallVBO;
		glGenVertexArrays(1, &wallVAO);
		glGenBuffers(1, &wallVBO);
		glBindVertexArray(wallVAO);
		glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);

		glBindVertexArray(wallVAO);
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	// render
	glBindVertexArray(wallVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}