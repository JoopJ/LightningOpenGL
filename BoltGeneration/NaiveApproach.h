#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "BoltGeneration/Line.h"

using glm::vec3;

vec3 DefineLine(std::shared_ptr<Line> linesPtr, vec3 startPos, int i, int* lineCountPtr);
int DefineLightningLines(vec3 startPos, std::shared_ptr<Line> linesPtr);
void GUINaiveApproach();
