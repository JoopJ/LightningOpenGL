#include "LightningPatterns.h"

// variables
int xVariation = 4;
int yVariation = 4;
float multiplyer = 1.25f;

float length = 1.5f;

// Pattern Generation Functions
// -------------------------
// Random Positions --------
// STATIC
std::shared_ptr<vec3[numSegmentsInPattern]> GenerateRandomPositionsLightningPattern(vec3 start,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	patternPtr.get()[0] = ConvertWorldToScreen(start);
	for (int i = 1; i < numSegmentsInPattern; i++) {
		start = NextPoint(start);
		patternPtr.get()[i] = ConvertWorldToScreen(start);
	}

	return patternPtr;
}

// DYNAMIC BOLT
vector<pair<vec3, vec3>>* GenerateRandomPositionsLightningPattern(vec3 start,
	vector<pair<vec3, vec3>>* patternPtr) {
	// clear the pattern
	patternPtr->clear();

	vec3 end;
	for (int i = 0; i < numSegmentsInPattern; i++) {
		end = NextPoint(start);
		patternPtr->push_back({ ConvertWorldToScreen(start), ConvertWorldToScreen(end) });
		start = end;
	}

	return patternPtr;
}

// Particle System ---------
// STATIC BOLT 
std::shared_ptr<vec3[numSegmentsInPattern]> GenerateParticleSystemPattern(vec3 start, vec3 seed,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	seed = normalize(seed);
	// get the axis to rotate around
	pair<vec3, vec3> seedPerpAxis = GetRotationAxis(seed);

	vec3 prevEnd = start;
	vec3 newPoint = start + seed * length;

	// add the first segment to the pattern
	patternPtr.get()[0] = ConvertWorldToScreen(start);
	for (int i = 1; i < numSegmentsInPattern; i++) {
		// add the new segment to the pattern
		patternPtr.get()[i] = ConvertWorldToScreen(newPoint);

		prevEnd = newPoint;

		// Get next point:
		// move the point along the seed vector
		vec3 newPointMove = seed * length;
		// rotate the point with respect to seed's perpendicular axis
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		newPoint = prevEnd + newPointMove;
	}

	return patternPtr;

}

// DYNAMIC BOLT
vector<pair<vec3, vec3>>* GenerateParticleSystemPattern(vec3 start, vec3 seed,
	vector<pair<vec3, vec3>>* patternPtr) {
	// clear the pattern
	patternPtr->clear();

	seed = normalize(seed);
	// get the axis to rotate around
	pair<vec3, vec3> seedPerpAxis = GetRotationAxis(seed);

	vec3 prevEnd = start;
	vec3 newPoint = start + seed * length;

	for (int i = 0; i < numSegmentsInPattern; i++) {
		// add the new segment to the pattern
		patternPtr->push_back({ ConvertWorldToScreen(prevEnd), ConvertWorldToScreen(newPoint) });

		prevEnd = newPoint;

		// get the transform for the next point
		vec3 newPointMove = seed * length;
		// roate with respect to the seed
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		newPoint = prevEnd + newPointMove;
	}

	return patternPtr;
}

// L-System ----------------
// STATIC
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vec3 start, vec3 end, vector<pair<vec3, vec3>>* patternPtr, int detail, int size) {
	// add start and end to the pattern
	float maxDisplacement = 10;
	

	for (int i = 0; i < detail; i++) {
		// get the midpoint
		vec3 mid = (start + end) / 2.0f;

		// get perpendicular axis
		vec3 perpAxis;

		// move point along perp axis by a random amount
		float displacement = (rand() % (int)maxDisplacement) / 100.0f;
		mid = mid + perpAxis * displacement;

 		// add the midpoint to the pattern

		// get the new start and end
		start = mid;
		end = mid;

		maxDisplacement *= 0.5f;
	}

	//


	return patternPtr;
}

// DYNAMIC


// -------------------------

// Helper Functions
// --------------------------

// General:

// Random Positions:
glm::vec3 NextPoint(glm::vec3 point) {
	// ger random variatins
	int dx = (rand() % xVariation * 2 + 1) - xVariation;
	int dy = (rand() % yVariation + 1);
	int dz = (rand() % xVariation * 2 + 1) - xVariation;

	point.x += dx * multiplyer;
	point.y -= dy * multiplyer;
	point.z += dz * multiplyer;

	return point;
}

// Particle System:
quaternion ConvertRotationQuaternion(vec3 seed, float angle) {
	// rotation quaternion
	quaternion qr = quaternion(cos(angle / 2), seed.x * sin(angle / 2), seed.y * sin(angle / 2), seed.z * sin(angle / 2));
	return qr;
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
	quaternion final = q2 * q1 * p * q1.makeInverse() * q2.makeInverse();

	// extract new point
	vec3 newPoint = vec3(final.X, final.Y, final.Z);
	return newPoint;
}
// -----------------------

// GUI
// method: 0 - Random, 1 - Particle, 2 - L-System
void BoltGenerationGUI(int method) {
	ImGui::Begin("Bolt Generation");
	ImGui::Text("Bolt Generation");
	ImGui::Separator();
	switch (method) {
	case 0:
		ImGui::Text("Random");
		ImGui::Text("Segment Position Variation");
		ImGui::SliderInt("X Variation", &xVariation, 2, 10);
		ImGui::SliderInt("Y Variation", &yVariation, 2, 10);
		ImGui::SliderFloat("Multiplyer", &multiplyer, 0.1, 2);
		break;
	case 1: 
		ImGui::Text("Particle System");
		ImGui::Text("Segment Length");
		ImGui::SliderFloat("Length", &length, 0.1f, 2.0f);
		break;
	case 2:
		ImGui::Text("L-System");
		break;
	}
	ImGui::Separator();
	ImGui::End();
}