#pragma once

#include <memory>
#include <vector>

#include "LineBoltSegment.h"
#include "TriangleBoltSegment.h"
#include "LightningPatterns.h"

// Functions
void DefineBoltLines(LineBoltSegment* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void DefineBoltLines(vector<LineBoltSegment>* lboltPtr, vector<pair<vec3, vec3>>* lightningPatternPtr);

void PositionBoltPointLights(vec3* lightPositionsPtr,
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr);
void PositionBoltPointLights(vector<vec3>* lightPositionsPtr,
	vector<pair<vec3, vec3>>* lightningPatternPtr);

void NewBolt(vector<LineBoltSegment>* segmentsPtr, vector<vec3>* lightsPtr,
	vec3 startPosition, vector<pair<vec3, vec3>>* lightningPatternPtr);
void NewBolt(LineBoltSegment* segmentsPtr, vec3* lightsPtr, vec3 startPosition,
	std::shared_ptr<vec3[numSegmentsInPattern]> lightningPatternPtr);

int GetLightPerSegment();
int GetCurrentMethod();

void SetLightPerSegment(int lightPerSegment);
void SetParticleSystemSeedSegment(vec3 seed);
void SetMethod(int m);