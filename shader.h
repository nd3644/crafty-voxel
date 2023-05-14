#ifndef SHADER_H
#define SHADER_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//extern GLuint myProgram;

bool CheckShaderErrors(GLuint shader);
bool CheckProgramLinkErrors(GLuint program);
void SetSolidFlag(int i);

extern glm::mat4 projMatrix;
extern glm::mat4 viewMatrix;
extern glm::mat4 modelMatrix;

extern GLuint texID;
extern GLuint myVAO;
extern GLuint myArrBuffer, myTexArrBuffer;

class Shader {
    public:

        ~Shader();
        Shader();
        enum Type {
            SHADER_VERTEX = 0,
            SHADER_FRAGMENT
        };

        void SetUniformVec3f(std::string name, float f[3]);
        void Initialize();
        void Initialize(std::string sVertexFile, std::string sFragmentFile);
        void Bind();

    private:
        std::string FileToString(std::string filename);
        void CompileShader(Type shaderType, std::string source);

    public:
        GLuint myShaders[2];
        GLuint myProgram;
        GLint tex[4];
        bool bLoaded;
};

#endif

