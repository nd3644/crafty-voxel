#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include "types.h"

extern int WIN_W;
extern int WIN_H;
extern int gblPolyCount;

extern bool bIsFullscreen;

extern int mouseWheelDelta;

extern float fAmbient;

extern std::vector<vec3_t>bricklist;
extern std::vector<vec3_t>cameralist;

extern int gAOLevel;
extern int gViewDist;
extern bool gEnableAO;

#endif
