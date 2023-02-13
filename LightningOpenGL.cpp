#include <iostream>
#include <vector>
#include <math.h>
#include <memory>
#include <random>
#include <functional>

#define PI 3.14159265

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using glm::vec3;
using glm::mat4;
using std::vector;
using glm::radians;
using glm::lookAt;

float vectorMagnitude(int x, int y, int z);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

class Camera {
public:
	vec3 position;
	Camera() {
		position = vec3(0, 0, 0);
	}
};
Camera camera;

class Line {
	int shaderProgram;
	unsigned int VBO, VAO;
	vector<float> vertices;
	vec3 startPoint;
	vec3 endPoint;
	mat4 MVP = mat4(1.0f);
	vec3 lineColor;

public:
	Line() {};		// use setup function so Lines made in array can be initialized
	Line(vec3 start, vec3 end) {
		setup(start, end);
	}

	void setup(vec3 start, vec3 end) {
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

	int setMVP(mat4 mvp) {
		MVP = mvp;
		return 1;
	}

	int setColor(vec3 color) {
		lineColor = color;
		return 1;
	}

	int draw() {
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

	int setPoints(vec3 start, vec3 end) {
		startPoint = start;
		endPoint = end;
		return 1;
	}

	~Line() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(shaderProgram);
	}
};

vec3 ConvertWorldToScreen(vec3 pos) {

	pos.x = 2 * pos.x / SCR_WIDTH - 1;
	pos.y = 2 * pos.y / SCR_HEIGHT - 1;
	pos.z = 2 * pos.z / SCR_WIDTH - 1;

	return pos;
}

const int xVariation = 300;
const int yVariationMax = 160;
const int yVariationMin = 80;
const int yMultiplyer = 2;
const int zVariation = 500;

const int numLines = 500;

std::default_random_engine generator;
std::uniform_int_distribution<int> x(0, xVariation * 2);
std::uniform_int_distribution<int> y(yVariationMin, yVariationMax);
std::uniform_int_distribution<int> z(0, zVariation * 2);

auto rollx = std::bind(x, generator);
auto rolly = std::bind(y, generator);
auto rollz = std::bind(z, generator);


// Returns the pointer to the array of lines that form the lightning
std::shared_ptr<Line> DefineLightningLines(vec3 startPos) {

	std::shared_ptr<Line> linesPtr(new Line[numLines], std::default_delete < Line[]>());

	std::srand(std::time(nullptr));

	vec3 point1 = startPos;
	vec3 point2 = point1;
	//int R = 1;
	// make all the lines
	for (int i = 0; i < numLines; i++) {

		int dx = rollx() - xVariation;
		int dy = -rolly() * yMultiplyer;
		int dz = rollz() - zVariation;



		//int dx = (std::rand() % 2*xVariation) - xVariation;
		//int dy = -(std::rand() % (yVariationMax - yVariationMin + 1) + yVariationMin);
		//int dz = (std::rand() % 2*zVariation) - zVariation;

		/*	// Spiral Patern going down
*
		point2.x += (float)cos(i) * R;
		point2.y += -(i * R / 4);
		point2.z += (float)sin(i) * R;*/

		point2.x += dx;
		point2.y += dy;
		point2.z += dz;

		linesPtr.get()[i].setup(ConvertWorldToScreen(point1), ConvertWorldToScreen(point2));
		linesPtr.get()[i].setColor(vec3(1, 1, 0));
		//std::cout << ConvertWorldToScreen(point1).x << "," << ConvertWorldToScreen(point1).y << "," << ConvertWorldToScreen(point1).z << std::endl;
		std::cout << dx << "  " << dy << " " << dz << std::endl;
		point1 = point2;
		// each next line has start equal to the previous
	}

	return linesPtr;
}

float vectorMagnitude(int x, int y, int z)
{
	// Stores the sum of squares
	  // of coordinates of a vector
	int sum = x * x + y * y + z * z;

	// Return the magnitude
	return sqrt(sum);
}

// process all input: query GLFW whether relevant kesyare pressed / released this from and react accordingly
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main() {
	// initialize and configure glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // REMOVE IF ISSUE WITH MACS

	// window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "drawing lines", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	camera.position = vec3(40, 40, 40);

	// projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	float angle = 0.0f;
	float rotationSpeed = 50.0f;

	// draw lines here -------------------------
	std::shared_ptr<Line> linesPtr(DefineLightningLines(vec3(400, 8000, 0)));

	//------------------------------------------

	// render loop

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;

		// input
		processInput(window);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// update camera position(rotating)
		angle = deltaTime * rotationSpeed;
		camera.position = vec3(40 * cos(radians(angle)), 40, 40 * sin(radians(angle)));

		mat4 view = lookAt(camera.position, vec3(0, 0, 0), vec3(0, 1, 0));

		for (int i = 0; i < 100; i++) {
			linesPtr.get()[i].setMVP(projection * view);
			linesPtr.get()[i].draw();
		}

		// glfw: swap buffers and poll IO events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}