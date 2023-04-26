#pragma once

#include <vector>
#include <iostream>
#include <glm/glm/glm.hpp>

#include "../FunctionLibrary.h"
#include "../BoltGeneration/LightningPatterns.h"
#include "../BoltGeneration/BoltSetup.h"
#include "../Timer.h"

using std::vector;
using std::pair;

enum TimerID {
	NEW_BOLT, GEOMETRY_PASS, SHADOW_MAPS, LIGHTING_PASS, 
	RENDER_BOLT, BLOOM, BLEND
};

const int numTimers = 7;

class PerformanceManager {
public:
	PerformanceManager();

	// Timers
	void Update(TimerID id, time_point<high_resolution_clock> t1);
	// Toggle Output
	void SetTimerOutput(TimerID id, bool set);
	// Time interval / Frame Count Target
	void SetTimerCountTarget(TimerID id, int countTarget);

	// GUI
	void TimersGUI();
	void PerformanceGUI();

	// Pattern Info
	void DynamicPatternInfo(vector<pair<vec3, vec3>>* pattern);
	void StaticPatternInfo(std::shared_ptr<vec3[numSegmentsInPattern]> pattern);
	void DynamicPatternGUI();
	void StaticPatternGUI();

private:
	// Timers
	// Timer: timer object
	// Bool: [False] update with averge (default) 
	//    or  [True] update with one frame (once)
	pair<Timer, bool> timers[numTimers];
	void SetupTimers();

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