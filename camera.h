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

        bool CheckCollision(Map &map) {
            float size = 16;
            Rect r(0,0,size,size);
            Rect c(0,0,16,16);

            int camX = position.x;
            int camY = position.y;
            int camZ = position.z;

            bground = false;
            // Draw cur level
            for(int x = camX-8;x < camX+8;x++) {
                r.x = x * size;
                for(int z = camZ-8;z < camZ+8;z++) {
                    r.y = z * size;
                    int y = camY;

                    // shin level
                    if(map.GetBrick(x,z,y-1) != 0) {
                        Rect p;
                        p.x = position.x * size;
                        p.y = position.z * size;
                        p.y -= (size/2);
                        p.w = p.h = 14;
                        p.x += 1;
                        p.y += 1;
                        if(p.IsColliding(r)) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
    private:
        float AngleX, AngleY;
        float fJumpVel;
        bool bFocus;
        bool bground;
        void CheckInput();
        void FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
};

#endif
