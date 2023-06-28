#include "camera.h"

#include "shader.h"

#include <SDL2/SDL.h>

#include "mesh.h"
#include "brick_selector_widget.h"
#include <imgui.h>

extern SDL_Window* myWindow;

Camera::Camera() {
    direction = glm::vec3(0,0,1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
//    position = glm::vec3(120,32,120);
    position = glm::vec3(8,8,0);
    bFocus = true;
}

Camera::~Camera() { }

void Camera::CheckInput() {

}

void Camera::Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget) {
	const Uint8 * keys = SDL_GetKeyboardState(0);

    FindTargettedBrick(myMap, input, selectWidget);

    float STRAFE_SPD = 0.05f;
//    const float pi = 3.14159f;

    myShader.viewMatrix = glm::lookAt(position, position + direction, up);

    // todo
    right = glm::normalize(glm::cross(direction, up));

    glm::vec3 moveDelta = glm::vec3(0,0,0);
    if (keys[SDL_SCANCODE_LSHIFT]) {
        STRAFE_SPD = 0.4f;
	}
    else if (keys[SDL_SCANCODE_Q]) {
        STRAFE_SPD = 0.01f;
	}
	if (keys[SDL_SCANCODE_A]) {
        moveDelta -= right * STRAFE_SPD;
	}
	else if (keys[SDL_SCANCODE_D]) {
        moveDelta += right * STRAFE_SPD;
	}

	if (keys[SDL_SCANCODE_W]) {
        glm::vec3 forward = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
        moveDelta += forward * STRAFE_SPD;
	}
	else if (keys[SDL_SCANCODE_S]) {
        glm::vec3 forward = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
        forward.y = 0;
        moveDelta -= forward * STRAFE_SPD;
	}

/*    for(float z = -distf;z < distf;z += 0.1f) {
        for(float x = -distf;x < distf;x += 0.1f) {
            if(myMap.GetBrick((int)((position.x+x+0.5f) + moveDelta.x), (int)(position.z+0.5f+z), (int)position.y-1) != 0) {
                xMoveAccepted = false;
            }
        }
    }*/
    position.x += moveDelta.x;
    if(CheckCollision(myMap)) {
        position.x -= moveDelta.x;
    }

    position.z += moveDelta.z;
    if(CheckCollision(myMap)) {
        position.z -= moveDelta.z;
    }

    static bool lastpress = 0;
    if (keys[SDL_SCANCODE_R]) {
        if(lastpress == false) {
            myMap.SetBrick((int)position.x, (int)position.z, (int)position.y, 3);
            myMap.AddLight((int)position.x, (int)position.z, (int)position.y, false);
            myMap.RebuildAll();
        }
        lastpress = true;
	}
    else {
        lastpress = false;
    }

    int mouseX = 0, mouseY = 0;
    if(SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(1)) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            bFocus = true;
        }
    }
    if(keys[SDL_SCANCODE_ESCAPE]) {
        bFocus = false;
    }

//    std::cout << direction.x << " , " << direction.y << " , " << direction.z << std::endl;
    if(bFocus) {
        if(!SDL_GetWindowGrab(myWindow)) {
            SDL_SetWindowGrab(myWindow,SDL_TRUE);
            SDL_ShowCursor(SDL_DISABLE);
        }
        float LookSens = 0.05f;

        float yDelta = (300 - mouseY) * LookSens;
        float xDelta = (400 - mouseX) * LookSens;

        float currentPitch = glm::degrees(std::asin(direction.y));
        float newPitch = currentPitch + yDelta;

        float clampedPitch = glm::clamp(newPitch, -89.0f, 89.0f);
        float pitchDelta = clampedPitch - currentPitch;

        glm::mat4 pitchRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(pitchDelta), right);
        direction = glm::vec3(pitchRotationMatrix * glm::vec4(direction, 1.0f));

        glm::mat4 yawRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(xDelta), glm::vec3(0.0f, 1.0f, 0.0f));
        direction = glm::vec3(yawRotationMatrix * glm::vec4(direction, 1.0f));

        SDL_WarpMouseInWindow(myWindow,400,300);
    }
    else {
        if(SDL_GetWindowGrab(myWindow)) {
            SDL_SetWindowGrab(myWindow,SDL_FALSE);
            SDL_ShowCursor(SDL_ENABLE);
        }
    }
    direction = glm::normalize(direction);

    GLuint view = glGetUniformLocation(myShader.myProgram, "View");
    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(myShader.viewMatrix));

    CheckGround(myMap);
    if(!bground) {
        fJumpVel += 0.005f;
    }
    else {
        fJumpVel = 0;
    }

    if(keys[SDL_SCANCODE_1]) {
        fJumpVel = 0;
        position.y += 0.25f;
    }

    if (keys[SDL_SCANCODE_SPACE] && bground) {
        fJumpVel = -0.1f;
        position.y += 0.25f;
	}
	else if (keys[SDL_SCANCODE_LCTRL]) {
        position -= up * 0.25f;
	}

    
    position.y -= fJumpVel;
    
//    std::cout << position.y << std::endl;
   //std::cout << position.x << " , " << position.y << " , " << position.z << std::endl;
}

void Camera::FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget) {
    glm::vec3 p = position;
    glm::vec3 outter;

    p.x += 0.5f;
    p.y += 0.5f;

    if(position.x < 0)
        p.x -= 1;
    if(position.z < 0)
        p.z -= 1;

    bool bFound = false;
    for(int i = 0;i < 1000;i++) {
        p += direction / 100.0f;
        if(myMap.GetBrick(p.x, p.z, p.y) != 0) {
            outter = p - (direction / 100.0f);
            bFound = true;
            break;
        }
    }

    if(!bFound) {
        targetted_brick = { -1, -1, -1 };
        return;
    }

    targetted_brick.x = (int)p.x;
    targetted_brick.y = (int)p.y;
    targetted_brick.z = (int)p.z;

    int chunkX = floor(targetted_brick.x / Map::CHUNK_SIZE);
    int chunkZ = floor(targetted_brick.z / Map::CHUNK_SIZE);
        
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_LEFT)) {
        std::cout << "deleting brick " << myMap.GetBrick(targetted_brick.x, targetted_brick.z, targetted_brick.y) << " at (" << targetted_brick.x << ", " << targetted_brick.y << ", " << targetted_brick.z << std::endl;
/*
        std::cout << "xchunk: " << floor(targetted_brick.x / Map::CHUNK_SIZE) << std::endl;
        std::cout << "zchunk: " << floor(targetted_brick.z / Map::CHUNK_SIZE) << std::endl;

        std::cout << "xtrans: " << abs((int)targetted_brick.x%Map::CHUNK_SIZE) << std::endl;
        std::cout << "ztrans: " << abs((int)targetted_brick.z%Map::CHUNK_SIZE) << std::endl;
*/
        int brickType = myMap.GetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y);
        myMap.SetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y,0);
        if(brickType == 3) { // torch
            myMap.AddLight((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y, true);
        }
        else {
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    myMap.GetChunk(x,z)->PushLights(myMap);                    
                }
            }
            myMap.SetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y,0);
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    myMap.GetChunk(x,z)->PopLights(myMap);                    
                }
            }
        }
        std::cout << "BRICKTYPE: " << brickType << std::endl;
        myMap.ScheduleMeshBuild({chunkX, chunkZ, Map::Priority::IMMEDIATE});
        myMap.ScheduleAdjacentChunkBuilds(chunkX,chunkZ, Map::Priority::ONE);
        //std::cout << "build: " << floor(targetted_brick.x / Map::CHUNK_SIZE) << std::endl;
    }
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_RIGHT)) {
        int brickType = selectWidget.GetSelectedBrickID();
        if(brickType == 3) {
            myMap.SetBrick((int)outter.x, (int)outter.z, (int)outter.y,brickType);
            myMap.AddLight((int)outter.x, (int)outter.z, (int)outter.y, false);
        }
        else {
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    myMap.GetChunk(x,z)->PushLights(myMap);                    
                }
            }
            myMap.SetBrick((int)outter.x, (int)outter.z, (int)outter.y,brickType);
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    myMap.GetChunk(x,z)->PopLights(myMap);                    
                }
            }
        }
        myMap.ScheduleMeshBuild({chunkX, chunkZ, Map::Priority::IMMEDIATE});
        myMap.ScheduleAdjacentChunkBuilds(chunkX,chunkZ, Map::Priority::ONE);
    }

//    std::cout << "brick: " << myMap.GetLightLevel((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y+1) << std::endl;
}