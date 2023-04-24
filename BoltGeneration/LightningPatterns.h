#pragma once

#include <random>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <irrlicht/include/quaternion.h>
#include <iostream>
#include <imgui/imgui.h>

#include "../FunctionLibrary.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using std::pair;
using irr::core::quaternion;

// Defines the number of segments in all STATIC lightning patterns
// Also used as the central bolt size of DYNAMIC lightning patterns
const int numSegmentsInPattern = 100;

// Random Positions -----
int GenerateRandomPositionsPattern(vec3 start,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateRandomPositionsPattern(vec3 start,
	vector<pair<vec3, vec3>>* patternPtr);
// ----------------------

// Particle System ------
int GenerateParticleSystemPattern(vec3 start, vec3 seed,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateParticleSystemPattern(vec3 start, vec3 seed,
	vector<pair<vec3, vec3>>* patternPtr);
// ----------------------

// L-System -------------
int GenerateLSystemPattern(vec3 start, 
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vec3 start,
	vector<pair<vec3, vec3>>* patternPtr);
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vec3 start,
	vector<pair<vec3, vec3>>* patternPtr, bool x);
// ----------------------

// GUI
void BoltGenerationGUI(int method);