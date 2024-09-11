// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <string>
#include <sstream>

#include "MinHook.h"
#include "Vanilla1121_functions.h"

using namespace std;

std::wstring utf8_to_utf16(const std::string& utf8);
int UnitXP_inSight(string unit0, string unit1);
float UnitXP_distanceBetween(string unit0, string unit1);

LUA_CFUNCTION p_original_UnitXP = NULL;
LUA_CFUNCTION p_UnitXP = reinterpret_cast<LUA_CFUNCTION>(0x00517350);

int __fastcall detoured_UnitXP(void* L) {
    if (lua_gettop(L) >= 2) {
        string cmd{ lua_tostring(L, 1) };

        if (cmd == "inSight" && lua_gettop(L) >= 3) {
            int result = UnitXP_inSight(lua_tostring(L, 2), lua_tostring(L, 3));
            if (result >= 0) {
                lua_pushboolean(L, result);
                return 1;
            }
            else {
                lua_pushnil(L);
                return 1;
            }
        }
        else if (cmd == "distanceBetween" && lua_gettop(L) >= 3) {
            float result = UnitXP_distanceBetween(lua_tostring(L, 2), lua_tostring(L, 3));
            if (result >= 0) {
                lua_pushnumber(L, result);
                return 1;
            }
            else {
                lua_pushnil(L);
                return 1;
            }
        }
    }
    return p_original_UnitXP(L);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        if (MH_Initialize() != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to initialize MinHook library.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_UnitXP, &detoured_UnitXP, reinterpret_cast<LPVOID*>(&p_original_UnitXP)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for UnitXP function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_EnableHook(p_UnitXP) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when enabling UnitXP function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        break;
    case DLL_PROCESS_DETACH:
        MH_Uninitialize();
        break;
    }
    return TRUE;
}

extern "C" {
__declspec(dllexport) void FirstEnterWorld(void) {
    lua_func_reg l[] = {
        {u8"UnitXP_SP3", p_UnitXP},
        {u8"UnitXP_SP3_inSight", p_UnitXP},
        {u8"UnitXP_SP3_distanceBetween", p_UnitXP},
        {NULL, NULL}
    };

    luaL_openlib(GetContext(), u8"Vanilla1121mod", l, 0);
}
}
