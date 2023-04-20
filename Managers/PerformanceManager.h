#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>

#include "../FunctionLibrary.h"
#include "../BoltGeneration/LightningPatterns.h"

using std::vector;
using std::pair;

class PerformanceManager {
public:
	PerformanceManager();

	void DynamicPatternInfo(vector<pair<vec3, vec3>>* pattern);
	void StaticPatternInfo(std::shared_ptr<vec3[numSegmentsInPattern]> pattern);

	void DynamicPatternGUI();
	void StaticPatternGUI();

private:
	// Pattern Info
	// Dynamic (vector)
	int vectorNumElements;
	int vectorCapacity;
	int vectorSizeBytes;
	// Static (array)
	int arrayNumElements;
	int arrayCapacity;
	int arraySizeBytes;
};