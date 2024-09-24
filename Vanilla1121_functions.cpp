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

// WoW C function
UNITGUID p_UnitGUID = reinterpret_cast<UNITGUID>(0x00515970);
CWORLD__INTERSECT p_CWorld_Intersect = reinterpret_cast<CWORLD__INTERSECT>(0x672170);
TARGET p_Target = reinterpret_cast<TARGET>(0x489a40);
UNITREACTION p_UnitReaction = reinterpret_cast<UNITREACTION>(0x6061e0);
CANATTACK p_CanAttack = reinterpret_cast<CANATTACK>(0x606980);
GETCREATURETYPE p_getCreatureType = reinterpret_cast<GETCREATURETYPE>(0x605570);

typedef uint32_t(__fastcall* GETACTIVECAMERA)(void);
GETACTIVECAMERA p_getCamera = reinterpret_cast<GETACTIVECAMERA>(0x4818F0);

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


// WoW C function
uint64_t UnitGUID(const char* unitID) {
    return p_UnitGUID(unitID);
}
bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance, unsigned int queryFlags) {
    return p_CWorld_Intersect(p1, p2, ignored, intersectPoint, distance, queryFlags);
}


// WoW Visiable Object
// Search visiable objects for GUID and return its address as uint32_t (Because uint32_t is easier to do math than void*)
// There is an official function at 0x464890 which could doing the same thing.
uint32_t vanilla1121_getVisiableObject(uint64_t targetGUID) {
    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);
    
    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);

        if (currentObjectGUID == targetGUID) {
            return i;
        }

        i = *reinterpret_cast<uint32_t*>(i + 0x3c);
    }

    return 0;
}

C3Vector vanilla1121_getObjectPosition(uint32_t object) {
    return {
        *reinterpret_cast<float*>(object + 0x09B8),
        *reinterpret_cast<float*>(object + 0x09B8 + 0x4),
        *reinterpret_cast<float*>(object + 0x09B8 + 0x8),
    };
}

// Return true for in-combat; false for not-in-combat or unchecked
bool vanilla1121_inCombat(uint32_t object) {
    if (object == 0) {
        return false;
    }

    // some kind of attribute, including in-combat information
    uint32_t attr = *reinterpret_cast<uint32_t*>(object + 0x110);
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
// return true for "in sight"; false for "not in sight";
bool vanilla1121_inLineOfSight(uint32_t object0, uint32_t object1) {
    C3Vector pos0 = vanilla1121_getObjectPosition(object0);
    C3Vector pos1 = vanilla1121_getObjectPosition(object1);

    C3Vector intersectPoint = { 0,0,0 };
    float distance = 1.0f;

    //TODO: I can't find height of object
    pos0.z += 2.4f;
    pos1.z += 2.4f;

    // This line was the flags I used in first place
    //bool result = p_CWorld_Intersect(&pos0, &pos1, 0, &intersectPoint, &distance, 0x100111);

    bool result = p_CWorld_Intersect(&pos0, &pos1, 0, &intersectPoint, &distance, 0x100171);

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
// Target the unit with GUID
void vanilla1121_target(uint64_t targetGUID) {
    p_Target(&targetGUID);
    return;
}
// Get in-game object type
int vanilla1121_getType(uint32_t targetObject) {
    if (targetObject == 0) {
        return OBJECT_TYPE_Null;
    }

    return *reinterpret_cast<uint32_t*>(targetObject + 0x14);
}

// Get in-game unit reaction, return -1 for error
int vanilla1121_getReaction(uint32_t targetObject) {
    uint32_t target = targetObject;
    uint32_t self = vanilla1121_getVisiableObject(UnitGUID("player"));
    if (target == 0 || self == 0) {
        return -1;
    }

    return p_UnitReaction(self, target);
}

// This function is different from getReaction because enemy player could turn off PvP
int vanilla1121_canAttack(uint32_t targetObject) {
    uint32_t target = targetObject;
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

// Return 1 for dead, 0 for alive, -1 for error
int vanilla1121_objIsDead(uint32_t object) {
    if (object == 0) {
        return -1;
    }

    // some kind of attribute, including death information
    uint32_t attr = *reinterpret_cast<uint32_t*>(object + 0x110);
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

// Return 1 for player controlling, 0 for not, -1 for error
// Somehow companions are not counted as player controlling
int vanilla1121_objIsControlledByPlayer(uint32_t object) {
    if (object == 0) {
        return -1;
    }

    // some kind of attribute, including in-combat information
    uint32_t attr = *reinterpret_cast<uint32_t*>(object + 0x110);
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

// Get object's target. This function has a delay when switching target. I suspect its data reqires network commnication to server.
uint64_t vanilla1121_getObject_s_targetGUID(uint32_t object) {
    if (object == 0) {
        return 0;
    }

    // some kind of attribute, including in-combat information
    uint32_t attr = *reinterpret_cast<uint32_t*>(object + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return 0;
    }

    uint64_t data = *reinterpret_cast<uint64_t*>(attr + 0x28);
    
    return data;
}
// Get object's classification: normal, elite, rare elite, world boss, rare. Return -1 for error.
int vanilla1121_getObject_s_classification(uint32_t object) {
    if (object == 0) {
        return -1;
   }

    // some kind of attribute
    uint32_t attr0 = *reinterpret_cast<uint32_t*>(object + 0xb30);
    if (attr0 == 0 || (attr0 & 1) != 0) {
        // we don't have attribute info.
        return -1;
    }

    uint32_t attr1 = *reinterpret_cast<uint32_t*>(object + 0x110);
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
// Get object's creature type
int vanilla1121_getObject_s_creatureType(uint32_t object) {
    if (object == 0) {
        return -1;
    }
    return p_getCreatureType(object);
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
