#include "camera.h"

#include "shader.h"

#include <SDL2/SDL.h>


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

void Camera::Update(Map &myMap, Shader &myShader) {
	const Uint8 * keys = SDL_GetKeyboardState(0);

    float STRAFE_SPD = 0.1f;
    const float pi = 3.14159f;

    myShader.viewMatrix = glm::lookAt(position, position + direction, up);

    // todo
    right = glm::normalize(glm::cross(direction, up));

    if (keys[SDL_SCANCODE_LSHIFT]) {
        STRAFE_SPD = 0.4f;
	}

	if (keys[SDL_SCANCODE_A]) {
        position -= right * STRAFE_SPD;
	}
	else if (keys[SDL_SCANCODE_D]) {
        position += right * STRAFE_SPD;
	}
	if (keys[SDL_SCANCODE_W]) {
        glm::vec3 forward = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
        position += forward * STRAFE_SPD;
	}
	else if (keys[SDL_SCANCODE_S]) {
        glm::vec3 forward = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
        forward.y = 0;
        position -= forward * STRAFE_SPD;
	}

    if (keys[SDL_SCANCODE_SPACE]) {
        position += up;
	}
	else if (keys[SDL_SCANCODE_LCTRL]) {
        position -= up;
	}

    static bool lastpress = 0;
    if (keys[SDL_SCANCODE_R]) {
        if(lastpress == false) {
            myMap.SetBrick((int)position.x, (int)position.z, (int)position.y, 3);
            myMap.AddLight((int)position.x, (int)position.z, (int)position.y);
            myMap.RebuildAll();
        }
        lastpress = true;
	}
    else {
        lastpress =false;
    }

    int mouseX = 0, mouseY = 0;
    if(SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(3)) {
        bFocus = true;
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

    bool bground = false;
    for(float x = -0.5f;x < 1.2f;x += 0.1f) {
        for(float z = -0.5f;z < 1.2f;z += 0.1f) {
            if( myMap.GetBrick((int)(position.x+x), (int)(position.z+z), (int)(position.y-1.5f)) != 0) {
                bground=true;
            }
        }
    }
    if(!bground) {
        position.y -= 0.1f;
    }
//    std::cout << position.y << std::endl;

   //std::cout << position.x << " , " << position.y << " , " << position.z << std::endl;
}