#pragma once

// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
typedef void(__thiscall* WEATHER_SETTYPE)(void*, int, float, bool);
extern WEATHER_SETTYPE p_weather_setType;
extern WEATHER_SETTYPE p_original_weather_setType;

extern bool weather_alwaysClear;

void __fastcall detoured_weather_setType(void* self, void* ignored, int type, float intensity, bool unknown);
