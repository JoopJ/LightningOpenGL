#include "Timer.h"

// constructor
Timer::Timer() {
	name = "Undefined";
	chronoOnce = false;
}

Timer::Timer(const char* name, int chronoTarget) {
	Setup(name, chronoTarget);
}

void Timer::Setup(const char* _name, int chronoTarget) {
	name = _name;
	chronoFrameTarget = chronoTarget;
}

void Timer::Update(time_point<high_resolution_clock> t1) {
	
	duration<double, std::milli> ms = high_resolution_clock::now() - t1;
	if (!chronoOnce) {
		// Average over multiple frames
		UpdateChrono(ms.count());
	}
	else {
		// Single frame
		UpdateChronoOnce(ms.count());
	}
}

// Take a chrono time point at the start of the time you want to measure
//  and call this update at the end of the time you want to measure.
void Timer::UpdateChrono(double time) {

	chronoTimeSum += time;
	chronoFrameCount++;

	if (chronoFrameCount >= chronoFrameTarget) {
		avgChrono = chronoTimeSum / chronoFrameCount;
		
		chronoTimeSum = 0;
		chronoFrameCount = 0;
	}
}

// Same as UpdateChrono but dosen't take average over multiple frames
void Timer::UpdateChronoOnce(double time) {

	avgChrono = time;
}

void Timer::GUI() {
	ImGui::Text(name);
	ImGui::Text("%.3f ms", float(avgChrono));
}

void Timer::SetChronoOnce(bool set) {
	chronoOnce = set;
}

void Timer::SetChronoFrameTarget(int val) {
	chronoFrameTarget = val;
}
