#include "camera.h"

#include "shader.h"

#include <SDL2/SDL.h>

#include "mesh.h"
#include <imgui.h>

extern SDL_Window* myWindow;

Camera::Camera() {
    direction = glm::vec3(0,0,1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
    position = glm::vec3(120,18,120);
    bFocus = true;
}

Camera::~Camera() { }

void Camera::CheckInput() {

}

void Camera::Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input) {
	const Uint8 * keys = SDL_GetKeyboardState(0);

    FindTargettedBrick(myMap, input);

    float STRAFE_SPD = 0.05f;
    const float pi = 3.14159f;

    myShader.viewMatrix = glm::lookAt(position, position + direction, up);

    // todo
    right = glm::normalize(glm::cross(direction, up));

    glm::vec3 moveDelta = glm::vec3(0,0,0);
    if (keys[SDL_SCANCODE_LSHIFT]) {
        STRAFE_SPD = 0.4f;
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

    bool xMoveAccepted = true;
    bool zMoveAccepted = true;

    float distf = 0.6f;
    for(float z = -distf;z < distf;z += 0.1f) {
        for(float x = -distf;x < distf;x += 0.1f) {
            if(myMap.GetBrick((int)((position.x+x+0.5f) + moveDelta.x), (int)(position.z+0.5f+z), (int)position.y-1) != 0) {
                xMoveAccepted = false;
            }
        }
    }
    if(xMoveAccepted) {
        position.x += moveDelta.x;
    }
    for(float z = -distf-0.2f;z < distf+0.2f;z += 0.1f) {
        for(float x = -distf;x < distf;x += 0.1f) {
            if(myMap.GetBrick((int)((position.x+x+0.5f)), (int)((position.z+0.5f+moveDelta.z)+z), (int)position.y-1) != 0) {
                zMoveAccepted = false;
            }
        }
    }
    if(zMoveAccepted) {
        position.z += moveDelta.z;
    }

    static bool lastpress = 0;
    if (keys[SDL_SCANCODE_R]) {
        if(lastpress == false) {
            myMap.SetBrick((int)position.x, (int)position.z, (int)position.y, 3);
//            myMap.AddLight((int)position.x, (int)position.z, (int)position.y);
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
    static int lastX = mouseX, lastY = mouseY;

//    std::cout << direction.x << " , " << direction.y << " , " << direction.z << std::endl;
    if(bFocus) {
        if(!SDL_GetWindowGrab(myWindow)) {
            SDL_SetWindowGrab(myWindow,SDL_TRUE);
            SDL_ShowCursor(SDL_DISABLE);
        }
        float LookSens = 0.05f;

        float yDelta = (300 - mouseY) * LookSens;
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians((yDelta)), right); // The rotation matrix
        direction = glm::vec3(rotationMatrix * glm::vec4(direction, 1.0f)); 
    
        float xDelta = (400 - mouseX) * LookSens;
        rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians((xDelta)), glm::vec3(0.0f, 1.0f, 0.0f)); // The rotation matrix
        direction = glm::vec3(rotationMatrix * glm::vec4(direction, 1.0f)); 

        lastX = mouseX;
        lastY = mouseY;

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

    float d = 0.1f;
    bool bground = false;
    for(float x = -0.20f;x < 1.1f;x += 0.01) {
        for(float z = -0.20f;z < 1.1f;z += 0.01f) {
            if( myMap.GetBrick((int)(position.x+x), (int)(position.z+z), (int)(position.y-1.5f)) != 0) {
                bground = true;
            }
        }
    }
    if(!bground) {
        fJumpVel += 0.005f;
    }
    else {
        fJumpVel = 0;
    }


    static int cooldown = 60;
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

void Camera::FindTargettedBrick(Map &myMap, Eternal::InputHandle &input) {
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
    int mx = 0, my = 0;
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_LEFT)) {
        std::cout << "deleting brick " << myMap.GetBrick(targetted_brick.x, targetted_brick.z, targetted_brick.y) << " at (" << targetted_brick.x << ", " << targetted_brick.y << ", " << targetted_brick.z << std::endl;
/*
        std::cout << "xchunk: " << floor(targetted_brick.x / Map::CHUNK_SIZE) << std::endl;
        std::cout << "zchunk: " << floor(targetted_brick.z / Map::CHUNK_SIZE) << std::endl;

        std::cout << "xtrans: " << abs((int)targetted_brick.x%Map::CHUNK_SIZE) << std::endl;
        std::cout << "ztrans: " << abs((int)targetted_brick.z%Map::CHUNK_SIZE) << std::endl;
*/
        myMap.SetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y,0);
        myMap.BuildChunk(floor(targetted_brick.x / Map::CHUNK_SIZE), floor(targetted_brick.z / Map::CHUNK_SIZE));
        //std::cout << "build: " << floor(targetted_brick.x / Map::CHUNK_SIZE) << std::endl;
    }
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_RIGHT)) {
        myMap.SetBrick((int)outter.x, (int)outter.z, (int)outter.y,1);
        myMap.BuildChunk((int)targetted_brick.x / Map::CHUNK_SIZE, (int)targetted_brick.z / Map::CHUNK_SIZE);
    }
}