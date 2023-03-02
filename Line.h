#ifndef Line

#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

class Line {
	int shaderProgram;
	unsigned int VBO, VAO;
	std::vector<float> vertices;
	glm::vec3 startPoint;
	glm::vec3 endPoint;
	glm::mat4 MVP = glm::mat4(1.0f);
	glm::vec3 lineColor;

public:
	Line();
	Line(glm::vec3 start, glm::vec3 end);
	void Setup(glm::vec3 start, glm::vec3 end);
	int SetMVP(glm::mat4 mvp);
	int SetColor(glm::vec3 color);
	int Draw();
	int SetPoints(glm::vec3 start, glm::vec3 end);
	~Line();
};


#endif