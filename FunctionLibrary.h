#pragma once

#include <glm/glm.hpp>
using glm::vec3;

vec3 ConvertWorldToScreen(vec3 pos);
float vectorMagnitude(int x, int y, int z);
void SetWidthAndHeight(unsigned int width, unsigned int height);

