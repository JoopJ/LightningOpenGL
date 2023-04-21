#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>

#include "../FunctionLibrary.h"
#include "../BoltGeneration/LightningPatterns.h"
#include "../BoltGeneration/BoltSetup.h"
#include "../Timer.h"

using std::vector;
using std::pair;

enum TimerID {
	GEOMETRY_PASS, RENDER_SHADOWS, LIGHTING_PASS, G_BUFFER_COPY, 
	RENDER_BOLT, BLOOM, BLEND, FRAME
};

const int numTimers = 8;

class PerformanceManager {
public:
	PerformanceManager();

	// Timers
	void StartTimer(TimerID id);
	void UpdateTimer(TimerID id);
	void SetTimerPerSecondOutput(TimerID id, bool outputPerSecond);
	void SetTimerAvgOutput(TimerID id, bool outputAvg);
	void SetTimerAvgTimeInterval(TimerID id, double timeInterval);

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
	Timer timers[numTimers];
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