#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "map.h"
#include "shader.h"
#include "input.h"
#include "map.h"

class Camera
{
    public:
        Camera();
        ~Camera();

        void Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input);
        glm::vec3 position, direction, up, right;

        glm::vec3 targetted_brick;
    private:
        float fJumpVel;
        bool bFocus;
        bool bground;
        void CheckInput();
        void FindTargettedBrick(Map &myMap, Eternal::InputHandle &input);
};

#endif
