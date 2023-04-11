#include "Renderer.h"

unsigned int quadVAO = 0;
unsigned int cubeVAO = 0;
unsigned int floorVAO = 0;
unsigned int wallVAO = 0;

// Random Boxes
vec3 boxPositions[10];
bool boxPositionsSet = false;
const int numBoxes = 5;
const int range = 15;


// Extras Render Functions
void RenderRoomCube(Shader shader, float scale, vec3 pos);
void RenderRandomBoxes(Shader shader);
void RenderAlcove(Shader shader);
void RenderSlab(Shader shader, mat4 model);


// Render Scene
// Only Sets the model matrix, other matrices should already be set
void RenderScene(const Shader& shader) {
	mat4 model = mat4(1.0f);

	//RenderRoomCube(shader, 20, vec3(0, 5, 0));
	//RenderRandomBoxes(shader);
	RenderAlcove(shader);
}

void RenderAlcove(Shader shader) {
	mat4 model = mat4(1.0f);

	// Back Wall
	model = glm::translate(model, vec3(-40, 0, 0));
	RenderSlab(shader, model);

	// Right Wall
	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(90.0f), vec3(0, 1, 0));
	model = glm::translate(model, vec3(40, 0, 0));
	RenderSlab(shader, model);

	// Left Wall
	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(-90.0f), vec3(0, 1, 0));
	model = glm::translate(model, vec3(40, 0, 0));
	RenderSlab(shader, model);

	// Floor
	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(90.0f), vec3(0, 0, 1));
	model = glm::translate(model, vec3(-40, 0, 0));
	RenderSlab(shader, model);

	// Objects
	model = mat4(1);
	model = glm::translate(model, vec3(0, -25, 6));
	model = glm::scale(model, vec3(5));
	shader.SetMat4("model", model);
	RenderCube();
	model = glm::translate(model, vec3(2, 3, -5));
	model = glm::scale(model, vec3(1, 4, 1));
	shader.SetMat4("model", model);
	RenderCube();
}

void RenderRoomCube(Shader shader, float scale, vec3 pos) {
	mat4 model = mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(scale));
	shader.SetMat4("model", model);
	glDisable(GL_CULL_FACE);
	shader.SetInt("reverse_normals", 1);
	RenderCube();
	shader.SetInt("reverse_normals", 0);
	glEnable(GL_CULL_FACE);
}

void RenderRandomBoxes(Shader shader) {
	// Shadow Testing Room
	mat4 model;
	if (!boxPositionsSet) { // set random box positions
		for (int box = 0; box < numBoxes; box++) {
			// get random positions
			float x = (rand() % (range * 2)) - range;
			float y = (rand() % (range * 2)) - range;
			float z = (rand() % (range * 2)) - range;
			boxPositions[box] = vec3(x, y, z);
		}
		boxPositionsSet = true;
	}

	for (int box = 0; box < numBoxes; box++) {
		model = mat4(1);
		model = glm::translate(model, boxPositions[box]);
		model = glm::scale(model, glm::vec3(3));
		shader.SetMat4("model", model);
		RenderCube();
	}
}

void RenderSlab(Shader shader, mat4 model) {
	model = glm::scale(model, vec3(1, 40, 40));
	shader.SetMat4("model", model);
	RenderCube();
}
// ---------------

void RenderQuad() {
	if (quadVAO == 0) {	// setup
		float quadVertices[] = {	// fills whole screen in NDC;
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};
		unsigned int quadVBO;
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void RenderCube() {
	if (cubeVAO == 0) {
		float cubeVertices[] = {
			// position 		   //normal           //texture
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};

		unsigned int cubeVBO;
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		glBindVertexArray(cubeVAO);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

}

void RenderFloor() {
	if (floorVAO == 0) {
		float width = 40.0f;
		float height = 0.0f;
		float floorVertices[48] = {
			 // positions            // normals          // texture coords
			 width, height, width,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
			-width, height, width,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
			-width, height, -width,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

			 width, height, width,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
			-width, height, -width,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
			 width, height, -width,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f
		};
		unsigned int floorVBO;
		glGenVertexArrays(1, &floorVAO);
		glGenBuffers(1, &floorVBO);
		glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

		glBindVertexArray(floorVAO);
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}
	// render
	glBindVertexArray(floorVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void RenderWall() {
	if (wallVAO == 0) {
		float width = 30.0f;
		float height = width;
		float offset = -30;
		float wallVertices[]{
			 // positions			  // normals		  // texture Coords
			 width,  height, offset,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			-width,  height, offset,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			-width, -height, offset,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

			 width,  height, offset,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			-width, -height, offset,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
			 width, -height, offset,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f
		};

		unsigned int wallVBO;
		glGenVertexArrays(1, &wallVAO);
		glGenBuffers(1, &wallVBO);
		glBindVertexArray(wallVAO);
		glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), wallVertices, GL_STATIC_DRAW);

		glBindVertexArray(wallVAO);
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	// render
	glBindVertexArray(wallVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}