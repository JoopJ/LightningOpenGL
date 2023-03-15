#pragma once

#include <random>
#include <glm/glm.hpp>
#include <memory>

#include <iostream>

#include "../FunctionLibrary.h"

const int numLinesInPattern = 100;

std::shared_ptr<glm::vec3[numLinesInPattern]> GenerateLightningPattern(glm::vec3 startPnt);
glm::vec3 NextPoint(glm::vec3 point);