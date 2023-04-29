#include "LightningPatterns.h"

// Variables
// General
vec3 boltStartPos;

// Random
int rNumSegments = numSegmentsInPattern;
int hVariationMin = 1;
int hVariationMax = 8;
int vVariationMin = 6;
int vVariationMax = 9;
float multiplyer = 0.15f;
bool alt = true;

// Particle
vec3 particleSeed;
int pNumSegments = 150;
float minLength = 0.3f;
float maxLength = 1.0f;
float lengthMultiplyer = 1.0f;
float angleDegrees = 9;
float angleVariance = 0.1f;
bool particleQuaternion = true;

// L-System
int lNumSegments = 0;
vec3 boltEndPos;
float startingMaxDisplacement = 30;
int LSystemDetail = 8;

// Branching
bool branching = true;
int minBranchLength = 5;
int maxBranchLength = 80;

float randomPositionsBranchChance = 0;
float particleSystemBranchChance = 0.8;
float LSystemBranchChance = 0;

float LSystemBranchScaler = 0.7f;

float LSystemBranchVariation = 0.5f;
int LSystemBranchMinDetail = 4;

// Random
std::random_device rd;
std::mt19937 gen(rd());

// --------------------------------------------------
// Private Functions --------------------------------

// General:
quat CreateRotationQuaternion(vec3 seed, float theta) {
	// Returns a quaternion that rotates about the given seed vector
	// by the given angle theta (in radians).
	float sin = glm::sin(theta / 2);

	return quat(cos(theta / 2), seed.x * sin, seed.y * sin, seed.z * sin);

}
bool RollBranchChance(float branchChance) {
	// Given an int in the range [0, 100], rolls a random number between 0 and 100
	// and if the random number is less than the given int, returns true.

	float roll = float(rand() % 1000) / 10;
	return roll < branchChance;
}
int BranchLength() {
	std::uniform_real_distribution<float> branches(minBranchLength, maxBranchLength);
	// Returns the number of segments in a branch.
	return branches(gen);
}
int RandomFlux() {
	return 1 - (rand() % 2) * 2;
}

// Random Positions:
glm::vec3 NextPoint(glm::vec3 point) {
	int vVariationDiff = vVariationMax - vVariationMin;

	// get random variatins
	float dx = hVariationMin + (rand() % (hVariationMax - hVariationMin + 1));
	float dy = vVariationMin + (rand() % (vVariationDiff + 1));
	float dz = hVariationMin + (rand() % (hVariationMax - hVariationMin + 1));

	// Scale dx and dz with dy.
	if (alt) {
		float s = (dy-float(vVariationMin)) / float(vVariationDiff);
		dx *= s;
		dz *= s;
	}

	// RandomFlux is either 1 or -1.
	point.x += dx * multiplyer * RandomFlux();
	point.y -= dy * multiplyer;
	point.z += dz * multiplyer * RandomFlux();

	return point;
}

// Particle System:
pair<vec3, vec3> GetRotationAxis(vec3 seed) {
	// Returns a pair of perpendicular axis to the seed.
	// The seed acts as the y axis and the two returned axis
	// will act as the x and z axis.
	// Seed should be normalized!

	// get arbitrary axis along yz plane of the seed
	vec3 arb = vec3(seed.x, -seed.z, seed.y);

	// get 2 perpendicular vectors to the seed
	vec3 perp1 = cross(seed, arb);
	//vec3 perp2 = cross(seed, perp1);

	return { perp1, cross(seed, perp1) };
}
vec3 RotatePointAboutSeedQuaternion(vec3 point, pair<vec3, vec3> seedPerpAxis, float r1, float r2) {

	// Manual method
	// construct rotation quaternions for both seed's perp axis
	quat qr1 = CreateRotationQuaternion(seedPerpAxis.first, r1);
	quat qr2 = CreateRotationQuaternion(seedPerpAxis.second, r2);

	// convert point to quaternion
	quat p = quat(0, point.x, point.y, point.z);

	// apply rotations
	quat q1 = qr1 * p * glm::inverse(qr1);
	quat q2 = qr2 * p * glm::inverse(qr2);
	quat final = q2 * q1 * p * glm::inverse(q1) * glm::inverse(q2);

	//std::cout << "Point     : " << point.x << ", " << point.y << ", " << point.z << std::endl;
	//std::cout << "New Point : " << final.x << ", " << final.y << ", " << final.z << std::endl;

	// In built GLM
	quat pA = quat(0, point.x, point.y, point.z);
	glm::rotate(pA, r1, seedPerpAxis.first);
	glm::rotate(pA, r2, seedPerpAxis.second);

	
	// extract new point
	// Manual
	return vec3(final.x, final.y, final.z);
	// In built GLM
	//return vec3(pA.x, pA.y, pA.z);
}

glm::mat3 ConstructRotationMatrix(vec3 V, float r) {

	// Pre calculate values
	float VxSq = V.x * V.x;
	float VySq = V.y * V.y;
	float VzSq = V.z * V.z;
	float cos = glm::cos(r);
	float sin = glm::sin(r);
	float oneMinusCos = 1 - cos;

	// construct rotation matrix
	glm::mat3 M = glm::mat3(
		VxSq + (VySq + VzSq) * cos, V.x * V.y * oneMinusCos - V.z * sin, V.x * V.z * oneMinusCos + V.y * sin,
		V.x * V.y * oneMinusCos + V.z * sin, VySq + (VxSq + VzSq) * cos, V.y * V.z * oneMinusCos - V.x * sin,
		V.x * V.z * oneMinusCos - V.y * sin, V.y * V.z * oneMinusCos + V.x * sin, VzSq + (VxSq + VySq) * cos);

	return M;
}

vec3 RotatePointAboutSeedMatrix(vec3 p, pair<vec3, vec3> seedPerpAxis, float r1, float r2) {

	glm::mat3 M1 = ConstructRotationMatrix(seedPerpAxis.first, r1);
	glm::mat3 M2 = ConstructRotationMatrix(seedPerpAxis.second, r2);

	p = M1 * p;
	p = M2 * p;
	
	return p;
}

vec3 RotatePointAboutSeed(vec3 point, pair<vec3, vec3> seedPerpaxis) {
	// normal distribution for angle
	std::normal_distribution<float> angle(angleDegrees, angleVariance);

	// get rotation values
	float degree1 = RandomFlux() * angle(gen);
	float degree2 = RandomFlux() * angle(gen);
	float r1 = glm::radians(degree1);
	float r2 = glm::radians(degree2);

	//std::cout << "Degree 1: " << degree1 << std::endl;
	//std::cout << "Degree 2: " << degree2 << std::endl;

	if (particleQuaternion) {
		return RotatePointAboutSeedQuaternion(point, seedPerpaxis, r1, r2);
	}
	else {
		return RotatePointAboutSeedMatrix(point, seedPerpaxis, r1, r2);
	}
}

// L-System:
vec3 GetPerpAxis(vec3 axis) {
	// Returns a perpendicular vector to the given axis.
	// the perp vector is rotated by a random angle around
	// the original axis.

	// get perpendicular axis
	vec3 perp = cross(axis, vec3(axis.x, -axis.z, axis.y));

	// get random angle
	float radian = glm::radians((float)(rand() % 360));

	// create rotation quaternion and it's inverse
	quat r = CreateRotationQuaternion(axis, radian);
	//r.fromAngleAxis(radian, irr::core::vector3df(axis.x, axis.y, axis.z));

	// convert perp to quaternion
	quat p = quat(perp.x, perp.y, perp.z, 0);

	// apply rotation
	p = r * p * glm::inverse(r);

	return normalize(vec3(p.x, p.y, p.z));
}
vec3 GetMidPnt(vec3 start, vec3 end, float maxDisplacement) {
	vec3 mid = (start + end) / 2.0f;

	// get perpendicular axis
	vec3 perp = GetPerpAxis(normalize(end - start));

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
	
	// uniform distribution for particle segment length
	std::uniform_real_distribution<float> length(minLength, maxLength);

	seed = normalize(seed);
	// get the axis to rotate around
	pair<vec3, vec3> seedPerpAxis = GetRotationAxis(seed);

	vec3 prevEnd = start;
	vec3 newPoint = start + seed * length(gen) * lengthMultiplyer;

	for (int i = 0; i < size; i++) {
		// add the new segment to the pattern
		patternPtr->push_back({ ConvertWorldToScreen(prevEnd), ConvertWorldToScreen(newPoint) });

		prevEnd = newPoint;

		// get the transfrom for the next point
		vec3 newPointMove = seed * length(gen) * lengthMultiplyer;
		// rotate with respect to the seed
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		newPoint = prevEnd + newPointMove;
	}
}
// --------------------------------------------------

// Public Functions ---------------------------------

// Random Positions --------
//STATIC BOLT
int GenerateRandomPositionsPattern(
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {
	
	vec3 start = boltStartPos;

	patternPtr.get()[0] = ConvertWorldToScreen(start);
	for (int i = 1; i < numSegmentsInPattern; i++) {
		start = NextPoint(start);
		patternPtr.get()[i] = ConvertWorldToScreen(start);
	}
	// will always have the same size
	return numSegmentsInPattern;
}
//DYNAMIC BOLT
vector<pair<vec3, vec3>>* GenerateRandomPositionsPattern(
	vector<pair<vec3, vec3>>* patternPtr) {
	// clear the pattern
	patternPtr->clear();

	vec3 end;
	vec3 start = boltStartPos;
	for (int i = 0; i < rNumSegments; i++) {
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
int GenerateParticleSystemPattern(
	std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	// uniform distribution for particle segment length
	std::uniform_real_distribution<float> length(minLength, maxLength);

	vec3 seed = particleSeed;
	// get the axis to rotate around
	pair<vec3, vec3> seedPerpAxis = GetRotationAxis(seed);

	vec3 prevEnd = boltStartPos;
	vec3 newPoint = prevEnd + seed * length(gen) * lengthMultiplyer;

	// add the first segment to the pattern
	patternPtr.get()[0] = ConvertWorldToScreen(prevEnd);
	for (int i = 1; i < numSegmentsInPattern; i++) {
		// add the new segment to the pattern
		patternPtr.get()[i] = ConvertWorldToScreen(newPoint);

		prevEnd = newPoint;

		// Get next point:
		// move the point along the seed vector
		vec3 newPointMove = seed * length(gen) * lengthMultiplyer;
		// rotate the point with respect to seed's perpendicular axis
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		newPoint = prevEnd + newPointMove;
	}
	// will always have the same size
	return numSegmentsInPattern;

}
//DYNAMIC BOLT
vector<pair<vec3, vec3>>* GenerateParticleSystemPattern(
	vector<pair<vec3, vec3>>* patternPtr) {

	// uniform distribution for particle segment length
	std::uniform_real_distribution<float> length(minLength, maxLength);

	// clear the pattern
	patternPtr->clear();

	vec3 seed = particleSeed;
	// get the axis to rotate around
	pair<vec3, vec3> seedPerpAxis = GetRotationAxis(seed);

	vec3 prevEnd = boltStartPos;
	vec3 newPoint = prevEnd + seed * length(gen) * lengthMultiplyer;

	for (int i = 0; i < pNumSegments; i++) {
		// add the new segment to the pattern
		patternPtr->push_back({ ConvertWorldToScreen(prevEnd), ConvertWorldToScreen(newPoint) });

		// Branch
		if (branching && RollBranchChance(particleSystemBranchChance)) {
			// the branch's seed is the previous segment
			ParticleSystemBranch(newPoint, (newPoint-prevEnd), BranchLength(), patternPtr);
		}

		prevEnd = newPoint;

		// get the transform for the next point
		vec3 newPointMove = seed * length(gen) * lengthMultiplyer;

		// roate with respect to the seed
		newPointMove = RotatePointAboutSeed(newPointMove, seedPerpAxis);
		//std::cout << "Lenght: " << len << std::endl;
		newPoint = prevEnd + newPointMove;

	}

	return patternPtr;
}
// -------------------------

// L-System ----------------
//STATIC BOLT
int GenerateLSystemPattern(std::shared_ptr<vec3[numSegmentsInPattern]> patternPtr) {

	int size = int(pow(2, LSystemDetail)) + 1;
	// check if the size is too big for array
	if (size > numSegmentsInPattern) {
		std::cout << "WARNING::L-SYSTEM::PATTERN SIZE EXCEEDS MAX PATTERN SIZE" << std::endl;
		return 0;
	}

	int startIndex = 0;
	int endIndex = size - 1;

	patternPtr[startIndex] = ConvertWorldToScreen(boltStartPos);
	patternPtr[endIndex] = ConvertWorldToScreen(boltEndPos);

	LSystemSubDivide(boltStartPos, boltEndPos, startIndex, endIndex,
		LSystemDetail, startingMaxDisplacement, patternPtr);

	return size;
}
//DYNAMIC BOLT
// Dynamic Version of Static L-System Pattern. No Branching!
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vector<pair<vec3, vec3>>* patternPtr) {

	const int size = int(pow(2, LSystemDetail)) + 1;

	// stores the points of the pattern
	vector<vec3> patternPoints;
	patternPoints.resize(size);
	vector<vec3>* patternPointsPtr = &patternPoints;

	int startIndex = 0;
	int endIndex = size - 1;

	patternPointsPtr->at(startIndex) = ConvertWorldToScreen(boltStartPos);
	patternPointsPtr->at(endIndex) = ConvertWorldToScreen(boltEndPos);

	LSystemSubDivide(boltStartPos, boltEndPos, startIndex, endIndex,
		LSystemDetail, (float)startingMaxDisplacement, patternPointsPtr);

	// add the points to the pattern as pairs
	patternPtr->clear();
	patternPtr->resize(size);
	for (int i = 0; i < size-1; i++) {
		pair<vec3, vec3> seg = { patternPoints.at(i), patternPoints.at(i + 1) };
		patternPtr->at(i) = seg;
	}

	return patternPtr;
}
// Alternate Dynamic L-System Pattern. Yes Branching!
vector<pair<vec3, vec3>>* GenerateLSystemPattern(vector<pair<vec3, vec3>>* patternPtr, bool x) {

	patternPtr->clear();

	// ping pong between two vectors
	vector<pair<vec3, vec3>> segments1;
	vector<pair<vec3, vec3>> segments2;

	vector<pair<vec3, vec3>>* segmentsRead = &segments1;
	vector<pair<vec3, vec3>>* segmentsWrite = &segments2;

	// add the first segment
	segmentsRead->push_back({ boltStartPos, boltEndPos });

	float maxDisplacement = startingMaxDisplacement;

	for (int d = LSystemDetail; d > 0; d--) {

		int numSegments = segmentsRead->size();
		segmentsWrite->clear();

		for (int seg = 0; seg < numSegments; seg++) {

			// Get the last segment, then remove it.
			pair<vec3, vec3> currentSeg = segmentsRead->back();
			segmentsRead->pop_back();

			// calculate mid point
			vec3 mid = GetMidPnt(currentSeg.first, currentSeg.second, maxDisplacement);

			// add the new segments
			segmentsWrite->push_back({ currentSeg.first, mid });
			segmentsWrite->push_back({ mid, currentSeg.second });

			// Branch
			// don't branch when d is below certain value. 
			// A branch with low detail looks unrealistic.
			if (branching && d > LSystemBranchMinDetail && RollBranchChance(LSystemBranchChance)) {

				vec3 dir = mid - currentSeg.first;

				// TODO : introduce a little variation to the direction

				vec3 branchEnd = mid + (dir * LSystemBranchScaler);

				if (branchEnd == mid);

				segmentsWrite->push_back({ mid, branchEnd });
			}
		}

		// Reduce maxDisplacement
		maxDisplacement *= 0.5f;

		// swap vectors
		vector<pair<vec3, vec3>>* temp = segmentsRead;
		segmentsRead = segmentsWrite;
		segmentsWrite = temp;
	}

	// add the segments to the pattern
	for (int i = 0; i < segmentsRead->size(); i++) {
		patternPtr->push_back({ segmentsRead->at(i).first, segmentsRead->at(i).second });
	}
	lNumSegments = segmentsRead->size();

	return patternPtr;
}
// --------------------------

// GUI ----------------------
// method: 0 - Random, 1 - Particle, 2 - L-System
void BoltGenerationGUI(int method) {
	ImGui::SetNextWindowPos(ImVec2(5, 383), ImGuiCond_Once);

	ImGui::Begin("Bolt Generation", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Bolt Generation Options");
	ImGui::Separator();
	switch (method) {
	case 0:
		ImGui::Text("Random");
		ImGui::Separator();
		ImGui::Text("Segment Position Variation:");
		ImGui::Text("Horizontal");
		ImGui::DragIntRange2("##Horizontal", &hVariationMin, &hVariationMax, 0.1f, 0, 15);
		ImGui::Text("Vertical");
		ImGui::DragIntRange2("##Vertical", &vVariationMin, &vVariationMax, 0.1f, 0, 15);
		ImGui::Text("Scale Variation");
		ImGui::Checkbox("##scaleAlt", &alt);
		ImGui::Text("Multiplyer");
		ImGui::SliderFloat("##Multiplyer", &multiplyer, 0.1f, 2.0f);
		break;
	case 1: 
		ImGui::Text("Particle System");
		ImGui::Separator();
		ImGui::Text("Segment Length");
		ImGui::DragFloatRange2("##segmentLength", &minLength, &maxLength, 0.01f, 0.01f, 5.0f, "Min: %.2f", "Max: %.2f");
		ImGui::InputFloat("##lengthMultiplyer", &lengthMultiplyer, 1.0f, 15.0f, "%.1f");
		ImGui::Text("Angle");
		if (ImGui::SliderFloat("##angle", &angleDegrees, 1.0f, 32.0f)) {

		}
		ImGui::Text("Angle Variance");
		ImGui::SliderFloat("##angleVariance", &angleVariance, 0.0f, 10.0f);
		break;
	case 2:
		ImGui::Text("L-System");
		ImGui::Separator();
		ImGui::Text("Max Displacement");
		ImGui::InputFloat("##lsDisplacement", &startingMaxDisplacement, 1, 5, "%.0f");
		ImGui::Text("Detail");
		ImGui::InputInt("##lsDetail", &LSystemDetail, 1, 2);
		ImGui::Text("Branch Scalar");
		ImGui::InputFloat("##branchScalar", &LSystemBranchScaler, 0.1f, 10.0f);
		break;
	}

	ImGui::Separator();
	ImGui::Text("Num Segments");
	switch (method) {
	case 0: 
		ImGui::InputInt("##rpnumSeg", &rNumSegments, 50, 300);
		break;
	case 1:
		ImGui::InputInt("##psnumSeg", &pNumSegments, 50, 300);
		break;
	case 2:
		ImGui::Text("%d", lNumSegments);
		break;
	}
		
	ImGui::Separator();
	ImGui::Text("Branch Chance");
	switch (method) {
	case 0:
		ImGui::InputFloat("##rpChance", &randomPositionsBranchChance, 0.1, 1, "%.1f");
		break;
	case 1:
		ImGui::InputFloat("##psChace", &particleSystemBranchChance, 0.1, 1, "%.1f");
		ImGui::Text("Branch Length");
		ImGui::DragIntRange2("##branchLength", &minBranchLength, &maxBranchLength, 1.0f, 0, pNumSegments, "Min:0% d", "Max: %d");
		break;
	case 2:
		ImGui::InputFloat("##lsChance", &LSystemBranchChance, 0.1, 1, "%.1f");
		break;
	}

	ImGui::End();
}
// --------------------------

// Set Method Options -------
void SetStartPos(vec3 pos) {
	boltStartPos = pos;
}

void SetEndPos(vec3 pos) {
	boltEndPos = pos;
}

void SetLSystemOptions(vec3 end, int detail, float maxDisplacement) {
	boltEndPos = end;
	LSystemDetail = detail;
	startingMaxDisplacement = maxDisplacement;
}

void SetRandomOptions(bool _alt) {
	alt = _alt;
}

void SetParticleOptions(vec3 seed) {
	particleSeed = normalize(seed);
}

void SetNumSegments(int num) {
	rNumSegments = num;
	pNumSegments = num;
}
// --------------------------

// --------------------------------------------------