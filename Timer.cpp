#include "Timer.h"

// constructor
Timer::Timer() {
	name = "Undefined";
	avgTimeInterval = -1;
}

Timer::Timer(const char* name, double avgTime) {
	Setup(name, avgTime);
}

void Timer::Setup(const char* _name, double avgTime) {
	name = _name;
	avgTimeInterval = avgTime;
}

// The time frame you wish to measure is controlled by calling
// Start() and Update(). The time inbetween those two calls is measured.

void Timer::Start() {
	perSecondStartTime = glfwGetTime();
	avgStartTime = glfwGetTime();
}

void Timer::Update() {
	double currentTime = glfwGetTime();
	double deltaTime = currentTime - prevTime;
	prevTime = currentTime;

	if (perSecond) {
		perSecondFrameCount++;
		perSecondTime += deltaTime;
		perSecondTimeSum += currentTime - perSecondStartTime;

		if (perSecondTime >= 1) {
			PerSecondOutput();
		}
	}

	if (average) {
		avgFrameCount++;
		avgTime += deltaTime;
		avgTimeSum += currentTime - avgStartTime;

		if (avgTime >= avgTimeInterval) {
			AvgOutput();
		}
	}
}

void Timer::PerSecondOutput() {
	// calculate avg speed in milliseconds
	perSecondMs = perSecondTimeSum / double(perSecondFrameCount);
	std::cout << std::endl << name << " ms: " << perSecondMs << std::endl;

	perSecondFrameCount = 0;
	perSecondTimeSum = 0;
	perSecondTime = 0;
}

void Timer::AvgOutput() {
	// calculate avg speed in milliseconds
	avgMs = avgTimeSum / double(avgFrameCount);

	std::cout << std::endl << name << " ms: " << avgMs << std::endl;

	avgFrameCount = 0;
	avgTimeSum = 0;
	avgTime = 0;
}

void Timer::GUI() {
	if (ImGui::CollapsingHeader(name)) {
		ImGui::Checkbox("Per Second", &perSecond);
		ImGui::Checkbox("Average", &average);

		if (perSecond)
			ImGui::Text("%.2f ms", perSecondMs);
		if (average)
			ImGui::Text("%.2f ms", avgMs);
	}
}

void Timer::SetPerSecond(bool set) {
	perSecond = set;
}

void Timer::SetAverage(bool set) {
	average = set;
}

void Timer::SetAvgTimeInterval(double val) {
	avgTimeInterval = val;
}

