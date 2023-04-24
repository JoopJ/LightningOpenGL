#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm/glm.hpp>

#include "../FunctionLibrary.h"

#include <iostream>

using glm::vec3;
using glm::mat4;
using std::vector;

class LineBoltSegment {
private:
	unsigned int VBO, VAO;
	float vertices[6];

	const char* id = "Unchanged";

public:
	LineBoltSegment(); // called when Line is created in array, Setup should be called afterwards
	LineBoltSegment(glm::vec3 start, glm::vec3 end); // Line Constructor
	void Setup(glm::vec3 start, glm::vec3 end);
	void Draw();
	//~LineBoltSegment();
	void printInfo();
};