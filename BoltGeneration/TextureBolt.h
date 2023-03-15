#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <glad/glad.h>

#include "../FunctionLibrary.h"

class TextureBolt {
private:
	unsigned int VAO, VBO, EBO;
	float vertices[32];
	int indices[6] = { 0, 1, 3, 1, 2, 3 };
	glm::vec3 startPoint;
	glm::vec3 endPoint;

public: 
	glm::vec3 color;
	TextureBolt();	// called when Tbolt is created in array
	void Draw();
	void Setup(glm::vec3 start, glm::vec3 end);
};
