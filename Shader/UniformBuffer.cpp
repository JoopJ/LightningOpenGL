#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const char* _name, int _bindingPoint, unsigned int _byteSize) {
	name = _name;
	bindingPoint = _bindingPoint;
	byteSize = _byteSize;

	glGenBuffers(1, &ID);
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	glBufferData(GL_UNIFORM_BUFFER, byteSize, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::SetBindingPoint(Shader* shader) {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	int index = glGetUniformBlockIndex(shader->ID, name);
	glUniformBlockBinding(shader->ID, index, bindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ID);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::Bind() {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
}