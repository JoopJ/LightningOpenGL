#include "TextureBolt.h"

TextureBolt::TextureBolt() {
	color = glm::vec3(1, 1, 0);
};


/// <summary>
/// Before this function is called, the EBO and texture for the bolt should
/// be bound. The corrent shader program should also be in use.
/// </summary>
void TextureBolt::Draw() {
	glBindVertexArray(VAO);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// set up Vertex data and buffers
void TextureBolt::Setup(glm::vec3 start, glm::vec3 end, int num) {
	startPoint = start;
	endPoint = end;
	boltNum = num;

	// we do many calculations twice with this method - can be optimized 
	// (the previous TextureBolt will have made the same calculations for
	// the start point of this one)
	float offSet = 0.1;
	float verticesSet[32] = {	// TODO: upgrade to 3D
		// positions										// color						// texture coords
		start.x+offSet, start.y+offSet, start.z + offSet,	color.x, color.y, color.z,		1.0, 1.0,	// top right
		end.x+offSet,   end.y-offSet,	end.z + offSet,		color.x, color.y, color.z,		1.0, 0.0,	// bottom right
		end.x-offSet,   end.y-offSet,	end.z - offSet,		color.x, color.y, color.z,		0.0, 0.0,	// bottom left
		start.x-offSet, start.y+offSet, start.z - offSet,	color.x, color.y, color.z,		0.0, 1.0	// top left
	};

	memcpy(vertices, verticesSet, sizeof(verticesSet));

	//std::cout << "BOLT VERTICIES : [" << vertices[0] << ", " << vertices[1] << ", " << vertices[2] << "]" << std::endl;
	//std::cout << " ------------- : [" << vertices[5] << "," << vertices[6] << ", " << vertices[7] << "] " << std::endl;
	//std::cout << " ------------- : [" << vertices[10] << "," << vertices[11] << ", " << vertices[12] <<  "] " << std::endl;
	//std::cout << " ------------- : [" << vertices[15] << "," << vertices[16] << ", " << vertices[17] << "] " << std::endl;
	// 
	// buffer objects
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
