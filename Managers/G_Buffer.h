#pragma once

#include <glad/glad.h>
#include <iostream>

#include "../Renderer.h"

class G_Buffer {
public:
	G_Buffer(unsigned int width, unsigned int height);
	void Bind();
	void BindRead();
	void BindTextures();
	void GeometryPass(const Shader& shader);
private:
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec, rboDepth;
};