#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "map.h"
#include "shader.h"
#include "input.h"
#include "map.h"

class BrickSelectorWidget;
class Camera
{
    public:
        Camera();
        ~Camera();

        void Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
        glm::vec3 position, direction, up, right;

        glm::vec3 targetted_brick;

        bool IsInfront(glm::vec3 point) {


            glm::vec3 p = position;

            p -= direction * (float)Map::CHUNK_SIZE;

            glm::vec3 to_point = point - p;

            glm::vec3 new_dir = direction;
            to_point.y = 0;
            new_dir.y = 0;
            float dot = glm::dot(to_point, new_dir);

            if(dot > 0) {
                return true;
            }
            return false;            
        }
    private:
        float fJumpVel;
        bool bFocus;
        bool bground;
        void CheckInput();
        void FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
};

#endif
