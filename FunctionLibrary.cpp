#include "FunctionLibrary.h"
#include <glm/glm.hpp>
using glm::vec3;

unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

vec3 ConvertWorldToScreen(vec3 pos) {

	pos.x = 2 * pos.x / SCR_WIDTH - 1;
	pos.y = 2 * pos.y / SCR_HEIGHT - 1;
	pos.z = 2 * pos.z / SCR_WIDTH - 1;

	return pos;
}

float vectorMagnitude(int x, int y, int z)
{
	// Stores the sum of squares
	  // of coordinates of a vector
	int sum = x * x + y * y + z * z;

	// Return the magnitude
	return sqrt(sum);
}

void SetWidthAndHeight(unsigned int width, unsigned int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}