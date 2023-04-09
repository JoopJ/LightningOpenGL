#include "performance.h"

// constructor
Performance::Performance(const char* name, double avgTime) {
	this->name = name;
	this->avgTime = avgTime;
}

void Performance::Start() {
	lastTime = glfwGetTime();
	lastTimeAvg = glfwGetTime();
}

// called at the end of the time frame you wish to measure
void Performance::Update() {
	currentTime = glfwGetTime();

	if (outputPerSecond) {
		//std::cout << (currentTime - lastTime) << std::endl;
		PerSecond();
	}
	if (outputAverage) {
		//std::cout << (currentTime - lastTimeAvg) << std::endl;
		Avg();
	}

}

void Performance::PerSecond() {
	numFrames++;
	if (currentTime - lastTime >= 1.0) {
		std::cout << std::endl << name << std::endl;
		std::cout << "FPS: " << double(numFrames) / (currentTime - lastTime) << std::endl;
		std::cout << "ms/Frame : " << 1000.0 / double(numFrames) << std::endl;
		numFrames = 0;
		lastTime += 1.0;
	}
}

void Performance::Avg() {
	numFramesAvg++;
	if (currentTime - lastTimeAvg >= avgTime) {
		std::cout << std::endl << "---------------------" << std::endl;
		std::cout << name << "Averge of " << avgTime << " seconds" << std::endl;
		std::cout << "Avg FPS: " << double(numFramesAvg) / (currentTime - lastTimeAvg) << std::endl;
		std::cout << "Avg ms/Frame : " << 1000.0 / double(numFramesAvg) << std::endl;
		std::cout << std::endl << "---------------------" << std::endl;
		numFramesAvg = 0;
		lastTimeAvg += avgTime;
	}
}

void Performance::SetPerSecondOutput(bool set) {
	outputPerSecond = set;
}

void Performance::SetAverageOutput(bool set) {
	outputAverage = set;
}

