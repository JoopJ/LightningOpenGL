// IMPROVEMENTS
// it would be better to get all the vertex data from each line together and put them in one buffer instead of one buffer per line


#include "LineBoltSegment.h"

// use setup function so Lines made in array can be initialized
LineBoltSegment::LineBoltSegment() { };

// Line Constructor
LineBoltSegment::LineBoltSegment(vec3 start, vec3 end) {
	Setup(start, end);
}


void LineBoltSegment::Setup(vec3 start, vec3 end) {
	id = "Set";
	float verticesSet[6] = {
		start.x, start.y, start.z,
		end.x, end.y, end.z,
	};
	memcpy(vertices, verticesSet, sizeof(verticesSet));

	// delete old buffers if they exist
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);

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

void LineBoltSegment::Draw() {
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 2);
}

// Don't want to delete buffers here because this is called
// when dynamic array (vector) is expanded and the old array 
// is deleted.
// We delete old buffers in Setup function.
/*
LineBoltSegment::~LineBoltSegment() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
} */

void LineBoltSegment::printInfo() {
	std::cout << "VAO: " << VAO << std::endl;
	std::cout << "VBO: " << VBO << std::endl;
	std::cout << "vertices: " << vertices << std::endl << std::endl;
}