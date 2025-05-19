#pragma once

#include <cstdint>

#include "Vanilla1121_functions.h"

typedef int(__fastcall* CGCAMERA_UPDATECALLBACK_0x511bc0)(void*, uint32_t);
int __fastcall detoured_CGCamera_updateCallback_0x511bc0(void* unknown1, uint32_t camera);

extern CGCAMERA_UPDATECALLBACK_0x511bc0 p_CGCamera_updateCallback_0x511bc0;
extern CGCAMERA_UPDATECALLBACK_0x511bc0 p_original_CGCamera_updateCallback_0x511bc0;

extern float cameraHorizontalAddend;
extern float cameraVerticalAddend;
extern bool cameraFollowTarget;
extern bool cameraFixedPosition;

// Get camera position after editing.
C3Vector editCamera_translatedPosition();
// Get camera position before editing.
C3Vector editCamera_originalPosition();
