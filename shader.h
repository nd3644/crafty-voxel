#ifndef SHADER_H
#define SHADER_H

#include <SDL.h>
#include <GL/glew.h>
#include <iostream>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

extern GLuint myProgram;

bool CheckShaderErrors(GLuint shader);
bool CheckProgramLinkErrors(GLuint program);
void SetSolidFlag(int i);

extern glm::mat4 projMatrix;
extern glm::mat4 viewMatrix;
extern glm::mat4 modelMatrix;

extern GLuint texID;
extern GLuint myVAO;
extern GLuint myArrBuffer, myTexArrBuffer;

#endif

