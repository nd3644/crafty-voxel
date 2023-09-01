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
    public: // Public methods
        Camera();
        ~Camera();

        /*!
            Updates myShader's view matrix by handling inputs and resolving any collisions with the map.
        */
        void Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
        bool IsInThirdPersonMode() const;
        float GetCurrentFovModifier() const;

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
        static constexpr float DEFAULT_STRAFE_SPD = 0.05f;
        float STRAFE_SPD = DEFAULT_STRAFE_SPD;
        
        glm::vec3 position, formerPosition, direction, up, right;
        glm::vec3 targetted_brick;

    private: // Private vars
        float fFovModifier;
        bool bSprinting;
        int ticksSinceLastForwardPress;

        bool bIsInThirdPersonMode;
        int jumpCooldown;

        Rect collidingBrick;
        glm::vec3 moveDelta;
        float fJumpVel;
        bool bFocus;
        bool bIsOnGround;
        glm::vec3 tmp;
};

#endif
