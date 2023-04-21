#pragma once

#include <GLFW/glfw3.h>
#include "include/imgui/imgui.h"
#include <iostream>

class Timer {
private:
	void PerSecondOutput();
	void AvgOutput();

	// Timer Options
	const char* name = "Undefined";
	double avgTimeInterval;	// time to take average
	double currentTime;

	// Measurement Toggles
	bool perSecond = false;
	bool average = false;

	// Per second variables
	double lastTime = 0;
	int numFrames = 0;

	// Avr variables
	double lastTimeAvg = 0;
	int numFramesAvg = 0;

	// Times
	double prevTime = 0;

	// time - records actual time, used to keep track of when to output.
	// timeSum - the sum of the time between Start() and Update().
	// startTime - the time when Start() is called.
	// frameCount - the number of times Update() is called.

	// Per Second
	double perSecondTime = 0;
	double perSecondTimeSum = 0;
	double perSecondStartTime = 0;
	int perSecondFrameCount = 0;

	double perSecondMs = 0;

	// Average
	double avgTime = 0;
	double avgTimeSum = 0;
	double avgStartTime = 0;
	int avgFrameCount = 0;

	double avgMs = 0;

public:
	Timer();
	Timer(const char* name, double avgTime);
	void Setup(const char* name, double avgTime);

	void GUI();

	void Start();
	void Update();
	void SetPerSecond(bool set);
	void SetAverage(bool set);
	void SetAvgTimeInterval(double val);
};