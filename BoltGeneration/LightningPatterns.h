#pragma once

#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include <iostream>

#include "../FunctionLibrary.h"

using glm::vec3;
using glm::mat4;

const int numSegmentsInPattern = 100;

std::shared_ptr<glm::vec3[numSegmentsInPattern]> GenerateLightningPattern(glm::vec3 startPnt);
vec3 NextPoint(vec3 point);
void GenerateParticalSystemPattern(vec3 seed);
// Partical System
void GenerateParticalSystemPattern(vec3 seed);
mat4 GetTransformsFromSeed(vec3 seed);
vec3 ParticalSystemNextPoint(vec3 point, mat4 ft, mat4 it);