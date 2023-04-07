#pragma once

#include <glad/glad.h>
#include <iostream>

class G_Buffer {
public:
	G_Buffer(unsigned int width, unsigned int height);
	void Bind();
	void BindRead();
	void BindTextures();
private:
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec, rboDepth;
};