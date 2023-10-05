#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "map.h"
#include "shader.h"
#include "input.h"
#include "map.h"
#include "globals.h"

class BrickSelectorWidget;
namespace Eternal { class Renderer; }
class Camera
{
    public: // Public methods
        Camera();
        ~Camera();

        /*!
            Updates myShader's view matrix by handling inputs and resolving any collisions with the map.
        */
        void Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
        bool IsInThirdPersonMode() const;
        float GetCurrentFovModifier() const;

        bool DoesLineIntersectFrustum(glm::vec3 &start, glm::vec3 &end) {
            return DoesLineIntersectFrustum(start,end,0,8);
        }
        bool DoesLineIntersectFrustum(glm::vec3 &start, glm::vec3 &end, int depth, int maxdepth) {
            if(depth >= maxdepth) {
                return false;
            }

            glm::vec3 point = (start+end) * 0.5f;

            if(    glm::dot(myFrustumPlanes[Camera::PLANE_LEFT].position - point,myFrustumPlanes[Camera::PLANE_LEFT].normal) <= 10
                && glm::dot(myFrustumPlanes[Camera::PLANE_RIGHT].position - point,myFrustumPlanes[Camera::PLANE_RIGHT].normal) <= 10
                && glm::dot(myFrustumPlanes[Camera::PLANE_NEAR].position - point,myFrustumPlanes[Camera::PLANE_NEAR].normal) <= 10
                && glm::dot(myFrustumPlanes[Camera::PLANE_TOP].position - point,myFrustumPlanes[Camera::PLANE_TOP].normal) <= 10
                && glm::dot(myFrustumPlanes[Camera::PLANE_BOTTOM].position - point,myFrustumPlanes[Camera::PLANE_BOTTOM].normal) <= 10) {
                return true;
            }

            return DoesLineIntersectFrustum(start,point,depth+1,maxdepth) || DoesLineIntersectFrustum(point,end, depth+1,maxdepth);
        }


        /* Both methods assume 2D rendering is possible */
        void DbgDrawCollision_FromTop(float xPos, float yPos, Eternal::Renderer &renderer, Map &myMap);
        void DbgDrawCollision_FromSide(float xPos, float yPos, Eternal::Renderer &renderer, Map &myMap);

    private: // Private methods
        void CheckInput(Eternal::InputHandle &input);
        void FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
        void RunMouseLogic(Eternal::InputHandle &input);
        void CalcNewFrustumPlanes();
        void CalcViewMatrix(Shader &myShader);
        void UpdatePositionFrmoMoveDelta(Map &myMap);
        bool CheckCollision(Map &map);
        void CheckGround(Map &map);

    public: // Public vars
        enum FrustumPlanes {
            PLANE_LEFT = 0,
            PLANE_RIGHT,
            PLANE_TOP,
            PLANE_BOTTOM,
            PLANE_NEAR,
            PLANE_FAR,
            NUM_PLANES
        };
        plane_t myFrustumPlanes[NUM_PLANES];
        static constexpr float DEFAULT_STRAFE_SPD = 2.5f;
        float STRAFE_SPD = DEFAULT_STRAFE_SPD;
        
        glm::vec3 position, formerPosition, direction, up, right;
        glm::vec3 targetted_brick;

    private: // Private vars

        glm::vec3 vMoveAccel;

        bool bIsInThirdPersonMode;
        int jumpCooldown;

        float forwardVel;

        Rect collidingBrick;
        glm::vec3 moveDelta;
        float fJumpVel;
        bool bFocus;
        bool bIsOnGround;
        glm::vec3 tmp;
};

#endif
