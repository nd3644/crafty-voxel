#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "map.h"
#include "shader.h"
#include "input.h"
#include "map.h"
#include "globals.h"

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

        void CheckGround(Map &map) {
            bground = false;
            float d = 0.25f;
            if(map.GetBrick((int)(position.x+0.5f - d), (int)(position.z - d), (int)position.y-2) > 0
            || map.GetBrick((int)(position.x+0.5f + d), (int)(position.z - d), (int)position.y-2) > 0
            || map.GetBrick((int)(position.x+0.5f + d), (int)(position.z + d), (int)position.y-2) > 0
            || map.GetBrick((int)(position.x+0.5f - d), (int)(position.z + d), (int)position.y-2) > 0) {
                bground = true;
                bricklist.emplace_back((int)(position.x+0.5f), (int)position.y-2, (int)(position.z));
            }
/*            else {
                Rect p;
                p.x = position.x;
                p.y = position.z;
                p.y -= (1.0f/2.0f);
                p.w = p.h = 0.5f;
                p.x += 0.25f;
                p.y += 0.25f;
                glm::vec2 n;
                if(p.IsColliding(collidingBrick,n)) {
                    position.x += n.x;
                    position.z += n.y;
                }
            }*/
        }

        bool CheckCollision(Map &map) {
            float size = 1;
            Rect r(0,0,size,size);

            int camX = position.x;
            int camY = position.y;
            int camZ = position.z;

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
                        p.w = p.h = 0.5f;
                        p.x += 0.25f;
                        p.y += 0.25f;
                        glm::vec2 n;
                        if(p.IsColliding(r, n)) {
                            collidingBrick = r;
                            return true;
                        }
                    }
                }
            }
            return false;
        }
    private:

        Rect collidingBrick;
        float AngleX, AngleY;
        float fJumpVel;
        bool bFocus;
        bool bground;
        void CheckInput();
        void FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
};

#endif
