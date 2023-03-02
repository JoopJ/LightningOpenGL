#include "Line.h"

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

using glm::vec3;
using glm::mat4;
using std::vector;

// use setup function so Lines made in array can be initialized
Line::Line() {};

// Line Constructor
Line::Line(glm::vec3 start, glm::vec3 end) {
	Setup(start, end);
}


void Line::Setup(vec3 start, vec3 end) {
	startPoint = start;
	endPoint = end;
	lineColor = vec3(1, 1, 1);

	const char* vertexShaderSource = "#version 450 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"uniform mat4 MVP;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";

	const char* fragmentShaderSource = "#version 450 core\n"
		"out vec4 FragColor;\n"
		"uniform vec3 color;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(color, 1.0f);\n"
		"}\n\0";

	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// fragmet shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// link shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	vertices = {
		start.x, start.y, start.z,
		end.x, end.y, end.z,
	};

	// genereate and 
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);

	// bind and initialize buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// define and enable array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int Line::SetMVP(mat4 mvp) {
	MVP = mvp;
	return 1;
}

int Line::SetColor(vec3 color) {
	lineColor = color;
	return 1;
}

int Line::Draw() {
	// installs program as part of current rendering state
	glUseProgram(shaderProgram);

	// create MVP and color matrixes
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &lineColor[0]);

	// Actually droaw the stuff to the matrix
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
	return -1;
}

int Line::SetPoints(vec3 start, vec3 end) {
	startPoint = start;
	endPoint = end;
	return 1;
}

Line::~Line() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
}