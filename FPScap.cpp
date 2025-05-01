#include "pch.h"

#include <libloaderapi.h>

#include "FPScap.h"
#include "performanceProfiling.h"

GXSCEBEPRESENT_0x58a960 p_GxScenePresent_0x58a960 = reinterpret_cast<GXSCEBEPRESENT_0x58a960>(0x58a960);
GXSCEBEPRESENT_0x58a960 p_original_GxScenePresent_0x58a960 = NULL;
extern NTDELAYEXECUTION NtDelayExecution = NULL;
int FPScap = 0;

static LARGE_INTEGER renderStart = {};

void __fastcall detoured_GxScenePresent_0x58a960(uint32_t unknown) {
	p_original_GxScenePresent_0x58a960(unknown);

	if (FPScap < 1) {
		return;
	}

	if (renderStart.QuadPart == 0) {
		// No need to sleep for first frame
		QueryPerformanceCounter(&renderStart);
		return;
	}

	LARGE_INTEGER targetFrameTime = {};
	targetFrameTime.QuadPart = getPerformanceCounterFrequency().QuadPart / FPScap;

	LARGE_INTEGER now = {};
	QueryPerformanceCounter(&now);

	if (now.QuadPart - renderStart.QuadPart < targetFrameTime.QuadPart) {
		LARGE_INTEGER sleep = {};

		// Convert sleep time to microseconds(us)
		// from https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps#using-qpc-in-native-code
		sleep.QuadPart = (targetFrameTime.QuadPart - (now.QuadPart - renderStart.QuadPart)) * 1000000;
		sleep.QuadPart /= getPerformanceCounterFrequency().QuadPart;

		// Align with NtDelayExecution
		// from https://ntdoc.m417z.com/ntdelayexecution
		sleep.QuadPart *= -10;

		NtDelayExecution(FALSE, &sleep);
	}

	QueryPerformanceCounter(&renderStart);
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
