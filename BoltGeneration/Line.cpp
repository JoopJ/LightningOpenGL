// IMPROVEMENTS
// it would be better to get all the vertex data from each line together and put them in one buffer instead of one buffer per line


#include "Line.h"

// use setup function so Lines made in array can be initialized
Line::Line() {};

// Line Constructor
Line::Line(glm::vec3 start, glm::vec3 end) {
	Setup(start, end);
}


void Line::Setup(vec3 start, vec3 end) {
	startPoint = start;
	endPoint = end;
	lineColor = vec3(1, 1, 0);
	projection = glm::mat4(1.0f);
	view = glm::mat4(1.0f);

	//std::cout << "(" << start.x << ", " << start.y << ", " << start.z << ")" << std::endl;

	const char* vertexShaderSource = "#version 450 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = projection * view * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
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

	// genereate, bind and initialize buffer's for bolt rendering
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	// bind the VBO to the GL_ARRAY_BUFFER target
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * vertices.size(), vertices.data(), GL_STATIC_DRAW);	// copy the vertex data into the currently bound Array Buffer (VBO at this moment)
	/* TODO: Maybe need to change to DYNAMIC_DRAW, not sure if it's changed frequently enough tho.
	GL_STREAM_DRAW: the data is set only onceand used by the GPU at most a few times.
	GL_STATIC_DRAW: the data is set only onceand used many times.
	GL_DYNAMIC_DRAW: the data is changed a lotand used many times.*/


	// define and enable array (VAO at this moment)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// unbind both buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int Line::SetColor(vec3 color) {
	lineColor = color;
	return 1;
}

void Line::SetProjection(glm::mat4 p) {
	projection = p;
}

void Line::SetView(glm::mat4 v) {
	view = v;
}

int Line::Draw() {
	// shader program should be set before calling this function
	glUseProgram(shaderProgram);

	// create MVP and color matrixes
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &lineColor[0]);

	// Actually draw to the matrix
	glBindVertexArray(VAO);	// bind back to VAO that we put the VBO data in, in the for of a vertex array
	glDrawArrays(GL_LINES, 0, 2);
	return -1;
}

Line::~Line() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteProgram(shaderProgram);
}