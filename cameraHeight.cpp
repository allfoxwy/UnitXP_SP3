#include "pch.h"

#include "cameraHeight.h"


extern FUNCTION_CAMERAHEIGHT_0x5126B0 p_cameraHeight_0x5126B0 = reinterpret_cast<FUNCTION_CAMERAHEIGHT_0x5126B0>(0x5126B0);
extern FUNCTION_CAMERAHEIGHT_0x5126B0 p_original_cameraHeight_0x5126B0 = NULL;

extern float cameraAddHeight = 0.0f;

int __fastcall detoured_cameraHeight_0x5126B0(void* self, void* ignored, float height, float unknown1, float unknown2, float unknown3) {
	return p_original_cameraHeight_0x5126B0(self, height + cameraAddHeight, unknown1, unknown2, unknown3);
}
