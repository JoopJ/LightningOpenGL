#include <iostream>
#include <random>
#include <functional>

#include <glm/glm.hpp>
#include "../include/imgui/imgui.h"

using glm::vec3;


#include "../BoltGeneration/Line.h"
#include "../FunctionLibrary.h"

// Naive Method Bolt generation Variables
int xVariation = 300;
int yVariationMax = 160;
int yVariationMin = 80;
int yMultiplyer = 2;
int zVariation = 500;

int numLines = 100;
int numLinesMax = 1000;
int linesArraySize = 2000;

int branchProbability = 0;

/// Random int generators for position variation
std::default_random_engine generator;
std::uniform_int_distribution<int> x(0, xVariation * 2);
std::uniform_int_distribution<int> y(yVariationMin, yVariationMax);
std::uniform_int_distribution<int> z(0, zVariation * 2);

auto rollx = std::bind(x, generator);
auto rolly = std::bind(y, generator);
auto rollz = std::bind(z, generator);


// Add a line to the lines array by setting their start and end points and createing a new branch randomly
vec3 DefineLine(std::shared_ptr<Line> linesPtr, vec3 startPos, int i, int* lineCountPtr) {

	vec3 point1 = startPos;
	vec3 point2 = point1;
	/**
		int dxo = rollx() - xVariation;
		int dx = (rand() % xVariation%2 + 1) - xVariation; // random number between -xVariation and xVariation
		int dyo = -rolly() * yMultiplyer;
		int dy = yVariationMin + (rand() % (yVariationMax - yVariationMin + 1));	// random number between yVariationMin and yVariationMax
		int dzo = rollz() - zVariation;
		int dz = (rand() % zVariation%2 + 1) - zVariation; // random number between -zVariation and zVariation
		**/
		// generate a random number between -xVariation and xVariation using rand
	int dx = (rand() % xVariation % 2 + 1) - xVariation; // random number between -xVariation and xVariation
	int dy = yVariationMin + (rand() % (yVariationMax - yVariationMin + 1));	// random number between yVariationMin and yVariationMax
	int dz = (rand() % zVariation % 2 + 1) - zVariation; // random number between -zVariation and zVariation

	std::cout << "X - Rand: " << dx << std::endl;
	std::cout << "Y - Rand: " << dy << std::endl;
	std::cout << "Z - Rand: " << dz << std::endl;


	point2.x += dx;
	point2.y += dy;
	point2.z += dz;

	linesPtr.get()[*lineCountPtr].Setup(ConvertWorldToScreen(point1), ConvertWorldToScreen(point2));
	linesPtr.get()[*lineCountPtr].SetColor(vec3(1, 1, 0));
	//std::cout << ConvertWorldToScreen(point1).x << "," << ConvertWorldToScreen(point1).y << "," << ConvertWorldToScreen(point1).z << std::endl;
	//std::cout << dx << "  " << dy << " " << dz << std::endl;
	point1 = point2;
	// each next line has start equal to the previous

	// roll for a branch
	int num = rand() % 100;
	if (num > (100 - branchProbability)) {
		vec3 branchPoint2 = point2;
		for (int j = i; j < numLines; j++) {
			*lineCountPtr = *lineCountPtr + 1;
			// create a new branch
			branchPoint2 = DefineLine(linesPtr, branchPoint2, j, lineCountPtr);
		}

	}
	return point2;
}

// Returns the pointer to the array of lines that form the lightning
int DefineLightningLines(vec3 startPos, std::shared_ptr<Line> linesPtr) {

	int lineCount = 0;
	int* lineCountPtr = &lineCount;

	vec3 point1 = startPos;

	// make all the lines
	for (int i = 0; i < numLines; i++) {
		lineCount += 1;
		point1 = DefineLine(linesPtr, point1, i, lineCountPtr);
	}

	return lineCount;
}

void GUINaiveApproach() {
	// Lightning Generation window
	ImGui::Text("Line generation variables:");
	ImGui::SliderInt("xVariation", &xVariation, 1, 1000);
	ImGui::SliderInt("yVariationMax", &yVariationMax, 1, 500);
	ImGui::SliderInt("yVariationMin", &yVariationMin, 1, 500);
	ImGui::SliderInt("zVariation", &zVariation, 1, 1000);
	ImGui::SliderInt("numLines", &numLines, 50, numLinesMax);
	ImGui::SliderInt("Branch Probability", &branchProbability, 0, 100);
	ImGui::End();
}