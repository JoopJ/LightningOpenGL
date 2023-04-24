#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

// screen
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void ProcessKeyboardInput(GLFWwindow* window);

// getters
glm::vec3 GetCameraPos();
glm::vec3 GetCameraFront();
glm::vec3 GetCameraUp();
float GetFOV();
float GetDeltaTime();
float GetLastFrame();
bool GetMouseEngaged();

// setters
void SetDeltaTime(float dt);
void SetLastFrame(float lf);
void SetMouseEngaged(bool me);
