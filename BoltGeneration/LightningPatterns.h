#pragma once

#include <random>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>
#include <iostream>
#include <cmath>
#include <imgui/imgui.h>

#include "../FunctionLibrary.h"

using glm::vec3;
using glm::mat4;
using glm::quat;
using std::vector;
using std::pair;

// Defines the number of segments in all STATIC lightning patterns
// Also used as the central bolt size of DYNAMIC lightning patterns
const int numSegmentsInPattern = 100;

// Random Positions -----
int GenerateRandomPositionsPattern(std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateRandomPositionsPattern(vector<pair<vec3, vec3>>* patternPtr);
// ----------------------

// Particle System ------
int GenerateParticleSystemPattern(std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateParticleSystemPattern(vector<pair<vec3, vec3>>* patternPtr);
// ----------------------

// L-System -------------
int GenerateLSystemPattern(std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vector<pair<vec3, vec3>>* patternPtr);
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vector<pair<vec3, vec3>>* patternPtr, bool x);
// ----------------------

// GUI
void BoltGenerationGUI(int method);

// Set Method Options
void SetStartPos(vec3 start);
void SetEndPos(vec3 end);
void SetLSystemOptions(vec3 end, int detail, float maxDisplacement);
void SetRandomOptions(bool _scale);
void SetParticleOptions(vec3 seed);
void SetNumSegments(int num);