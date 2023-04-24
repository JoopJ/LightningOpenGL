#pragma once

#include <glm/glm/glm.hpp>

using glm::vec3;

#include "Shader.h"


// functions
void SetShaderPointLightProperties(Shader shader, int numLights, vec3* lightPosPtr,
	float plLinear, float plQuadratic, vec3 color);
void SetShaderMaterialProperties(Shader shader, vec3 color, float shininess);