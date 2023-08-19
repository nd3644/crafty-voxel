#include "globals.h"

int WIN_W = 1366;
int WIN_H = 768;
float fAspect = WIN_W / WIN_H;
int gblPolyCount = 0;

bool bIsFullscreen = false;
int mouseWheelDelta = 0;

float fAmbient = 0.8f;

std::vector<vec3_t>bricklist;
std::vector<vec3_t>cameralist;

int gAOLevel = 90;
bool gEnableAO = true;

bool gbFrustumTopView = false;

int gViewDist = 40;
float fFov = 75.0f;
float gfZNear = 0.1f;
float gfZFar = 1000.0f;