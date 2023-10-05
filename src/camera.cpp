#include "camera.h"

#include "shader.h"

#include <SDL2/SDL.h>

#include "mesh.h"
#include "brick_selector_widget.h" 
#include "renderer.h"

#include <types.h>
#include <imgui.h>

extern SDL_Window* myWindow;

Camera::Camera() {
    direction = glm::vec3(-0.5,0,0.5);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
//    position = formerPosition = glm::vec3(4117, 96, 2527); // This is a good starting point. Below is debugging only
    position = formerPosition = glm::vec3(107,60,88);
    bFocus = true;
    bIsInThirdPersonMode = false;
    bIsOnGround = false;
}

Camera::~Camera() { }

void Camera::CalcViewMatrix(Shader &myShader) {
    // The "position" of the camera depends on the 3rd or first person state
    // The position isn't really different but the view is kind of "zoomed out"
    if(bIsInThirdPersonMode) {
        glm::vec3 newPosition = position;
        newPosition -= (direction*6.0f);
        myShader.viewMatrix = glm::lookAt(newPosition, position + direction, up);
    }
    else {
        myShader.viewMatrix = glm::lookAt(position, position + direction, up);        
    }

    // Update the uniform
    GLuint view = glGetUniformLocation(myShader.myProgram, "View");
    glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(myShader.viewMatrix));

}

void Camera::Update(Map &myMap, Shader &myShader, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget) {
    using namespace Eternal;

    CalcNewFrustumPlanes();
    FindTargettedBrick(myMap, input, selectWidget);
    CalcViewMatrix(myShader);

    CheckInput(input);
    UpdatePositionFrmoMoveDelta(myMap);

    RunMouseLogic(input);

    CheckGround(myMap);
    if(!bIsOnGround) {
        fJumpVel += 50.0f * gfDeltaTime;
    }
    else {
        fJumpVel = 0;
    }

    if (input.IsKeyDown(InputHandle::KEY_SPACE) && bIsOnGround) {
        fJumpVel = -10;
//        position.y += 0.05f;
	}
	else if (input.IsKeyDown(InputHandle::KEY_LCTRL)) {
        position -= up * 0.25f;
	}

    position.y -= fJumpVel * gfDeltaTime;
    
    // Push the position for drawing the collider in debug modes
    cameralist.emplace_back(position.x,position.y,position.z - 0.5f);

    // This is just a debug "go up" button
    if(input.IsKeyDown(InputHandle::KEY_1)) {
        fJumpVel = 0;
        position.y += 0.25f;
    }

}

void Camera::CheckInput(Eternal::InputHandle &input) {
    using namespace Eternal;

    moveDelta = glm::vec3(0,0,0);

    glm::vec3 forward = glm::normalize(glm::vec3(direction.x, 0.0f, direction.z));
    right = glm::normalize(glm::cross(direction, up));

    float fAccelModifier = 0.8f;

    STRAFE_SPD = DEFAULT_STRAFE_SPD / 10.0f;
    if (input.IsKeyDown(InputHandle::KEY_LSHIFT)) {
        STRAFE_SPD *= 5.0f;
    }

	if (input.IsKeyDown(InputHandle::KEY_A)) {
//        moveDelta -= right * STRAFE_SPD * gfDeltaTime;
        vMoveAccel -= right * STRAFE_SPD * gfDeltaTime * fAccelModifier;
	}
	else if (input.IsKeyDown(InputHandle::KEY_D)) {
        //moveDelta += right * STRAFE_SPD * gfDeltaTime;
        vMoveAccel += right * STRAFE_SPD * gfDeltaTime * fAccelModifier;
	}

    if (input.IsKeyTap(InputHandle::KEY_F5)) {
        bIsInThirdPersonMode = !bIsInThirdPersonMode;
	}

	if (input.IsKeyDown(InputHandle::KEY_W)) {
        vMoveAccel += forward * STRAFE_SPD * gfDeltaTime * fAccelModifier;
	}
    else {
        if (input.IsKeyDown(InputHandle::KEY_S)) {
            forward.y = 0;
            vMoveAccel -= forward * STRAFE_SPD * gfDeltaTime * fAccelModifier;
        }
    }

    glm::vec3 startValue(1.0f, 2.0f, 3.0f);
    glm::vec3 endValue(4.0f, 5.0f, 6.0f);

    // Interpolate between the two values with a factor of 0.5
    float interpolationFactor = 1.f;
    glm::vec3 interpolatedValue = glm::mix(startValue, endValue, interpolationFactor);

    
    if(glm::length(vMoveAccel) > STRAFE_SPD) {
        vMoveAccel = glm::normalize(vMoveAccel) * STRAFE_SPD;
    }
    vMoveAccel = glm::mix(vMoveAccel, glm::vec3(0,0,0), 6.0f * gfDeltaTime);
}

void Camera::UpdatePositionFrmoMoveDelta(Map &myMap) {

    // We have to shift the y position a little otherwise we get false positives on X/Z collision that will
    // cause the camera to move more slowly on the ground. TODO: Fix this.
    float yOffset = 0.1f;

    moveDelta = vMoveAccel;

    position.y += yOffset;
    position.x += moveDelta.x;
    if(CheckCollision(myMap)) {
        position.x -= moveDelta.x;
//        std::cout << "x ";
    }

    position.z += moveDelta.z;
    if(CheckCollision(myMap)) {
        position.z -= moveDelta.z;
//        std::cout << "z ";
    }
    std::cout.flush();
    position.y -= yOffset;
}

void Camera::RunMouseLogic(Eternal::InputHandle &input) {
    using namespace Eternal;

    int SCR_CENTREX = WIN_W / 2;
    int SCR_CENTREY = WIN_H / 2;

    int mouseX = 0, mouseY = 0;
    if(SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(1)) {
        #ifdef ENABLE_IMGUI
            ImGuiIO& io = ImGui::GetIO();
            if (!io.WantCaptureMouse) {
                bFocus = true;
            }
        #else
            bFocus = true;
        #endif
    }
    if(input.IsKeyDown(InputHandle::KEY_ESCAPE)) {
        bFocus = false;
    }

    if(bFocus) {
        if(!SDL_GetWindowGrab(myWindow)) {
            SDL_SetWindowGrab(myWindow,SDL_TRUE);
            SDL_ShowCursor(SDL_DISABLE);
        }
        float LookSens = 0.05f;

        float yDelta = (SCR_CENTREY - mouseY) * LookSens;
        float xDelta = (SCR_CENTREX - mouseX) * LookSens;

        float currentPitch = glm::degrees(std::asin(direction.y));
        float newPitch = currentPitch + yDelta;

        float clampedPitch = glm::clamp(newPitch, -89.0f, 89.0f);
        float pitchDelta = clampedPitch - currentPitch;

        glm::mat4 pitchRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(pitchDelta), right);
        direction = glm::vec3(pitchRotationMatrix * glm::vec4(direction, 1.0f));

        glm::mat4 yawRotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(xDelta), glm::vec3(0.0f, 1.0f, 0.0f));
        direction = glm::vec3(yawRotationMatrix * glm::vec4(direction, 1.0f));

        SDL_WarpMouseInWindow(myWindow,SCR_CENTREX,SCR_CENTREY);
    }
    else {
        if(SDL_GetWindowGrab(myWindow)) {
            SDL_SetWindowGrab(myWindow,SDL_FALSE);
            SDL_ShowCursor(SDL_ENABLE);
        }
    }

    direction = glm::normalize(direction);
}

void Camera::FindTargettedBrick(Map &myMap, Eternal::InputHandle &input, BrickSelectorWidget &selectWidget) {
    glm::vec3 p = position;
    glm::vec3 outter;

    p.x += 0.5f;
    p.z += 0.5f;
    p.y += 0.5f;

    if((int)position.x <= 0)
        p.x -= 1;
    if((int)position.z <= 0)
        p.z -= 1;

    bool bFound = false;
    for(int i = 0;i < 1000;i++) {
        p += direction / 100.0f;
        if(myMap.GetBrick(p.x, p.z, p.y) != 0) {
            outter = p - (direction / 100.0f);
            bFound = true;
            break;
        }
    }

    if(!bFound) {
        targetted_brick = { -1, -1, -1 };
        return;
    }

    targetted_brick.x = (int)p.x;
    targetted_brick.y = (int)p.y;
    targetted_brick.z = (int)p.z;

    int chunkX = std::floor(targetted_brick.x / chunk_t::CHUNK_SIZE);
    int chunkZ = std::floor(targetted_brick.z / chunk_t::CHUNK_SIZE);
        
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_LEFT)) {

        if(myMap.GetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y) != myMap.IdFromName("bedrock")) {
            myMap.SetBrick((int)targetted_brick.x, (int)targetted_brick.z, (int)targetted_brick.y,0);
            myMap.BuildChunk(chunkX, chunkZ);
        }
    }
    if(input.IsMouseClick(Eternal::InputHandle::MBUTTON_RIGHT)) {
        int brickType = selectWidget.GetSelectedBrickID();
        myMap.SetBrick((int)outter.x, (int)outter.z, (int)outter.y,brickType);
        myMap.BuildChunk(chunkX, chunkZ);
    }

    formerPosition = position;
}

bool Camera::IsInThirdPersonMode() const {
    return bIsInThirdPersonMode;
}

void Camera::CalcNewFrustumPlanes() {
    // All frustum planes intersect the camera position
    for(int i = 0;i < NUM_PLANES;i++) {
        myFrustumPlanes[i].position = position;
    }

    float hFov = glm::degrees(2 * glm::atan(glm::tan(glm::radians(fFov / 2)) * static_cast<float>(WIN_W) / static_cast<float>(WIN_H)));

    // near
    myFrustumPlanes[PLANE_NEAR].normal = direction;


    // far
    myFrustumPlanes[PLANE_FAR].normal = -direction;
    myFrustumPlanes[PLANE_FAR].position += glm::normalize(direction) * gfZFar;

    // right
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(((hFov) / 2.0f) - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_RIGHT].normal = rotatedVector;

    // left
    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians((-((hFov) / 2.0f)) + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_LEFT].normal = rotatedVector;
    
    // top
    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians((fFov / 2.0f) - 90.0f), right);
    rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_TOP].normal = rotatedVector;

    // bottom
    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-(fFov / 2.0f) + 90.0f), right);
    rotatedVector = rotationMatrix * glm::vec4(direction, 1.0f);
    myFrustumPlanes[PLANE_BOTTOM].normal = rotatedVector;
}

void Camera::CheckGround(Map &map) {
    bIsOnGround = false;
    float d = 0.25f;
    if(map.GetBrick((int)(floor(position.x + 1.0f - d)), (int)(floor(position.z + 1.0f - d)), (int)position.y-1) > 0
    || map.GetBrick((int)(floor(position.x + 1.0f + d)), (int)(floor(position.z + 1.0f - d)), (int)position.y-1) > 0
    || map.GetBrick((int)(floor(position.x + 1.0f + d)), (int)(floor(position.z + 1.0f + d)), (int)position.y-1) > 0
    || map.GetBrick((int)(floor(position.x + 1.0f - d)), (int)(floor(position.z + 1.0f + d)), (int)position.y-1) > 0) {
        bIsOnGround = true;
        float standingPos = ((int)position.y-1)+2.0f;
        if(position.y < standingPos)
            position.y = standingPos;
        bricklist.emplace_back((int)(position.x+0.5f), (int)position.y-1, (int)(position.z));
    }

/*     // Check the roof just for fun
    if(map.GetBrick((int)(floor(position.x - d)), (int)(floor(position.z - d)), (int)position.y+1) > 0
    || map.GetBrick((int)(floor(position.x + d)), (int)(floor(position.z - d)), (int)position.y+1) > 0
    || map.GetBrick((int)(floor(position.x + d)), (int)(floor(position.z + d)), (int)position.y+1) > 0
    || map.GetBrick((int)(floor(position.x - d)), (int)(floor(position.z + d)), (int)position.y+1) > 0) {
        if(fJumpVel < 0)
            fJumpVel = 0;
    } */
}

bool Camera::CheckCollision(Map &map) {
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
                p.x += 0.5f;
                p.y += 0.5f;
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

void Camera::DbgDrawCollision_FromTop(float xPos, float yPos, Eternal::Renderer &renderer, Map &myMap) {
    int xindex = 0;
    int zindex = 0;

    // This is just the width of each brick in pixels. This helps to display the fractional intersection into bricks
    float width = 16.0f;

    // These scroll values are intended to keep this "widget" on the screen
    float xOffs = (position.x > 50) ? ((position.x-50)*width) : 0;
    float yOffs = (position.z > 25) ? ((position.z-25)*(width)) : 0;

    xOffs -= xPos;
    yOffs -= yPos;

    // d represents the "viewdistance" for the widget
    int d = 5;
    int startx = position.x - d;
    int endx = position.x + d;

    int startz = position.z - d;
    int endz = position.z + d;

    for (int x = startx; x < endx;x++) {
        for (int z = startz;z < endz;z++) {

            // Empty space is white, occupied space is gray
            bool bShaded = (myMap.GetBrick(x, z, position.y) == 0) ? false : true;
            renderer.SetColor(1, 1, 1, 1);

            if (bShaded) {
                renderer.SetColor(0.5, 0.5, 0.5, 1.0);
            }

            Rect r(x*width,z*width,width,width);
            r.x -= xOffs;
            r.y -= yOffs;

            renderer.DrawQuad(r);

        }
    }

    Rect r(position.x*width,position.z*width,width,width);
    r.x -= xOffs;
    r.y -= yOffs;
    renderer.SetColor(1.0f,0,0,1.0f);
    renderer.DrawQuad(r);
}


void Camera::DbgDrawCollision_FromSide(float xPos, float yPos, Eternal::Renderer &renderer, Map &myMap) {
    int xindex = 0;
    int zindex = 0;

    // This is just the width of each brick in pixels. This helps to display the fractional intersection into bricks
    float width = 16.0f;

    // These scroll values are intended to keep this "widget" on the screen
    float xOffs = (position.x > 50) ? ((position.x-50)*width) : 0;
    float yOffs = (position.z > 25) ? ((position.y-25)*(width)) : 0;

    xOffs -= xPos;
    yOffs -= yPos;

    // d represents the "viewdistance" for the widget
    int d = 5;
    int starty = position.y - d;
    int endy = position.y + d;

    int startx = position.x - d;
    int endx = position.x + d;

    for (int x = startx; x < endx;x++) {
        for (int y = starty;y < endy;y++) {
            // Empty space is white, occupied space is gray
            bool bShaded = (myMap.GetBrick(x, position.z, y) == 0) ? false : true;
            renderer.SetColor(1, 1, 1, 1);

            if (bShaded) {
                renderer.SetColor(0.5, 0.5, 0.5, 1.0);
            }

            Rect r(x*width,(y+0.75f)*width,width,width);
            r.x -= xOffs;
            r.y -= yOffs;

            renderer.DrawQuad(r);

        }
    }

    Rect r(position.x*width,(position.y-0.25f)*width,width,width);
    r.x -= xOffs;
    r.y -= yOffs;
    renderer.SetColor(1.0f,0,0,1.0f);
    renderer.DrawQuad(r);
}
