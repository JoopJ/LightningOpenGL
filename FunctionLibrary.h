#pragma once

#include <glm/glm.hpp>
using glm::vec3;
using glm::mat4;
#include <glad/glad.h>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Shader/Shader.h"

void LoadTextures();
std::string ProjectBasePath();
unsigned int LoadTexture(const char* path);
void SetVPMatricies(Shader shader, mat4 view, mat4 projection);
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 projection);
void BindBoltTexture();
vec3 ConvertWorldToScreen(vec3 pos);
void SetWidthAndHeight(unsigned int width, unsigned int height);

