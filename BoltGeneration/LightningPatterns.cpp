#include "LightningPatterns.h"

// variables
int xVariation = 450;
int yVariation = 425;

std::shared_ptr<glm::vec3[numSegmentsInPattern]> GenerateLightningPattern(glm::vec3 startPnt) {
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> patternPtr = std::make_shared<glm::vec3[numSegmentsInPattern]>();

	patternPtr.get()[0] = ConvertWorldToScreen(startPnt);
	for (int i = 1; i < numSegmentsInPattern; i++) {
		//std::cout << "[" << prevPnt.x << "," << prevPnt.y << "]" << std::endl;
		startPnt = NextPoint(startPnt);
		patternPtr.get()[i] = ConvertWorldToScreen(startPnt);
	}

	return patternPtr;
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

void GenerateParticalSystemPattern(vec3 seed) {

	mat4 forwardTransform, inverseTransform = GetTransformsFromSeed(seed);

	// generate points
	// TODO
}

mat4 GetTransformsFromSeed(vec3 seed) {
	const vec3 u = seed;

	// translate to origin
	mat4 t1 = glm::translate(mat4(1), -u);
	mat4 t1i = glm::inverse(t1);

	// rotate about the x axis to lie in the xz plane (y = 0)
	float rxAngle = glm::dot(vec3(0, 0, u.z), vec3(0, u.y, u.z));
	mat4 rx = glm::rotate(mat4(1), glm::radians(rxAngle), vec3(1, 0, 0));
	mat4 rxi = glm::inverse(rx);

	// rotate about the y axis to lie along the z axis (x = 0)
	float ryAngle = glm::dot(vec3(0, 0, u.z), vec3(u.x, 0, u.z));
	mat4 ry = glm::rotate(mat4(1), glm::radians(ryAngle), vec3(0, 1, 0));
	mat4 ryi = glm::inverse(ry);

	// apply matricies together
	mat4 forwardTransform = t1 * rx * ry;
	mat4 inverseTransform = ryi * rxi * t1i;

	return forwardTransform, inverseTransform;
}

vec3 ParticalSystemNextPoint(vec3 point, mat4 ft, mat4 it) {
	// make rotation
	// TODO normal distribution for angles
	float rotation1 = glm::radians(13.4f);
	float rotation2 = glm::radians(18.9f);
	mat4 r = glm::rotate(mat4(1), rotation1, vec3(1, 0, 0));
	r = glm::rotate(r, rotation2, vec3(0, 1, 0));

	// add rotation to transform
	mat4 transform = ft * r * it;

	// apply to point
	vec3 newPos = transform * glm::vec4(point, 1);
	return newPos;
}

// proof of concept
/*
	const vec3 u = seed;

	mat4 t1 = glm::translate(mat4(1), -u);
	mat4 t1i = glm::inverse(t1);
	std::cout << "Seed: ";
	OutputVec3(u);
	std::cout << "T1: " << std::endl;
	OutputMat4(t1);

	// rotate about the x axis to lie in the xz plane (y = 0)
	float rxAngle = glm::dot(vec3(0, 0, u.z), vec3(0, u.y, u.z));
	mat4 rx = glm::rotate(mat4(1), glm::radians(rxAngle), vec3(1, 0, 0));
	mat4 rxi = glm::inverse(rx);
	std::cout << "RX: " << std::endl;
	OutputMat4(rx);

	// rotate about the y axis to lie along the z axis (x = 0)
	float ryAngle = glm::dot(vec3(0, 0, u.z), vec3(u.x, 0, u.z));
	mat4 ry = glm::rotate(mat4(1), glm::radians(ryAngle), vec3(0, 1, 0));
	mat4 ryi = glm::inverse(ry);
	std::cout << "RY: " << std::endl;
	OutputMat4(ry);

	// rotate by 16 degrees about the x and y axis for desired output
	mat4 r = glm::rotate(mat4(1), glm::radians(16.0f), vec3(1, 0, 0));
	r = glm::rotate(r, glm::radians(16.0f), vec3(0, 1, 0));

	// apply matricies together
	mat4 forwardTransform = t1 * rx * ry;
	mat4 inverseTransform = ryi * rxi * t1i;
	mat4 transform = forwardTransform * r * inverseTransform;

	vec3 newPos = transform * glm::vec4(u, 1);
	std::cout << "Forward Transform:" << std::endl;
	OutputMat4(forwardTransform);
	std::cout << "Inverse Transform:" << std::endl;
	OutputMat4(inverseTransform);
	std::cout << "New Position:" << std::endl;
	OutputVec3(newPos);*/