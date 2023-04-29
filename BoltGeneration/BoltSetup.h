#pragma once

#include <memory>
#include <vector>

#include "LineBoltSegment.h"
#include "LightningPatterns.h"

// Functions
void DefineBoltLines(LineBoltSegment* lboltPtr, 
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> patternPtr);
void DefineBoltLines(vector<LineBoltSegment>* lboltPtr, 
	vector<pair<vec3, vec3>>* patternPtr);

void PositionBoltPointLights(vec3* lightPositionsPtr,
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> patternPtr);
void PositionBoltPointLights(vector<vec3>* lightPositionsPtr,
	vector<pair<vec3, vec3>>* patternPtr);

// DYNAMIC
void NewBolt(vector<LineBoltSegment>* segmentsPtr, vector<vec3>* lightsPtr, 
	vector<pair<vec3, vec3>>* patternPtr);
// STATIC
void NewBolt(LineBoltSegment* segmentsPtr, vec3* lightsPtr,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr);

// Getters / Setters
float GetLightPerSegment();
int GetCurrentMethod();
int GetNumActiveLights();
int GetNumActiveSegments();
int GetNumLights();

void SetNumLights(int num);
void SetParticleSystemSeedSegment(vec3 seed);
void SetMethod(int m);