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
#include "timer.h"
#include "cameraHeight.h"
#include "gameQuit.h"
#include "coffTimeDateStamp.h"
#include "screenshot.h"
#include "gameSocket.h"

using namespace std;

extern HMODULE moduleSelf = 0;

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
        else if (cmd == "target") {
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
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                modernNameplateDistance = true;
            }
            else if (subcmd == "disable") {
                modernNameplateDistance = false;
            }
            lua_pushboolean(L, modernNameplateDistance);
            return 1;
        }
        else if (cmd == "prioritizeTargetNameplate") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                prioritizeTargetNameplate = true;
            }
            else if (subcmd == "disable") {
                prioritizeTargetNameplate = false;
            }
            lua_pushboolean(L, prioritizeTargetNameplate);
            return 1;
        }
        else if (cmd == "prioritizeMarkedNameplate") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                prioritizeMarkedNameplate = true;
            }
            else if (subcmd == "disable") {
                prioritizeMarkedNameplate = false;
            }
            lua_pushboolean(L, prioritizeMarkedNameplate);
            return 1;
        }
        else if (cmd == "timer") {
            string subcmd{ lua_tostring(L,2) };
            if (subcmd == "arm" && lua_gettop(L) >= 5 && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isstring(L, 5)) {
                lua_pushnumber(L, gTimer.add(static_cast<uint64_t>(lua_tonumber(L, 3)), lua_tostring(L, 5), static_cast<uint64_t>(lua_tonumber(L, 4))));
                return 1;
            }
            if (subcmd == "disarm" && lua_gettop(L) >= 3 && lua_isnumber(L, 3)) {
                lua_pushboolean(L, gTimer.remove(static_cast<CppTime::timer_id>(lua_tonumber(L, 3))));
                return 1;
            }
            if (subcmd == "size") {
                lua_pushnumber(L, gTimer.size());
                return 1;
            }
            lua_pushnil(L);
            return 1;
        }
        else if (cmd == "cameraHeight") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "set" && lua_isnumber(L, 3)) {
                float userValue = static_cast<float>(lua_tonumber(L, 3));
                if (userValue < 0.0f) {
                    userValue = 0.0f;
                }
                if (userValue > 6.0f) {
                    userValue = 6.0f;
                }
                cameraAddHeight = userValue;
            }
            lua_pushnumber(L, cameraAddHeight);
            return 1;
        }
        else if (cmd == "notify") {
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

            lua_pushnil(L);
            return 1;
        }
        else if (cmd == "nop") {
            lua_pushboolean(L, true);
            return 1;
        }
        else if (cmd == "version") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "coffTimeDateStamp") {
                lua_pushnumber(L, coffTimeDateStamp());
                return 1;
            }
            if (subcmd == "additionalInformation") {
                lua_pushstring(L, "Konaka-tcp_quickack");
                return 1;
            }
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
        // We should not disable Thread Library Calls even if we are not using it.
        // Because static version of C run-time library needs it: https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
        //DisableThreadLibraryCalls(hModule);

        // Store module handle
        moduleSelf = hModule;

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
        if (MH_CreateHook(p_cameraHeight_0x5126B0, &detoured_cameraHeight_0x5126B0, reinterpret_cast<LPVOID*>(&p_original_cameraHeight_0x5126B0)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for cameraHeight function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_gameQuit_0x41f9b0, &detoured_gameQuit_0x41f9b0, reinterpret_cast<LPVOID*>(&p_original_gameQuit_0x41f9b0)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for gameQuit function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_CTgaFile_Write_0x5a4810, &detoured_CTgaFile_Write_0x5a4810, reinterpret_cast<LPVOID*>(&p_original_CTgaFile_Write_0x5a4810)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for screenShot function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHookApi(L"WSOCK32.DLL", "send", &detoured_send, reinterpret_cast<LPVOID*>(&p_original_send)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for gameSocket send function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHookApi(L"WSOCK32.DLL", "recv", &detoured_recv, reinterpret_cast<LPVOID*>(&p_original_recv)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for gameSocket recv function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when enabling hooks.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:
        // According to https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
        // We should only free resources when lpReserved == NULL
        // Otherwise we should wait for OS reclaim the memory
        if (lpReserved == NULL) {
            if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to disable hooks. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_original_recv) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for gameSocket recv function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_original_send) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for gameSocket send function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_CTgaFile_Write_0x5a4810) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for screenShot function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_gameQuit_0x41f9b0) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for gameQuit function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_cameraHeight_0x5126B0) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for cameraHeight function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
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
        }
        break;
    }
    return TRUE;
}


