#include "pch.h"

#include <libloaderapi.h>

#include "FPScap.h"
#include "performanceProfiling.h"

GXSCENEPRESENT_0x58a960 p_GxScenePresent_0x58a960 = reinterpret_cast<GXSCENEPRESENT_0x58a960>(0x58a960);
GXSCENEPRESENT_0x58a960 p_original_GxScenePresent_0x58a960 = NULL;
extern NTDELAYEXECUTION NtDelayExecution = NULL;
int FPScap = 0;

static LARGE_INTEGER nextFrameTime = {};

void __fastcall detoured_GxScenePresent_0x58a960(uint32_t unknown) {
	if (FPScap < 1) {
		p_original_GxScenePresent_0x58a960(unknown);
		return;
	}

	// From https://github.com/doitsujin/dxvk/blob/4799558d322f67d1ff8f2c3958ff03e776b65bc6/src/util/util_fps_limiter.cpp#L51

	LARGE_INTEGER targetFrameInteval = {};
	targetFrameInteval.QuadPart = getPerformanceCounterFrequency().QuadPart / FPScap;

	if (nextFrameTime.QuadPart == 0) {
		// No need to sleep for first frame
		QueryPerformanceCounter(&nextFrameTime);
		nextFrameTime.QuadPart += targetFrameInteval.QuadPart;

		p_original_GxScenePresent_0x58a960(unknown);
		return;
	}

	LARGE_INTEGER now = {};
	QueryPerformanceCounter(&now);

	if (now.QuadPart < nextFrameTime.QuadPart) {
		LARGE_INTEGER sleep = {};

		// Convert sleep time to microseconds(us) from https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps#using-qpc-in-native-code
		// Align with NtDelayExecution from https://ntdoc.m417z.com/ntdelayexecution
		sleep.QuadPart = (nextFrameTime.QuadPart - now.QuadPart) * 1000000 * -10;
		sleep.QuadPart /= getPerformanceCounterFrequency().QuadPart;

		NtDelayExecution(FALSE, &sleep);
	}

	p_original_GxScenePresent_0x58a960(unknown);

	nextFrameTime.QuadPart = (now.QuadPart < nextFrameTime.QuadPart + targetFrameInteval.QuadPart)
		? nextFrameTime.QuadPart + targetFrameInteval.QuadPart
		: now.QuadPart + targetFrameInteval.QuadPart;
}

int loadNtDelayExecution() {
	// from https://github.com/doitsujin/dxvk/blob/4799558d322f67d1ff8f2c3958ff03e776b65bc6/src/util/util_sleep.cpp#L38
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	if (ntdll) {
		NtDelayExecution = reinterpret_cast<NTDELAYEXECUTION>(GetProcAddress(ntdll, "NtDelayExecution"));
		if (NtDelayExecution) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		return 0;
	}
}
