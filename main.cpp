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

#include "globals.h"

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

extern void DrawMiniMap(Camera &myCamera, Map &myMap);
extern void DrawDebugUI(Camera &myCamera);
extern void DrawCursor();
extern void DrawBrickTarget(Camera &myCamera);

bool bEnableVSync = true;
int frameCounter = 0;
int fpsTimer = SDL_GetTicks();
int lastAvgFps = 0;
std::chrono::milliseconds CameraUpdateDuration;
std::chrono::milliseconds MapDrawDuration, MapBuildDuration, FrameTime;

bool bDebugMenuOpen = true;

Eternal::Sprite cursorTex;

Shader myShader, myShader2D;

int main(int argc, char* args[]) {
	CompileArr();

    Eternal::InputHandle myInputHandle;

	myShader.projMatrix = glm::mat4(1);
	myShader.viewMatrix = glm::mat4(1);
	myShader.modelMatrix = glm::mat4(1);

    myShader2D.projMatrix = glm::mat4(1);
    myShader2D.modelMatrix = glm::mat4(1);
    myShader2D.viewMatrix = glm::mat4(1);
	Init();

    Camera myCamera;
    Map myMap(myCamera);

    myMap.FromBMP("textures/heightmap.bmp");
    
	int iTicks = SDL_GetTicks();
	float fdelta = 0.0f;
	bool bDone = false;

    static int counter = 0;
	while (bDone == false) {
        auto frameStartTime = std::chrono::high_resolution_clock::now();
        if(counter++ == 30) {
            //exit(1);
        }
		fdelta += 0.01f;
		while (SDL_PollEvent(&myEvent)) {
            ImGui_ImplSDL2_ProcessEvent(&myEvent);
			if ((myEvent.type == SDL_WINDOWEVENT && myEvent.window.event == SDL_WINDOWEVENT_CLOSE) || SDL_GetKeyboardState(0)[SDL_SCANCODE_GRAVE]) {
				bDone = true;
			}
            if(myEvent.type == SDL_KEYDOWN && myEvent.key.keysym.scancode == SDL_SCANCODE_F11) {
                bIsFullscreen = !bIsFullscreen;
                SDL_SetWindowFullscreen(myWindow, bIsFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                SDL_GetWindowSize(myWindow, &WIN_W, &WIN_H);
            }
		}
        myInputHandle.PollInputs();

        glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.projMatrix = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

        myShader.Bind();

        // Update the camera
        auto start = std::chrono::high_resolution_clock::now();
        myCamera.Update(myMap, myShader, myInputHandle);
        auto end = std::chrono::high_resolution_clock::now();
        CameraUpdateDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);


        // Build the map
        start = std::chrono::high_resolution_clock::now();
        myMap.RunBuilder();
        end = std::chrono::high_resolution_clock::now();
        MapBuildDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Draw the map
        start = std::chrono::high_resolution_clock::now();
        myMap.Draw();
        end = std::chrono::high_resolution_clock::now();
        MapDrawDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        DrawBrickTarget(myCamera);

        // 2D rendering
        myShader2D.projMatrix = glm::ortho(0.0f,(float)WIN_W,(float)WIN_H,0.0f,-100.0f,100.0f);
        myShader2D.Bind();
        DrawCursor();

        DrawDebugUI(myCamera);
//        DrawMiniMap();

        if(SDL_GetTicks() - fpsTimer > 1000) {
            fpsTimer = SDL_GetTicks();
            lastAvgFps = frameCounter;
            frameCounter = 0;
        }

        SDL_GL_SwapWindow(myWindow);

        frameCounter++;
        gblPolyCount = 0;

        auto frameEndTime = std::chrono::high_resolution_clock::now();
        FrameTime = std::chrono::duration_cast<std::chrono::milliseconds>(frameEndTime - frameStartTime);
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

	myWindow = SDL_CreateWindow("glm", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
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

	glClearColor(0.494, 0.752, 0.933f, 0);   

    // Init shader
    myShader.Initialize("shaders/terrain.vs", "shaders/terrain.fs");

    // Init 2D shader
    myShader2D.Initialize("shaders/default.vs", "shaders/default.fs");

    cursorTex.Load("textures/cursor.png");

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

    // SDL/GL
	SDL_GL_DeleteContext(myGLContext);
	SDL_DestroyWindow(myWindow);

	SDL_Quit();
}

void DrawCursor() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    Mesh myMesh;
    glPointSize(6);
    for(int i = 0;i < 3;i++) {
        myMesh.SetTranslation(i*16,i*16,0);
        myMesh.Color4(1,1,1,1);
        myMesh.Index1(1);
        myMesh.Vert3(400,600,0);
        myMesh.TexCoord2(0,0);
    }
//    myMesh.Draw(Mesh::MODE_POINTS);
    
    Rect c;
    c.x = c.y = 0; c.w = c.h = 16;
    Rect r;
    r.x = (WIN_W/2) - 8;
    r.y = (WIN_H/2) - 8;
    r.w = r.h = 16;
    cursorTex.Bind();
    cursorTex.Draw(r,c);
}

void DrawMiniMap(Camera &myCamera, Map &myMap) {
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
    myMesh.Draw(Mesh::MODE_POINTS);
}

void DrawBrickTarget(Camera &myCamera) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Mesh m;
    m.SetTranslation((int)myCamera.targetted_brick.x,(int)myCamera.targetted_brick.y,(int)myCamera.targetted_brick.z);
    for(int i = 0;i < vertList.size();i++) {
        m.Index1(1); m.Vert3(vertList[i].x, vertList[i].y, vertList[i].z+0.5f);
        m.TexCoord2(uvList[i].x, uvList[i].y);
        m.Color4(0,0,0,1);
    }
    m.Draw(Mesh::MODE_TRIANGLES);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DrawDebugUI(Camera &myCamera) {
    glDisable(GL_DEPTH_TEST);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(-1, 256));
    // ImGui content goes here

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));  // Set text color to red
    ImGui::Begin("Debug",&bDebugMenuOpen, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    std::string str = "MapDraw: " + std::to_string(MapDrawDuration.count()) + "ms";
    ImGui::Text(str.c_str());
    str = "MapBuild: " + std::to_string(MapBuildDuration.count()) + "ms";
    ImGui::Text(str.c_str());
    str = "CamUpdate: " + std::to_string(CameraUpdateDuration.count()) + "ms";
    ImGui::Text(str.c_str());
    str = "FrameTime: " + std::to_string(FrameTime.count()) + "ms";
    ImGui::Text(str.c_str());
    str = "avg fps: " + std::to_string(lastAvgFps);
    ImGui::Text(str.c_str());
    str = "polycount: : " + std::to_string(gblPolyCount);
    ImGui::Text(str.c_str());
    str = "CamXYZ: (" + std::to_string(myCamera.position.x) + " , " + std::to_string(myCamera.position.y) + " , " + std::to_string(myCamera.position.z) + ")";
    ImGui::Text(str.c_str());
    str = "ChunkXYZ: (" + std::to_string((int)myCamera.position.x / Map::CHUNK_SIZE) + " , " + std::to_string((int)myCamera.position.z / Map::CHUNK_SIZE) + ")";
    ImGui::Text(str.c_str());

    ImGui::Separator();
    ImGui::Checkbox("V-Sync", &bEnableVSync);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SDL_GL_SetSwapInterval(bEnableVSync ? 1 : 0);
    }
    ImGui::End();
    ImGui::PopStyleColor(1);

    ImGui::Render();
    glViewport(0, 0, WIN_W, WIN_H);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}