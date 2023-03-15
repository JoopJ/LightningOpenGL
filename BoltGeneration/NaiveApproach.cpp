#include <iostream>
#include <random>
#include <functional>

#include <glm/glm.hpp>
#include "../include/imgui/imgui.h"

using glm::vec3;


#include "../BoltGeneration/Line.h"
#include "../FunctionLibrary.h"

// Naive Method Bolt generation Variables
int horizontalVariation = 450;
int verticalVariation = 425;

int linesToDefine = 100;
int linesArraySize;

int branchProbability = 0;
int numBranches = 0;

// Add a line to the lines array by setting their start and end points and createing a new branch randomly
vec3 DefineLine(std::shared_ptr<Line> linesPtr, vec3 startPos, int i, int* lineCountPtr) {

	vec3 endPos = startPos;

	int dx = (rand() % horizontalVariation * 2 + 1) - horizontalVariation;
	int dz = (rand() % horizontalVariation * 2 + 1) - horizontalVariation;
	int dvertical = (rand() % verticalVariation + 1);
	//int dvertical = verticalVariationMin + (rand() % (verticalVariationMax - verticalVariationMin + 1));

	endPos.x += dx;
	endPos.y -= dvertical;
	endPos.z += dz;

	linesPtr.get()[*lineCountPtr].Setup(ConvertWorldToScreen(startPos), ConvertWorldToScreen(endPos));
	linesPtr.get()[*lineCountPtr].SetColor(vec3(1, 1, 0));

	// roll for a branch
	if (rand() % 100 > (100 - branchProbability)) {
		// create a new branch and all the lines that follow from it
		vec3 branchPos = endPos;
		numBranches++;
		for (int j = i; j < linesToDefine && (*lineCountPtr) < linesArraySize; j++) {
			(*lineCountPtr)++;
			branchPos = DefineLine(linesPtr, branchPos, j, lineCountPtr);
		}

	}
	return endPos;
}

// Generates the lightning pattern in an array of lines, returns the number of Line objs in the array
int DefineBoltLinesNA(vec3 startPos, std::shared_ptr<Line> linesPtr) {
	std::cout << "Lines array size: " << linesArraySize << std::endl;
	int lineCount = 0;
	int* lineCountPtr = &lineCount;

	vec3 point1 = startPos;

	// make all the lines
	for (int i = 0; i < linesToDefine; i++) {
		(*lineCountPtr)++;
		point1 = DefineLine(linesPtr, point1, i, lineCountPtr);

		if (*lineCountPtr == linesArraySize) {
			std::cout << "Array full" << std::endl;
			// stop defining lines;
			return lineCount;
		}
	}
	std::cout << "Line Count: " << lineCount << " - " << *lineCountPtr << std::endl;
	return lineCount;
}

void GUINaiveApproach() {
	// Lightning Generation window
	ImGui::Text("Line generation variables:");
	ImGui::SliderInt("Horizontal Variation", &horizontalVariation, 1, 1000);
	ImGui::SliderInt("Vertical Variation", &verticalVariation, 1, 500);
	ImGui::SliderInt("numLines", &linesToDefine, 50, linesToDefine);
	ImGui::SliderInt("Branch Probability", &branchProbability, 0, 10);
	ImGui::End();
}

void SetLineArraySize(int lineArraySize) {
	linesArraySize = lineArraySize - 1;	// -1 because we need to check if the next line will go over the array size
}