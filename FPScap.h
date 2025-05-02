#pragma once

#include <cstdint>

typedef void(__fastcall* GXSCENEPRESENT_0x58a960)(uint32_t);
extern GXSCENEPRESENT_0x58a960 p_GxScenePresent_0x58a960;
extern GXSCENEPRESENT_0x58a960 p_original_GxScenePresent_0x58a960;
void __fastcall detoured_GxScenePresent_0x58a960(uint32_t unknown);

typedef UINT(WINAPI* NTDELAYEXECUTION)(BOOL, LARGE_INTEGER*);
extern NTDELAYEXECUTION NtDelayExecution;
int initFPScap();

extern LARGE_INTEGER targetFrameInterval;


