#include "CameraControl.h"

// camera settings
glm::vec3 cameraPos = glm::vec3(-19.0f, 15.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float fov = 90;

bool firstMouse = true;
bool mouseEngaged = true;

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// update viewpoert when window size if updated
	glViewport(0, 0, width, height);
}

// called whenever the mouse moves
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	if (mouseEngaged) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.5f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// don't flip when pitch is out of bounds
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
	}
}

// called whenever the mouse scroll wheel scrolls
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 90)
		fov = 90;
}

// Process Input
void ProcessKeyboardInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// directional control, based on camera's front and up vectors
	float cameraSpeed = static_cast<float>(9.5 * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraSpeed = static_cast<float>(19.5 * deltaTime);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

// getters
// --------------------
glm::vec3 GetCameraPos() {
	return cameraPos;
}

glm::vec3 GetCameraFront() {
	return cameraFront;
}

glm::vec3 GetCameraUp() {
	return cameraUp;
}

float GetFOV() {
	return fov;
}

float GetDeltaTime() {
	return deltaTime;
}

float GetLastFrame() {
	return lastFrame;
}

bool GetMouseEngaged() {
	return mouseEngaged;
}
// --------------------
// setters
// --------------------
void SetDeltaTime(float dt) {
	deltaTime = dt;
}

void SetLastFrame(float lf) {
	lastFrame = lf;
}

void SetMouseEngaged(bool me) {
	mouseEngaged = me;
}
// --------------------