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
#include "BoltGeneration/Line.h"
#include "BoltGeneration/TextureBolt.h"
#include "BoltGeneration/BoltTriangleColor.h"
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
// post processing
int amount = 1;
// light
vec3 lightPos(1.2f, 1.0f, 2.0f);

enum Method { Lines, TrianglesColor };
Method methods[2] = { Lines, TrianglesColor };
const char* methodNames[2] = { "Line", "TriangleColor" };
int methodChoice = 1;

// function prototypes
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 porjection);
void SetVPMatricies(Shader shader, mat4 view, mat4 projection);
int DefineBoltLines(Line* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DefineBoltTextures(TextureBolt* tboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DefineBoltColors(BoltTriangleColor* cboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DrawBoltTriangleColor(BoltTriangleColor* cboltPtr);
void ProcessMiscInput(GLFWwindow* window, bool* firstButtonPress);
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount,
	Line* lboltPtr, BoltTriangleColor* cboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
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
	// initial setting of pattern
	lightningPatternPtr = GenerateLightningPattern(glm::vec3(400, 8000, 0));
	switch (methods[methodChoice]) {
	case Lines:
		lineCount = DefineBoltLines(lboltPtr, lightningPatternPtr);
		break;
	case TrianglesColor:
		DefineBoltColors(cboltPtr, lightningPatternPtr);
		break;
	}
	// ---------------

	// Shaders
	// ---------------
	std::string projectBase = ProjectBasePath();

	std::string boltColorVertexPath = projectBase + "\\Shader\\bolt_triangle_color.vs";
	std::string boltColorFragmentPath = projectBase + "\\Shader\\bolt_triangle_color.fs";

	std::string screenVertexPath = projectBase + "\\Shader\\screen.vs";
	std::string screenFragmentPath = projectBase + "\\Shader\\screen.fs";

	std::string blurVertexPath = projectBase + "\\Shader\\TwoPassGaussianBlur.vs";
	std::string blurFragmentPath = projectBase + "\\Shader\\TwoPassGaussianBlur.fs";

	std::string lightVartexPath = projectBase + "\\Shader\\light.vs";
	std::string lightFragmentPath = projectBase + "\\Shader\\light.fs";

	std::string objectVertexPath = projectBase + "\\Shader\\object.vs";
	std::string objectFragmentPath = projectBase + "\\Shader\\object.fs";

	//std::cout << "Vertex Path: " << vertexPath << std::endl;
	//std::cout << "Fragment Path: " << fragmentPath << std::endl;

	Shader boltShaderColor(boltColorVertexPath.c_str(), boltColorFragmentPath.c_str());
	Shader screenShader(screenVertexPath.c_str(), screenFragmentPath.c_str());
	Shader blurShader(blurVertexPath.c_str(), blurFragmentPath.c_str());
	Shader lightShader(lightVartexPath.c_str(), lightFragmentPath.c_str());
	Shader objectShader(objectVertexPath.c_str(), objectFragmentPath.c_str());
	// ---------------

	// Textures
	// ---------------
	// LoadTexture("");
	// ---------------

	// Lightning
	// ---------------
	float cubeVertices[] = {
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

	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindVertexArray(lightVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// no normal for light sources


	// Scene Objects
	// ---------------
	// Floor
	float floorVertices[48] = {
		// positions          // normals           // texture coords
		25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f,   25.0f, 0.0f,
		-25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		25.0f, -0.5f, 25.0f,  0.0f, 1.0f, 0.0f,   25.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   25.0f, 25.0f
	};
	unsigned int floorVAO, floorVBO;
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
	// -------------


	// Postprocessing
	// ---------------
	// framebuffer object
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	
	// texture color buffer object
	unsigned int tcbo;
	// genereate and attach to framebuffer object (fbo)
	glGenTextures(1, &tcbo);
	glBindTexture(GL_TEXTURE_2D, tcbo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tcbo, 0);

	// depth and stencil buffer object
	unsigned int rbo; // can be a renderbuffer object as we don't need to sample from it
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// allocate storage and unbind
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// attach to the framebuffer object (fbo)
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// check fbo is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER,0);

	// screen quad VAO
	float quadVertices[] = {	// fills whole screen in NDC;
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2 , GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

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
		ProcessMiscInput(window, &firstMouseKeyPress);	// TODO: move GUI stuff into separate file
		ProcessLightningControlInput(window, &lineCount, lboltPtr, cboltPtr, lightningPatternPtr);

		// Render Lightning
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

		// first pass, to framebuffer object (fbo)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// Drawing
		switch (methods[methodChoice]) {
		case Lines:
			// Line Bolt
			for (int i = 0; i < lineCount; i++) {
				// alternate color, useful to see structure
				// if (i % 2 == 0) { lboltPtr[i].SetColor(vec3(1, 0, 1)); }
				lboltPtr[i].SetProjection(projection);
				lboltPtr[i].SetView(view);
				lboltPtr[i].Draw();
			}
			break;
		case TrianglesColor:
			// Color Triangle Bolt
			boltShaderColor.Use();
			SetVPMatricies(boltShaderColor, view, projection);
			DrawBoltTriangleColor(cboltPtr);
			break;
		}
		// light
		lightShader.Use();
		mat4 model = mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, vec3(2));
		SetMVPMatricies(lightShader, model, view, projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// floor
		objectShader.Use();
		objectShader.SetVec3("light.position", lightPos);
		objectShader.SetVec3("viewPos", GetCameraPos());

		// light properties
		vec3 lightColor = vec3(1.0, 1.0, 1.0);
		vec3 diffuseColor = lightColor * vec3(0.5f); // decrease the influence
		vec3 ambientColor = diffuseColor * vec3(0.2f); // low influence
		objectShader.SetVec3("light.ambient", ambientColor);
		objectShader.SetVec3("light.diffuse", diffuseColor);
		objectShader.SetVec3("light.specular", vec3 (1.0f, 1.0f, 1.0f));

		// material properties 
		objectShader.SetVec3("material.ambient", vec3 (0.5f, 0.5f, 0.5f));
		objectShader.SetVec3("material.diffuse", vec3 (0.5f, 0.5f, 0.5f));
		objectShader.SetVec3("material.specular", vec3 (0.5f, 0.5f, 0.5f));
		objectShader.SetFloat("material.shininess", 32.0f);

		// view / projection transformations
		model = mat4(1.0f);
		SetMVPMatricies(objectShader, model, view, projection);

		// render
		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

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
			glBindTexture(GL_TEXTURE_2D, first_iteration ? tcbo : pingpongBuffer[!horizontal]);
			// render quad
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
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

		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// ---------------
		// 
		// Render GUI
		// ---------------
		RenderImGui();

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &fbo);

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
	Line* lboltPtr, BoltTriangleColor* cboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {

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
			DefineBoltColors(cboltPtr, lightningPatternPtr);
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