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
	const char* timerNames[numTimers] = { "Geometry Pass", "Render Shadows", "Lighting Pass",
	"G-Buffer Copy", "Render Bolt", "Bloom", "Blend", "Frame"};

	for (int i = 0; i < numTimers; i++) {
		timers[i].Setup(timerNames[i], avgTimeInterval);
	}
}

void PerformanceManager::PerformanceGUI() {
	ImGui::Begin("Performance");

	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

	ImGui::Text("Vector Elements: %d", vectorNumElements);

	ImGui::Text("Point Lights: %d", GetNumActiveLights());

	ImGui::End();
}

// Timers
void PerformanceManager::TimersGUI() {
	ImGui::Begin("Timers");

	for (int i = 0; i < numTimers; i++) {
		ImGui::Separator();
		timers[i].GUI();
	}

	ImGui::End();
}

void PerformanceManager::StartTimer(TimerID id) {
	timers[id].Start();
}

void PerformanceManager::UpdateTimer(TimerID id) {
	timers[id].Update();
}

void PerformanceManager::SetTimerPerSecondOutput(TimerID id, bool outputPerSecond) {
	timers[id].SetPerSecond(outputPerSecond);
}

void PerformanceManager::SetTimerAvgOutput(TimerID id, bool outputAvg) {
	timers[id].SetAverage(outputAvg);
}

void PerformanceManager::SetTimerAvgTimeInterval(TimerID id, double avgTimeInterval) {
	timers[id].SetAvgTimeInterval(avgTimeInterval);
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