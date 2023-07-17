#include "shader.h"

#include <iostream>
#include <fstream>

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
        exit(1);
		return true;
	}
	return false;
}


Shader::Shader() {
    projMatrix = modelMatrix = viewMatrix = glm::mat4(1);
}

Shader::~Shader() {
    if(!bLoaded) {
        return;
    }
    glDetachShader(myProgram, myShaders[0]);
    glDetachShader(myProgram, myShaders[1]);

    glDeleteShader(myShaders[0]);
    glDeleteShader(myShaders[1]);
    glDeleteProgram(myProgram);
}

void Shader::Initialize() {
    std::clog << "(initialized with default shaders)" << std::endl;
    Initialize("shaders/basic_shader.vs", "shaders/basic_shader.fs");
}

void Shader::SetUniformVec3f(std::string name, float f[3]) {
    GLint location = glGetUniformLocation(myProgram, name.c_str());
    glProgramUniform3f(myProgram, location, f[0], f[1], f[2]);
}

std::string Shader::FileToString(std::string sfilename) {

    std::string output;
    std::ifstream infile;
    std::string sline;
    // Load shaders from file
    infile.open(sfilename);
    if(!infile) {
        std::cerr << "\tERROR: couldn't open shader file" << std::endl;
        exit(1);
    }
    else {
        std::clog << "\tattempting to read from " << sfilename << std::endl;
        while(std::getline(infile, sline)) {
            output += sline + "\n";
        }
    }
    infile.close();
    std::cout << std::endl;

    if(output.length() <= 1) {
        std::cerr << "ERROR: shader source <= 1 length\n" << std::endl;
        exit(1);
    }

    return output;
}

void Shader::CompileShader(Type shaderType, std::string source) {
     // Compile the vertex shader
	const char *shaderSource = source.c_str();
	GLint shaderLength = source.length();
    
	glShaderSource(myShaders[shaderType], 1, &shaderSource, &shaderLength);
	glCompileShader(myShaders[shaderType]);

    GLchar buffer[256];
    GLsizei size;
	glGetShaderInfoLog(myShaders[shaderType], 256, &size, buffer);
    std::clog << "Shader LOG: " << buffer << std::endl;
}

void Shader::Initialize(std::string sVertexFile, std::string sFragmentFile) {
    std::clog << "*Trying to initialize shaders with " << sVertexFile << " and" << sFragmentFile <<  std::endl;

    // To store shader code
    std::string sVertex, sFragment;

    // Pull the source from files
    sVertex = FileToString(sVertexFile);
    sFragment = FileToString(sFragmentFile);

    // Debug print out the source 
    //std::clog << "vertex shader source: " << sVertex << std::endl;
    //std::clog << "fragment shader source: " << sFragment << std::endl;

    // Create the shaders
    myShaders[Type::SHADER_VERTEX] = glCreateShader(GL_VERTEX_SHADER);
    myShaders[Type::SHADER_FRAGMENT] = glCreateShader(GL_FRAGMENT_SHADER);

    // Check if shaders could be created
    if(myShaders[Type::SHADER_VERTEX] == 0)
        std::cerr << "Couldn't create vertex shader" << std::endl;

    if(myShaders[Type::SHADER_FRAGMENT] == 0)
        std::cerr << "Couldn't create frag shader" << std::endl;

    // Compile the vertex shader
	CompileShader(Type::SHADER_VERTEX, sVertex);

	// Compile frag shader
    CompileShader(Type::SHADER_FRAGMENT, sFragment);

    // Generate shader
    myProgram = glCreateProgram();

	// attach shaders
	glAttachShader(myProgram, myShaders[0]);
	glAttachShader(myProgram, myShaders[1]);

	glLinkProgram(myProgram);
	glValidateProgram(myProgram);

    Bind();
    UpdateUniforms();

    bLoaded = true;

    std::clog << "*Successfully loaded shader" << std::endl;
}

void Shader::Bind() {
    glUseProgram(myProgram);
    UpdateUniforms();
}

void Shader::UpdateUniforms() {
    GLuint model = glGetUniformLocation(myProgram, "Model");
	glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	GLuint view = glGetUniformLocation(myProgram, "View");
	glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	GLuint proj = glGetUniformLocation(myProgram, "Proj");
	glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(projMatrix));
}

