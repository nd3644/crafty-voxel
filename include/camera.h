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

        void Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);
        void CheckGround(Map &map);

        bool CheckCollision(Map &map);

        glm::vec3 tmp;

        bool IsInThirdPersonMode() const;

        float GetCurrentFovModifier() const;
    private: // Private methods
        void CheckInput(Eternal::InputHandle &input);
        void RunMouseLogic();
        void CalcNewFrustumPlanes();

    public: // Public vars
        static constexpr float DEFAULT_STRAFE_SPD = 0.05f;
        float STRAFE_SPD = DEFAULT_STRAFE_SPD;
        
        glm::vec3 position, formerPosition, direction, up, right;
        glm::vec3 targetted_brick;

        enum FrustumPlanes {
            PLANE_LEFT = 0,
            PLANE_RIGHT,
            PLANE_TOP,
            PLANE_BOTTOM,
            PLANE_NEAR,
            PLANE_FAR,
            NUM_PLANES
        };

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
        void CheckInput();
        void FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget);

    public:
        plane_t myFrustumPlanes[NUM_PLANES];
};

#endif
