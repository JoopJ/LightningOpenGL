#pragma once

#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>

#include <iostream>

#include "../include/imgui/imgui.h"
#include "../FunctionLibrary.h"
#include "../include/Irrlicht/include/quaternion.h"

using glm::vec3;
using glm::mat4;
using std::vector;
using std::pair;
using irr::core::quaternion;

// Defines the number of segments in all STATIC lightning patterns
// Also used as the central bolt size of DYNAMIC lightning patterns
const int numSegmentsInPattern = 50;

// Random Positions
std::shared_ptr<vec3[numSegmentsInPattern]> GenerateRandomPositionsLightningPattern(vec3 start,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateRandomPositionsLightningPattern(vec3 start,
	vector<pair<vec3, vec3>>* patternPtr);

vec3 NextPoint(vec3 point);

// Particle System
std::shared_ptr<vec3[numSegmentsInPattern]> GenerateParticleSystemPattern(vec3 start, vec3 seed,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateParticleSystemPattern(vec3 start, vec3 seed,
	vector<pair<vec3, vec3>>* patternPtr);

quaternion ConvertRotationQuaternion(vec3 seed, float angle);
pair<vec3, vec3> GetRotationAxis(vec3 seed);
vec3 RotatePointAboutSeed(vec3 point, pair<vec3, vec3> seedPerpAxis);

// L-System
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vec3 start, vec3 seed,
		vector<pair<vec3, vec3>>* patternPtr);


//  GUI
void BoltGenerationGUI(int method);