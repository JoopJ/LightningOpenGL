#pragma once

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <iostream>
#include <chrono>

using namespace std::chrono;

class Timer {
private:
	// Timer Options
	const char* name;
	// ChronoOnce: False - average over multiple frames, True - one frame
	bool chronoOnce;

	// Chrono
	double chronoTimeSum = 0;
	int chronoFrameCount = 0;
	int chronoFrameTarget = 0;

	double avgChrono = 0;

	void UpdateChrono(double time);
	void UpdateChronoOnce(double time);

	// Output Results
	double sum = 0;
	int count = 0;
	int target = 100;
	bool outputResults = false;

public:
	Timer();
	Timer(const char* namee, int chronoTarget);
	void Setup(const char* name, int chronoTarget);

	void GUI();

	void Update(time_point<high_resolution_clock> t1, 
		time_point<high_resolution_clock> t2);
	void SetChronoOnce(bool set);
	void SetChronoFrameTarget(int val);
	void SetOutputResults(bool set);

	void Info();
};