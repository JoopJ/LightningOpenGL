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
bool bloom = true;
// lighting options
int attenuationChoice = 3;
int atteunationRadius;
float boltAlpha = 1.0f;
vec3 boltColor = vec3(1.0f, 1.0f, 0.0f);
float lightPerSeg = 1;	// the number of light boxes, including the start, that are placed per segment

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
void DefineBoltLines(LineBoltSegment* lboltPtr, vector<pair<vec3, vec3>>* lightningPatternPtr);
void DefineTriangleBolt(TriangleBoltSegment* tboltPtr, vector<pair<vec3, vec3>>* lightningPatternPtr);
void PositionBoltPointLights(vec3* lightPositionsPtr, 
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void PositionBoltPointLights(vec3* lightPositionsPtr,
	vector<pair<vec3, vec3>>* lightningPatternPtr);
void NewBolt(LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* lightPositionsPtr,
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void NewBolt(LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* lightPositionsPtr,
	vector<pair<vec3, vec3>>* lightningPatternPtr);
// Drawing
void DrawTriangleBolt(TriangleBoltSegment* tboltPtr);
void DrawLineBolt(LineBoltSegment* lboltPtr);
// Input
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress);
void ProcessLightningControlInput(GLFWwindow* window,
	LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* bplpPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void ProcessLightningControlInput(GLFWwindow* window,
	LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* bplpPtr, vector<pair<vec3, vec3>>* lightningPatternPtr);
// Config
void ConfigureWindow();
void RenderImGui();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);
// renderers
void RenderScene(const Shader& shader);

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

	// Bolt Objects
	// ---------------
	// Lines	- Colored line primatives
	LineBoltSegment boltLines[numSegmentsInPattern+1];
	LineBoltSegment* lboltPtr = &boltLines[0];
	// Triangles - Colored triangle primatives
	TriangleBoltSegment* tboltPtr;
	TriangleBoltSegment tsegments[numSegmentsInPattern+1];
	tboltPtr = &tsegments[0];
	// segmetns are initiall setup as static, TODO allow them to be changed to dynamic

	// Point Lights - Point lights at each point in the lightning pattern
	const int maxNumPointLights = 3000;
	vec3 boltPointLightPositions[maxNumPointLights];
	vec3* boltPointLightPositionsPtr;
	boltPointLightPositionsPtr = &boltPointLightPositions[0];
	// ---------------

	// Lightning Pattern
	// ---------------
	// 2D array of points
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr;
	vec3 seed = vec3(6, -1000, -4);
	vector<pair<vec3, vec3>> lightningDynamicPattern = GenerateParticalSystemPattern(startPnt, seed);
	vector<pair<vec3, vec3>>* lightningDynamicPatternPtr = &lightningDynamicPattern;
	// initial setting of pattern
	//lightningPatternPtr = GenerateRandomPositionsLightningPattern(startPnt);
	//DefineBoltLines(lboltPtr, lightningPatternPtr);
	//DefineTriangleBolt(tboltPtr, lightningPatternPtr);
	DefineBoltLines(lboltPtr, lightningDynamicPatternPtr);
	//PositionBoltPointLights(boltPointLightPositionsPtr, lightningDynamicPatternPtr);
	PositionBoltPointLights(boltPointLightPositionsPtr, lightningDynamicPatternPtr);

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
	
	objectMultiLightShader.Use();
	objectMultiLightShader.SetInt("material.diffuse", 0);
	screenShader.Use();
	screenShader.SetInt("screenTexture", 0);
	screenShader.SetInt("bloomBlur", 1);
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
	vec3 plColor = vec3(1, 1, 1);	// point light color (lightning light color)
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
	// ----------------

	// -----------------
	// Performance Constructors
	// -----------------

	Performance mainLoop("Main Loop", 10.0);
	mainLoop.SetPerSecondOutput(false);
	mainLoop.SetAverageOutput(false);
	mainLoop.Start();

	// specifi measure
	double totalTime = 0;
	double lastTime = 0;
	int numPasses = 0;
	double outputTime = 60;
	// -----------------

	// Testing ---------
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
		ProcessLightningControlInput(window, lboltPtr, tboltPtr, boltPointLightPositionsPtr, lightningDynamicPatternPtr);

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
		boltShader.SetVec3("color", boltColor);
		boltShader.SetFloat("alpha", boltAlpha);
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
		// Objects
		// ---------------
		// point lights positioned along the bolt

		if (lightBoxesEnable) {
			lightShader.Use();
			SetVPMatricies(lightShader, view, projection);
			for (int i = 0; i < lightPerSeg * numSegmentsInPattern; i++) {
				model = mat4(1.0f);
				model = glm::translate(model, boltPointLightPositions[i]);
				model = glm::scale(model, vec3(0.3f));
				lightShader.SetMat4("model", model);
				RenderCube();
			}
		}

		// point lights and walls
		objectMultiLightShader.Use();
		// get attenuation options
		// x = radius, y = linear, z = quadratic

		atteunationRadius = attenuationOptions[attenuationChoice].x;

		// camera
		objectMultiLightShader.SetVec3("viewPos", GetCameraPos());

		// set properties
		// spot lights
		SetShaderPointLightProperties(objectMultiLightShader, numSegmentsInPattern * lightPerSeg,
			boltPointLightPositionsPtr, attenuationOptions[attenuationChoice].y,
								attenuationOptions[attenuationChoice].z, plColor);
		// material
		SetShaderMaterialProperties(objectMultiLightShader, wallColor, 32.0f);
		// bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, metalWallTexture);
		// walls - using point lights
		model = mat4(1.0f);
		SetMVPMatricies(objectMultiLightShader, model, view, projection);
		//double time1 = glfwGetTime();
		RenderScene(objectMultiLightShader);
		/*
		double time2 = glfwGetTime();
		totalTime += time2 - time1;
		numPasses++;
		// output average every 1000 passes
		if (numPasses >= 5000) {
			std::cout << "----------------------------" << std::endl;
			std::cout << "Number of Lights: " << lightPerSeg * numSegmentsInPattern << std::endl;
			std::cout << "ms: " << 1000.0 * (totalTime) / double(numPasses) << std::endl;
			std::cout << "----------------------------" << std::endl << std::endl;
			numPasses = 0;
			totalTime = 0;
		}
		*/
		// ---------------
		
		// Post Processing
		// ---------------
		// Blur
		bool horizontal = true, first_iteration = true;
		blurShader.Use();
		
		//double time1 = glfwGetTime();
		for (int i = 0; i < amount+1; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			blurShader.SetInt("horizontal", horizontal);
			// bind texutre of other framebuffer, or the texture to blur if first iteration
			glBindTexture(GL_TEXTURE_2D, first_iteration ? tcbo[1] : pingpongBuffer[!horizontal]);
			// render quad
			RenderQuad();
			// swap buffers
			horizontal = !horizontal;
			if (first_iteration) {
				first_iteration = false;
			}
		}
		/*
		double time2 = glfwGetTime();
		totalTime += time2 - time1;
		numPasses++;
		// output average every 1000 passes
		if (numPasses >= 1000) {
			std::cout << "----------------------------" << std::endl;
			std::cout << "Number of Blur Passes: " << amount << std::endl;
			std::cout << "ms: " << 1000.0 * (totalTime) / double(numPasses) << std::endl;
			std::cout << "----------------------------" << std::endl << std::endl;
			numPasses = 0;
			totalTime = 0;
			amount += 10;
		}
		*/
		

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// ---------------

		// second pass, to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0, 0.5, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		screenShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tcbo[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		screenShader.SetBool("bloom", bloom);
		screenShader.SetFloat("exposure", exposure);
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

// Renderers
// ---------------
// Render Scene
void RenderScene(const Shader& shader) {

	// floor
	RenderFloor();

	// walls
	mat4 model = mat4(1.0f);
	model = glm::translate(model, vec3(0, 30, 0));
	shader.SetMat4("model", model);
	RenderWall();
	// render 2 more walls, to make a square room
	for (int i = 0; i < 2; i++) {
		model = glm::rotate(model, glm::radians(90.0f), vec3(0, 1, 0));
		shader.SetMat4("model", model);
		RenderWall();
	}

	// boxes
	model = mat4(1.0f);
	model = glm::translate(model, vec3(8, 10, 15));
	model = glm::scale(model, vec3(10));
	shader.SetMat4("model", model);
	RenderCube();
	model = mat4(1.0f);
	model = glm::translate(model, vec3(-3, 20, -19));
	model = glm::scale(model, vec3(20));
	shader.SetMat4("model", model);
	RenderCube();
}
// ---------------

// Bolt segment Setup
// -------------
// Static bolts
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
// Dynamic bolt
void DefineBoltLines(LineBoltSegment* lboltPtr, vector<pair<vec3, vec3>>* lightningPatternPtr) {
	for (int i = 0; i < lightningPatternPtr->size(); i++) {
		lboltPtr[i].Setup(lightningPatternPtr->at(i).first, lightningPatternPtr->at(i).second);
	}
}

void DefineTriangleBolt(TriangleBoltSegment* tboltPtr, vector<pair<vec3, vec3>>* lightningPatternPtr) {
	for (int i = 0; i < lightningPatternPtr->size(); i++) {
		tboltPtr[i].Setup(lightningPatternPtr->at(i).first, lightningPatternPtr->at(i).second);
	}
}

// Position a number of light between each bolt segments start and end points
void PositionBoltPointLights(vec3* lightPositionsPtr, 
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	
	unsigned int lighPosIndex = 0;
	for (int seg = 0; seg < numSegmentsInPattern-1; seg++) {
		vec3 segDir = (lightningPatternPtr[seg + 1] - lightningPatternPtr[seg]) / lightPerSeg;
		for (int light = 0; light < lightPerSeg; light++) {
			*(lightPositionsPtr + lighPosIndex) = lightningPatternPtr[seg] + segDir * (float)light;
			lighPosIndex++;
		}
	}
}

// Using a dynamic number of segments:
// Position a number of light between each bolt segments start and end points
void PositionBoltPointLights(vec3* lightPositionsPtr,
	vector<pair<vec3, vec3>>* lightningPatternPtr) {
	unsigned int lightPosIndex = 0;
	for (int seg = 0; seg < lightningPatternPtr->size(); seg++) {
		vec3 segDir = (lightningPatternPtr->at(seg).second - lightningPatternPtr->at(seg).first) / lightPerSeg;
		for (int light = 0; light < lightPerSeg; light++) {
			*(lightPositionsPtr + lightPosIndex) = lightningPatternPtr->at(seg).first + segDir * (float)light;
			lightPosIndex++;
		}
	}
}

// Static bolt
void NewBolt(LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* lightPositionsPtr, 
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	lightningPatternPtr = GenerateRandomPositionsLightningPattern(startPnt);
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
// Dynamic bolt
void NewBolt(LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* lightPositionsPtr,
	vector<pair<vec3, vec3>>* lightningPatternPtr) {
	vector<pair<vec3, vec3>> lightningPattern = GenerateParticalSystemPattern(startPnt, vec3(6, -1000, -4));
	lightningPatternPtr = &lightningPattern;
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
		// std::cout << "New Strike" << std::endl;
		spaceHeld = true;
		NewBolt(lboltPtr, tboltPtr, bplpPtr, lightningPatternPtr);
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
		spaceHeld = false;
	}
}

// Dynamic bolt
// ProcessLightningControlInput, process inputs relating to control of the lightning
void ProcessLightningControlInput(GLFWwindow* window,
	LineBoltSegment* lboltPtr, TriangleBoltSegment* tboltPtr, vec3* bplpPtr, vector<pair<vec3, vec3>>* lightningPatternPtr) {

	// recalculate lines	TODO: method choices should choose between lines and tbolts
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		// std::cout << "New Strike" << std::endl;
		spaceHeld = true;
		NewBolt(lboltPtr, tboltPtr, bplpPtr, lightningPatternPtr);
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
	ImGui::SliderFloat("Bolt Alpha", &boltAlpha, 0, 1);

	ImGui::End();

	ImGui::Begin("Post Processing");
	ImGui::SliderInt("Blur Amount", &amount, 0, 1000);
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

