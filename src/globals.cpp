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

int gAOLevel = 88;
bool gEnableAO = true;

bool gbFrustumTopView = false;
bool gbWireFrameEnabled = false;

int gViewDist = 20;
float fFov = 75.0f;
float gfZNear = 0.1f;
float gfZFar = 1000.0f;
float gfDeltaTime = 0.0f;

int gFrustumSkips = 0; // How many chunks were skipped this frame

bool bDbgCollisionViews = false;

GlobalRenderModes gRenderMode = GlobalRenderModes::RENDER_MODE_DEFAULT;