#include "pch.h"

#define _USE_MATH_DEFINES

#include <cmath>

#include "Vanilla1121_functions.h"
#include "edit_CWorld_Intersect.h"
#include "utf8_to_utf16.h"
#include "performanceProfiling.h"
#include "distanceBetween.h"

// Signatures/Prototypes
typedef void(__fastcall* LUA_PUSHSTRING)(void* L, const char* s);
typedef void(__fastcall* TARGET)(uint64_t* GUID);
typedef int(__thiscall* UNITREACTION)(uint32_t self, uint32_t targetObj);
typedef bool(__thiscall* CANATTACK)(uint32_t self, uint32_t targetObj);
typedef int(__fastcall* GETCREATURETYPE)(uint32_t obj);
typedef uint32_t(__fastcall* GETACTIVECAMERA)(void);
typedef void(__fastcall* LUAL_OPENLIB)(void* L, const char* name_space, lua_func_reg func_list[], int upvalues);
typedef void* (__fastcall* GETCONTEXT)(void);
typedef void(__fastcall* LUA_PUSHNIL)(void* L);
typedef void(__fastcall* LUA_PUSHBOOLEAN)(void* L, int boolean_value);
typedef void(__fastcall* LUA_PUSHNUMBER)(void* L, double n);
typedef double(__fastcall* LUA_TONUMBER)(void* L, int index);
typedef int(__fastcall* LUA_ISNUMBER)(void* L, int index);
typedef const char* (__fastcall* LUA_TOSTRING)(void* L, int index);
typedef uint64_t(__fastcall* UNITGUID)(const char* unitID);
typedef int(__fastcall* LUA_TYPE)(void* L, int);
typedef const char* (__fastcall* LUA_TYPENAME)(void*, int);
typedef void(__fastcall* LUA_SETTOP)(void*, int);
typedef int(__fastcall* LUA_TOBOOLEAN)(void*, int);
typedef void(__fastcall* LUA_NEWTABLE)(void*);
typedef void(__fastcall* LUA_GETTABLE)(void*, int);
typedef void(__fastcall* LUA_SETTABLE)(void*, int);
typedef int(__fastcall* LUA_NEXT)(void*, int);
typedef void(__fastcall* LUA_PUSHVALUE)(void*, int);
typedef void(__fastcall* LUA_REMOVE)(void*, int);
typedef void(__fastcall* LUA_INSERT)(void*, int);
typedef double(__fastcall* LUAL_CHECKNUMBER)(void*, int);


// To get lua_State pointer
static auto p_GetContext = reinterpret_cast<GETCONTEXT>(0x7040D0);

// LUA language
static auto p_lua_pushstring = reinterpret_cast<LUA_PUSHSTRING>(0x006F3890);
static auto p_luaL_openlib = reinterpret_cast<LUAL_OPENLIB>(0x006F4DC0);
static auto p_lua_tostring = reinterpret_cast<LUA_TOSTRING>(0x006F3690);
static auto p_lua_gettop = reinterpret_cast<LUA_CFUNCTION>(0x006F3070);
static auto p_lua_pushnil = reinterpret_cast<LUA_PUSHNIL>(0x006F37F0);
static auto p_lua_pushboolean = reinterpret_cast<LUA_PUSHBOOLEAN>(0x006F39F0);
static auto p_lua_pushnumber = reinterpret_cast<LUA_PUSHNUMBER>(0x006F3810);
static auto p_lua_tonumber = reinterpret_cast<LUA_TONUMBER>(0x006F3620);
static auto p_lua_isnumber = reinterpret_cast<LUA_ISNUMBER>(0x006F34D0);
static auto p_lua_isstring = reinterpret_cast<LUA_ISNUMBER>(0x6F3510);
static auto p_lua_type = reinterpret_cast<LUA_TYPE>(0x6F3400);
static auto p_lua_typename = reinterpret_cast<LUA_TYPENAME>(0x6F3480);
static auto p_lua_settop = reinterpret_cast<LUA_SETTOP>(0x6F3080);
static auto p_lua_toboolean = reinterpret_cast<LUA_TOBOOLEAN>(0x6F3660);
static auto p_lua_newtable = reinterpret_cast<LUA_NEWTABLE>(0x6F3C90);
static auto p_lua_settable = reinterpret_cast<LUA_SETTABLE>(0x6F3E20);
static auto p_lua_gettable = reinterpret_cast<LUA_GETTABLE>(0x6F3A40);
static auto p_lua_next = reinterpret_cast<LUA_NEXT>(0x6F4450);
static auto p_lua_pushvalue = reinterpret_cast<LUA_PUSHVALUE>(0x6F3350);
static auto p_lua_remove = reinterpret_cast<LUA_REMOVE>(0x6F30D0);
static auto p_lua_insert = reinterpret_cast<LUA_INSERT>(0x6F31A0);
static auto p_luaL_checknumber = reinterpret_cast<LUAL_CHECKNUMBER>(0x6f4c80);


// WoW C function
static auto p_UnitGUID = reinterpret_cast<UNITGUID>(0x00515970);
static auto p_Target = reinterpret_cast<TARGET>(0x489a40);
static auto p_UnitReaction = reinterpret_cast<UNITREACTION>(0x6061e0);
static auto p_CanAttack = reinterpret_cast<CANATTACK>(0x606980);
static auto p_getCreatureType = reinterpret_cast<GETCREATURETYPE>(0x605570);
static auto p_getCamera = reinterpret_cast<GETACTIVECAMERA>(0x4818F0);

const float cameraIntersectBlur = 0.01f;

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
int lua_type(void* L, int index) {
    return p_lua_type(L, index);
}
std::string lua_typename(void* L, int type) {
    const char* ptr = p_lua_typename(L, type);
    if (ptr) {
        std::string result{ ptr };
        return result;
    }
    else {
        return "";
    }
}
int lua_gettop(void* L) {
    return p_lua_gettop(L);
}
void lua_settop(void* L, int index) {
    return p_lua_settop(L, index);
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
int lua_toboolean(void* L, int index) {
    return p_lua_toboolean(L, index);
}
int lua_isnumber(void* L, int index) {
    return p_lua_isnumber(L, index);
}
int lua_isstring(void* L, int index) {
    return p_lua_isstring(L, index);
}
double luaL_checknumber(void* L, int index) {
    return p_luaL_checknumber(L, index);
}

// Get GUID from UNIT_ID
uint64_t UnitGUID(const char* unitID) {
    return p_UnitGUID(unitID);
}

bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, C3Vector* intersectPoint, float* distance, uint32_t intersectFlag) {
    // The common knowledge of flag is 0x100171 or 0x100111:
    // *- 0x100171 would cause game crash in Turtle WoW Hateforge Quarry.
    // *- 0x100111 works well.

    // Internet says distance needed to be initialized to 1.0f
    *distance = 1.0f;
    *intersectPoint = {};

    // I witnessed 1 crash without distance guard.
    const float distanceGuard = 150.0f;
    float checkDistance = UnitXP_distanceBetween(*p1, *p2);
    if (checkDistance > distanceGuard) {
        *distance = 0.5f;
        intersectPoint->x = (p2->x + p1->x) / 2;
        intersectPoint->y = (p2->y + p1->y) / 2;
        intersectPoint->z = (p2->z + p1->z) / 2;
        return true;
    }
    if (checkDistance <= std::numeric_limits<float>::epsilon()) {
        return false;
    }

    std::string perfName = "CWorld_Intersect";
    perfSetSlotName(0, perfName);
    perfMarkStart(0);
    bool result = p_CWorld_Intersect(p1, p2, 0, intersectPoint, distance, intersectFlag);
    perfMarkEnd(0);

    return result;
}

/*
* This implementation is the same as 0x468380
* However it's a O(n) linear search
uint32_t vanilla1121_getVisiableObject(const uint64_t targetGUID) {
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
*/

uint32_t vanilla1121_getVisiableObject(const uint64_t targetGUID) {
    // In 0x464870 the significant 4 bytes of GUID is a hash bucket index, then double checked the lower 4 bytes for sure.
    // It would be certainly faster than O(n)
    typedef uint32_t(__fastcall* GETOBJECT_BYGUID)(uint64_t);
    GETOBJECT_BYGUID getObject_byGUID = reinterpret_cast<GETOBJECT_BYGUID>(0x464870);

    return getObject_byGUID(targetGUID);
}


/*
* This implementation is based on direct memory reading. (Which is common knowledge)
* However when player jump onto transport, it does not take the trasport coordinates into consideration.

C3Vector vanilla1121_unitPosition(uint32_t unit) {
    float* positionPtr = reinterpret_cast<float*>(vanilla1121_unitCMovement(unit) + 0x10);

    C3Vector result = {};
    result.x = positionPtr[0];
    result.y = positionPtr[1];
    result.z = positionPtr[2];

    return result;
}
*/

// This implementation is from 0x50e9ce
// It would lead to float * __thiscall CGUnit_GetPosition_0x5f1f10(void *this,float *returnC3Vector)
C3Vector vanilla1121_unitPosition(uint32_t unit) {
    C3Vector result = {};

    if (unit == 0 || (unit & 1) != 0) {
        return result;
    }

    uint32_t memberFunctions = *reinterpret_cast<uint32_t*>(unit);
    if (memberFunctions == 0 || (memberFunctions & 1) != 0) {
        return result;
    }

    uint32_t getPositionPtr = *reinterpret_cast<uint32_t*>(memberFunctions + 0x14);
    if (getPositionPtr == 0 || (getPositionPtr & 1) != 0) {
        return result;
    }

    typedef C3Vector* (__thiscall* UNIT_GETPOSITION)(uint32_t, C3Vector*);
    UNIT_GETPOSITION p_getPosition = reinterpret_cast<UNIT_GETPOSITION>(getPositionPtr);
    p_getPosition(unit, &result);

    return result;
}

float vanilla1121_unitFacing(uint32_t unit) {
    return *reinterpret_cast<float*>(vanilla1121_unitCMovement(unit) + 0x1c);
}

uint32_t vanilla1121_unitMovementFlags(uint32_t unit) {
    return *reinterpret_cast<uint32_t*>(vanilla1121_unitCMovement(unit) + 0x40);
}

bool vanilla1121_unitIsMoving(uint32_t unit) {
    return (vanilla1121_unitMovementFlags(unit) & MOVEFLAG_MASK_MOVING_OR_TURN) > 0;
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

double vanilla1121_unitCurrentHP(uint32_t unit) {
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

    return *reinterpret_cast<uint32_t*>(attr + 0x40);
}

double vanilla1121_unitMaximumHP(uint32_t unit) {
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

    return *reinterpret_cast<uint32_t*>(attr + 0x58);
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
    if (unit0 == unit1) {
        return true;
    }

    if (vanilla1121_unitCollisionBoxHeight(unit0) > vanilla1121_unitCollisionBoxHeight(unit1)) {
        // We keep unit 1 is taller
        uint32_t temp = unit0;
        unit0 = unit1;
        unit1 = temp;
    }

    C3Vector pos0 = vanilla1121_unitPosition(unit0);
    C3Vector pos1 = vanilla1121_unitPosition(unit1);

    C3Vector intersectPoint = {};
    float distance = 1.0f;

    // First test: unit 0 try to look at unit 1 at the same height
    // Not a typo
    pos0.z += vanilla1121_unitCollisionBoxHeight(unit0);
    pos1.z += vanilla1121_unitCollisionBoxHeight(unit0);

    bool result = CWorld_Intersect(&pos0, &pos1, &intersectPoint, &distance);

    if (result) {
        if (distance <= 1 && distance >= 0) {
            // 1st test: intersect between points, loss sight
            if (vanilla1121_unitCollisionBoxHeight(unit0) == vanilla1121_unitCollisionBoxHeight(unit1)) {
                return false;
            }

            // Second test: unit 0 try to look up at unit 1
            intersectPoint = {};
            distance = 1.0f;
            pos0 = vanilla1121_unitPosition(unit0);
            pos1 = vanilla1121_unitPosition(unit1);
            pos0.z += vanilla1121_unitCollisionBoxHeight(unit0);
            pos1.z += vanilla1121_unitCollisionBoxHeight(unit1);

            result = CWorld_Intersect(&pos0, &pos1, &intersectPoint, &distance);
            if (result) {
                if (distance <= 1 && distance >= 0) {
                    // 2nd test: intersect between points, loss sight
                    return false;
                }
                else {
                    // 2nd test: intersect after points, still in sight
                    return true;
                }
            }
            else {
                // 2nd test: no intersect, in sight
                return true;
            }
        }
        else {
            // 1st test: intersect after points, still in sight
            return true;
        }
    }
    else {
        // 1st test: no intersect, in sight
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

C3Vector vanilla1121_getCameraPosition(uint32_t camera) {
    float* positionPtr = reinterpret_cast<float*>(camera + 0x8);
    C3Vector result = {};
    result.x = positionPtr[0];
    result.y = positionPtr[1];
    result.z = positionPtr[2];

    return result;
}

float vanilla1121_getCameraFoV(uint32_t camera) {
    return *reinterpret_cast<float*>(camera + 0x40);
}

uint64_t vanilla1121_getCameraLookingAtGUID(uint32_t camera) {
    return *reinterpret_cast<uint64_t*>(camera + 0x88);
}

float vanilla1121_getCameraCurrentDistance(uint32_t camera) {
    return *reinterpret_cast<float*>(camera + 0xec);
}

float vanilla1121_getCameraDesiredDistance(uint32_t camera) {
    return *reinterpret_cast<float*>(camera + 0x198);
}

float vanilla1121_getCameraNearClip(uint32_t camera) {
    return *reinterpret_cast<float*>(camera + 0x38);
}

float vanilla1121_getCameraFarClip(uint32_t camera) {
    return *reinterpret_cast<float*>(camera + 0x3c);
}

float vanilla1121_getCameraAspectRatio(uint32_t camera) {
    return *reinterpret_cast<float*>(camera + 0x44);
}

C3Vector vanilla1121_getCameraForwardVector(uint32_t camera) {
    C3Vector result = {};
    float* matCamera = reinterpret_cast<float*>(camera + 0x14);
    result.x = matCamera[0];
    result.y = matCamera[1];
    result.z = matCamera[2];
    return result;
}

C3Vector vanilla1121_getCameraRightVector(uint32_t camera) {
    C3Vector result = {};
    float* matCamera = reinterpret_cast<float*>(camera + 0x14);
    result.x = matCamera[3];
    result.y = matCamera[4];
    result.z = matCamera[5];
    return result;
}

C3Vector vanilla1121_getCameraUpVector(uint32_t camera) {
    C3Vector result = {};
    float* matCamera = reinterpret_cast<float*>(camera + 0x14);
    result.x = matCamera[6];
    result.y = matCamera[7];
    result.z = matCamera[8];
    return result;
}

void vanilla1121_setCameraForwardVector(uint32_t camera, const C3Vector& v) {
    float* matCamera = reinterpret_cast<float*>(camera + 0x14);
    matCamera[0] = v.x;
    matCamera[1] = v.y;
    matCamera[2] = v.z;
}

void vanilla1121_setCameraRightVector(uint32_t camera, const C3Vector& v) {
    float* matCamera = reinterpret_cast<float*>(camera + 0x14);
    matCamera[3] = v.x;
    matCamera[4] = v.y;
    matCamera[5] = v.z;
}

void vanilla1121_setCameraUpVector(uint32_t camera, const C3Vector& v) {
    float* matCamera = reinterpret_cast<float*>(camera + 0x14);
    matCamera[6] = v.x;
    matCamera[7] = v.y;
    matCamera[8] = v.z;
}

uint32_t vanilla1121_getCameraIntersectFlag() {
    // According to game's camera collision detect logic (position 0x50e61a in CGCamera_CollideCameraWithWorld_50E570),
    // There is a switch to determine what flag to use. The switch is the game option of Water Collision.
    uint32_t intersectFlag = 0;
    if (*reinterpret_cast<uint32_t*>(*reinterpret_cast<uint32_t*>(0xBE1088) + 0x28) != 0) {
        intersectFlag = 0x1F0171;
    }
    else {
        intersectFlag = 0x100171;
    }
    return intersectFlag;
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

void vanilla1121_runScript(std::string luaScript) {
    typedef int(__fastcall* SCRIPT_RUNSCRIPT)(void*);
    auto p_Script_RunScript = reinterpret_cast<SCRIPT_RUNSCRIPT>(0x48B980);

    void* L = GetContext();

    // The game's RunScript is designed to work with addons. So it would read stack item no.1 as the script (which is Lua argument no.1)
    // However we are using it as a general purpose RunScript, there is no guarantee that the stack is clean
    // So we try to move the script to stack item no.1
    lua_pushstring(L, luaScript);
    lua_insert(L, 1);
    p_Script_RunScript(L);
    lua_remove(L, 1);
}

float vectorLength(C3Vector& vec) {
    return std::sqrt(std::pow(vec.x, 2.0f) + std::pow(vec.y, 2.0f) + std::pow(vec.z, 2.0f));
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
    if (len > 0.0f) {
        vec.x /= len;
        vec.y /= len;
        vec.z /= len;
    }
}

float angleBetweenVectors(C3Vector& a, C3Vector& b) {
    float len = vectorLength(a);
    len *= vectorLength(b);
    if (len > 0.0f) {
        float dotProduct = vectorDotProduct(a, b);

        float cosValue = dotProduct / len;
        if (cosValue > 1.0f) {
            cosValue = 1.0f;
        }
        if (cosValue < -1.0f) {
            cosValue = -1.0f;
        }

        return std::acos(cosValue);
    }
    else {
        return static_cast<float>(4 * M_PI);
    }
}

void lua_newtable(void* L) {
    return p_lua_newtable(L);
}

void lua_settable(void* L, int index) {
    return p_lua_settable(L, index);
}

void lua_gettable(void* L, int index) {
    return p_lua_gettable(L, index);
}

int lua_next(void* L, int index) {
    return p_lua_next(L, index);
}

void lua_pushvalue(void* L, int index) {
    return p_lua_pushvalue(L, index);
}

void lua_remove(void* L, int index) {
    return p_lua_remove(L, index);
}

void lua_insert(void* L, int index) {
    return p_lua_insert(L, index);
}

uint32_t vanilla1121_unitCMovement(uint32_t unit) {
    return *reinterpret_cast<uint32_t*>(unit + 0x118);
}

uint32_t vanilla1121_gameTick() {
    return *reinterpret_cast<uint32_t*>(0xcf0bc8);
}

float vanilla1121_unitCollisionBoxHeight(uint32_t unit) {
    return *reinterpret_cast<float*>(vanilla1121_unitCMovement(unit) + 0xb4);
}

uint32_t vanilla1121_unitMountDisplayID(uint32_t unit) {
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

    return *reinterpret_cast<uint32_t*>(attr + 0x1fc);
}

bool vanilla1121_unitIsMounted(uint32_t unit) {
    return vanilla1121_unitMountDisplayID(unit) != 0;
}

bool vectorsAreNear(C3Vector& a, C3Vector& b) {
    if (UnitXP_distanceBetween(a, b) <= std::numeric_limits<float>::epsilon()) {
        return true;
    }
    else {
        return false;
    }
}
bool vectorAlmostZero(C3Vector& vec) {
    if (vectorLength(vec) <= std::numeric_limits<float>::epsilon()) {
        return true;
    }
    else {
        return false;
    }
}
