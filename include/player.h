#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>

class Player {
    public:
        Player();
        ~Player();

        void Update();
        void Render();
    private:
        glm::vec3 vPosition;
        

};

#endif
