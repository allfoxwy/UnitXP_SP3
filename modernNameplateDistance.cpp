#include "pch.h"

#include "Vanilla1121_functions.h"
#include "modernNameplateDistance.h"
#include "inSight.h"
#include "distanceBetween.h"
#include "timer.h"
#include "performanceProfiling.h"
#include "editCamera.h"


// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
ADDNAMEPLATE p_addNameplate = reinterpret_cast<ADDNAMEPLATE>(0x6086e0);
ADDNAMEPLATE p_original_addNameplate = NULL;

RENDERWORLD p_renderWorld = reinterpret_cast<RENDERWORLD>(0x482D70);
RENDERWORLD p_original_renderWorld = NULL;

REMOVENAMEPLATE p_removeNameplate = reinterpret_cast<REMOVENAMEPLATE>(0x608A10);

bool modernNameplateDistance = true;
bool prioritizeTargetNameplate = false;
bool prioritizeMarkedNameplate = false;
bool nameplateCombatFilter = false;
bool showInCombatNameplatesNearPlayer = false;
bool hideCritterNameplate = true;

static bool nameplatesHasMarkOnThem = false;

// -1 for error, 0 for no, 1 for yes
static int shouldHaveNameplate(void* voidUnit) {
    if (!voidUnit) {
        return -1;
    }

    uint64_t guidUnderNameplate = *reinterpret_cast<uint64_t*>(reinterpret_cast<uint32_t>(voidUnit) + 0x30);

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

    if (hideCritterNameplate) {
        bool typeCheck = (vanilla1121_objectType(reinterpret_cast<uint32_t>(voidUnit)) == OBJECT_TYPE_Unit || vanilla1121_objectType(reinterpret_cast<uint32_t>(voidUnit)) == OBJECT_TYPE_Player)
            && vanilla1121_unitCreatureType(reinterpret_cast<uint32_t>(voidUnit)) == 8;
        if (typeCheck) {
            bool inCombat = vanilla1121_unitInCombat(reinterpret_cast<uint32_t>(voidUnit));
            if (inCombat == false) {
                return 0;
            }
        }
    }

    if (nameplateCombatFilter) {
        bool fullHP = vanilla1121_unitCurrentHP(reinterpret_cast<uint32_t>(voidUnit)) >= vanilla1121_unitMaximumHP(reinterpret_cast<uint32_t>(voidUnit));
        bool inCombat = vanilla1121_unitInCombat(reinterpret_cast<uint32_t>(voidUnit)) || (vanilla1121_objectType(reinterpret_cast<uint32_t>(voidUnit)) == OBJECT_TYPE_Player && vanilla1121_unitCanBeAttacked(reinterpret_cast<uint32_t>(voidUnit)) == 1);
        if (fullHP == true && inCombat == false) {
            return 0;
        }
    }

    const float nearPlayerDistance = 8.0f;
    if (showInCombatNameplatesNearPlayer && vanilla1121_unitInCombat(reinterpret_cast<uint32_t>(voidUnit))) {
        float distance = UnitXP_distanceBetween(guidUnderNameplate, UnitGUID("player"), METER_RANGED);
        if (distance >= 0.0f && distance < nearPlayerDistance) {
            return 1;
        }
    }

    bool inSight = (camera_inSight(voidUnit) > 0);

    C3Vector pos0 = editCamera_translatedPosition();
    C3Vector pos1 = vanilla1121_unitPosition(reinterpret_cast<uint32_t>(voidUnit));
    float distance = UnitXP_distanceBetween(pos0, pos1);

    const float cameraSeeThroughWallDistance = 10.0f;

    // We are not adding nameplate to loss sight unit
    if (distance > cameraSeeThroughWallDistance && inSight == false) {
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
        // perfSetSlotName(1, "remove nameplate");
        // perfMarkStart(1);

        if (prioritizeMarkedNameplate) {
            refreshMarkStatus();
        }

        // Linked list saving all nameplates
        uint32_t nameplate_item = *reinterpret_cast<uint32_t*>(0xc4d92c);

        while (nameplate_item != 0 && (nameplate_item & 1) == 0) {
            // Save Next address, because we might delete current node
            uint32_t next_item = *reinterpret_cast<uint32_t*>(nameplate_item + 0x4e0);
            uint64_t guidUnderNameplate = *reinterpret_cast<uint64_t*>(nameplate_item + 0x4e8);

            uint32_t unitUnderNameplate = vanilla1121_getVisiableObject(guidUnderNameplate);

            if (unitUnderNameplate == 0 || (unitUnderNameplate & 1) != 0) {
                nameplate_item = next_item;
                continue;
            }

            // While it unlikely exists, we skip object which is not Unit.
            int type = vanilla1121_objectType(unitUnderNameplate);
            if (type != OBJECT_TYPE_Unit && type != OBJECT_TYPE_Player) {
                nameplate_item = next_item;
                continue;
            }

            if (shouldHaveNameplate(reinterpret_cast<void*>(unitUnderNameplate)) == 0) {
                p_removeNameplate(unitUnderNameplate);
            }

            nameplate_item = next_item;
        }

        // perfMarkEnd(1);
    }

    if (self) {
        perfSetSlotName(4, "timer callback");
        perfMarkStart(4);
        gTimer.execute();
        perfMarkEnd(4);
    }

    perfSetSlotName(3, "original RenderWorld");
    perfMarkStart(3);
    int result = p_original_renderWorld(self);
    perfMarkEnd(3);

    return result;
}


void __fastcall detoured_addNameplate(void* self, void* ignored, void* unknown1, void* unknown2) {
    if (self && modernNameplateDistance) {
        // perfSetSlotName(2, "add nameplate");
        // perfMarkStart(2);

        uint32_t unit = reinterpret_cast<uint32_t>(self);
        if (unit == 0 || (unit & 1) != 0) {
            //perfMarkEnd(2);
            return;
        }

        // While it unlikely exists, we skip object which is not Unit.
        int type = vanilla1121_objectType(unit);
        if (type == OBJECT_TYPE_Unit || type == OBJECT_TYPE_Player) {
            if (shouldHaveNameplate(self) == 0) {
                //perfMarkEnd(2);
                return;
            }
        }
        // perfMarkEnd(2);
    }
    return p_original_addNameplate(self, unknown1, unknown2);
}

