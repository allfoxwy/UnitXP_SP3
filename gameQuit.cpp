#include "pch.h"

#include "gameQuit.h"
#include "timer.h"

extern FUNCTION_GAMEQUIT_0x41f9b0 p_gameQuit_0x41f9b0 = reinterpret_cast<FUNCTION_GAMEQUIT_0x41f9b0>(0x41f9b0);
extern FUNCTION_GAMEQUIT_0x41f9b0 p_original_gameQuit_0x41f9b0 = NULL;


void __fastcall detoured_gameQuit_0x41f9b0(uint32_t unknown) {
	gTimer.end();

	return p_original_gameQuit_0x41f9b0(unknown);
}
