#include "LightningPatterns.h"

// Variables
// Random
int xVariation = 4;
int yVariation = 4;
float multiplyer = 1.25f;

// Particle
float length = 1.5f;

// L-System
float startingMaxDisplacement = 30;
int LSystemDetail = 3;
vec3 LSystemEndPosition = vec3(0,-20,0);

// Branching
bool branching = true;
int minBranchLength = 5;
int maxBranchLength = 50;

int randomPositionsBranchChance = 5;
int particleSystemBranchChance = 5;
int LSystemBranchChance = 5;

// --------------------------------------------------
// Private Functions --------------------------------

// General:
quaternion ConvertRotationQuaternion(vec3 seed, float angle) {
	// Returns a quaternion that rotates about the given seed vector
	// by the given angle (in radians).

	return quaternion(cos(angle / 2), seed.x * sin(angle / 2), seed.y * sin(angle / 2), seed.z * sin(angle / 2));
}
bool RollBranchChance(int branchChance) {
	// Given an int in the range [0, 100], rolls a random number between 0 and 100
	// and if the random number is less than the given int, returns true.
	// Effectively a chance roll of: [branchChance]% probability.

	int roll = rand() % 100;
	return roll < branchChance;
}
int BranchLength() {
	// TODO: make normal distribution instead of rand.
	// Return random number between minBranchLength and maxBranchLength

	return (rand() % (maxBranchLength - minBranchLength) + minBranchLength);
}

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
pair<vec3, vec3> GetRotationAxis(vec3 seed) {
	// Returns a pair of perpendicular axis to the seed.
	// The seed acts as the y axis and the two returned axis
	// will act as the x and z axis.
	// Seed should be normalized

	// get arbitrary axis along yz plane of the seed
	vec3 arb = vec3(seed.x, -seed.z, seed.y);

	// get 2 perpendicular vectors to the seed
	vec3 perp1 = cross(seed, arb);
	//vec3 perp2 = cross(seed, perp1);

	return { perp1, cross(seed, perp1) };
}
vec3 RotatePointAboutSeed(vec3 point, pair<vec3, vec3> seedPerpAxis) {
	// get rotation values
	// TODO normal distribution for angles
	float degree1 = float((rand() % 32) - 16);
	float degree2 = float((rand() % 32) - 16);
	float r1 = glm::radians(degree1);
	float r2 = glm::radians(degree2);

	// set rotation quaternions
	quaternion qr1 = ConvertRotationQuaternion(seedPerpAxis.first, r1);
	quaternion qr2 = ConvertRotationQuaternion(seedPerpAxis.second, r2);

	// convert point to quaternion
	quaternion p = quaternion(point.x, point.y, point.z, 0);

	// apply rotations
	quaternion q1 = qr1 * p * qr1.makeInverse();
	quaternion q2 = qr2 * p * qr2.makeInverse();
	quaternion final = q2 * q1 * p * q1.makeInverse() * q2.makeInverse();

	// extract new point
	vec3 newPoint = vec3(final.X, final.Y, final.Z);
	return newPoint;
}

// L-System:
vec3 GetPerpAxis(vec3 axis) {
	// Returns a perpendicular vector to the given axis.
	// the perp vector is rotated by a random angle around
	// the original axis.

	// get perpendicular axis
	vec3 perp = normalize(cross(axis, vec3(axis.x, axis.z, axis.y)));

	// get random angle
	float radian = glm::radians((float)(rand() % 360));

	// convert perp to quaternion
	quaternion p = quaternion(perp.x, perp.y, perp.z, 0);

	// create rotation quaternion
	quaternion r = ConvertRotationQuaternion(axis, radian);

	// apply rotation
	p = r * p * r.makeInverse();

	return normalize(vec3(p.X, p.Y, p.Z));
}
vec3 GetMidPnt(vec3 start, vec3 end, int maxDisplacement) {
	vec3 mid = (start + end) / 2.0f;

	// get perpendicular axis
	vec3 perp = GetPerpAxis(end - start);

	// displace mid point along the perpendicular axis by
	// a random magnitude between 0 and maxDisplacement.
	mid += perp * (float(rand()) / float((RAND_MAX)) * maxDisplacement);

	return mid;
}

//STATIC
void LSystemSubDivide(vec3 start, vec3 end, int startIndex, int endIndex, int detail,
	float maxDisplacement, std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	// calculate mid and add to pattern
	vec3 mid = GetMidPnt(start, end, maxDisplacement);
	int midIndex = (startIndex + endIndex) / 2;

	patternPtr[midIndex] = ConvertWorldToScreen(mid);

	detail -= 1;
	maxDisplacement /= 2;

	if (detail > 0) {
		// iterate on left side (start to mid)
		LSystemSubDivide(start, mid, startIndex, midIndex, detail, maxDisplacement, patternPtr);
		// iterate on right side (mid to end)
		LSystemSubDivide(mid, end, midIndex, endIndex, detail, maxDisplacement, patternPtr);
	}
}
//DYNAMIC
void LSystemSubDivide(vec3 start, vec3 end, int startIndex, int endIndex, int detail,
	float maxDisplacement, vector<vec3>* patternPtr) {

	// calculate mid and add to pattern
	vec3 mid = GetMidPnt(start, end, maxDisplacement);
	int midIndex = (startIndex + endIndex) / 2;

	patternPtr->at(midIndex) = ConvertWorldToScreen(mid);

	detail -= 1;
	maxDisplacement /= 2;

	if (detail > 0) {
		// iterate on left side (start to mid)
		LSystemSubDivide(start, mid, startIndex, midIndex, detail, maxDisplacement, patternPtr);
		// iterate on right side (mid to end)
		LSystemSubDivide(mid, end, midIndex, endIndex, detail, maxDisplacement, patternPtr);
	}
}

// Branching:
void RandomPositionsBranch(vec3 start, int size, vector<pair<vec3, vec3>>* patternPtr) {

	vec3 end;
	for (int i = 0; i < size; i++) {
		end = NextPoint(start);
		patternPtr->push_back({ ConvertWorldToScreen(start), ConvertWorldToScreen(end) });
		start = end;
	}
}
void ParticleSystemBranch(vec3 start, vec3 seed, int size, vector<pair<vec3, vec3>>* patternPtr) {
	// The seed used is the previous segment. This will prevent the bolts from
	// branching in the same direction asthe main bolt (and other branches).

	seed = normalize(seed);
	// get the axis to rotate around
	pair<vec3, vec3> seedPerpAxis = GetRotationAxis(seed);

	vec3 prevEnd = start;
	vec3 newPoint = start + seed * length;

	for (int i = 0; i < size; i++) {
		// add the new segment to the pattern
		patternPtr->push_back({ ConvertWorldToScreen(prevEnd), ConvertWorldToScreen(newPoint) });

		prevEnd = newPoint;

		// get the transfrom for the next point
		vec3 newPointMove = seed * length;
		// rotate with respect to the seed
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		newPoint = prevEnd + newPointMove;
	}
}
// --------------------------------------------------

// Public Functions ---------------------------------

// Random Positions --------
//STATIC BOLT
int GenerateRandomPositionsPattern(vec3 start,
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	patternPtr.get()[0] = ConvertWorldToScreen(start);
	for (int i = 1; i < numSegmentsInPattern; i++) {
		start = NextPoint(start);
		patternPtr.get()[i] = ConvertWorldToScreen(start);
	}
	// will always have the same size
	return numSegmentsInPattern;
}
//DYNAMIC BOLT
vector<pair<vec3, vec3>>* GenerateRandomPositionsPattern(vec3 start,
	vector<pair<vec3, vec3>>* patternPtr) {
	// clear the pattern
	patternPtr->clear();

	vec3 end;
	for (int i = 0; i < numSegmentsInPattern; i++) {
		end = NextPoint(start);
		patternPtr->push_back({ ConvertWorldToScreen(start), ConvertWorldToScreen(end) });

		// Branch
		if (branching && RollBranchChance(randomPositionsBranchChance)) {
			RandomPositionsBranch(end, BranchLength(), patternPtr);
		}

		start = end;
	}

	return patternPtr;
}
// -------------------------

// Particle System ---------
//STATIC BOLT 
int GenerateParticleSystemPattern(vec3 start, vec3 seed,
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
	// will always have the same size
	return numSegmentsInPattern;

}
//DYNAMIC BOLT
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

		// Branch
		if (branching && RollBranchChance(particleSystemBranchChance)) {
			ParticleSystemBranch(newPoint, (newPoint-prevEnd), BranchLength(), patternPtr);
		}

		prevEnd = newPoint;

		// get the transform for the next point
		vec3 newPointMove = seed * length;
		// roate with respect to the seed
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		newPoint = prevEnd + newPointMove;
	}

	return patternPtr;
}
// -------------------------

// L-System ----------------
//STATIC BOLT
int GenerateLSystemPattern(vec3 start, 
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	int size = int(pow(2, LSystemDetail)) + 1;
	// check if the size is too big for array
	if (size > numSegmentsInPattern) {
		std::cout << "WARNING::L-SYSTEM::PATTERN SIZE EXCEEDS MAX PATTERN SIZE" << std::endl;
		return 0;
	}

	int startIndex = 0;
	int endIndex = size - 1;

	patternPtr[startIndex] = ConvertWorldToScreen(start);
	patternPtr[endIndex] = ConvertWorldToScreen(LSystemEndPosition);

	LSystemSubDivide(start, LSystemEndPosition, startIndex, endIndex, 
		LSystemDetail, startingMaxDisplacement, patternPtr);

	// variable size
	return size;
}
//DYNAMIC BOLT
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vec3 start,
	vector<pair<vec3, vec3>>* patternPtr) {

	const int size = int(pow(2, LSystemDetail)) + 1;

	// stores the points of the pattern
	vector<vec3> patternPoints;
	patternPoints.resize(size);
	vector<vec3>* patternPointsPtr = &patternPoints;

	int startIndex = 0;
	int endIndex = size - 1;

	patternPointsPtr->at(startIndex) = ConvertWorldToScreen(start);
	patternPointsPtr->at(endIndex) = ConvertWorldToScreen(LSystemEndPosition);

	LSystemSubDivide(start, LSystemEndPosition, startIndex, endIndex, 
		LSystemDetail, (float)startingMaxDisplacement, patternPointsPtr);

	// add the points to the pattern as pairs
	// TODO : Handling branching will go here.
	patternPtr->clear();
	patternPtr->resize(size);
	for (int i = 0; i < size-1; i++) {
		pair<vec3, vec3> seg = { patternPoints.at(i), patternPoints.at(i + 1) };
		patternPtr->at(i) = seg;
	}

	return patternPtr;
}
// --------------------------

// GUI ----------------------
// method: 0 - Random, 1 - Particle, 2 - L-System
void BoltGenerationGUI(int method) {
	ImGui::Begin("Bolt Generation");
	ImGui::Text("Bolt Generation Options");
	ImGui::Separator();
	switch (method) {
	case 0:
		ImGui::Text("Random");
		ImGui::Text("Segment Position Variation");
		ImGui::SliderInt("X Variation", &xVariation, 2, 10);
		ImGui::SliderInt("Y Variation", &yVariation, 2, 10);
		ImGui::SliderFloat("Multiplyer", &multiplyer, 0.1f, 2.0f);
		break;
	case 1: 
		ImGui::Text("Particle System");
		ImGui::Text("Segment Length");
		ImGui::SliderFloat("Length", &length, 0.1f, 2.0f);
		break;
	case 2:
		ImGui::Text("L-System");
		ImGui::Text("Max Displacement");
		ImGui::InputFloat("##", &startingMaxDisplacement, 1, 10, "%.0f");

		ImGui::Text("Detail");
		ImGui::InputInt("###", &LSystemDetail, 1, 5);
		break;
	}

	ImGui::Separator();
	ImGui::Text("Branch Chance");
	switch (method) {
	case 0:
		ImGui::InputInt("###", &randomPositionsBranchChance, 1, 100);
		break;
	case 1:
		ImGui::InputInt("###", &particleSystemBranchChance, 1, 100);
		break;
	case 2:
		ImGui::InputInt("###", &LSystemBranchChance, 1, 100);
		break;

	}
	ImGui::End();
}
// --------------------------

// --------------------------------------------------