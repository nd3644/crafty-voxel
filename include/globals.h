#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include "types.h"

extern int WIN_W;
extern int WIN_H;
extern float fAspect;
extern int gblPolyCount;

extern bool bIsFullscreen;

extern int mouseWheelDelta;

extern float fAmbient;
extern float fFov;
extern float gfZNear;
extern float gfZFar;

extern std::vector<vec3_t>bricklist;
extern std::vector<vec3_t>cameralist;

extern int gAOLevel;
extern int gViewDist;
extern bool gEnableAO;
extern bool gbFrustumTopView;
extern bool gbWireFrameEnabled;

extern bool bDbgFrustumView;

extern int gFrustumSkips; // How many chunks were skipped this frame

enum class GlobalRenderModes {
    RENDER_MODE_DEFAULT = 0,
    RENDER_MODE_WIRES = 1,
    NUM_RENDER_MODES
};

extern GlobalRenderModes gRenderMode;

#endif
