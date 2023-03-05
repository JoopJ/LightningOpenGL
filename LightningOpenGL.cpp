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
#include "FunctionLibrary.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using glm::radians;
using glm::lookAt;

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

class Camera {
public:
	vec3 position;
	Camera() {
		position = vec3(0, 0, 0);
	}
};
Camera camera;


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

// ProcessLightningControlInput, process inputs relating to control of the lightning
void ProcessLightningControlInput(GLFWwindow* window, int* lineCount, std::shared_ptr<Line> linesPtr) {
	// recalculate lines
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		*lineCount = DefineLightningLines(vec3(400, 8000, 0), linesPtr);
	}
}

// render imgui
void RenderImGui(std::shared_ptr<float> rotationSpeed) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Lightning");

	GUINaiveApproach();

	// Camera Control window
	ImGui::Begin("Camera");
	ImGui::Text("Camera options:");
	ImGui::SliderFloat("rotationSpeed", rotationSpeed.get(), 0.0f, 100.0f);
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

	// camera and projection setup
	camera.position = vec3(cameraPosx, cameraPosy, cameraPosz);
	glm::mat4 projection = glm::perspective(glm::radians(cameraAngle), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	float angle = 0.0f;
	// shared pointer, so it can be passed to the imgui window function
	std::shared_ptr<float> rotationSpeed(new float(30.0f), std::default_delete<float>());

	// TODO: Allow Different methods of bolt generation to be chosen through GUI
	// Bolt Lines ptr
	std::shared_ptr<Line> linesPtr(new Line[2000], std::default_delete < Line[]>());
	// draw lines here -------------------------
	SetLineArraySize(2000);
	int lineCount = DefineLightningLines(vec3(400, 8000, 0), linesPtr);
	//------------------------------------------

	InitImGui(window);

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// set background color
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// keep time for rotation speed
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;

		// input
		ProcessMiscInput(window);
		ProcessCameraInput(window, cameraMoveSpeed, &cameraPosx, &cameraPosy);
		ProcessLightningControlInput(window, &lineCount, linesPtr);

		// update camera position(rotating)
		angle = deltaTime * *rotationSpeed;
		camera.position = vec3(cameraPosx * cos(radians(angle)), cameraPosy, cameraPosz * sin(radians(angle)));
		mat4 view = lookAt(camera.position, vec3(0, 0, 0), vec3(0, 1, 0));

		// draw lines
		int numLines = 0;
		for (int i = 0; i < lineCount; i++) {
			linesPtr.get()[i].SetMVP(projection * view);
			linesPtr.get()[i].Draw();
			//std::cout << "Line " << i << " drawn" << std::endl;
		}

		RenderImGui(rotationSpeed);

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