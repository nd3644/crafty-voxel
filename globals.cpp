#include "globals.h"

#define _IM 

int WIN_W = 1366;
int WIN_H = 768;
int gblPolyCount = 0;

bool bIsFullscreen = false;
int mouseWheelDelta = 0;

float fAmbient = 0.8f;

std::vector<vec3_t>bricklist;
std::vector<vec3_t>cameralist;

int gAOLevel = 90;
bool gEnableAO = true;

int gViewDist = 16;
