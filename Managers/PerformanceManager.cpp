#include "PerformanceManager.h"

PerformanceManager::PerformanceManager() {

	// Timers
	SetupTimers();
	
	// Pattern Info
	vectorNumElements = 0;
	vectorCapacity = 0;
	vectorSizeBytes = 0;

	arrayNumElements = 0;
	arrayCapacity = 0;
	arraySizeBytes = 0;
}

void PerformanceManager::SetupTimers() {
	double avgTimeInterval = 10.0;
	int chronoCountTarget = 100;

	const char* timerNames[numTimers] = { "New Bolt", "Geometry Pass",
		"Render Shadow Maps", "Lighting Pass", "Render Bolt", "Bloom",
		"Blend"};

	for (int i = 0; i < numTimers; i++) {
		timers[i].first.Setup(timerNames[i], chronoCountTarget);
		timers[i].second = false;	// default: update with average
	}
}

void PerformanceManager::PerformanceGUI() {
	ImGui::Begin("Performance");

	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

	ImGui::Text("Vector Elements: %d", vectorNumElements);

	ImGui::Text("Point Lights: %d", GetNumActiveLights());

	ImGui::Text("Lights Per Seg: %.5f", GetLightPerSegment());

	ImGui::End();
}

// Timers
void PerformanceManager::TimersGUI() {
	ImGui::Begin("Timers");
	ImGui::Separator();

	ImGui::Text("Average");
	for (int i = 0; i < numTimers; i++) {
		if (!timers[i].second) {
			//ImGui::Separator();
			timers[i].first.GUI();
		}
	}
	ImGui::Separator();

	ImGui::Text("Once");
	for (int i = 0; i < numTimers; i++) {
		if (timers[i].second) {
			//ImGui::Separator();
			timers[i].first.GUI();
		}
	}

	ImGui::End();
}

void PerformanceManager::Update(TimerID id, time_point<high_resolution_clock> t1, 
	time_point<high_resolution_clock> t2) {
	timers[id].first.Update(t1, t2);
}

void PerformanceManager::SetTimerCountTarget(TimerID id, int countTarget) {
	timers[id].first.SetChronoFrameTarget(countTarget);
}

// True: update once, False: update with average
void PerformanceManager::SetTimerUpdateType(TimerID id, bool set) {
	timers[id].first.SetChronoOnce(set);
	timers[id].second = set;
}

void PerformanceManager::SetOutputResults(TimerID id, bool set) {
	timers[id].first.SetOutputResults(set);
}

// Pettern Info
void PerformanceManager::DynamicPatternInfo(vector<pair<vec3, vec3>>* pattern) {
	vectorNumElements = pattern->size();
	vectorCapacity = pattern->capacity();
	vectorSizeBytes = pattern->size() * sizeof(pair<vec3, vec3>);
}

void PerformanceManager::StaticPatternInfo(std::shared_ptr<vec3[numSegmentsInPattern]> pattern) {
	arrayNumElements = numSegmentsInPattern;
	arrayCapacity = numSegmentsInPattern;
	arraySizeBytes = numSegmentsInPattern * sizeof(vec3);
}

void PerformanceManager::DynamicPatternGUI() {
	ImGui::Text("Number of Elements: %d", vectorNumElements);
	ImGui::Text("Capacity: %d", vectorCapacity);
	ImGui::Text("Size (Bytes): %d", vectorSizeBytes);
}

void PerformanceManager::StaticPatternGUI() {
	ImGui::Text("Number of Elements: %d", arrayNumElements);
	ImGui::Text("Capacity: %d", arrayCapacity);
	ImGui::Text("Size (Bytes): %d", arraySizeBytes);
}