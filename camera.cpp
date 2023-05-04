#include "camera.h"

#include "shader.h"

#include <SDL2/SDL.h>

Camera::Camera() {
    direction = glm::vec3(0,0,1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
    position = glm::vec3(0,0,0);
}

Camera::~Camera() { }

void Camera::CheckInput() {

}

void Camera::Update() {
	const Uint8 * keys = SDL_GetKeyboardState(0);

    const float STRAFE_SPD = 2.0f;
    const float pi = 3.14159f;

    viewMatrix = glm::lookAt(position, position + direction, up);

    // todo
    right = glm::normalize(glm::cross(direction, up));

	if (keys[SDL_SCANCODE_LEFT]) {
        position -= right;
	}
	else if (keys[SDL_SCANCODE_RIGHT]) {
        position += right;
	}

	if (keys[SDL_SCANCODE_UP]) {
        position += direction;
	}
	else if (keys[SDL_SCANCODE_DOWN]) {
        position -= direction;
	}

    if (keys[SDL_SCANCODE_SPACE]) {
        position += up;
	}
	else if (keys[SDL_SCANCODE_LCTRL]) {
        position -= up;
	}


    const float RSPD = 2.0f;
    if (keys[SDL_SCANCODE_Q]) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(RSPD), glm::vec3(0.0f, 1.0f, 0.0f)); // The rotation matrix
        direction = glm::vec3(rotationMatrix * glm::vec4(direction, 1.0f)); 
	}
	else if (keys[SDL_SCANCODE_E]) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-RSPD), glm::vec3(0.0f, 1.0f, 0.0f)); // The rotation matrix
        direction = glm::vec3(rotationMatrix * glm::vec4(direction, 1.0f)); 
	}
    direction = glm::normalize(direction);


    GLuint view = glGetUniformLocation(myProgram, "View");
    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(viewMatrix));
}