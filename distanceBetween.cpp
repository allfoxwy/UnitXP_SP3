#include "pch.h"

#include <string>
#include <sstream>

#include "distanceBetween.h"
#include "Vanilla1121_functions.h"

using namespace std;


float UnitXP_distanceBetween(const C3Vector& pos0, const C3Vector& pos1) {
    C3Vector v = {};
    v.x = pos0.x - pos1.x;
    v.y = pos0.y - pos1.y;
    v.z = pos0.z - pos1.z;
    return vectorLength(v);
}

// return -1 for error
// This function is using void* to prevent implicit conversion from uint32_t to uint64_t
float UnitXP_distanceBetween(const void* unit0void, const void* unit1void, distanceMeters meter) {
    if (!unit0void || !unit1void) {
        return -1;
    }

    uint32_t unit0 = reinterpret_cast<uint32_t>(unit0void);
    uint32_t unit1 = reinterpret_cast<uint32_t>(unit1void);

    if ((unit0 & 1) != 0 || (unit1 & 1) != 0) {
        return -1;
    }

    if (unit0 == unit1) {
        return 0.0f;
    }

    if (vanilla1121_objectType(unit0) != OBJECT_TYPE_Unit &&
        vanilla1121_objectType(unit0) != OBJECT_TYPE_Player) {
        return -1;
    }
    if (vanilla1121_objectType(unit1) != OBJECT_TYPE_Unit &&
        vanilla1121_objectType(unit1) != OBJECT_TYPE_Player) {
        return -1;
    }

    C3Vector pos0 = vanilla1121_unitPosition(unit0);
    C3Vector pos1 = vanilla1121_unitPosition(unit1);

    // We are ignoring error from vanilla1121_unitCombatReach()
    float combatReach0 = max(0.0f, vanilla1121_unitCombatReach(unit0));
    float combatReach1 = max(0.0f, vanilla1121_unitCombatReach(unit1));

    if (meter == METER_MELEE_AUTOATTACK && abs(pos0.z - pos1.z) < 6.0f) {
        // Melee distance calculation is following https://github.com/vmangos/core/blob/4aaec500a70d32e1234010e432e87982f6e4a527/src/game/Objects/Unit.cpp#L10526

        combatReach0 = max(1.5f, combatReach0);
        combatReach1 = max(1.5f, combatReach1);

        float totalReach = max(5.0f, combatReach0 + combatReach1 + 1.333333373069763f);

        C3Vector v = {};
        v.x = pos0.x - pos1.x;
        v.y = pos0.y - pos1.y;

        return max(0.0f, vectorLength(v) - totalReach);
    }
    else if (meter == METER_AOE) {
        // AoE distance is following Balake's fix https://github.com/vmangos/core/commit/fc0d6cfd6192b5c90072d77ab289f165ea540a00

        // only 1 reach would be used
        float totalReach = 0.0f;

        // By Balake: testing on classic shows aoe range is bigger vs mob compared to vs player
        // this probably means combat reach is not used vs player targets
        if (vanilla1121_objectType(unit0) == OBJECT_TYPE_Unit) {
            totalReach = combatReach0;
        }
        if (vanilla1121_objectType(unit1) == OBJECT_TYPE_Unit) {
            totalReach = combatReach1;
        }

        return max(0.0f, UnitXP_distanceBetween(pos0, pos1) - totalReach);
    }
    else if (meter == METER_CHAINS) {
        // Chains distance is following https://github.com/vmangos/core/blob/9f099e58be8e97dd6ee8215f18feb9ab65b5958c/src/game/Spells/Spell.cpp#L2655

        // We are ignoring error from vanilla1121_unitBoundingRadius()
        float boundingRadius0 = max(0.0f, vanilla1121_unitBoundingRadius(unit0));
        float boundingRadius1 = max(0.0f, vanilla1121_unitBoundingRadius(unit1));

        return max(0.0f, UnitXP_distanceBetween(pos0, pos1) - boundingRadius0 - boundingRadius1);
    }
    else if (meter == METER_RANGED) {
        return max(0.0f, UnitXP_distanceBetween(pos0, pos1) - combatReach0 - combatReach1);
    }
    else {
        // Default to METER_GAUSSIAN
        // While in-DLL we default to METER_GAUSSIAN, for Lua we default to METER_RANGED
        return UnitXP_distanceBetween(pos0, pos1);
    }
}

// return -1 for error
float UnitXP_distanceBetween(const uint64_t guid0, const uint64_t guid1, distanceMeters meter) {
    if (guid0 == 0 || guid1 == 0) {
        return -1.0f;
    }

    if (guid0 == guid1) {
        return 0.0f;
    }

    return UnitXP_distanceBetween(
        reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid0)),
        reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid1)),
        meter);
}

// return -1 for error
float UnitXP_distanceBetween(const string unit0, const string unit1, distanceMeters meter) {
    uint64_t guid0 = 0, guid1 = 0;

    if (unit0.empty() || unit1.empty()) {
        return -1.0f;
    }

    if (unit0.find(u8"0x") != unit0.npos) {
        stringstream ss{ unit0 };
        ss >> hex >> guid0;
        if (ss.fail()) {
            return -1.0f;
        }
    }
    else {
        guid0 = UnitGUID(unit0.data());
        if (guid0 == 0) {
            return -1.0f;
        }
    }

    if (unit1.find(u8"0x") != unit1.npos) {
        stringstream ss{ unit1 };
        ss >> hex >> guid1;
        if (ss.fail()) {
            return -1.0f;
        }
    }
    else {
        guid1 = UnitGUID(unit1.data());
        if (guid1 == 0) {
            return -1.0f;
        }
    }

    return UnitXP_distanceBetween(guid0, guid1, meter);
}
