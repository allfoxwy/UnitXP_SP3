#pragma once

// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
typedef int(__thiscall* FUNCTION_CAMERAHEIGHT_0x5126B0)(void* self, float height, float unknown1, float unknown2, float unknown3);
int __fastcall detoured_cameraHeight_0x5126B0(void* self, void* ignored, float height, float unknown1, float unknown2, float unknown3);

extern FUNCTION_CAMERAHEIGHT_0x5126B0 p_cameraHeight_0x5126B0;
extern FUNCTION_CAMERAHEIGHT_0x5126B0 p_original_cameraHeight_0x5126B0;

extern float cameraAddHeight;
