#include "pch.h"

#include <cstdint>

#include "Vanilla1121_functions.h"

// To get lua_State pointer
GETCONTEXT p_GetContext = reinterpret_cast<GETCONTEXT>(0x7040D0);

// LUA language
LUA_PUSHSTRING p_lua_pushstring = reinterpret_cast<LUA_PUSHSTRING>(0x006F3890);
LUAL_OPENLIB p_luaL_openlib = reinterpret_cast<LUAL_OPENLIB>(0x006F4DC0);
LUA_TOSTRING p_lua_tostring = reinterpret_cast<LUA_TOSTRING>(0x006F3690);
LUA_CFUNCTION p_lua_gettop = reinterpret_cast<LUA_CFUNCTION>(0x006F3070);
LUA_PUSHNIL p_lua_pushnil = reinterpret_cast<LUA_PUSHNIL>(0x006F37F0);
LUA_PUSHBOOLEAN p_lua_pushboolean = reinterpret_cast<LUA_PUSHBOOLEAN>(0x006F39F0);
LUA_PUSHNUMBER p_lua_pushnumber = reinterpret_cast<LUA_PUSHNUMBER>(0x006F3810);
LUA_TONUMBER p_lua_tonumber = reinterpret_cast<LUA_TONUMBER>(0x006F3620);
LUA_ISNUMBER p_lua_isnumber = reinterpret_cast<LUA_ISNUMBER>(0x006F34D0);
LUA_ISNUMBER p_lua_isstring = reinterpret_cast<LUA_ISNUMBER>(0x6F3510);
LUA_GETTABLE p_lua_gettable = reinterpret_cast<LUA_GETTABLE>(0x6F3A40);
LUA_PCALL p_lua_pcall = reinterpret_cast<LUA_PCALL>(0x6F41A0);


// WoW C function
UNITGUID p_UnitGUID = reinterpret_cast<UNITGUID>(0x00515970);
CWORLD__INTERSECT p_CWorld_Intersect = reinterpret_cast<CWORLD__INTERSECT>(0x672170);
TARGET p_Target = reinterpret_cast<TARGET>(0x489a40);
UNITREACTION p_UnitReaction = reinterpret_cast<UNITREACTION>(0x6061e0);
CANATTACK p_CanAttack = reinterpret_cast<CANATTACK>(0x606980);
GETCREATURETYPE p_getCreatureType = reinterpret_cast<GETCREATURETYPE>(0x605570);

typedef uint32_t(__fastcall* GETACTIVECAMERA)(void);
GETACTIVECAMERA p_getCamera = reinterpret_cast<GETACTIVECAMERA>(0x4818F0);

extern float guardAgainstTransportsCoordinates = 200.0f;

// To get lua_State pointer
void* GetContext(void) {
    return p_GetContext();
}


// LUA language
void lua_pushstring(void* L, const char* str) {
    p_lua_pushstring(L, str);
    return;
}
void luaL_openlib(void* L, const char* name_space, lua_func_reg function_list[], int upvalues) {
    p_luaL_openlib(L, name_space, function_list, upvalues);
    return;
}
const char* lua_tostring(void* L, int index) {
    return p_lua_tostring(L, index);
}
int lua_gettop(void* L) {
    return p_lua_gettop(L);
}
void lua_pushnil(void* L) {
    p_lua_pushnil(L);
    return;
}
void lua_pushboolean(void* L, int boolean_value) {
    p_lua_pushboolean(L, boolean_value);
    return;
}
void lua_pushnumber(void* L, double n) {
    p_lua_pushnumber(L, n);
    return;
}
double lua_tonumber(void* L, int index) {
    return p_lua_tonumber(L, index);
}
int lua_isnumber(void* L, int index) {
    return p_lua_isnumber(L, index);
}
int lua_isstring(void* L, int index) {
    return p_lua_isstring(L, index);
}
void lua_gettable(void* L, int index) {
    return p_lua_gettable(L, index);
}
int lua_pcall(void* L, int nArgs, int nResults, int errFunction) {
    return p_lua_pcall(L, nArgs, nResults, errFunction);
}


// WoW C function
uint64_t UnitGUID(const char* unitID) {
    return p_UnitGUID(unitID);
}
bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance) {
    // 0x100171 flag would cause game crash in Hateforge Quarry
    return p_CWorld_Intersect(p1, p2, ignored, intersectPoint, distance, 0x100111);
}


uint32_t vanilla1121_getVisiableObject(uint64_t targetGUID) {
    uint32_t objects = *reinterpret_cast<uint32_t*>(0xb41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);
    
    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);

        if (currentObjectGUID == targetGUID) {
            return i;
        }

        i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
    }

    return 0;
}


C3Vector vanilla1121_unitPosition(uint32_t unit) {
    return {
        *reinterpret_cast<float*>(unit + 0x09B8),
        *reinterpret_cast<float*>(unit + 0x09B8 + 0x4),
        *reinterpret_cast<float*>(unit + 0x09B8 + 0x8),
    };
}


bool vanilla1121_unitInCombat(uint32_t unit) {
    if (unit == 0) {
        return false;
    }

    // some kind of attribute, including in-combat information
    uint32_t attr = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return false;
    }

    uint32_t flags = *reinterpret_cast<uint32_t*>(attr + 0xa0);
    if ((flags & 0x80000) != 0) {
        return true;
    }
    else {
        return false;
    }
}

bool vanilla1121_unitInLineOfSight(uint32_t unit0, uint32_t unit1) {
    C3Vector pos0 = vanilla1121_unitPosition(unit0);
    C3Vector pos1 = vanilla1121_unitPosition(unit1);

    C3Vector intersectPoint = { 0,0,0 };
    float distance = 1.0f;

    //TODO: I can't find height of object
    pos0.z += 2.4f;
    pos1.z += 2.4f;

    bool result = CWorld_Intersect(&pos0, &pos1, 0, &intersectPoint, &distance);

    if (result) {
        if (distance <= 1 && distance >= 0) {
            // intersect between points, loss sight
            return false;
        }
        else {
            // intersect after points, still in sight
            return true;
        }
    }
    else {
        // no intersect, in sight
        return true;
    }
}

void vanilla1121_target(uint64_t targetGUID) {
    p_Target(&targetGUID);
    return;
}

int vanilla1121_objectType(uint32_t targetObject) {
    if (targetObject == 0) {
        return OBJECT_TYPE_Null;
    }

    return *reinterpret_cast<uint32_t*>(targetObject + 0x14);
}


int vanilla1121_unitReaction(uint32_t unit) {
    uint32_t target = unit;
    uint32_t self = vanilla1121_getVisiableObject(UnitGUID("player"));
    if (target == 0 || self == 0) {
        return -1;
    }

    return p_UnitReaction(self, target);
}


int vanilla1121_unitCanBeAttacked(uint32_t unit) {
    uint32_t target = unit;
    uint32_t self = vanilla1121_getVisiableObject(UnitGUID("player"));
    if (target == 0 || self == 0) {
        return -1;
    }

    if (p_CanAttack(self, target)) {
        return 1;
    }
    else {
        return 0;
    }
}


int vanilla1121_unitIsDead(uint32_t unit) {
    if (unit == 0) {
        return -1;
    }

    // some kind of attribute, including death information
    uint32_t attr = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return -1;
    }

    uint32_t flag0 = *reinterpret_cast<uint32_t*>(attr + 0x40);
    uint32_t flag1 = *reinterpret_cast<uint32_t*>(attr + 0x224);
    if (flag0 < 1 || (flag1 & 0x20) != 0) {
        return 1;
    }
    else {
        return 0;
    }
}


int vanilla1121_unitIsControlledByPlayer(uint32_t unit) {
    if (unit == 0) {
        return -1;
    }

    // some kind of attribute, including in-combat information
    uint32_t attr = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return -1;
    }

    uint32_t data = *reinterpret_cast<uint32_t*>(attr + 0xa0);
    if ((data & 8) != 0) {
        return 1;
    }
    else {
        return 0;
    }
}


uint64_t vanilla1121_unitTargetGUID(uint32_t unit) {
    if (unit == 0) {
        return 0;
    }

    // some kind of attribute, including in-combat information
    uint32_t attr = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return 0;
    }

    uint64_t data = *reinterpret_cast<uint64_t*>(attr + 0x28);
    
    return data;
}

int vanilla1121_unitClassification(uint32_t unit) {
    if (unit == 0) {
        return -1;
   }

    // some kind of attribute
    uint32_t attr0 = *reinterpret_cast<uint32_t*>(unit + 0xb30);
    if (attr0 == 0 || (attr0 & 1) != 0) {
        // we don't have attribute info.
        return -1;
    }

    uint32_t attr1 = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr1 == 0 || (attr1 & 1) != 0) {
        // we don't have attribute info.
        return -1;
    }

    uint32_t data1 = *reinterpret_cast<uint32_t*>(attr1 + 0x214);

    if (attr0 != 0 && data1 == 0) {
        int data0 = *reinterpret_cast<int*>(attr0 + 0x20);
        return data0;
    }
    else {
        return -1;
    }
}

int vanilla1121_unitCreatureType(uint32_t unit) {
    if (unit == 0) {
        return -1;
    }
    return p_getCreatureType(unit);
}
C3Vector vanilla1121_getCameraPosition() {
    uint32_t cptr = p_getCamera();
    if (cptr == 0) {
        return { 0,0,0 };
    }

    return {
        *reinterpret_cast<float*>(cptr + 0x8),
        *reinterpret_cast<float*>(cptr + 0x8 + 0x4),
        *reinterpret_cast<float*>(cptr + 0x8 + 0x8)
    };
}

float vanilla1121_getCameraFoV() {
    return *reinterpret_cast<float*>(0x8089B4);
}

int vanilla1121_getTargetMark(uint64_t targetGUID) {
    if (targetGUID == 0) {
        return -1;
    }

    for (int result = 0; result < 8; ++result) {
        if (*reinterpret_cast<uint64_t*>(0xb71368 + result * 8) == targetGUID) {
            return result + 1;
        }
    }

    return -1;
}


