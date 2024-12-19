#pragma once

#include <cstdint>

typedef void(__fastcall* FUNCTION_GAMEQUIT_0x41f9b0)(uint32_t unknown);
void __fastcall detoured_gameQuit_0x41f9b0(uint32_t unknown);

extern FUNCTION_GAMEQUIT_0x41f9b0 p_gameQuit_0x41f9b0;
extern FUNCTION_GAMEQUIT_0x41f9b0 p_original_gameQuit_0x41f9b0;



