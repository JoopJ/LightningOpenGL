#include "BoltTriangleColor.h"

// constructor
BoltTriangleColor::BoltTriangleColor() {
	color = vec3(1, 1, 0); // default color
}

void BoltTriangleColor::Draw() {
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

/// <summary>
/// Must be called for very instance of this object before Draw()
/// Sets up the vertices and buffers
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
void BoltTriangleColor::Setup(vec3 start, vec3 end) {
	// vertices
	float offSet = 0.1;
	float verticesSet[32] = {
		// positions											// color
		start.x + offSet, start.y + offSet, start.z + offSet,	color.x, color.y, color.z,	// top right
		end.x + offSet,   end.y - offSet,	end.z + offSet,		color.x, color.y, color.z,	// bottom right
		end.x - offSet,   end.y - offSet,	end.z - offSet,		color.x, color.y, color.z,	// bottom left
		start.x - offSet, start.y + offSet, start.z - offSet,	color.x, color.y, color.z,	// top left
	};
	memcpy(vertices, verticesSet, sizeof(verticesSet));

	// buffer objects
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}