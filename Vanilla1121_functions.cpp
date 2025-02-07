#include "pch.h"

#define _USE_MATH_DEFINES

#include <cmath>

#include "Vanilla1121_functions.h"
#include "utf8_to_utf16.h"
#include "performanceProfiling.h"

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
    void* result = p_GetContext();
    if (!result) {
        MessageBoxW(NULL, utf8_to_utf16(u8"Somehow, the Lua state is NULL. Game would crash soon.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
    }
    return result;
}

// LUA language
void lua_pushstring(void* L, std::string str) {
    p_lua_pushstring(L, str.data());
    return;
}
void luaL_openlib(void* L, std::string name_space, lua_func_reg function_list[], int upvalues) {
    p_luaL_openlib(L, name_space.data(), function_list, upvalues);
    return;
}
std::string lua_tostring(void* L, int index) {
    // This pointer can not directly return to other function.
    // According to https://www.lua.org/manual/5.0/manual.html
    // "Because Lua has garbage collection, there is no guarantee that the pointer returned by lua_tostring will be valid after the corresponding value is removed from the stack."
    // "If you need the string after the current function returns, then you should duplicate it or put it into the registry (see 3.18). "
    const char* ptr = p_lua_tostring(L, index);

    if (!ptr) {
        return "";
    }

    std::string result{ ptr };
    return result;
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

// Get GUID from UNIT_ID
uint64_t UnitGUID(const char* unitID) {
    return p_UnitGUID(unitID);
}

bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, C3Vector* intersectPoint, float* distance) {
    // Internet says distance needed to be initialized to 1.0f
    *distance = 1.0f;
    *intersectPoint = {};

    // The common knowledge of flag is 0x100171 or 0x100111:
    // *- 0x100171 would cause game crash in Turtle WoW Hateforge Quarry.
    // *- 0x100111 works well.
    //uint32_t intersectFlag = 0x100111;

    // According to game's camera collision detect logic (position 0x50e61a in CGCamera_CollideCameraWithWorld_50E570),
    // there is a switch to determine what flag to use. In practice it means 0x1f0171, which is slower than 0x100111.
    uint32_t intersectFlag = 0;
    std::string perfName = "";
    if (*reinterpret_cast<uint32_t*>(*reinterpret_cast<uint32_t*>(0xBE1088) + 0x28) != 0) {
        intersectFlag = 0x1F0171;
        perfName = "CWorld_Intersect with flag 0x1F0171";
    }
    else {
        intersectFlag = 0x100171;
        perfName = "CWorld_Intersect with flag 0x100171";
    }

    perfSetSlotName(0, perfName);
    perfMarkStart(0);
    bool result = p_CWorld_Intersect(p1, p2, 0, intersectPoint, distance, intersectFlag);
    perfMarkEnd(0);

    return result;
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
    float* positionPtr = reinterpret_cast<float*>(unit + 0x9b8);
    C3Vector result = {};

    result.x = positionPtr[0];
    result.y = positionPtr[1];
    result.z = positionPtr[2];

    return result;
}

float vanilla1121_unitFacing(uint32_t unit) {
    return *reinterpret_cast<float*>(unit + 0x9c4);
}

bool vanilla1121_unitInCombat(uint32_t unit) {
    if (unit == 0) {
        return false;
    }

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
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

float vanilla1121_unitBoundingRadius(uint32_t unit) {
    if (unit == 0) {
        return -1.0f;
    }

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
    uint32_t attr = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return -1.0f;
    }

    return *reinterpret_cast<float*>(attr + 0x1ec);
}

float vanilla1121_unitCombatReach(uint32_t unit) {
    if (unit == 0) {
        return -1.0f;
    }

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
    uint32_t attr = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return -1.0f;
    }

    return *reinterpret_cast<float*>(attr + 0x1f0);
}

float vanilla1121_unitScaleX(uint32_t unit) {
    if (unit == 0) {
        return -1.0f;
    }

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
    uint32_t attr = *reinterpret_cast<uint32_t*>(unit + 0x110);
    if (attr == 0 || (attr & 1) != 0) {
        // we don't have attribute info.
        return -1.0f;
    }

    // Get OBJECT_FIELD_SCALE_X. It's not in Unit but in Object descriptor so it's a minus.
    return *reinterpret_cast<float*>(attr - 0x4 * 2);
}

bool vanilla1121_unitInLineOfSight(uint32_t unit0, uint32_t unit1) {
    C3Vector pos0 = vanilla1121_unitPosition(unit0);
    C3Vector pos1 = vanilla1121_unitPosition(unit1);

    C3Vector intersectPoint = {};
    float distance = 1.0f;

    //TODO: I can't find height of object
    pos0.z += 2.4f;
    pos1.z += 2.4f;

    bool result = CWorld_Intersect(&pos0, &pos1, &intersectPoint, &distance);

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

    return *reinterpret_cast<int*>(targetObject + 0x14);
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

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
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

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
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

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
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

    // Unit descriptor (right after the Object descriptor).
    // The 0x110 is what I read from the game, but the common knowleadge of object + 0x8 = object descriptor is also fit.
    // I guess it's a compiler decision to make up this 0x110 magic number.
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

uint32_t vanilla1121_getCamera() {
    return p_getCamera();
}

C3Vector vanilla1121_getCameraPosition() {
    uint32_t cptr = vanilla1121_getCamera();
    if (cptr == 0) {
        C3Vector nullResult = {};
        nullResult.x = 0;
        nullResult.y = 0;
        nullResult.z = 0;
        return nullResult;
    }

    float* positionPtr = reinterpret_cast<float*>(cptr + 0x8);
    C3Vector result = {};
    result.x = positionPtr[0];
    result.y = positionPtr[1];
    result.z = positionPtr[2];

    return result;
}

float vanilla1121_getCameraFoV() {
    uint32_t cptr = vanilla1121_getCamera();
    if (cptr == 0) {
        return -1.0f;
    }

    return *reinterpret_cast<float*>(cptr + 0x40);
}

uint64_t vanilla1121_getCameraLookingAtGUID() {
    uint32_t cptr = vanilla1121_getCamera();
    if (cptr == 0) {
        return 0;
    }

    return *reinterpret_cast<uint64_t*>(cptr + 0x88);
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

float vectorLength(const C3Vector& vec) {
    return std::hypot(vec.x, vec.y, vec.z);
}

C3Vector vectorCrossProduct(const C3Vector& a, const C3Vector& b) {
    C3Vector result = {};

    result.x = a.y * b.z - a.z * b.y;
    result.y = -(a.x * b.z - a.z * b.x);
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

float vectorDotProduct(const C3Vector& a, const C3Vector& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

void vectorNormalize(C3Vector& vec) {
    float len = vectorLength(vec);
    vec.x /= len;
    vec.y /= len;
    vec.z /= len;
}

float angleBetweenVectors(const C3Vector& a, const C3Vector& b) {
    float lenA = vectorLength(a);
    float lenB = vectorLength(b);
    
    if (lenA == 0.0f || lenB == 0.0f) {
        return static_cast<float>(4 * M_PI);
    }

    float dotProduct = vectorDotProduct(a, b);

    float cosValue = dotProduct / (lenA * lenB);
    if (cosValue > 1.0f || cosValue < -1.0f) {
        return static_cast<float>(4 * M_PI);
    }

    return std::acos(cosValue);
}
