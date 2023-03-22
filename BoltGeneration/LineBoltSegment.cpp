// IMPROVEMENTS
// it would be better to get all the vertex data from each line together and put them in one buffer instead of one buffer per line


#include "LineBoltSegment.h"

// use setup function so Lines made in array can be initialized
LineBoltSegment::LineBoltSegment() { };

// Line Constructor
LineBoltSegment::LineBoltSegment(glm::vec3 start, glm::vec3 end) {
	Setup(start, end);
}


void LineBoltSegment::Setup(vec3 start, vec3 end) {

	float verticesSet[6] = {
		start.x, start.y, start.z,
		end.x, end.y, end.z,
	};
	memcpy(vertices, verticesSet, sizeof(verticesSet));

	// genereate, bind and initialize buffer's for bolt rendering
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	// bind the VBO to the GL_ARRAY_BUFFER target
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// define and enable array
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind both buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int LineBoltSegment::SetColor(vec3 colorSet) {
	color = colorSet;
	return 1;
}

void LineBoltSegment::Draw() {
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
}

LineBoltSegment::~LineBoltSegment() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteProgram(shaderProgram);
}