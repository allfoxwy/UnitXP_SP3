// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <string>
#include <sstream>
#include <limits>

#include "MinHook.h"
#include "utf8_to_utf16.h"
#include "Vanilla1121_functions.h"
#include "inSight.h"
#include "distanceBetween.h"
#include "modernNameplateDistance.h"
#include "targeting.h"
#include "notifyOS.h"

using namespace std;


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
        else if (cmd == "target" && lua_gettop(L) >= 2) {
            string subcmd{ lua_tostring(L,2) };
            if (subcmd == "nearestEnemy") {
                lua_pushboolean(L, targetNearestEnemy(FLT_MAX));
                return 1;
            }
            if (subcmd == "nextEnemyConsideringDistance") {
                lua_pushboolean(L, targetEnemyConsideringDistance(&selectNext));
                return 1;
            }
            if (subcmd == "previousEnemyConsideringDistance") {
                lua_pushboolean(L, targetEnemyConsideringDistance(&selectPrevious));
                return 1;
            }
            if (subcmd == "nextEnemyInCycle") {
                lua_pushboolean(L, targetEnemyInCycle(&selectNext));
                return 1;
            }
            if (subcmd == "previousEnemyInCycle") {
                lua_pushboolean(L, targetEnemyInCycle(&selectPrevious));
                return 1;
            }
            if (subcmd == "nextMarkedEnemyInCycle") {
                if (lua_gettop(L) >= 3 && lua_isstring(L, 3)) {
                    lua_pushboolean(L, targetMarkedEnemyInCycle(&selectNextMark, lua_tostring(L, 3)));
                }
                else {
                    lua_pushboolean(L, targetMarkedEnemyInCycle(&selectNextMark, ""));
                }
                return 1;
            }
            if (subcmd == "previousMarkedEnemyInCycle") {
                if (lua_gettop(L) >= 3 && lua_isstring(L, 3)) {
                    lua_pushboolean(L, targetMarkedEnemyInCycle(&selectPreviousMark, lua_tostring(L, 3)));
                }
                else {
                    lua_pushboolean(L, targetMarkedEnemyInCycle(&selectPreviousMark, ""));
                }
                return 1;
            }
            if (subcmd == "worldBoss") {
                lua_pushboolean(L, targetWorldBoss(FLT_MAX));
                return 1;
            }
            if (subcmd == "rangeCone") {
                if (lua_gettop(L) >= 3 && lua_isnumber(L, 3)) {
                    double n = lua_tonumber(L, 3);
                    if (n > 1.99 && n < FLT_MAX) {
                        targetingRangeCone = static_cast<float>(n);
                    }
                }
                lua_pushnumber(L, targetingRangeCone);
                return 1;
            }
            lua_pushnil(L);
            return 1;
        }
        else if (cmd == "modernNameplateDistance") {
            if (lua_gettop(L) >= 2) {
                string subcmd{ lua_tostring(L, 2) };
                if (subcmd == "enable") {
                    modernNameplateDistance = true;
                }
                else if (subcmd == "disable") {
                    modernNameplateDistance = false;
                }
            }
            lua_pushboolean(L, modernNameplateDistance);
            return 1;
        }
        else if (cmd == "notify") {
            if (lua_gettop(L) >= 2) {
                string subcmd{ lua_tostring(L, 2) };
                if (subcmd == "taskbarIcon") {
                    flashTaskbarIcon();
                    lua_pushboolean(L, true);
                    return 1;
                }

                if (subcmd == "systemSound" && lua_gettop(L) >= 3) {
                    string soundName{ lua_tostring(L, 3) };
                    lua_pushboolean(L, playSystemSound(soundName));
                    return 1;
                }
            }
            lua_pushboolean(L, false);
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
        if (MH_CreateHook(p_addNameplate, &detoured_addNameplate, reinterpret_cast<LPVOID*>(&p_original_addNameplate)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for addNameplate function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_renderWorld, &detoured_renderWorld, reinterpret_cast<LPVOID*>(&p_original_renderWorld)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for renderWorld function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when enabling hooks.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:
        if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to disable hooks. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }

        if (MH_RemoveHook(p_renderWorld) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for renderWorld function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }

        if (MH_RemoveHook(p_addNameplate) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for addNameplate function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }

        if (MH_RemoveHook(p_UnitXP) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for UnitXP function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }

        if (MH_Uninitialize() != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to uninitialize MinHook. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        break;
    }
    return TRUE;
}


extern "C" {
    // This function would be called by vanilla-dll-sideloader when EnterWorld event happens
    __declspec(dllexport) void FirstEnterWorld(void) {
        lua_func_reg l[] = {
            {u8"UnitXP_SP3", p_UnitXP},
            {u8"UnitXP_SP3_inSight", p_UnitXP},
            {u8"UnitXP_SP3_distanceBetween", p_UnitXP},
            {u8"UnitXP_SP3_modernNameplateDistance", p_UnitXP},
            {u8"UnitXP_SP3_target", p_UnitXP},
            {u8"UnitXP_SP3_notify", p_UnitXP},
            {NULL, NULL}
        };

        luaL_openlib(GetContext(), u8"Vanilla1121mod", l, 0);
    }
}
