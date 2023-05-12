#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "perlin.h"
#include "map.h"
#include "shader.h"
#include "cube.h"
#include "camera.h"

#include "mesh.h"

#include <chrono>


#define BUFFER_OFFSET(i) ((void*)(i))

SDL_Window* myWindow;
SDL_GLContext myGLContext;
SDL_Event myEvent;

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

extern void Init();
extern void Cleanup();
extern void CompileArr();

int main(int argc, char* args[]) {

	Map myMap;
    Camera myCamera;
	myMap.FromBMP("textures/heightmap.bmp");

	CompileArr();

	projMatrix = glm::mat4(1);
	viewMatrix = glm::mat4(1);
	modelMatrix = glm::mat4(1);

//	projMatrix = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 50.0f);
    projMatrix = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

	Init();

	int iTicks = SDL_GetTicks();
	float fdelta = 0.0f;

	bool bDone = false;
	while (SDL_GetTicks() - iTicks < 100000 && bDone == false) {
		fdelta += 0.01f;
		while (SDL_PollEvent(&myEvent)) {
			if ((myEvent.type == SDL_WINDOWEVENT && myEvent.window.event == SDL_WINDOWEVENT_CLOSE) || SDL_GetKeyboardState(0)[SDL_SCANCODE_ESCAPE]) {
				bDone = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texID);

        projMatrix = glm::ortho(0,1,1,0);

        viewMatrix = glm::mat4(1);
        modelMatrix = glm::mat4(1);
        projMatrix = glm::mat4(1);

        Mesh myMesh;

        myMesh.Color4(1,1,1,1);myMesh.Color4(1,1,1,1);myMesh.Color4(1,1,1,1);
        myMesh.TexCoord2(0,0);
        myMesh.TexCoord2(1,0);
        myMesh.TexCoord2(1,1);
        
        myMesh.Vert3(0,0,0);
        myMesh.Vert3(64,0,0);
        myMesh.Vert3(64,64,0);

        projMatrix = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

        myCamera.Update(myMap);
        auto start = std::chrono::high_resolution_clock::now();
        myMap.Draw();
        auto end = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

//        std::cout << delta.count() << std::endl;

		SDL_GL_SwapWindow(myWindow);
	}

	Cleanup();
	return 0;
}

void Init() {
	std::cout << "trying init---\n";
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Problem SDL_Init()" << std::endl;
		std::cout << "\t *" << SDL_GetError() << std::endl;
		exit(0);
	}

	myWindow = SDL_CreateWindow("glm", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
	if (myWindow == NULL) {
		std::cout << "Couldn't create window" << std::endl;
		std::cout << "\t *" << SDL_GetError() << std::endl;
		exit(0);
	}

	myGLContext = SDL_GL_CreateContext(myWindow);
	if (myGLContext == NULL) {
		std::cout << "Problem creating GL context" << std::endl;
		std::cout << "\t *" << SDL_GetError() << std::endl;
		exit(0);
	}

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Couldn't init GLEW" << std::endl;
	}

	SDL_GL_SetSwapInterval(1);

	glClearColor(0, 0, 0.25f, 0);

	// compile vert shader
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* source = VertShaderStr.c_str();

	glShaderSource(vertShader, 1, &source, 0);
	glCompileShader(vertShader);
	CheckShaderErrors(vertShader);

	// compile frag shader
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	source = FragShaderStr.c_str();
	glShaderSource(fragShader, 1, &source, 0);
	glCompileShader(fragShader);
	CheckShaderErrors(fragShader);

	// Create program
	myProgram = glCreateProgram();

	glAttachShader(myProgram, vertShader);
	glAttachShader(myProgram, fragShader);

	glLinkProgram(myProgram);
	CheckProgramLinkErrors(myProgram);

	glUseProgram(myProgram);

	// Set up the uniforms
	GLuint model = glGetUniformLocation(myProgram, "Model");
	glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	GLuint view = glGetUniformLocation(myProgram, "View");
	glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	GLuint proj = glGetUniformLocation(myProgram, "Proj");
	glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(projMatrix));

	// Setup vertex array & attach buffer
	glGenVertexArrays(1, &myVAO);
	glBindVertexArray(myVAO);

	glGenBuffers(1, &myArrBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, myArrBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * vertList.size(), &vertList[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &myTexArrBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, myTexArrBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2_t) * uvList.size(), &uvList[0], GL_STATIC_DRAW);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	// Load the texture
	SDL_Surface* surf = IMG_Load("textures/grass.png");
	if (surf == NULL) {
		std::cout << "Couldn't load logo.bmp" << std::endl;
		std::cout << "\t *" << SDL_GetError() << std::endl;
		exit(0);
	}

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, surf->w, surf->h, 0, GL_BGR, GL_UNSIGNED_BYTE, surf->pixels);

	SDL_FreeSurface(surf);

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //glPolygonMode(GL_FRONT, GL_LINE);
   // glPolygonMode(GL_BACK, GL_LINE);

}

void Cleanup() {

	glDeleteBuffers(1, &myArrBuffer);
	glDeleteBuffers(1, &myTexArrBuffer);
	glDeleteVertexArrays(1, &myVAO);

	glDeleteTextures(1, &texID);

	SDL_GL_DeleteContext(myGLContext);
	SDL_DestroyWindow(myWindow);

	SDL_Quit();
}
