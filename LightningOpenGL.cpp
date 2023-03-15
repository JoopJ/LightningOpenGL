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
#include "BoltGeneration/NaiveApproach.h"
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

enum Method { Naive, ParticalSystem };
Method methods[2] = { Naive, ParticalSystem };
std::string methodNames[2] = { "Naive", "Partical System" };
int methodChoice = 0;

// function prototypes
int DefineBoltLines(vec3 startPos, std::shared_ptr<Line> linesPtr);
void ProcessMiscInput(GLFWwindow* window);
void ProcessCameraInput(GLFWwindow* window, float cameraMoveSpeed, float* cameraPosxPtr, float* cameraPosyPtr);
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount, std::shared_ptr<Line> linesPtr);
void RenderImGui(std::shared_ptr<float> rotationSpeed);
void ConfigureWindow();
GLFWwindow* CreateWindow();
void InitImGui(GLFWwindow* window);

int main() {
	// Camera controls
	float cameraPosx = 40;
	float cameraPosy = 40;
	float cameraPosz = 40;
	float cameraAngle = 45;
	float cameraMoveSpeed = 0.1f;

	// seed
	std::srand(std::time(NULL));
	SetWidthAndHeight(SCR_WIDTH, SCR_HEIGHT);

	ConfigureWindow();
	GLFWwindow* window = CreateWindow();

	if (window == NULL) {	// exit if window creation fails
		return -1;
	}

	// TODO: Allow Different methods of bolt generation to be chosen through GUI
	// Bolt Lines ptr
	std::shared_ptr<Line> linesPtr(new Line[2000], std::default_delete < Line[]>());
	// draw lines here -------------------------
	SetLineArraySize(2000);
	int lineCount = DefineBoltLines(vec3(400, 8000, 0), linesPtr);
	//------------------------------------------

	InitImGui(window);

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// set background color
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// pre-frame time logic
		// -----------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		SetDeltaTime(currentFrame - GetLastFrame());
		SetLastFrame(currentFrame);

		// input
		ProcessMiscInput(window);
		ProcessCameraInput(window, cameraMoveSpeed, &cameraPosx, &cameraPosy);
		ProcessLightningControlInput(window, &lineCount, linesPtr);

		double strikeDuration = 0.1;
		// draw lines one by one over time
		double currentTime = glfwGetTime();
		int numLinesToDraw = std::min((int)((currentTime - strikeStartTime) / (strikeDuration / lineCount)), lineCount);
		if (numLinesToDraw != lineCount) {
			std::cout << numLinesToDraw << std::endl;
		}


		mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
		mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		for (int i = 0; i < numLinesToDraw; i++) {
			if (i % 2 == 0) { linesPtr.get()[i].SetColor(vec3(1, 0, 1)); }
			linesPtr.get()[i].SetProjection(projection);
			linesPtr.get()[i].SetView(view);
			linesPtr.get()[i].Draw();
		}

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

void SetVPMatricies(Shader shader) {
	// camera and projection setup
	mat4 view = lookAt(GetCameraPos(), GetCameraPos() + GetCameraFront(), GetCameraUp());
	shader.SetMat4("view", view);
	mat4 projection = glm::perspective(glm::radians(GetFOV()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	shader.SetMat4("projection", projection);
}

int DefineBoltLines(vec3 startPos, std::shared_ptr<Line> linesPtr) {
	switch (methods[methodChoice]) {
	case Naive:
		return DefineBoltLinesNA(startPos, linesPtr);
		break;
	default:
		return 0;
		break;
	}
}

// Input Processing:
// ProcessMiscInput, process inputs relating to control of the application
void ProcessMiscInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// ProcessCameraInput, process inputs relating to control of the camera
void ProcessCameraInput(GLFWwindow* window, float cameraMoveSpeed, float* cameraPosxPtr, float* cameraPosyPtr) {
	// move camera position
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		*cameraPosyPtr += cameraMoveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		*cameraPosyPtr -= cameraMoveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		*cameraPosxPtr -= cameraMoveSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		*cameraPosxPtr += cameraMoveSpeed;
	}
}

bool spaceHeld = false;

// ProcessLightningControlInput, process inputs relating to control of the lightning
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount, std::shared_ptr<Line> linesPtr) {
	// recalculate lines
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceHeld) {
		std::cout << "New Strike" << std::endl;
		*lineCount = DefineBoltLines(vec3(400, 8000, 0), linesPtr);
		spaceHeld = true;
		strikeStartTime = glfwGetTime();
	}
	else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
		spaceHeld = false;
	}
}

// render imgui
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

// initialize and configure glfw
void ConfigureWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // REMOVE IF ISSUE WITH MACS

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
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