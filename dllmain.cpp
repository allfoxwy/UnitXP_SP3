// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#define _USE_MATH_DEFINES

#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
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
#include "gameQuit.h"
#include "coffTimeDateStamp.h"
#include "screenshot.h"
#include "gameSocket.h"
#include "editCamera.h"
#include "performanceProfiling.h"
#include "LuaDebug.h"
#include "FPScap.h"
#include "edit_CWorld_Intersect.h"
#include "weather.h"
#include "polyfill.h"

using namespace std;

HMODULE moduleSelf = 0;

LUA_CFUNCTION p_original_UnitXP = NULL;
auto p_UnitXP = reinterpret_cast<LUA_CFUNCTION>(0x517350);

typedef void(__fastcall* FUNCTION_ADDRESS_CHECK_0x42a320)(uint32_t);
static auto p_function_address_check_0x42a320 = reinterpret_cast<FUNCTION_ADDRESS_CHECK_0x42a320>(0x42a320);
static FUNCTION_ADDRESS_CHECK_0x42a320 p_original_function_address_check_0x42a320 = NULL;
void __fastcall disabled_function_address_check_0x42a320(uint32_t addr) {
    return;
}

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
            distanceMeters meter = METER_RANGED; // While in-DLL we default to METER_GAUSSIAN, for Lua we default to METER_RANGED
            if (lua_gettop(L) >= 4) {
                string meterName{ lua_tostring(L, 4) };
                if (meterName == "meleeAutoAttack") {
                    meter = METER_MELEE_AUTOATTACK;
                }
                else if (meterName == "AoE") {
                    meter = METER_AOE;
                }
                else if (meterName == "chains") {
                    meter = METER_CHAINS;
                }
                else if (meterName == "Gaussian") {
                    meter = METER_GAUSSIAN;
                }
            }

            float result = UnitXP_distanceBetween(lua_tostring(L, 2), lua_tostring(L, 3), meter);
            if (result >= 0) {
                lua_pushnumber(L, result);
                return 1;
            }
            else {
                lua_pushnil(L);
                return 1;
            }
        }
        else if (cmd == "behind" && lua_gettop(L) >= 3) {
            int result = UnitXP_behind(lua_tostring(L, 2), lua_tostring(L, 3));
            if (result >= 0) {
                lua_pushboolean(L, result);
                return 1;
            }
            else {
                lua_pushnil(L);
                return 1;
            }
        }
        else if (cmd == "target") {
            string subcmd{ lua_tostring(L, 2) };
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
            if (subcmd == "mostHP") {
                lua_pushboolean(L, targetEnemyWithMostHP(targetingFarRange));
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
            if (subcmd == "farRange") {
                if (lua_gettop(L) >= 3 && lua_isnumber(L, 3)) {
                    double n = lua_tonumber(L, 3);
                    if (n > 25.0 && n < 61.0) {
                        targetingFarRange = static_cast<float>(n);
                    }
                }
                lua_pushnumber(L, targetingFarRange);
                return 1;
            }
            if (subcmd == "disableInCombatFilter") {
                targetingInCombatFilter = false;
                lua_pushboolean(L, targetingInCombatFilter);
                return 1;
            }
            if (subcmd == "enableInCombatFilter") {
                targetingInCombatFilter = true;
                lua_pushboolean(L, targetingInCombatFilter);
                return 1;
            }
            lua_pushnil(L);
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
        else if (cmd == "hideCritterNameplate") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                hideCritterNameplate = true;
            }
            else if (subcmd == "disable") {
                hideCritterNameplate = false;
            }
            lua_pushboolean(L, hideCritterNameplate);
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
        else if (cmd == "nameplateCombatFilter") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                nameplateCombatFilter = true;
            }
            else if (subcmd == "disable") {
                nameplateCombatFilter = false;
            }
            lua_pushboolean(L, nameplateCombatFilter);
            return 1;
        }
        else if (cmd == "showInCombatNameplatesNearPlayer") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                showInCombatNameplatesNearPlayer = true;
            }
            else if (subcmd == "disable") {
                showInCombatNameplatesNearPlayer = false;
            }
            lua_pushboolean(L, showInCombatNameplatesNearPlayer);
            return 1;
        }

        else if (cmd == "FPScap") {
            if (lua_isnumber(L, 2)) {
                double v = lua_tonumber(L, 2);
                if (v < 1) {
                    targetFrameInterval.QuadPart = 0;
                }
                else if (v > 500) {
                    targetFrameInterval.QuadPart = getPerformanceCounterFrequency().QuadPart / 500;
                }
                else {
                    targetFrameInterval.QuadPart = getPerformanceCounterFrequency().QuadPart / static_cast<LONGLONG>(v);
                }
            }
            if (targetFrameInterval.QuadPart > 0) {
                lua_pushnumber(L, static_cast<double>(getPerformanceCounterFrequency().QuadPart / targetFrameInterval.QuadPart));
            }
            else {
                lua_pushnumber(L, 0);
            }
            return 1;
        }
        else if (cmd == "debug") {
            string subcmd{ lua_tostring(L,2) };
            if (subcmd == "breakpoint") {
                int result = LuaDebug_breakpoint();
                lua_pushnumber(L, result);
                return 1;
            }
        }
        else if (cmd == "cameraHeight") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "set" && lua_isnumber(L, 3)) {
                float userValue = static_cast<float>(lua_tonumber(L, 3));
                if (userValue < 0.0f) {
                    userValue = 0.0f;
                }
                if (userValue > 4.0f) {
                    userValue = 4.0f;
                }
                cameraVerticalAddend = userValue;
            }
            lua_pushnumber(L, cameraVerticalAddend);
            return 1;
        }
        else if (cmd == "cameraVerticalDisplacement") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "set" && lua_isnumber(L, 3)) {
                float userValue = static_cast<float>(lua_tonumber(L, 3));
                if (userValue < -1.0f) {
                    userValue = -1.0f;
                }
                if (userValue > 4.0f) {
                    userValue = 4.0f;
                }
                cameraVerticalAddend = userValue;
            }
            lua_pushnumber(L, cameraVerticalAddend);
            return 1;
        }
        else if (cmd == "cameraHorizontalDisplacement") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "set" && lua_isnumber(L, 3)) {
                float userValue = static_cast<float>(lua_tonumber(L, 3));
                if (userValue < -4.0f) {
                    userValue = -4.0f;
                }
                if (userValue > 4.0f) {
                    userValue = 4.0f;
                }
                cameraHorizontalAddend = userValue;
            }
            lua_pushnumber(L, cameraHorizontalAddend);
            return 1;
        }
        else if (cmd == "cameraPitch") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "set" && lua_isnumber(L, 3)) {
                float userValue = static_cast<float>(lua_tonumber(L, 3));
                if (userValue < 0.0f) {
                    userValue = 0.0f;
                }
                if (userValue > 0.3f) {
                    userValue = 0.3f;
                }
                cameraPitchAddend = userValue;
            }
            lua_pushnumber(L, cameraPitchAddend);
            return 1;
        }
        else if (cmd == "cameraFollowTarget") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                cameraFollowTarget = true;
            }
            if (subcmd == "disable") {
                cameraFollowTarget = false;
            }
            lua_pushboolean(L, cameraFollowTarget);
            return 1;
        }
        else if (cmd == "cameraOrganicSmooth") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                cameraOrganicSmooth = true;
            }
            if (subcmd == "disable") {
                cameraOrganicSmooth = false;
            }
            lua_pushboolean(L, cameraOrganicSmooth);
            return 1;
        }
        else if (cmd == "cameraPinHeight") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                cameraPinHeight = true;
            }
            if (subcmd == "disable") {
                cameraPinHeight = false;
            }
            lua_pushboolean(L, cameraPinHeight);
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
                lua_pushstring(L, "Konaka-main");
                return 1;
            }
        }
        else if (cmd == "weatherAlwaysClear") {
            string subcmd{ lua_tostring(L, 2) };
            if (subcmd == "enable") {
                weather_alwaysClear = true;
            }
            if (subcmd == "disable") {
                weather_alwaysClear = false;
            }
            lua_pushboolean(L, weather_alwaysClear);
            return 1;
        }
        else if (cmd == "behindThreshold") {
            string subcmd{ lua_tostring(L,2) };
            if (subcmd == "set" && lua_gettop(L) >= 3 && lua_isnumber(L, 3)) {
                double n = lua_tonumber(L, 3);
                if (n < 0) {
                    n = 0;
                }
                if (n > M_PI) {
                    n = M_PI;
                }
                behind_threshold = static_cast<float>(n);
            }
            lua_pushnumber(L, behind_threshold);
            return 1;
        }
        else if (cmd == "performanceProfile") {
            lua_pushstring(L, perfSummary().data());

            string subcmd{ lua_tostring(L,2) };
            if (subcmd == "reset") {
                perfReset();
            }

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
    {
        // We should not disable Thread Library Calls even we are not using it.
        // Because static version of C run-time library needs it: https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
        //DisableThreadLibraryCalls(hModule);

        if (MH_Initialize() != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to initialize MinHook library.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }

        // Store module handle
        moduleSelf = hModule;

        // Initialize performance profiling
        perfReset();

        polyfill_checkCPU();

        editCamera_init();

        if (initFPScap() != 1) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to load NtDelayExecution function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }

        if (edit_CWorld_Intersect_init() == false) {
            return FALSE;
        }

        if (MH_CreateHook(p_function_address_check_0x42a320, &disabled_function_address_check_0x42a320, reinterpret_cast<LPVOID*>(&p_original_function_address_check_0x42a320)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to disabled function address check.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
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
        if (MH_CreateHook(p_gameQuit_0x41f9b0, &detoured_gameQuit_0x41f9b0, reinterpret_cast<LPVOID*>(&p_original_gameQuit_0x41f9b0)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for gameQuit function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_CTgaFile_Write_0x5a4810, &detoured_CTgaFile_Write_0x5a4810, reinterpret_cast<LPVOID*>(&p_original_CTgaFile_Write_0x5a4810)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for screenShot function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHookApiEx(L"WSOCK32.DLL", "connect", &detoured_connect, reinterpret_cast<LPVOID*>(&p_original_connect), reinterpret_cast<LPVOID*>(&p_connect)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for gameSocket send function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_CGCamera_updateCallback_0x511bc0, &detoured_CGCamera_updateCallback_0x511bc0, reinterpret_cast<LPVOID*>(&p_original_CGCamera_updateCallback_0x511bc0)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for camera updateCallback function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_GxScenePresent_0x58a960, &detoured_GxScenePresent_0x58a960, reinterpret_cast<LPVOID*>(&p_original_GxScenePresent_0x58a960)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for GxScenePresent function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_OrganicSmooth, &detoured_OrganicSmooth, reinterpret_cast<LPVOID*>(&p_original_OrganicSmooth)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for OrganicSmooth function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_weather_setType, &detoured_weather_setType, reinterpret_cast<LPVOID*>(&p_original_weather_setType)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for weather_setType function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_operator_multiply_1, &detoured_operator_multiply_1, reinterpret_cast<LPVOID*>(&p_original_operator_multiply_1)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for operator_multiply_1 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_operator_multiply_2, &detoured_operator_multiply_2, reinterpret_cast<LPVOID*>(&p_original_operator_multiply_2)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for operator_multiply_2 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_operator_multiply_3, &detoured_operator_multiply_3, reinterpret_cast<LPVOID*>(&p_original_operator_multiply_3)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for operator_multiply_3 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_operator_multiply_4, &detoured_operator_multiply_4, reinterpret_cast<LPVOID*>(&p_original_operator_multiply_4)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for operator_multiply_4 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_operator_multiply_6, &detoured_operator_multiply_6, reinterpret_cast<LPVOID*>(&p_original_operator_multiply_6)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for operator_multiply_6 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_operator_multiply_assign_1, &detoured_operator_multiply_assign_1, reinterpret_cast<LPVOID*>(&p_original_operator_multiply_assign_1)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for operator_multiply_assign_1 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_matrix_translate_1, &detoured_matrix_translate_1, reinterpret_cast<LPVOID*>(&p_original_matrix_translate_1)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for matrix_translate_1 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_matrix_scale_1, &detoured_matrix_scale_1, reinterpret_cast<LPVOID*>(&p_original_matrix_scale_1)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for matrix_scale_1 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_matrix_scale_2, &detoured_matrix_scale_2, reinterpret_cast<LPVOID*>(&p_original_matrix_scale_2)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for matrix_scale_2 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_fun_0x7be490, &detoured_fun_0x7be490, reinterpret_cast<LPVOID*>(&p_original_fun_0x7be490)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for fun_0x7be490 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_fun_0x7bdfc0, &detoured_fun_0x7bdfc0, reinterpret_cast<LPVOID*>(&p_original_fun_0x7bdfc0)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for fun_0x7bdfc0 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_matrix_rotate_1, &detoured_matrix_rotate_1, reinterpret_cast<LPVOID*>(&p_original_matrix_rotate_1)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for matrix_rotate_1 function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_transformAABox, &detoured_transformAABox, reinterpret_cast<LPVOID*>(&p_original_transformAABox)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for transformAABox function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_lua_sqrt, &detoured_lua_sqrt, reinterpret_cast<LPVOID*>(&p_original_lua_sqrt)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for lua_sqrt function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (ERMS) {
            if (MH_CreateHook(p_blit_hub, &detoured_blit_hub, reinterpret_cast<LPVOID*>(&p_original_blit_hub)) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for blit_hub function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
        }
        if (MH_CreateHook(p_squaredMagnitude, &detoured_squaredMagnitude, reinterpret_cast<LPVOID*>(&p_original_squaredMagnitude)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for squaredMagnitude function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_CreateHook(p_calculatePlaneNormal, &detoured_calculatePlaneNormal, reinterpret_cast<LPVOID*>(&p_original_calculatePlaneNormal)) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed to create hook for calculatePlaneNormal function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
            MessageBoxW(NULL, utf8_to_utf16(u8"Failed when enabling hooks.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            return FALSE;
        }
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        // According to https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
        // We should only free resources when lpReserved == NULL
        // Otherwise we should wait for OS to reclaim the memory
        if (lpReserved == NULL) {
            if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to disable hooks. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_calculatePlaneNormal) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for calculatePlaneNormal function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_squaredMagnitude) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for squaredMagnitude function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (ERMS) {
                if (MH_RemoveHook(p_blit_hub) != MH_OK) {
                    MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for blit_hub function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                    return FALSE;
                }
            }
            if (MH_RemoveHook(p_lua_sqrt) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for lua_sqrt function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_transformAABox) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for transformAABox function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_matrix_rotate_1) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for matrix_rotate_1 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_fun_0x7bdfc0) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for fun_0x7bdfc0 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_fun_0x7be490) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for fun_0x7be490 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_matrix_scale_2) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for matrix_scale_2 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_matrix_scale_1) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for matrix_scale_1 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_matrix_translate_1) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for matrix_translate_1 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_operator_multiply_assign_1) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for operator_multiply_assign_1 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_operator_multiply_6) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for operator_multiply_6 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_operator_multiply_4) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for operator_multiply_4 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_operator_multiply_3) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for operator_multiply_3 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_operator_multiply_2) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for operator_multiply_2 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_operator_multiply_1) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for operator_multiply_1 function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_weather_setType) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for weather_setType function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_OrganicSmooth) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for OrganicSmooth function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_GxScenePresent_0x58a960) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for GxScenePresent function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_CGCamera_updateCallback_0x511bc0) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for camera updateCallback function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (MH_RemoveHook(p_connect) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for gameSocket connect function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
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
            if (MH_RemoveHook(p_function_address_check_0x42a320) != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for function address check. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
            if (edit_CWorld_Intersect_end() == false) {
                return FALSE;
            }
            if (MH_Uninitialize() != MH_OK) {
                MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to uninitialize MinHook. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                return FALSE;
            }
        }
        break;
    }
    }
    return TRUE;
}


