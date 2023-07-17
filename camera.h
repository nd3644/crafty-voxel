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
        enum FrustumPlanes {
            PLANE_LEFT = 0,
            PLANE_RIGHT,
            PLANE_TOP,
            PLANE_BOTTOM,
            PLANE_FRONT,
            PLANE_BACK,
            NUM_PLANES
        };
        Camera();
        ~Camera();

        static constexpr float DEFAULT_STRAFE_SPD = 0.05f;
        float STRAFE_SPD = DEFAULT_STRAFE_SPD;

        void Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
        glm::vec3 position, formerPosition, direction, up, right;

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
            if(map.GetBrick((int)(floor(position.x+0.5f - d)), (int)(floor(position.z - d)), (int)position.y-1) > 0
            || map.GetBrick((int)(floor(position.x+0.5f + d)), (int)(floor(position.z - d)), (int)position.y-1) > 0
            || map.GetBrick((int)(floor(position.x+0.5f + d)), (int)(floor(position.z + d)), (int)position.y-1) > 0
            || map.GetBrick((int)(floor(position.x+0.5f - d)), (int)(floor(position.z + d)), (int)position.y-1) > 0) {
                bground = true;
                float standingPos = ((int)position.y-1)+2.0f;
                if(position.y < standingPos)
                    position.y = standingPos;
                bricklist.emplace_back((int)(position.x+0.5f), (int)position.y-1, (int)(position.z));
            }

            // Check the roof just for fun
            if(map.GetBrick((int)(floor(position.x+0.5f - d)), (int)(floor(position.z - d)), (int)position.y+1) > 0
            || map.GetBrick((int)(floor(position.x+0.5f + d)), (int)(floor(position.z - d)), (int)position.y+1) > 0
            || map.GetBrick((int)(floor(position.x+0.5f + d)), (int)(floor(position.z + d)), (int)position.y+1) > 0
            || map.GetBrick((int)(floor(position.x+0.5f - d)), (int)(floor(position.z + d)), (int)position.y+1) > 0) {
                if(fJumpVel < 0)
                    fJumpVel = 0;
            }
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
                    if(map.GetBrick(x,z,y) != 0
                    || map.GetBrick(x,z,y-1)) {
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


                            tmp.x = p.x - r.x;
                            tmp.z = p.y - r.y;
                            
                            return true;
                        }
                    }
                }
            }
            return false; 
        }

        glm::vec3 tmp;

        bool IsThirdPerson() const;
    private:
        void CheckInput(Eternal::InputHandle &input);
        void RunMouseLogic();
        void CalcNewFrustumPlanes();
        
    private:
        plane_t FrustumPlanes();
        bool bThirdPerson;
        int jumpCooldown;

        Rect collidingBrick;
        glm::vec3 moveDelta;
        float fJumpVel;
        bool bFocus;
        bool bground;
        void CheckInput();
        void FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
};

#endif
