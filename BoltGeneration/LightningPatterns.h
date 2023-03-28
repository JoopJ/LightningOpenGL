#pragma once

#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#include <iostream>

#include "../FunctionLibrary.h"
#include "../include/Irrlicht/include/quaternion.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using std::pair;
using irr::core::quaternion;

const int numSegmentsInPattern = 100;

// Random Positions
std::shared_ptr<glm::vec3[numSegmentsInPattern]> GenerateRandomPositionsLightningPattern(glm::vec3 startPnt);
vec3 NextPoint(vec3 point);
// Partical System
vector<pair<vec3, vec3>> GenerateParticalSystemPattern(vec3 start, vec3 seed);
vec3 RotatePointAboutSeed(vec3 point, pair<vec3, vec3> seedPerpAxis);