#include "camera.h"

#include <SDL.h>

Camera::Camera() { }

Camera::~Camera() { }

void Camera::Update() {
	const Uint8 * keys = SDL_GetKeyboardState(0);

	if (keys[SDL_SCANCODE_LEFT]) {

	}
	else if (keys[SDL_SCANCODE_RIGHT]) {

	}

	if (keys[SDL_SCANCODE_UP]) {

	}
	else if (keys[SDL_SCANCODE_DOWN]) {

	}

}