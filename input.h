#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

namespace Eternal {
    class InputHandle {
        public:
            enum Key { 
                KEY_LEFT = SDL_SCANCODE_LEFT,
                KEY_RIGHT = SDL_SCANCODE_RIGHT,
                KEY_UP = SDL_SCANCODE_UP,
                KEY_DOWN = SDL_SCANCODE_DOWN,
                KEY_SPACE = SDL_SCANCODE_SPACE,
                KEY_ESCAPE = SDL_SCANCODE_ESCAPE,
                KEY_START = SDL_SCANCODE_RETURN,
                KEY_GRAVE = SDL_SCANCODE_GRAVE,
                KEY_RETURN = SDL_SCANCODE_RETURN
            };

            enum MouseButtons {
                MBUTTON_LEFT = SDL_BUTTON(SDL_BUTTON_LEFT),
                MBUTTON_RIGHT = SDL_BUTTON(SDL_BUTTON_RIGHT),

                MBUTTON_COUNT
            };
            InputHandle();
            ~InputHandle();

            void PollInputs();
            bool IsKeyDown(Key key);
            bool IsKeyTap(Key key);

            bool IsMouseClick(MouseButtons b);
            bool IsMouseDown(MouseButtons b);

            int GetMouseX() const;
            int GetMouseY() const;
            int GetFormerMouseX() const;
            int GetFormerMouseY() const;

        private:
            Uint32 myMouse;
            int iMouseX, iMouseY, iFormerMouseX, iFormerMouseY;
            bool bKeyStates[256];
            Uint32 prevMouseState;
            bool bPrevKeyStates[256];

            bool bMouseButtonState[MBUTTON_COUNT];
    };
}

#endif