#pragma once

#include <glm/glm/glm.hpp>
#include <glad/glad.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Shader/Shader.h"

using glm::vec3;
using glm::mat4;

// Textures
void LoadTextures();
unsigned int LoadTexture(const char* path);

// Shaders
Shader LoadShader(const char* vertex, const char* fragment,
	const char* geometry = nullptr);

// Paths
std::string ProjectBasePath();

// Shaders
void SetVPMatricies(Shader shader, mat4 view, mat4 projection);
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 projection);
void BindBoltTexture();

// Screen
vec3 ConvertWorldToScreen(vec3 pos);
void SetWidthAndHeight(unsigned int width, unsigned int height);

// General Outputs
void OutputVec3(vec3 vec);
void OutputMat4(mat4 mat);