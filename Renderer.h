#pragma once

#include <glad/glad.h>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>

#include "Shader/Shader.h"
#include "Models/Model.h"
#include "FunctionLibrary.h"

using glm::mat4;
using glm::vec3;

void RenderScene(const Shader& shader);
void LoadModels();

void RenderGUI();

// Basics
void RenderQuad();
void RenderCube();
void RenderFloor();
void RenderWall();