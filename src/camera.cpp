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
//    position = formerPosition = glm::vec3(8,120,0);
    position = formerPosition = glm::vec3(8 + 2000,120,0 + 2000);
    bFocus = true;
    bThirdPerson = false;
    bground = false;
}

Camera::~Camera() { }

void Camera::CheckInput() {

}

void Camera::Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget) {
	const Uint8 *keys = SDL_GetKeyboardState(0);
    CalcNewFrustumPlanes();

    FindTargettedBrick(myMap, input, selectWidget);
//    const float pi = 3.14159f;


    // The "position" of the camera depends on the 3rd or first person state
    if(bThirdPerson) {
        glm::vec3 newPosition = position;
        newPosition -= (direction*64.0f);
        myShader.viewMatrix = glm::lookAt(newPosition, position + direction, up);
    }
    else {
        myShader.viewMatrix = glm::lookAt(position, position + direction, up);        
    }

    // Update the uniform
    GLuint view = glGetUniformLocation(myShader.myProgram, "View");
    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(myShader.viewMatrix));

    moveDelta = glm::vec3(0,0,0);
    CheckInput(input);

    position.x += moveDelta.x;
    if(CheckCollision(myMap)) {
        position.x -= moveDelta.x;
    }

    position.z += moveDelta.z;
    if(CheckCollision(myMap)) {
        position.z -= moveDelta.z;   
    }

    RunMouseLogic();

    CheckGround(myMap);
    if(!bground) {
        fJumpVel += 0.001f;
    }
    else {
        fJumpVel = 0;
    }

    if (keys[SDL_SCANCODE_SPACE] && bground) {
        fJumpVel = -0.05f;
        position.y += 0.05f;
	}
	else if (keys[SDL_SCANCODE_LCTRL]) {
        position -= up * 0.25f;
	}

    position.y -= fJumpVel;
    
    // Push the position for drawing the collider in debug modes
    cameralist.emplace_back(position.x,position.y,position.z - 0.5f);

    // This is just a debug "go up" button
    if(keys[SDL_SCANCODE_1]) {
        fJumpVel = 0;
        position.y += 0.25f;
    }

}

void Camera::CheckInput(Eternal::InputHandle &input) {
    right = glm::normalize(glm::cross(direction, up));
    const Uint8 * keys = SDL_GetKeyboardState(0);

    STRAFE_SPD = DEFAULT_STRAFE_SPD;
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

    if (keys[SDL_SCANCODE_F5]) {
        bThirdPerson = false;
	}
    if (keys[SDL_SCANCODE_F6]) {
        bThirdPerson = true;
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

    position.x = std::max(position.x, 0.0f);
    position.z = std::max(position.z, 0.0f);
}

void Camera::RunMouseLogic() {
    const Uint8 *keys = SDL_GetKeyboardState(0);

    int mouseX = 0, mouseY = 0;
    if(SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(1)) {
        #ifdef ENABLE_IMGUI
            ImGuiIO& io = ImGui::GetIO();
            if (!io.WantCaptureMouse) {
                bFocus = true;
            }
        #else
            bFocus = true;
        #endif
    }
    if(keys[SDL_SCANCODE_ESCAPE]) {
        bFocus = false;
    }

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
}

void Camera::FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget) {
    glm::vec3 p = position;
    glm::vec3 outter;

    p.x += 0.5f;
    p.y += 0.5f;

    if((int)position.x <= 0)
        p.x -= 1;
    if((int)position.z <= 0)
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

    int chunkX = std::floor(targetted_brick.x / Map::CHUNK_SIZE);
    int chunkZ = std::floor(targetted_brick.z / Map::CHUNK_SIZE);
        
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_LEFT)) {
//        std::cout << "deleting brick " << myMap.GetBrick(targetted_brick.x, targetted_brick.z, targetted_brick.y) << " at (" << targetted_brick.x << ", " << targetted_brick.y << ", " << targetted_brick.z << std::endl;
        int brickType = myMap.GetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y);
        myMap.SetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y,0);
        if(brickType == 3) { // torch
            myMap.AddLight((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y, true);
        }
        else {
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    //myMap.GetChunk(x,z)->PushLights(myMap);                    
                }
            }
            myMap.SetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y,0);
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    //myMap.GetChunk(x,z)->PopLights(myMap);                    
                }
            }
        }
        std::cout << "BRICKTYPE: " << brickType << std::endl;
        //myMap.ScheduleMeshBuild({chunkX, chunkZ, Map::Priority::IMMEDIATE});
        //myMap.ScheduleAdjacentChunkBuilds(chunkX,chunkZ, Map::Priority::ONE);
        //myMap.ScheduleMeshBuild({chunkX, chunkZ, Map::Priority::IMMEDIATE});
        myMap.BuildChunkAO(chunkX,chunkZ);
        myMap.BuildChunk(chunkX, chunkZ);
    }
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_RIGHT)) {
        int brickType = selectWidget.GetSelectedBrickID();
        if(brickType == 3) {
            //myMap.SetBrick((int)outter.x, (int)outter.z, (int)outter.y,brickType);
            //myMap.AddLight((int)outter.x, (int)outter.z, (int)outter.y, false);
        }
        else {
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    //myMap.GetChunk(x,z)->PushLights(myMap);                    
                }
            }
            myMap.SetBrick((int)outter.x, (int)outter.z, (int)outter.y,brickType);
            for(int x = chunkX-2;x < chunkX+2;x++) {
                for(int z = chunkZ-2;z < chunkZ+2;z++) {
                    //myMap.GetChunk(x,z)->PopLights(myMap);                    
                }
            }
        }
        //myMap.ScheduleMeshBuild({chunkX, chunkZ, Map::Priority::IMMEDIATE});
        //myMap.ScheduleAdjacentChunkBuilds(chunkX,chunkZ, Map::Priority::ONE);

        //myMap.ScheduleMeshBuild({chunkX, chunkZ, Map::Priority::IMMEDIATE});
        myMap.BuildChunkAO(chunkX,chunkZ);
        myMap.BuildChunk(chunkX, chunkZ);
    }

    formerPosition = position;
}

bool Camera::IsThirdPerson() const {
    return bThirdPerson;
}

void Camera::CalcNewFrustumPlanes() {

    // All frustum planes intersect the camera position
    for(int i = 0;i < NUM_PLANES;i++) {
        myFrustumPlanes[i].position = position;
    }

    float hFov = glm::degrees(2 * glm::atan(glm::tan(glm::radians(fFov / 2)) * static_cast<float>(WIN_W) / static_cast<float>(WIN_H)));

//    std::cout << "fov: " << hFov << std::endl;

    // near
    myFrustumPlanes[PLANE_NEAR].normal = direction;


    // far
    myFrustumPlanes[PLANE_FAR].normal = -direction;
    myFrustumPlanes[PLANE_FAR].position += glm::normalize(direction) * gfZFar;

    // right
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(((hFov) / 2.0f) - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_RIGHT].normal = rotatedVector;

    // left
    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians((-((hFov) / 2.0f)) + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_LEFT].normal = rotatedVector;
    
    // top
    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians((fFov / 2.0f) - 90.0f), right);
    rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_TOP].normal = rotatedVector;

    // bottom
    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-(fFov / 2.0f) + 90.0f), right);
    rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_BOTTOM].normal = rotatedVector;

//    std::cout << "(" << rotatedVector.x << ", " << rotatedVector.y << ", " << rotatedVector.z << std::endl;



/*    float halfSin = sin(fFov / 2.0f);
    float halfCos = cos(fFov / 2.0f);
    myFrustumPlanes[PLANE_LEFT].normal = { halfCos, 0.0f, halfSin };

    myFrustumPlanes[PLANE_RIGHT].normal = { -halfCos, 0.0f, halfSin };

    myFrustumPlanes[PLANE_TOP].normal = { 0.0f, -halfCos, halfSin };

    myFrustumPlanes[PLANE_BOTTOM].normal = { 0.0f, halfCos, halfSin };
    myFrustumPlanes[PLANE_BOTTOM].normal = { 0.0f, halfCos, halfSin };*/

}