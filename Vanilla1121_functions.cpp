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

// WoW C function
UNITGUID p_UnitGUID = reinterpret_cast<UNITGUID>(0x00515970);
CWORLD__INTERSECT p_CWorld_Intersect = reinterpret_cast<CWORLD__INTERSECT>(0x672170);


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


// WoW C function
uint64_t UnitGUID(const char* unitID) {
    return p_UnitGUID(unitID);
}
bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance, unsigned int queryFlags) {
    return p_CWorld_Intersect(p1, p2, ignored, intersectPoint, distance, queryFlags);
}


// WoW Visiable Object
// Search visiable objects for GUID and return its address as uint32_t (Because uint32_t is easier to do math than void*)
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
C3Vector vanilla1121_getPosition(uint32_t object) {
    return {
        *reinterpret_cast<float*>(object + 0x09B8),
        *reinterpret_cast<float*>(object + 0x09B8 + 0x4),
        *reinterpret_cast<float*>(object + 0x09B8 + 0x8),
    };
}
// return true for "in sight"; false for "not in sight";
bool vanilla1121_inLineOfSight(uint32_t object0, uint32_t object1) {
    C3Vector pos0 = vanilla1121_getPosition(object0);
    C3Vector pos1 = vanilla1121_getPosition(object1);

    C3Vector intersectPoint = { 0,0,0 };
    float distance = 1.0f;

    //TODO: I can't find height of object, 2.0f is human height
    pos0.z += 2.0f;
    pos1.z += 2.0f;

    bool result = p_CWorld_Intersect(&pos0, &pos1, 0, &intersectPoint, &distance, 0x100111);

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

