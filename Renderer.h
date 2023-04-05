#pragma once

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader/Shader.h"

using glm::mat4;
using glm::vec3;

void RenderScene(const Shader& shader);

void RenderQuad();
void RenderCube();
void RenderFloor();
void RenderWall();