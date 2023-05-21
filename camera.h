#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "map.h"
#include "shader.h"

class Camera
{
    public:
        Camera();
        ~Camera();

        void Update(Map &myMap, Shader &myShader);
        glm::vec3 position, direction, up, right;
    private:
        float fJumpVel;
        bool bFocus;
        bool bground;
        void CheckInput();
};

#endif
