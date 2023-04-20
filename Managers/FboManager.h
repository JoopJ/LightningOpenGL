#pragma once

#include <glad/glad.h>
#include <iostream>

#include "../Shader/Shader.h"
#include "../Renderer.h"

class FboManager {
public:
	FboManager(unsigned int width, unsigned int height);
	unsigned int GetFbo();
	void Bind();
	void BindDraw();
	void BindSceneAndBloom();
	bool ApplyBloom(Shader shader, int amount);
	void OutputBuffers();
private:
	unsigned int fbo;
	unsigned int tcbo[2];
	unsigned int rbo;
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];

	bool horizontal = true;

};