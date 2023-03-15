#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "../FunctionLibrary.h"

#include <iostream>

using glm::vec3;
using glm::mat4;
using std::vector;

class Line {
private:
	int shaderProgram;
	unsigned int VBO, VAO;
	std::vector<float> vertices;
	glm::vec3 startPoint;
	glm::vec3 endPoint;
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 lineColor;

public:
	Line(); // called when Line is created in array, Setup should be called afterwards
	Line(glm::vec3 start, glm::vec3 end); // Line Constructor
	void Setup(glm::vec3 start, glm::vec3 end);
	int SetColor(glm::vec3 color);
	void SetProjection(glm::mat4 proj);
	void SetView(glm::mat4 view);
	int Draw();
	~Line();
};