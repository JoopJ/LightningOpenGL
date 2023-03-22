#pragma once

#include <GLFW/glfw3.h>
#include <iostream>

class Performance {
	void PerSecond();
	void Avg();
	// Performance Options
	const char* name = "Undefined";
	double avgTime;	// time to take average
	double currentTime;
	// Options of what to output
	bool outputPerSecond = true;
	bool outputAverage = true;
	// Per second variables
	double lastTime;
	int numFrames = 0;
	// Avr variables
	double lastTimeAvg;
	int numFramesAvg = 0;
	
	
public:
	Performance(const char* name, double avgTime);
	void Start();
	void Update();
	void SetPerSecondOutput(bool set);
	void SetAverageOutput(bool set);
};