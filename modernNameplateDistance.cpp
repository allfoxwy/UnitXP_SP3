#include "pch.h"

#include <cmath>

#include "Vanilla1121_functions.h"
#include "modernNameplateDistance.h"
#include "inSight.h"
#include "distanceBetween.h"
#include "timer.h"


// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
typedef void(__thiscall* ADDNAMEPLATE)(void*, void*, void*);
extern ADDNAMEPLATE p_addNameplate = reinterpret_cast<ADDNAMEPLATE>(0x6086e0);
extern ADDNAMEPLATE p_original_addNameplate = NULL;

typedef int(__thiscall* RENDERWORLD)(void* self);
extern RENDERWORLD p_renderWorld = reinterpret_cast<RENDERWORLD>(0x482D70);
extern RENDERWORLD p_original_renderWorld = NULL;

extern REMOVENAMEPLATE p_removeNameplate = reinterpret_cast<REMOVENAMEPLATE>(0x608A10);

extern bool modernNameplateDistance = true;
extern bool prioritizeTargetNameplate = false;
extern bool prioritizeMarkedNameplate = false;

static bool nameplatesHasMarkOnThem = false;


// -1 for error, 0 for no, 1 for yes
static int shouldHaveNameplate(void* unit) {
    if (!unit) {
        return -1;
    }

    uint64_t guidUnderNameplate = *reinterpret_cast<uint64_t*>(reinterpret_cast<uint32_t>(unit) + 0x30);

    // It's super complex about the combination of prioritize target or/and mark
    if (prioritizeMarkedNameplate || prioritizeTargetNameplate) {
        uint64_t targetGUID = UnitGUID("target");
        if (targetGUID > 0 || nameplatesHasMarkOnThem) {
            if (prioritizeTargetNameplate && prioritizeMarkedNameplate) {
                if (guidUnderNameplate == targetGUID || vanilla1121_getTargetMark(guidUnderNameplate) > 0) {
                    return 1;
                }
                else {
                    return 0;
                }
            }
            if (prioritizeTargetNameplate && !prioritizeMarkedNameplate) {
                if (targetGUID > 0) {
                    if (guidUnderNameplate == targetGUID) {
                        return 1;
                    }
                    else {
                        return 0;
                    }
                }
            }
            if (!prioritizeTargetNameplate && prioritizeMarkedNameplate) {
                if (nameplatesHasMarkOnThem) {
                    if (vanilla1121_getTargetMark(guidUnderNameplate) > 0) {
                        return 1;
                    }
                    else {
                        return 0;
                    }
                }
            }
        }
    }

    bool inSight = (camera_inSight(unit) > 0);
    
    C3Vector pos0 = vanilla1121_getCameraPosition();
    C3Vector pos1 = vanilla1121_unitPosition(reinterpret_cast<uint32_t>(unit));
    float distance = hypot(pos0.x - pos1.x, pos0.y - pos1.y, pos0.z - pos1.z);


    const float seeThroughWallDistance = 10.0f;

    // We are not adding nameplate to loss sight unit
    if (distance > seeThroughWallDistance && inSight == false) {
        return 0;
    }
    else {
        return 1;
    }
}

// For target mark, we need check all nameplates before we make decision of should 1 of them have nameplate
static void refreshMarkStatus() {
    nameplatesHasMarkOnThem = false;

    // Linked list saving all nameplates
    uint32_t nameplate_item = *reinterpret_cast<uint32_t*>(0xc4d92c);

    while (nameplate_item != 0 && (nameplate_item & 1) == 0) {
        uint32_t next_item = *reinterpret_cast<uint32_t*>(nameplate_item + 0x4e0);
        uint64_t guidUnderNameplate = *reinterpret_cast<uint64_t*>(nameplate_item + 0x4e8);

        if (vanilla1121_getTargetMark(guidUnderNameplate) > 0) {
            nameplatesHasMarkOnThem = true;
            return;
        }

        nameplate_item = next_item;
    }
}

int __fastcall detoured_renderWorld(void* self, void* ignored) {
    if (self && modernNameplateDistance) {

        if (prioritizeMarkedNameplate) {
            refreshMarkStatus();
        }

        // Linked list saving all nameplates
        uint32_t nameplate_item = *reinterpret_cast<uint32_t*>(0xc4d92c);

        while (nameplate_item != 0 && (nameplate_item & 1) == 0) {
            // Save Next address, because we might delete current node
            uint32_t next_item = *reinterpret_cast<uint32_t*>(nameplate_item + 0x4e0);
            uint64_t guidUnderNameplate = *reinterpret_cast<uint64_t*>(nameplate_item + 0x4e8);

            void* unitUnderNameplate = reinterpret_cast<void*>(vanilla1121_getVisiableObject(guidUnderNameplate));

            // While it unlikely exists, we skip object which is not Unit.
            int type = vanilla1121_objectType(reinterpret_cast<uint32_t>(unitUnderNameplate));
            if (type != OBJECT_TYPE_Unit && type != OBJECT_TYPE_Player) {
                nameplate_item = next_item;
                continue;
            }
            
            if (shouldHaveNameplate(unitUnderNameplate) == 0) {
                p_removeNameplate(reinterpret_cast<uint32_t>(unitUnderNameplate));
            }

            nameplate_item = next_item;
        }
    }

    if (self) {
        gTimer.execute();
    }

    return p_original_renderWorld(self);
}


void __fastcall detoured_addNameplate(void* self, void* ignored, void* unknown1, void* unknown2) {
    if (self && modernNameplateDistance) {
        // While it unlikely exists, we skip object which is not Unit.
        int type = vanilla1121_objectType(reinterpret_cast<uint32_t>(self));
        if (type == OBJECT_TYPE_Unit || type == OBJECT_TYPE_Player) {
            if (shouldHaveNameplate(self) == 0) {
                return;
            }
        }
    }
    return p_original_addNameplate(self, unknown1, unknown2);
}

