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
enum Method { Random, Particle };
Method methods[2] = { Random, Particle };
int currentMethod = 0;

// Variables
int lightPerSeg = 1;
vec3 particleSystemSeedSegment;
// --------------------

// BoltSegment Setup
// -------------
// STATIC BOLT
void DefineBoltLines(LineBoltSegment* lboltPtr, std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {
	for (int i = 0; i < numSegmentsInPattern - 1; i++) {
		lboltPtr[i].Setup(lightningPatternPtr[i], lightningPatternPtr[i + 1]);
	}
}

// DYNAMIC BOLT
void DefineBoltLines(vector<LineBoltSegment>* lboltPtr, vector<pair<vec3, vec3>>* lightningPatternPtr) {
	lboltPtr->clear();
	for (int i = 0; i < lightningPatternPtr->size(); i++) {
		lboltPtr->emplace_back(lightningPatternPtr->at(i).first, lightningPatternPtr->at(i).second);
	}
}
// -----------

// Bolt Light Setup
// -----------
// Position a number of lights between each bolt segment's start and end points, 
// including the start point but not the end point, this will create 
// a smooth number of lights along the bolt
// STATIC BOLT
void PositionBoltPointLights(vec3* lightPositionsPtr, 
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> lightningPatternPtr) {

	unsigned int lighPosIndex = 0;
	for (int seg = 0; seg < numSegmentsInPattern - 1; seg++) {
		vec3 segDir = (lightningPatternPtr[seg + 1] - lightningPatternPtr[seg]) / (float)lightPerSeg;
		for (int light = 0; light < lightPerSeg; light++) {
			*(lightPositionsPtr + lighPosIndex) = lightningPatternPtr[seg] + segDir * (float)light;
			lighPosIndex++;
		}
	}
}

// DYNAMIC BOLT
void PositionBoltPointLights(vector<vec3>* lightPositionsPtr,
	vector<pair<vec3, vec3>>* lightningPatternPtr) {
	// remove old light positions
	lightPositionsPtr->clear();

	for (int seg = 0; seg < lightningPatternPtr->size(); seg++) {
		vec3 segDir = (lightningPatternPtr->at(seg).second - lightningPatternPtr->at(seg).first) / (float)lightPerSeg;
		for (int light = 0; light < lightPerSeg; light++) {
			lightPositionsPtr->push_back(lightningPatternPtr->at(seg).first + segDir * (float)light);
		}
	}
}
// ----------

// Generate a New Bolt and set line and light positions
// ----------
// DYNAMIC BOLT
void NewBolt(vector<LineBoltSegment>* segmentsPtr, vector<vec3>* lightsPtr, 
	vec3 startPosition,	vector<pair<vec3, vec3>>* lightningPatternPtr) {
	// Generate New Bolt Pattern
	switch (methods[currentMethod]) {
	case Random:
		lightningPatternPtr = GenerateRandomPositionsLightningPattern(startPosition, 
			lightningPatternPtr);
		break;
	case Particle:
		lightningPatternPtr = GenerateParticleSystemPattern(startPosition, 
			particleSystemSeedSegment, lightningPatternPtr);
		break;
	}

	// Set the LineSegment's Positions
	DefineBoltLines(segmentsPtr, lightningPatternPtr);
	// Set the PointLight's Positions
	PositionBoltPointLights(lightsPtr, lightningPatternPtr);
}

// STATIC BOLT
void NewBolt(LineBoltSegment* segmentsPtr, vec3* lightsPtr, vec3 startPosition,
	std::shared_ptr<vec3[numSegmentsInPattern]> lightningPatternPtr) {
	// Generate New Bolt Pattern
	switch (methods[currentMethod]) {
	case Random:
		lightningPatternPtr = GenerateRandomPositionsLightningPattern(startPosition, 
			lightningPatternPtr);
		break;
	case Particle:
		lightningPatternPtr = GenerateParticleSystemPattern(startPosition, 
			particleSystemSeedSegment, lightningPatternPtr);
		break;
	}

	// Set the LingSegment's Positions
	DefineBoltLines(segmentsPtr, lightningPatternPtr);
	// Set the PointLight's Positions
	PositionBoltPointLights(lightsPtr, lightningPatternPtr);
}
// ---------

// Getters
int GetLightPerSegment() {
	return lightPerSeg;
}

int GetCurrentMethod() {
	return currentMethod;
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
}