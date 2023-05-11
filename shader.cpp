#include "shader.h"

#include <iostream>
#include <fstream>

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


// New code
constexpr char fs[] =
"#version 430\n"

"in vec2 texCoord0;\n"
"in vec4 color0;\n"

"uniform sampler2D tex1;\n"

"out vec4 diffuseColor;\n"
"void main()\n"
"{\n"
"	if(textureSize(tex1, 0).x > 1) {\n"
"		diffuseColor = texture(tex1, texCoord0) * color0;\n"
"	}\n"
"	else {\n"
"		diffuseColor = color0;\n"
"	}\n"
"}";

constexpr char vs[] = 
"#version 430\n"

"layout(location = 0) in vec2 InPos;\n"
"layout(location = 1) in vec2 InTexCoord;\n"
"layout(location = 2) in vec4 InColor;\n"

"out vec2 texCoord0;\n"
"out vec4 color0;\n"

"void main()\n"
"{\n"
"	gl_Position = vec4(vec3(InPos.x - 1.0, 1.0 - InPos.y, 0), 1.0);\n"
"	texCoord0 = InTexCoord;\n"
"	color0 = InColor;\n"
"}";


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

void Shader::Initialize(std::string sVertexFile, std::string sFragmentFile) {

    std::clog << "*Trying to initialize shaders with " << sVertexFile << " and" << sFragmentFile <<  std::endl;
    std::ifstream infile;
    std::string sline;
    std::string sVertex, sFragment;

    // Load shaders from file
    infile.open(sVertexFile);
    if(!infile) {
        std::cerr << "\tERROR: couldn't open VS file" << std::endl;
        std::clog << "\ttrying default vertex shader" << std::endl;
        sVertex = vs;
    }
    else {
        std::clog << "\tattempting to read from " << sVertexFile << std::endl;
    //    sVertex = vs;
        while(std::getline(infile, sline)) {
            sVertex += sline + "\n";
        }
    }
    infile.close();
    std::cout << std::endl;

    infile.open(sFragmentFile);
    if(!infile) {
        std::cerr << "\tERROR: couldn't open FS file" << std::endl;
        std::clog << "\ttrying default fragment shader" << std::endl;
        sFragment = fs;
    }
    else {
        while(std::getline(infile, sline)) {
            sFragment += sline + "\n";
        }
    }
    infile.close();

    if(sVertex.length() <= 1) {
        std::cerr << "ERROR: Vertex source <= 1 length\n\tUsing default" << std::endl;
        sVertex = vs;
    }
    if(sFragment.length() <= 1) {
        std::cerr << "ERROR: Fragment source <= 1 length\n\tUsing default" << std::endl;
        sFragment = fs;
    }

    std::clog << "vertex shader source: " << sVertex << std::endl;
    std::clog << "fragment shader source: " << sFragment << std::endl;

    /* Generate shaders */
    myProgram = glCreateProgram();

    myShaders[Type::SHADER_VERTEX] = glCreateShader(GL_VERTEX_SHADER);
    myShaders[Type::SHADER_FRAGMENT] = glCreateShader(GL_FRAGMENT_SHADER);
    if(myShaders[Type::SHADER_VERTEX] == 0)
        std::cerr << "Couldn't create vertex shader" << std::endl;

    if(myShaders[Type::SHADER_FRAGMENT] == 0)
        std::cerr << "Couldn't create frag shader" << std::endl;

	// Compile vert shader
    const GLchar *p[1];
	GLint lengths[1];

//    sVertex = vs;
	p[0] = sVertex.c_str();
	lengths[0] = sVertex.length();

	glShaderSource(myShaders[Type::SHADER_VERTEX], 1, p, lengths);
	glCompileShader(myShaders[Type::SHADER_VERTEX]);

    GLchar buffer[256];
    GLsizei size;
	glGetShaderInfoLog(myShaders[Type::SHADER_VERTEX], 256, &size, buffer);
    std::clog << "\tSHADER_VERTEX LOG: " << buffer << std::endl;

	// Compile frag shader
	const GLchar *ap[1];
	GLint alengths[1];

//    sFragment = fs;
	ap[0] = sFragment.c_str();
	alengths[0] = sFragment.length();

	glShaderSource(myShaders[Type::SHADER_FRAGMENT], 1, ap, alengths);
	glCompileShader(myShaders[Type::SHADER_FRAGMENT]);
    glGetShaderInfoLog(myShaders[Type::SHADER_FRAGMENT], 256, &size, buffer);
    std::clog << SHADER_FRAGMENT << "ShaderLog result: " << buffer << std::endl;

	// attach shaders
	glAttachShader(myProgram, myShaders[0]);
	glAttachShader(myProgram, myShaders[1]);

	glLinkProgram(myProgram);
	glValidateProgram(myProgram);

    bLoaded = true;

    std::clog << "*Successfully loaded shader" << std::endl;
}

void Shader::Bind() {
    glUseProgram(myProgram);
    tex[0] = glGetUniformLocation(myProgram, "tex1");
    glUniform1i(tex[0], 0);
    tex[1] = glGetUniformLocation(myProgram, "tex2");
    glUniform1i(tex[1], 1);
    tex[2] = glGetUniformLocation(myProgram, "tex3");
    glUniform1i(tex[2], 2);
    tex[3] = glGetUniformLocation(myProgram, "tex4");
    glUniform1i(tex[3], 3);
    tex[4] = glGetUniformLocation(myProgram, "tex5");
    glUniform1i(tex[4], 4);
}