#include "LightningPatterns.h"

// variables
int xVariation = 450;
int yVariation = 425;

std::shared_ptr<glm::vec3[numLinesInPattern]> GenerateLightningPattern(glm::vec3 startPnt) {
	std::shared_ptr<glm::vec3[numLinesInPattern]> lightningPattern = std::make_shared<glm::vec3[numLinesInPattern]>();

	lightningPattern.get()[0] = ConvertWorldToScreen(startPnt);
	for (int i = 1; i < numLinesInPattern; i++) {
		//std::cout << "[" << prevPnt.x << "," << prevPnt.y << "]" << std::endl;
		startPnt = NextPoint(startPnt);
		lightningPattern.get()[i] = ConvertWorldToScreen(startPnt);
	}
	return lightningPattern;
}

glm::vec3 NextPoint(glm::vec3 point) {
	// ger random variatins
	int dx = (rand() % xVariation * 2 + 1) - xVariation;
	int dy = (rand() % yVariation + 1);
	int dz = (rand() % xVariation * 2 + 1) - xVariation;

	point.x += dx;
	point.y -= dy;
	point.z += dz;

	return point;
}