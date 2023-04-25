#include "Testing.h"

void RunNumSegs(int numSegs, int count);
void RunDetail(int detail, int count);
void RunLSystem(int count, vector<pair<vec3, vec3>>* patternPtr);
void RunPSystem(int count, vector<pair<vec3, vec3>>* patternPtr);
void RunRandom(int count, vector<pair<vec3, vec3>>* patternPtr);

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

void BeginTesting() {

	// number of times to run
	int count = 1000;

	int detail = 15;
	// 1 - 15 . Detail
	std::cout << "LSystem" << std::endl;
	for (int i = 1; i < detail+1; i++) {
		RunDetail(i, count);
	}

	int segs = 1000;
	// 100 - 1000 . Segs
	std::cout << "Partical" << std::endl << "Random" << std::endl;
	for (int i = 100; i < segs+1; i += 100) {
		RunNumSegs(i, count);
	}
}

void RunNumSegs(int numSegs, int count) {
	vector<pair<vec3, vec3>> pattern;
	vector<pair<vec3, vec3>>* patternPtr;
	patternPtr = &pattern;

	SetNumSegments(numSegs);

	std::cout << std::endl << numSegs << std::endl;

	RunPSystem(count, patternPtr);
	RunRandom(count, patternPtr);
}

void RunDetail(int detail, int count) {
	vector<pair<vec3, vec3>> pattern;
	vector<pair<vec3, vec3>>* patternPtr;
	patternPtr = &pattern;

	SetLSystemOptions(vec3(0.0f, 0.0f, 0.0f), detail, 0.0f);

	std::cout << std::endl << detail << std::endl;

	RunLSystem(count, patternPtr);
}

void RunLSystem(int count, vector<pair<vec3, vec3>>* patternPtr) {
	double sum = 0.0;

	for (int i = 0; i < count; i++) {

		auto t1 = high_resolution_clock::now();
		GenerateLSystemPattern(vec3(0.0f, 90.0f, 0.0f), patternPtr, true);
		auto t2 = high_resolution_clock::now();

		duration<double, std::milli> ms_double = t2 - t1;

		sum += ms_double.count();
	}

	std::cout << sum / double(count) << " ms" << std::endl;
}

void RunPSystem(int count, vector<pair<vec3, vec3>>* patternPtr) {
	double sum = 0.0;

	for (int i = 0; i < count; i++) {

		auto t1 = high_resolution_clock::now();
		GenerateParticleSystemPattern(vec3(0.0f, 90.0f, 0.0f), vec3(23.0f, -100.0f, -9.0f), patternPtr);
		auto t2 = high_resolution_clock::now();

		duration<double, std::milli> ms_double = t2 - t1;

		sum += ms_double.count();
	}

	std::cout << sum / double(count) << " ms" << std::endl;
}

void RunRandom(int count, vector<pair<vec3, vec3>>* patternPtr) {
	double sum = 0.0;

	for (int i = 0; i < count; i++) {

		auto t1 = high_resolution_clock::now();
		GenerateRandomPositionsPattern(vec3(0.0f, 90.0f, 0.0f), patternPtr);
		auto t2 = high_resolution_clock::now();

		duration<double, std::milli> ms_double = t2 - t1;

		sum += ms_double.count();
	}

	std::cout << sum / double(count) << " ms" << std::endl;
}