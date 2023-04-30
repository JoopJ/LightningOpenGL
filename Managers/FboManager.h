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
	void PrepareScreenShader(Shader* shader);
	void ApplyGlow(Shader shader);
	void OutputBuffers();
	bool GetGlowEnabled();
	void PostProcessingGUI();
private:
	// Buffer Objects
	unsigned int fbo;
	unsigned int tcbo[2];
	unsigned int rbo;
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];

	// Glow
	bool glowEnabled = true;
	int glow = 4;
	int weightType = 0;

	// Gamma & Exposure
	bool gammaCorrectionEnabled = true;
	float gamma = 2.2f;
	bool exposureEnabled = true;
	float exposure = 1.0f;


	bool horizontal = true;

	void BindSceneAndGlow();
	void SetScreenShaderUniforms(Shader* shader);

};