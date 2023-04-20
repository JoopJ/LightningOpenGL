#include "PerformanceManager.h"

PerformanceManager::PerformanceManager() {
	// Pattern Info
	vectorNumElements = 0;
	vectorCapacity = 0;
	vectorSizeBytes = 0;
	arrayNumElements = 0;
	arrayCapacity = 0;
	arraySizeBytes = 0;


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