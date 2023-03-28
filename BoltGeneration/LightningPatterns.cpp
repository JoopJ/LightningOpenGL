#include "LightningPatterns.h"

// variables
int xVariation = 450;
int yVariation = 425;

std::shared_ptr<glm::vec3[numSegmentsInPattern]> GenerateRandomPositionsLightningPattern(glm::vec3 startPnt) {
	std::shared_ptr<glm::vec3[numSegmentsInPattern]> patternPtr = std::make_shared<glm::vec3[numSegmentsInPattern]>();

	patternPtr.get()[0] = ConvertWorldToScreen(startPnt);
	for (int i = 1; i < numSegmentsInPattern; i++) {
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

// Returns a pair of perpendicular axis to the seed.
// The seed acts as the y axis and the two returned axis
// will act as the x and z axis.
// Seed should be normalized
pair<vec3, vec3> GetRotationAxis(vec3 seed) {
	// get arbitrary axis along yz plane of the seed
	vec3 arb = vec3(seed.x, -seed.z, seed.y);

	// get 2 perpendicular vectors to the seed
	vec3 perp1 = cross(seed, arb);
	vec3 perp2 = cross(seed, perp1);

	return { perp1, perp2 };
}

vector<pair<vec3, vec3>> GenerateParticalSystemPattern(vec3 start, vec3 seed) {
	// dynamic vector of segments
	vector<pair<vec3, vec3>> pattern;

	float length = 4; // TODO random length for each segment
	seed = normalize(seed);
	// get the axis to rotate around
	pair<vec3, vec3> seedPerpAxis = GetRotationAxis(seed);

	vec3 prevEnd = start;
	vec3 newPoint = start + seed * length;

	for (int i = 0; i < 100; i++) {
		// add the new segment to the pattern
		pattern.push_back({ ConvertWorldToScreen(prevEnd), ConvertWorldToScreen(newPoint) });

		prevEnd = newPoint;

		// get the transform for the next point
		vec3 newPointMove = seed * length;
		// roate with respect to the seed
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		newPoint = prevEnd + newPointMove;
	}

	return pattern;
}

quaternion ConvertRotationQuaternion(vec3 seed, float angle) {
	// rotation quaternion
	quaternion qr = quaternion(cos(angle / 2), seed.x * sin(angle / 2), seed.y * sin(angle / 2), seed.z * sin(angle / 2));
	return qr;
}

vec3 RotatePointAboutSeed(vec3 point, pair<vec3, vec3> seedPerpAxis) {
	// get rotation values
	float degree1 = (rand() % 32) - 16;
	float degree2 = (rand() % 32) - 16;
	float r1 = glm::radians(degree1);	// TODO normal distribution for angles
	float r2 = glm::radians(degree2);

	// set rotation quaternions
	quaternion qr1 = ConvertRotationQuaternion(seedPerpAxis.first, r1);
	quaternion qr2 = ConvertRotationQuaternion(seedPerpAxis.second, r2);

	// convert point to quaternion
	quaternion p = quaternion(point.x, point.y, point.z, 0);

	// apply rotation
	quaternion q1 = qr1 * p * qr1.makeInverse();
	quaternion q2 = qr2 * p * qr2.makeInverse();
	quaternion final = q2*q1 * p * q1.makeInverse() * q2.makeInverse();

	// extract new point
	vec3 newPoint = vec3(final.X, final.Y, final.Z);
	return newPoint;
}