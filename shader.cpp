#include "shader.h"

#include <iostream>
#include <fstream>

//GLuint myProgram;

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
        exit(1);
		return true;
	}
	return false;
}

std::string VertShaderStr = "#version 430 core\n\
							layout(location = 0) in vec3 vPosition;\n\
							layout(location = 1) in vec2 vTexCoord;\n\
                            layout(location = 2) in vec4 vColor;\n\
							out vec2 texCoord0;\n\
                            out vec4 color0;\n\
							uniform mat4 View;\n\
							uniform mat4 Model;\n\
							uniform mat4 Proj;\n\
							void main() \n\
							{ \n\
								texCoord0 = vec2(vTexCoord.x, vTexCoord.y); \n\
                                color0 = vColor; \n\
								gl_Position = Proj * View * Model * vec4(vPosition.x, vPosition.y, vPosition.z, 1);\n\
                                //gl_Position = vec4(vPosition.x, vPosition.y, vPosition.z, 1);\n\
							}\n";
std::string FragShaderStr = "#version 430 core\n\
							in vec2 texCoord0;\n\
                            in vec4 color0; \n\
							uniform sampler2D tex;\n\
							out vec4 fColor;\n\
							void main() \n\
							{ \n\
                                fColor = texture(tex, texCoord0) * color0; \n\
							}\n";

Shader::Shader() {
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

    bLoaded = true;

    std::clog << "*Successfully loaded shader" << std::endl;
}

void Shader::Bind() {
    glUseProgram(myProgram);
    tex[0] = glGetUniformLocation(myProgram, "tex1");
}