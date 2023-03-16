#include "FunctionLibrary.h"

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

unsigned int boltTexture;

void LoadTextures() {
	// load textures
	std::string filename = "wall.png";
	std::cout << "Loading texture: " << filename << std::endl;
	unsigned int boltTexture = LoadTexture(filename.c_str());
	if (boltTexture == 0)
		std::cout << "Failed to load texture: " << filename << std::endl;
	else std::cout << "Successfully loaded: " << filename << std::endl;
}

// Get the project base path
std::string ProjectBasePath() {
	//  "..\ProjectBase\x64\Debug" - sometimes path of exe
	// get project path by removing "\x64\Debug" from current path
	std::string projectBase = std::filesystem::current_path().string();
	projectBase.erase(projectBase.end() - 10, projectBase.end());

	return projectBase;
}


// Textures
unsigned int LoadTexture(const char* path) {
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 3);
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

void BindBoltTexture() {
	glBindTexture(GL_TEXTURE_2D, boltTexture);
}

vec3 ConvertWorldToScreen(vec3 pos) {

	pos.x = 2 * pos.x / SCR_WIDTH - 1;
	pos.y = 2 * pos.y / SCR_HEIGHT - 1;
	pos.z = 2 * pos.z / SCR_WIDTH - 1;

	return pos;
}

void SetWidthAndHeight(unsigned int width, unsigned int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}