#pragma once

#include <cstdint>

#include "Vanilla1121_functions.h"

typedef int(__fastcall* CGCAMERA_UPDATECALLBACK_0x511bc0)(void*, uint32_t);
int __fastcall detoured_CGCamera_updateCallback_0x511bc0(void* unknown1, uint32_t camera);
extern CGCAMERA_UPDATECALLBACK_0x511bc0 p_CGCamera_updateCallback_0x511bc0;
extern CGCAMERA_UPDATECALLBACK_0x511bc0 p_original_CGCamera_updateCallback_0x511bc0;

typedef double(__fastcall* ORGANICSMOOTH_0x5b7bb0)(float, float, float);
double __fastcall detoured_OrganicSmooth(float start, float end, float step);
extern ORGANICSMOOTH_0x5b7bb0 p_OrganicSmooth;
extern ORGANICSMOOTH_0x5b7bb0 p_original_OrganicSmooth;

extern float cameraHorizontalAddend;
extern float cameraVerticalAddend;
extern float cameraPitchAddend;
extern bool cameraFollowTarget;
extern bool cameraOrganicSmooth;
extern bool cameraPinHeight;

// Get camera position after editing.
C3Vector editCamera_translatedPosition();
C3Vector editCamera_rotatedForword();
// Get camera position before editing.
C3Vector editCamera_originalPosition();
C3Vector editCamera_originalForword();

void editCamera_init();

