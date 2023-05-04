#include "shader.h"

GLuint myProgram;

glm::mat4 projMatrix;
glm::mat4 viewMatrix;
glm::mat4 modelMatrix;

GLuint texID;
GLuint myVAO;
GLuint myArrBuffer, myTexArrBuffer;

bool CheckShaderErrors(GLuint shader) {
	GLint myReturn = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &myReturn);
	if (myReturn == GL_FALSE) {
		GLchar buffer[512];
		GLint size = 0;
		glGetShaderInfoLog(shader, 512, &size, buffer);
		return true;
	}
	return false;
}

bool CheckProgramLinkErrors(GLuint program) {
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE) {
		GLint size = 0;
		GLchar buffer[512];
		std::cout << "link error" << std::endl;

		glGetProgramInfoLog(program, 512, &size, buffer);

		std::cout << buffer << std::endl;
		return true;
	}
	return false;
}

void SetSolidFlag(int i) {
	glUniform1i(myProgram, i);
}