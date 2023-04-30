#include "FboManager.h"

// constructor
FboManager::FboManager(unsigned int SCR_WIDTH, unsigned int SCR_HEIGHT) {
	// FBO
	// stores the scene in tcbo[0] and the bloom in tcbo[1], they are blended together in 4.5.
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// TCBO[2]
	// genereate and attach to framebuffer object (fbo)
	glGenTextures(2, tcbo);
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, tcbo[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tcbo[i], 0);
	}

	// RBO
	// depth and stencil buffer object
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	// allocate storage and unbind
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// attach to the framebuffer object (fbo)
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	// tell opengl which color attachments to use for rendering (of this framebuffer, fbo)
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// PINGPONG FBOs & BUFFERS
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);

	for (unsigned int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
	}
}

// PUBLIC
void FboManager::ApplyGlow(Shader shader) {
	shader.Use();
	shader.SetInt("weightType", weightType);
	horizontal = true;
	bool firstIteration = true;

	for (int i = 0; i < glow; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		shader.SetInt("horizontal", horizontal);
		// bind texutre of other framebuffer, or the texture to blur if first iteration
		glBindTexture(GL_TEXTURE_2D, firstIteration ?
			tcbo[1] : pingpongBuffer[!horizontal]);
		// render quad
		RenderQuad();
		// swap buffers
		horizontal = !horizontal;
		if (firstIteration)
			firstIteration = false;
	}
}

void FboManager::PostProcessingGUI() {
	static const char* weightNames[2] = { "Gaussian", "Custom" };

	const ImVec2 startPos = ImVec2(575, 119);
	ImGui::SetNextWindowPos(startPos, ImGuiCond_Once);
	ImGui::Begin("Post Processing", NULL, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Separator();
	ImGui::Text("Glow");
	ImGui::Checkbox("##glowEnabled", &glowEnabled);
	if (glowEnabled) {
		ImGui::SliderInt("##glow", &glow, 1, 20);
		ImGui::Text("Weight Type");
		ImGui::Combo("", &weightType, weightNames, 2);
	}

	ImGui::Separator();
	ImGui::Text("Gamma Correction");
	ImGui::Checkbox("##gammaEnabled", &gammaCorrectionEnabled);
	if (gammaCorrectionEnabled) {
		ImGui::SliderFloat("##gamma", &gamma, 0.1f, 5.0f, "%.1f");
	}

	ImGui::Separator();
	ImGui::Text("Exposure");
	ImGui::Checkbox("###exposureEnabled", &exposureEnabled);
	if (exposureEnabled) {
		ImGui::SliderFloat("##exposure", &exposure, 0.1f, 30, "%.1f");
	}

	ImGui::End();
}

bool FboManager::GetGlowEnabled() {
	return glowEnabled;
}

unsigned int FboManager::GetFbo() {
	return fbo;
}

void FboManager::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void FboManager::BindDraw() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}

void FboManager::PrepareScreenShader(Shader* shader) {
	BindSceneAndGlow();
	SetScreenShaderUniforms(shader);
}

void FboManager::OutputBuffers() {
	std::cout << "fbo: " << fbo << std::endl;
	std::cout << "tcbo[0]: " << tcbo[0] << std::endl;
	std::cout << "tcbo[1]: " << tcbo[1] << std::endl;
	std::cout << "rbo: " << rbo << std::endl;
	std::cout << "pingpongFBO[0]: " << pingpongFBO[0] << std::endl;
	std::cout << "pingpongFBO[1]: " << pingpongFBO[1] << std::endl;
	std::cout << "pingpongBuffer[0]: " << pingpongBuffer[0] << std::endl;
	std::cout << "pingpongBuffer[1]: " << pingpongBuffer[1] << std::endl;
}

// PRIVATE
void FboManager::SetScreenShaderUniforms(Shader* shader) {

	shader->SetBool("bloomEnabled", glowEnabled);

	shader->SetBool("gammaEnabled", gammaCorrectionEnabled);
	shader->SetFloat("gamma", gamma);
	shader->SetBool("exposureEnabled", exposureEnabled);
	shader->SetFloat("exposure", exposure);

}

void FboManager::BindSceneAndGlow() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tcbo[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
}

