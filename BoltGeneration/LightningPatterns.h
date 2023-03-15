#pragma once

#include <random>
#include <glm/glm.hpp>
#include <memory>

#include <iostream>

#include "../FunctionLibrary.h"

const int numSegmentsInPattern = 100;

std::shared_ptr<glm::vec3[numSegmentsInPattern]> GenerateLightningPattern(glm::vec3 startPnt);
vec3 NextPoint(vec3 point);