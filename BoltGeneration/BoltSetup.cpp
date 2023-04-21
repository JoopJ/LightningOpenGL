/*
Handles the setup of the bolt, including: calling the appropriate pattern generation function and using the generated pattern 
to define the positions of the bolt segments and point lights;
Provides two options: STATIC and DYNAMIC refering to the type of arrays used

STATIC: all arrays have a fixed size, set by numSegmentsInPattern in LightningPatterns.h, thought the point lights position array
will have to be bigger to accomidate more than one light per segment, the point lights position array has it's own size: maxNumPointLight
in main(). LightsPerSegment cannot excede maxNumPointLight / numSegmentsInPattern, otherwise the point lights position array will overflow.

DYNAMIC: all arrays have a variable size, this is the most flexible option, but will require more memory and processing power. The main
advnatage of dynamic arrays is for branching: all branching is random so the number of segments in the bolt is not known in advance.

The STATIC option is the default, to use the DYNAMIC option, set the DYNAMIC_BOLT flag in main.cpp.
*/

#include "BoltSetup.h"

// -------------------
// Bolt Generation Method choices
enum Method { Random, Particle, LSystem };
Method methods[3] = { Random, Particle, LSystem };
int currentMethod = 0;
float lightPerSegRandom = 0.75f;
float lightPerSegParticle = 0.75f;
float lightPerSegLSystem = 0.5f;
// scales the number of lights per segment down as detail increases
float lightPerSegLSystemScalar = 0.5f;

// Variables
float lightPerSeg;
int numActiveLights;
int numActiveSegments;

vec3 particleSystemSeedSegment;
// --------------------

// BoltSegment Setup
// -------------
// STATIC BOLT
void DefineBoltLines(LineBoltSegment* lboltPtr, int numActiveSegments, 
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> patternPtr) {

	for (int i = 0; i < numActiveSegments - 1; i++) {
		lboltPtr[i].Setup(patternPtr[i], patternPtr[i + 1]);
	}
}

// DYNAMIC BOLT
void DefineBoltLines(vector<LineBoltSegment>* lboltPtr, 
	vector<pair<vec3, vec3>>* patternPtr) {

	lboltPtr->clear();
	for (int i = 0; i < patternPtr->size(); i++) {
		lboltPtr->emplace_back(patternPtr->at(i).first, patternPtr->at(i).second);
	}
}
// -----------

// Bolt Light Setup
// -----------
// Position a number of lights between each bolt segment's start and end points, 
// including the start point but not the end point, this will create 
// a smooth number of lights along the bolt

// STATIC BOLT
void PositionBoltPointLights(vec3* lightPositionsPtr, int numActiveSegments,
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> patternPtr) {

	unsigned int lighPosIndex = 0;
	float count = 0;

	for (int seg = 0; seg < numActiveSegments - 1; seg++) {
		count += lightPerSeg;

		if (count > 1) {
			// position light in the middle of the segment
			vec3 segDir = (patternPtr[seg + 1] - patternPtr[seg]);
			*(lightPositionsPtr + lighPosIndex) = patternPtr[seg] + segDir * 0.5f;
			lighPosIndex++;
			count -= 1;
		};
	}

	numActiveLights = lighPosIndex;
}

// DYNAMIC BOLT
void PositionBoltPointLights(vector<vec3>* lightPositionsPtr,
	vector<pair<vec3, vec3>>* patternPtr) {
	// remove old light positions
	lightPositionsPtr->clear();
	numActiveLights = 0;

	float count = 0;

	for (int seg = 0; seg < patternPtr->size(); seg++) {
		count += lightPerSeg;

		if (count >= 1) {
			// position light in the middle of the segment
			vec3 segDir = (patternPtr->at(seg).second - patternPtr->at(seg).first);
			lightPositionsPtr->push_back(patternPtr->at(seg).first + segDir * 0.5f);

			numActiveLights++;
			count -= 1;
		};
	}
}
// ----------

// Generate a New Bolt and set line and light positions
// ----------
// DYNAMIC BOLT
void NewBolt(vector<LineBoltSegment>* segmentsPtr, vector<vec3>* lightsPtr, 
	vec3 startPosition,	vector<pair<vec3, vec3>>* patternPtr) {

	// Generate New Bolt Pattern
	switch (methods[currentMethod]) {
	case Random:
		patternPtr = GenerateRandomPositionsPattern(startPosition,
			patternPtr);
		break;
	case Particle:
		patternPtr = GenerateParticleSystemPattern(startPosition,
			particleSystemSeedSegment, patternPtr);
		break;
	case LSystem:
		patternPtr = GenerateLSystemPattern(startPosition, patternPtr);
		break;

	}

	// Set the LineSegment's Positions
	DefineBoltLines(segmentsPtr, patternPtr);
	// Set the PointLight's Positions
	PositionBoltPointLights(lightsPtr, patternPtr);
}

// STATIC BOLT
void NewBolt(LineBoltSegment* segmentsPtr, vec3* lightsPtr, vec3 startPosition,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	// Generate New Bolt Pattern
	numActiveSegments = 0;
	switch (methods[currentMethod]) {
	case Random:
		numActiveSegments = GenerateRandomPositionsPattern(startPosition, 
			patternPtr);
		break;
	case Particle:
		numActiveSegments = GenerateParticleSystemPattern(startPosition, 
			particleSystemSeedSegment, patternPtr);
		break;
	case LSystem:
		numActiveSegments = GenerateLSystemPattern(startPosition, patternPtr);
		break;
	}

	// Set the LingSegment's Positions
	DefineBoltLines(segmentsPtr, numActiveSegments, patternPtr);
	// Set the PointLight's Positions
	PositionBoltPointLights(lightsPtr, numActiveSegments, patternPtr);
}
// ---------

// Getters
int GetLightPerSegment() {
	return lightPerSeg;
}

int GetCurrentMethod() {
	return currentMethod;
}

int GetNumActiveLights() {
	return numActiveLights;
}

int GetNumActiveSegments() {
	return numActiveSegments;
}

// Setters
void SetLightPerSegment(int lightPerSegment) {
	lightPerSeg = lightPerSegment;
}

void SetParticleSystemSeedSegment(vec3 seed) {
	particleSystemSeedSegment = seed;
}

void SetMethod(int m) {
	currentMethod = m;

	// Set the number of lights per segment based on the method
	switch (m) {
		case 0:
			lightPerSeg = lightPerSegRandom;
			break;
		case 1:
			lightPerSeg = lightPerSegParticle;
			break;
		case 2:
			lightPerSeg = lightPerSegLSystem;

			// scale the number of lights per segment based on the LSystem detail
			if (GetLSystemDetail() > 3) {
				lightPerSeg *= pow(lightPerSegLSystemScalar, (GetLSystemDetail() - 3));
			}
			break;
	}
}