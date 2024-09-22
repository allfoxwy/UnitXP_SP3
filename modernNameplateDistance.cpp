#include "pch.h"

#include "sysinfoapi.h" // We need Windows Vista or newer for GetTickCount64()

#include "Vanilla1121_functions.h"
#include "modernNameplateDistance.h"
#include "inSight.h"
#include "distanceBetween.h"


// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
typedef void(__thiscall* ADDNAMEPLATE)(void*, void*, void*);
extern ADDNAMEPLATE p_addNameplate = reinterpret_cast<ADDNAMEPLATE>(0x6086e0);
extern ADDNAMEPLATE p_original_addNameplate = NULL;

typedef int(__thiscall* RENDERWORLD)(void* self);
extern RENDERWORLD p_renderWorld = reinterpret_cast<RENDERWORLD>(0x482D70);
extern RENDERWORLD p_original_renderWorld = NULL;


extern bool modernNameplateDistance = true;
extern bool modernNameplateDistanceRefreshEnemy = true;
extern bool modernNameplateDistanceRefreshFriend = false;

int __fastcall detoured_renderWorld(void* self, void* ignored) {
    if (self && modernNameplateDistance) {
        // A small function controling nameplate
        typedef void(__fastcall* CONTROLSHOWNAMEPLATE)(char show);
        CONTROLSHOWNAMEPLATE p_controlEnemyNameplate = reinterpret_cast<CONTROLSHOWNAMEPLATE>(0x6054f0);
        CONTROLSHOWNAMEPLATE p_controlFriendNameplate = reinterpret_cast<CONTROLSHOWNAMEPLATE>(0x605510);

        static uint64_t lastEnemyNameplateRefreshTime = 0;
        static uint64_t lastFriendNameplateRefreshTime = 0;

        uint64_t now = GetTickCount64();
        const uint64_t delay = 2000;

        if (modernNameplateDistanceRefreshEnemy) {
            static bool refreshEnemyNameplate = false;

            if (refreshEnemyNameplate) {
                refreshEnemyNameplate = false;
                p_controlEnemyNameplate(1);
                lastEnemyNameplateRefreshTime = now;
            }

            if (now - lastEnemyNameplateRefreshTime > delay) {
                refreshEnemyNameplate = true;
                p_controlEnemyNameplate(0);
            }
        }

        if (modernNameplateDistanceRefreshFriend) {
            static bool refreshFriendNameplate = false;

            if (refreshFriendNameplate) {
                refreshFriendNameplate = false;
                p_controlFriendNameplate(1);
                lastFriendNameplateRefreshTime = now;
            }

            if (now - lastFriendNameplateRefreshTime > delay) {
                refreshFriendNameplate = true;
                p_controlFriendNameplate(0);
            }
        }
    }
    return p_original_renderWorld(self);
}


void __fastcall detoured_addNameplate(void* self, void* ignored, void* unknown1, void* unknown2) {
    if (modernNameplateDistance && self && unknown1 && unknown2) {
        uint64_t guidUnderNameplate = *reinterpret_cast<uint64_t*>(reinterpret_cast<uint32_t>(self) + 0x30);
        uint64_t player = UnitGUID(u8"player");
        if (guidUnderNameplate != 0 && player != 0) {
            bool inSight = (UnitXP_inSight(player, guidUnderNameplate) > 0);
            float distance = UnitXP_distanceBetween(player, guidUnderNameplate);
            const float seeThroughWallDistance = 8.0f;

            if (distance > seeThroughWallDistance && inSight == false) {
                // We are not adding nameplate to loss sight unit
                return;
            }
        }
    }
    return p_original_addNameplate(self, unknown1, unknown2);
}

