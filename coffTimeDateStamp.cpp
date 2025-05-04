#include "pch.h"

#include <cstdint>

#include <psapi.h>
#include <processthreadsapi.h>

#include "coffTimeDateStamp.h"



double coffTimeDateStamp() {
    MODULEINFO info = {};
    BOOL r = GetModuleInformation(GetCurrentProcess(), moduleSelf, &info, sizeof(info));
    if (r == FALSE) {
        return -1;
    }

    uint32_t base = reinterpret_cast<uint32_t>(info.lpBaseOfDll);

    uint32_t peHeaderOffset = *reinterpret_cast<uint32_t*>(base + 0x3c);

    // TimeDateStamp of COFF header
    return *reinterpret_cast<uint32_t*>(base + peHeaderOffset + 0x8);
}
