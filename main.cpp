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
#include "brick_selector_widget.h"

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
extern void DrawDebugUI(Camera &myCamera, Map &myMap);
extern void DrawCursor();
extern void DrawBrickTarget(Camera &myCamera, Mesh &brickTargetMesh);

bool bEnableVSync = true;
bool bDrawColliders = false;
int frameCounter = 0;
int fpsTimer = 0;
int lastAvgFps = 0;
std::chrono::milliseconds CameraUpdateDuration;
std::chrono::milliseconds MapDrawDuration, MapBuildDuration, FrameTime, SwapTime, OrthoTime;

bool bDebugMenuOpen = true;

Eternal::Sprite cursorTex;

Shader myShader, myShader2D;

int main(int argc, char* args[]) {
	CompileArr();

	myShader.projMatrix = glm::mat4(1);
	myShader.viewMatrix = glm::mat4(1);
	myShader.modelMatrix = glm::mat4(1);

    myShader2D.projMatrix = glm::mat4(1);
    myShader2D.modelMatrix = glm::mat4(1);
    myShader2D.viewMatrix = glm::mat4(1);
	Init();
    Eternal::InputHandle myInputHandle;
    Mesh brickTargetMesh;

    Camera myCamera;
    Map myMap(myCamera);
    BrickSelectorWidget myBrickWidget;

    myMap.FromBMP("textures/heightmap.bmp");
    myBrickWidget.Init(myMap);
    
	float fdelta = 0.0f;
	bool bDone = false;

    static int counter = 0;
	while (bDone == false) {
        auto frameStartTime = std::chrono::high_resolution_clock::now();
        if(counter++ == 30) {
            //exit(1);
        }
		fdelta += 0.01f;
        mouseWheelDelta = 0; // Reset this every frame
		while (SDL_PollEvent(&myEvent)) {
            ImGui_ImplSDL2_ProcessEvent(&myEvent);
            if(myEvent.type == SDL_MOUSEWHEEL) {
                if(myEvent.wheel.y != 0) {
                    mouseWheelDelta += myEvent.wheel.y;
                }
            }
            else if(myEvent.type == SDL_KEYDOWN && myEvent.key.keysym.scancode == SDL_SCANCODE_F11) {
                bIsFullscreen = !bIsFullscreen;
                SDL_SetWindowFullscreen(myWindow, bIsFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                SDL_GetWindowSize(myWindow, &WIN_W, &WIN_H);
            }
            else if ((myEvent.type == SDL_WINDOWEVENT && myEvent.window.event == SDL_WINDOWEVENT_CLOSE) || SDL_GetKeyboardState(0)[SDL_SCANCODE_GRAVE]) {
				bDone = true;
			}
		}
        myInputHandle.PollInputs();

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.494 * fAmbient, 0.752 * fAmbient, 0.933f * fAmbient, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myShader.projMatrix = glm::perspective(glm::radians(75.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

        myShader.Bind();

        // Update the camera
        auto start = std::chrono::high_resolution_clock::now();
        myCamera.Update(myMap, myShader, myInputHandle, myBrickWidget);
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

        start = std::chrono::high_resolution_clock::now();
        DrawBrickTarget(myCamera, brickTargetMesh);

        // 2D rendering
        myShader2D.projMatrix = glm::ortho(0.0f,(float)WIN_W,(float)WIN_H,0.0f,-100.0f,100.0f);
        myShader2D.modelMatrix = glm::mat4(1);
        myShader2D.viewMatrix = glm::mat4(1);

        myShader2D.Bind();

        DrawCursor();
        myBrickWidget.Draw();
//        DrawMiniMap(myCamera, myMap);

        DrawDebugUI(myCamera, myMap);
        end = std::chrono::high_resolution_clock::now();
        OrthoTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if(SDL_GetTicks() - fpsTimer > 1000) {
            fpsTimer = SDL_GetTicks();
            lastAvgFps = frameCounter;
            frameCounter = 0;
        }

        start = std::chrono::high_resolution_clock::now();
        SDL_GL_SwapWindow(myWindow);
        end = std::chrono::high_resolution_clock::now();
        SwapTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

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

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    // Handle the debug message as needed
    // You can print the message or perform other actions based on severity, type, etc.
    // For example, you can use printf to print the message:
    printf("OpenGL Debug Message: %s\n", message);
}


void Init() {
	std::cout << "trying init---\n";
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Problem SDL_Init()" << std::endl;
		std::cout << "\t *" << SDL_GetError() << std::endl;
		exit(0);
	}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);


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

    if (GLEW_ARB_debug_output) {
        // Retrieve function pointers for debug output functions
        PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)SDL_GL_GetProcAddress("glDebugMessageCallback");
        PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)SDL_GL_GetProcAddress("glDebugMessageControl");

        if (glDebugMessageCallback && glDebugMessageControl) {
            // Enable debug output and set up the callback function
            glDebugMessageCallback(DebugCallback, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
        }
        else {
            std::cout << "ahh";
        }
    }
    else {
        std::cout << "ahhhh" << std::endl;
    }

	SDL_GL_SetSwapInterval(1);

    SetupImgui();

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

    Eternal::Sprite square, grey;
    square.Load("textures/square.png");
    grey.Load("textures/grey.png");

    int size = 16;
    Rect r(0,0,size,size);
    Rect c(0,0,16,16);

    int camY = myCamera.position.y;

    // Draw cur level
    for(int x = 0;x < 32;x++) {
        r.x = x * size;
        for(int z = 0;z < 32;z++) {
            r.y = z * size;
            int y = camY;

            // ground
            if(myMap.GetBrick(x,z,y-2) != 0) {
                grey.Bind();
                grey.Draw(r,c);
            }

            // shin level
            if(myMap.GetBrick(x,z,y-1) != 0) {
                square.Bind();
                square.Draw(r,c);
            }
        }
    }

    // Draw player
    Eternal::Sprite player;
    player.Load("textures/red.png");

    r.x = myCamera.position.x * size;
    r.y = myCamera.position.z * size;
    r.y -= (size/2);
    r.w = r.h = 8;
    r.x += 4;
    r.y += 4;
    
    player.Bind();
    player.Draw(r,c);
}

void DrawBrickTarget(Camera &myCamera, Mesh &brickTargetMesh) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    brickTargetMesh.Clean();
    brickTargetMesh.SetTranslation((int)myCamera.targetted_brick.x,(int)myCamera.targetted_brick.y,(int)myCamera.targetted_brick.z);
    for(size_t i = 0;i < vertList.size();i++) {
        brickTargetMesh.Index1(1);
        brickTargetMesh.Vert3(vertList[i].x, vertList[i].y, vertList[i].z+0.5f);
        brickTargetMesh.TexCoord2(uvList[i].x, uvList[i].y);
        brickTargetMesh.Color4(0,0,0,1);
    }
    brickTargetMesh.BindBufferData();

//    std::cout << glGetError() << std::endl;
    brickTargetMesh.Draw(Mesh::MODE_TRIANGLES);

    if(!bDrawColliders) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        return;
    }

    // Draw colliders
    for(auto &vec: bricklist) {
        brickTargetMesh.Clean();
        brickTargetMesh.SetTranslation((int)vec.x,(int)vec.y,(int)vec.z);
        for(size_t i = 0;i < vertList.size();i++) {
            brickTargetMesh.Index1(1); brickTargetMesh.Vert3(vertList[i].x, vertList[i].y, vertList[i].z+0.5f);
            brickTargetMesh.TexCoord2(uvList[i].x, uvList[i].y);
            brickTargetMesh.Color4(1,0,0,1);
        }
        brickTargetMesh.BindBufferData();
        brickTargetMesh.Draw(Mesh::MODE_TRIANGLES);
    }

    bricklist.clear();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    if(myCamera.IsThirdPerson()) {
        glDisable(GL_TEXTURE_2D);
        // Draw camera box
        for(auto &vec: cameralist) {
            brickTargetMesh.Clean();
            brickTargetMesh.SetTranslation(vec.x,vec.y,vec.z);
            for(size_t i = 0;i < vertList.size();i++) {
                brickTargetMesh.Index1(1); brickTargetMesh.Vert3(vertList[i].x * 0.75f, vertList[i].y * 0.75f, (vertList[i].z+0.5f) * 0.75f);
                brickTargetMesh.TexCoord2(uvList[i].x, uvList[i].y);
                brickTargetMesh.Color4(1,0,0,1);
            }
            brickTargetMesh.BindBufferData();
            brickTargetMesh.Draw(Mesh::MODE_TRIANGLES);
        }
    }

    cameralist.clear();
}

void DrawDebugUI(Camera &myCamera, Map &map) {
    glDisable(GL_DEPTH_TEST);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::SetNextWindowSize(ImVec2(-1, 280));
    // ImGui content goes here

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); 
    ImGui::Begin("Debug",&bDebugMenuOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    std::string str = "MapDraw: " + std::to_string(MapDrawDuration.count()) + "ms";
    ImGui::Text("%s", str.c_str());

    str = "MapBuild: " + std::to_string(MapBuildDuration.count()) + "ms";
    ImGui::Text("%s", str.c_str());

    str = "CamUpdate: " + std::to_string(CameraUpdateDuration.count()) + "ms";
    ImGui::Text("%s", str.c_str());

    str = "FrameTime: " + std::to_string(FrameTime.count()) + "ms";
    ImGui::Text("%s", str.c_str());
    
    str = "SwapTime: " + std::to_string(SwapTime.count()) + "ms";
    ImGui::Text("%s", str.c_str());

    str = "OrthoTime: " + std::to_string(OrthoTime.count()) + "ms";
    ImGui::Text("%s", str.c_str());

    str = "avg fps: " + std::to_string(lastAvgFps);
    ImGui::Text("%s", str.c_str());

/*    std::string polyNumber = std::to_string(gblPolyCount);
    polyNumber.insert(polyNumber.begin()+3,',');
    str = "polycount: : " + polyNumber;
    ImGui::Text("%s", str.c_str());*/

    str = "CamXYZ: (" + std::to_string((int)myCamera.position.x) + " , " + std::to_string((int)myCamera.position.y) + " , " + std::to_string((int)myCamera.position.z) + ")";
    ImGui::Text("%s", str.c_str());
    str = "ChunkXYZ: (" + std::to_string((int)myCamera.position.x / Map::CHUNK_SIZE) + " , " + std::to_string((int)myCamera.position.z / Map::CHUNK_SIZE) + ")";
    ImGui::Text("%s", str.c_str());

    ImGui::Separator();
    ImGui::Checkbox("V-Sync", &bEnableVSync);
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        SDL_GL_SetSwapInterval(bEnableVSync ? 1 : 0);
    }
    ImGui::Checkbox("Colliders", &bDrawColliders);
    if(ImGui::SliderFloat("Ambient", &fAmbient, 0.0f, 1.0f)) {
        map.RebuildAllVisible();
    }
    ImGui::End();
    ImGui::PopStyleColor(1);

    ImGui::Render();
    glViewport(0, 0, WIN_W, WIN_H);
    glUseProgram(0);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}