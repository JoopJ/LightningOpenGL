#pragma once

#include <glm/glm.hpp>
using glm::vec3;
#include <glad/glad.h>

#include "../FunctionLibrary.h"

class BoltTriangleColor {
private:
	unsigned int VAO, VBO, EBO;	
	float vertices[32];
	int indices[6] = { 0, 1, 3, 1, 2, 3 };

public:
	glm::vec3 color;
	BoltTriangleColor();
	void Draw();
	void Setup(glm::vec3 start, glm::vec3 end);
};