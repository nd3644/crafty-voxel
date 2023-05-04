#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
    public:
        Camera();
        ~Camera();

        void Update();
    private:
        void CheckInput();
        glm::vec3 position, direction, up, right;
};

#endif
