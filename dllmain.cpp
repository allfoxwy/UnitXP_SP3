// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "sysinfoapi.h" // We need Windows Vista or newer for GetTickCount64()

#include <string>
#include <sstream>

#include "MinHook.h"
#include "Vanilla1121_functions.h"
#include "inSight.h"
#include "distanceBetween.h"
#include "modernNameplateDistance.h"

using namespace std;


std::wstring utf8_to_utf16(const std::string& utf8);

LUA_CFUNCTION p_original_UnitXP = NULL;
LUA_CFUNCTION p_UnitXP = reinterpret_cast<LUA_CFUNCTION>(0x517350);


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
        else if (cmd == "getTickCount") {
            lua_pushnumber(L, (double)GetTickCount64());
            return 1;
        }
        else if (cmd == "modernNameplateDistance" && lua_gettop(L) >= 2) {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                modernNameplateDistance = true;
            }
            else if (subcmd == "disable") {
                modernNameplateDistance = false;
            }
            else if (subcmd == "enableFriendRefresh") {
                modernNameplateDistanceRefreshFriend = true;
            }
            else if (subcmd == "disableFriendRefresh") {
                modernNameplateDistanceRefreshFriend = false;
            }
            else if (subcmd == "enableEnemyRefresh") {
                modernNameplateDistanceRefreshEnemy = true;
            }
            else if (subcmd == "disableEnemyRefresh") {
                modernNameplateDistanceRefreshEnemy = false;
            }
            lua_pushboolean(L, modernNameplateDistance);
            return 1;
        }
    }
    return p_original_UnitXP(L);
}



BOOL APIENTRY DllMain(HMODULE hModule,
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
        if (MH_CreateHook(p_addNameplate, &detoured_addNameplate, reinterpret_cast<LPVOID*>(&p_original_addNameplate)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for addNameplate function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_EnableHook(p_addNameplate) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when enabling addNameplate function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_renderWorld, &detoured_renderWorld, reinterpret_cast<LPVOID*>(&p_original_renderWorld)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for renderWorld function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_EnableHook(p_renderWorld) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when enabling renderWorld function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
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
            {u8"UnitXP_SP3_modernNameplateDistance", p_UnitXP},
            {NULL, NULL}
        };

        luaL_openlib(GetContext(), u8"Vanilla1121mod", l, 0);
    }
}
