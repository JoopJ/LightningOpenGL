#include "FunctionLibrary.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

unsigned int boltTexture;

std::string projectBase = "";
std::string vertexDir = ProjectBasePath() + "\\Shader\\VertexShaders";
std::string fragmentDir = ProjectBasePath() + "\\Shader\\FragmentShaders";
std::string geometryDir = ProjectBasePath() + "\\Shader\\GeometryShaders";

Shader LoadShader(const char* vertex, const char* fragment, const char* geometry) {
	std::string vertexPath = vertexDir + "\\" + vertex;
	std::string fragmentPath = fragmentDir + "\\" + fragment;
	if (geometry != nullptr) {
		std::string geometryPath = geometryDir + "\\" + geometry;
		return Shader(vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str());
	}
	else {
		return Shader(vertexPath.c_str(), fragmentPath.c_str());
	}
}

// Returns the path to the project base folder
std::string ProjectBasePath() {
	if (projectBase == "") {
		projectBase = std::filesystem::current_path().string();
		// if current path is in the debug folder, erase down to the project base
		if (projectBase.substr(projectBase.length() - 10) == "\\x64\\Debug") {
			projectBase.erase(projectBase.end() - 10, projectBase.end());
		}
	}
	return projectBase;
}


// Textures
unsigned int LoadTexture(const char* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	std::string projectBase = ProjectBasePath();
	std::string fullPath = projectBase + path;
	path = fullPath.c_str();

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (stbi_failure_reason())
		std::cout << "Failed reason: " << stbi_failure_reason() << std::endl;
	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		// generate a texture using the image data
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
	}
	stbi_image_free(data);

	return textureID;
}

// setting MVPs
void SetVPMatricies(Shader shader, mat4 view, mat4 projection) {
	// camera and projection setting
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
}
void SetMVPMatricies(Shader shader, mat4 model, mat4 view, mat4 projection) {
	shader.SetMat4("model", model);
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
}

void BindBoltTexture() {
	glBindTexture(GL_TEXTURE_2D, boltTexture);
}

vec3 ConvertWorldToScreen(vec3 pos) {

	//pos.x = 2 * pos.x / SCR_WIDTH - 1;
	//pos.y = 2 * pos.y / SCR_HEIGHT - 1; 
	//pos.z = 2 * pos.z / SCR_WIDTH - 1;

	return pos;
}

void SetWidthAndHeight(unsigned int width, unsigned int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

void OutputVec3(vec3 vec) {
	std::cout << "[" << vec.x << "," << vec.y << "," << vec.z << "]" << std::endl;
}

void OutputMat4(mat4 mat) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << mat[j][i] << " ";
		}
		std::cout << std::endl;
	}
}