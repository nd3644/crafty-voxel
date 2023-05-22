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

#include "input.h"

#include "sprite.h"

#include <chrono>

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#define BUFFER_OFFSET(i) ((void*)(i))

SDL_Window* myWindow;
SDL_GLContext myGLContext;
SDL_Event myEvent;

extern void Init();
extern void Cleanup();
extern void CompileArr();

extern void DrawMiniMap();
extern void DrawDebugUI();
extern void DrawCursor();

bool bEnableVSync = true;
int frameCounter = 0;
int fpsTimer = SDL_GetTicks();
int lastAvgFps = 0;
std::chrono::milliseconds MapDrawDuration;

Camera myCamera;
Map myMap(myCamera);

Shader myShader, myShader2D;
int main(int argc, char* args[]) {

	CompileArr();

    Eternal::InputHandle myInputHandle;

	myShader.projMatrix = glm::mat4(1);
	myShader.viewMatrix = glm::mat4(1);
	myShader.modelMatrix = glm::mat4(1);

//	projMatrix = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 50.0f);

    myShader2D.projMatrix = glm::mat4(1);//glm::ortho(0,800,600,0);
    myShader2D.modelMatrix = glm::mat4(1);
    myShader2D.viewMatrix = glm::mat4(1);

    myShader.projMatrix = glm::perspective(glm::radians(70.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

	Init();

    myMap.FromBMP("textures/heightmap.bmp");

    Eternal::Sprite spr;
    spr.Load("textures/cursor.png");
    
	int iTicks = SDL_GetTicks();
	float fdelta = 0.0f;
	bool bDone = false;
	while (bDone == false) {
		fdelta += 0.01f;
		while (SDL_PollEvent(&myEvent)) {
            ImGui_ImplSDL2_ProcessEvent(&myEvent);
			if ((myEvent.type == SDL_WINDOWEVENT && myEvent.window.event == SDL_WINDOWEVENT_CLOSE) || SDL_GetKeyboardState(0)[SDL_SCANCODE_GRAVE]) {
				bDone = true;
			}
		}
        myInputHandle.PollInputs();

        glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texID);

        myShader.projMatrix = glm::ortho(0,800,600,0);

        //myShader.projMatrix = glm::perspective(glm::radians(70.0f), 800.0f / 600.0f, 1.0f, 1000.0f);

        myShader.Bind();

        myCamera.Update(myMap, myShader, myInputHandle);

        auto start = std::chrono::high_resolution_clock::now();
        myMap.RebuildAll();
        myMap.Draw();
        auto end = std::chrono::high_resolution_clock::now();
        MapDrawDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        glBindVertexArray(myVAO);
		glDrawArrays(GL_TRIANGLES, 0, vertList.size());

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Mesh m;
        m.SetTranslation((int)myCamera.targetted_brick.x,(int)myCamera.targetted_brick.y,(int)myCamera.targetted_brick.z);
        for(int i = 0;i < vertList.size();i++) {
            m.Index1(1); m.Vert3(vertList[i].x, vertList[i].y, vertList[i].z+0.5f);
            m.TexCoord2(uvList[i].x, uvList[i].y);
            m.Color4(10,10,10,0.25f);
        }
        m.Draw(Mesh::MODE_TRIANGLES);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 2D rendering
        myShader2D.Bind();
        DrawCursor();


        DrawDebugUI();

        if(SDL_GetTicks() - fpsTimer > 1000) {
            fpsTimer = SDL_GetTicks();
            lastAvgFps = frameCounter;
            frameCounter = 0;
        }

        DrawMiniMap();

		SDL_GL_SwapWindow(myWindow);

        frameCounter++;
	}

	Cleanup();
	return 0;
}

void SetupImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(myWindow, myGLContext);
    ImGui_ImplOpenGL3_Init();
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
    SDL_GL_MakeCurrent(myWindow, myGLContext);
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

    SetupImgui();

	glClearColor(0, 0, 0.25f, 0);   

    // TODO: Clean this up

    // Init shader
    myShader.Initialize("shaders/terrain.vs", "shaders/terrain.fs");
	GLuint model = glGetUniformLocation(myShader.myProgram, "Model");
	glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(myShader.modelMatrix));

	GLuint view = glGetUniformLocation(myShader.myProgram, "View");
	glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(myShader.viewMatrix));

	GLuint proj = glGetUniformLocation(myShader.myProgram, "Proj");
	glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(myShader.projMatrix));

    // Init 2D shader
    myShader2D.Initialize("shaders/default.vs", "shaders/default.fs");
    model = glGetUniformLocation(myShader.myProgram, "Model");
	glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(myShader2D.modelMatrix));

	view = glGetUniformLocation(myShader.myProgram, "View");
	glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(myShader2D.viewMatrix));

	proj = glGetUniformLocation(myShader.myProgram, "Proj");
	glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(myShader2D.projMatrix));

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // glPolygonMode(GL_FRONT, GL_LINE);
    // glPolygonMode(GL_BACK, GL_LINE);
}

void Cleanup() {

    // Imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Data
	glDeleteBuffers(1, &myArrBuffer);
	glDeleteBuffers(1, &myTexArrBuffer);
	glDeleteVertexArrays(1, &myVAO);

	glDeleteTextures(1, &texID);

    // SDL/GL
	SDL_GL_DeleteContext(myGLContext);
	SDL_DestroyWindow(myWindow);

	SDL_Quit();
}

void DrawCursor() {
    Mesh myMesh;

    glDisable(GL_TEXTURE_2D);

    glPointSize(2);
    for(int i = 0;i < 3;i++) {
        myMesh.Color4(1,1,1,1);
        myMesh.Index1(1);
        myMesh.Vert3(0,0,0);
        myMesh.TexCoord2(0,0);
    }
    myMesh.Draw(Mesh::MODE_POINTS);
}

void DrawMiniMap() {
    Mesh myMesh;

    glDisable(GL_TEXTURE_2D);

    int level = (int)myCamera.position.y-1;

    glPointSize(4);
    for(int x = 0;x < 256;x++) {
        for(int z = 0;z < 256;z++) {
            for(int i = 0;i < 3;i++) { // lol
            myMesh.SetTranslation(-200,-200,0);
                if(x == (int)myCamera.position.x && z == (int)myCamera.position.z) {
                    myMesh.Color4(1,0.2,0.2,1);
                    myMesh.Index1(1);
                    myMesh.Vert3(x*4,z*4,0);
                    myMesh.TexCoord2(0,0);
                }
                else {
                    if(myMap.GetBrick(x,z,level) != 0) {
                        myMesh.Color4(0.2f,0.2f,0.2f,1);
                        myMesh.Index1(1);
                        myMesh.Vert3(x*4,z*4,0);
                        myMesh.TexCoord2(0,0);
                    }
                }
            }
        }
    }
//    myMesh.Draw(Mesh::MODE_POINTS);
}

void DrawDebugUI() {
    glDisable(GL_DEPTH_TEST);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();


    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(-1, 256));
    // ImGui content goes here

    bool open = true;
    ImGui::Begin("Debug",&open, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    std::string str = "MapDraw: " + std::to_string(MapDrawDuration.count()) + "ms";
    ImGui::Text(str.c_str());
    str = "avg fps: " + std::to_string(lastAvgFps);
    ImGui::Text(str.c_str());
    str = "CamXYZ: (" + std::to_string(myCamera.position.x) + " , " + std::to_string(myCamera.position.x) + " , " + std::to_string(myCamera.position.z) + ")";
    ImGui::Text(str.c_str());

    ImGui::Separator();
    ImGui::Checkbox("V-Sync", &bEnableVSync);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SDL_GL_SetSwapInterval(bEnableVSync ? 1 : 0);
        std::cout << "b: " << bEnableVSync << std::endl;
    }
    ImGui::End();

    ImGui::Render();
    glViewport(0, 0, 800, 600);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}