#pragma once

#include <cstdint>

typedef void(__fastcall* GXSCEBEPRESENT_0x58a960)(uint32_t);
extern GXSCEBEPRESENT_0x58a960 p_GxScenePresent_0x58a960;
extern GXSCEBEPRESENT_0x58a960 p_original_GxScenePresent_0x58a960;
void __fastcall detoured_GxScenePresent_0x58a960(uint32_t unknown);


typedef UINT(WINAPI* NTDELAYEXECUTION)(BOOL, LARGE_INTEGER*);
extern NTDELAYEXECUTION NtDelayExecution;
int loadNtDelayExecution();

extern int FPScap;


