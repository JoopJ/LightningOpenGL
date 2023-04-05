#pragma once

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"

class UniformBuffer {

public:
	UniformBuffer(const char* name, int bindingPoint, unsigned int byteSize);
	void SetBindingPoint(Shader* shader);
	void Bind();
private:
	unsigned int ID;
	const char* name;
	unsigned int byteSize;
	int bindingPoint;
};