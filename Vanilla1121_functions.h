#pragma once

#include <cstdint>

// Signatures/Prototypes
typedef void(__fastcall* LUA_PUSHSTRING)(void* L, const char* s);
typedef int(__fastcall* LUA_CFUNCTION)(void* L);
typedef struct {
    const char* name;
    LUA_CFUNCTION func;
}lua_func_reg; // Struct paired with luaL_openlib
typedef void(__fastcall* LUAL_OPENLIB)(void* L, const char* name_space, lua_func_reg func_list[], int upvalues);
typedef void* (__fastcall* GETCONTEXT)(void);
typedef void(__fastcall* LUA_PUSHNIL)(void* L);
typedef void(__fastcall* LUA_PUSHBOOLEAN)(void* L, int boolean_value);
typedef void(__fastcall* LUA_PUSHNUMBER)(void* L, double n);
typedef const char* (__fastcall* LUA_TOSTRING)(void* L, int index);
typedef uint64_t(__fastcall* UNITGUID)(const char* unitID);
typedef struct {
    float y;
    float x;
    float z;
} C3Vector;
typedef bool(__fastcall* CWORLD__INTERSECT)(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance, unsigned int queryFlags);
typedef void(__fastcall* TARGET)(uint64_t* GUID);
typedef int(__thiscall* UNITREACTION)(uint32_t self, uint32_t targetObj);
typedef bool(__thiscall* CANATTACK)(uint32_t self, uint32_t targetObj);
enum InGameObjectType {
    OBJECT_TYPE_Null,
    OBJECT_TYPE_Item,
    OBJECT_TYPE_Container,
    OBJECT_TYPE_Unit,
    OBJECT_TYPE_Player,
    OBJECT_TYPE_GameObject,
    OBJECT_TYPE_DynamicObject,
    OBJECT_TYPE_Corpse
};
enum InGameReactions {
    UNIT_REACTION_HATED,
    UNIT_REACTION_HOSTILE,
    UNIT_REACTION_UNFRIENDLY,
    UNIT_REACTION_NEUTRAL,
    UNIT_REACTION_AMIABLE,
    UNIT_REACTION_FRIENDLY,
    UNIT_REACTION_REVERED
};


// To get lua_State pointer
void* GetContext(void);


// LUA language
void lua_pushstring(void* L, const char* str);
void luaL_openlib(void* L, const char* name_space, lua_func_reg function_list[], int upvalues);
const char* lua_tostring(void* L, int index);
int lua_gettop(void* L);
void lua_pushnil(void* L);
void lua_pushboolean(void* L, int boolean_value);
void lua_pushnumber(void* L, double n);


// WoW C function
uint64_t UnitGUID(const char* unitID);
bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance, unsigned int queryFlags);
// Target the unit with GUID
void vanilla1121_target(uint64_t targetGUID);
// Get in-game unit reaction, return -1 for error
int vanilla1121_getReaction(uint64_t targetGUID);
// -1 for error. This function is different from getReaction because enemy player could turn off PvP
int vanilla1121_canAttack(uint64_t targetGUID);
// Return 1 for dead, 0 for alive, -1 for error
int vanilla1121_isDead(uint32_t object);


// WoW Visiable Object
// Search visiable objects for GUID and return its address as uint32_t (Because uint32_t is easier to do math than void*)
uint32_t vanilla1121_getVisiableObject(uint64_t targetGUID);
C3Vector vanilla1121_getPosition(uint32_t object);
// Return true for "in sight"; false for "not in sight";
bool vanilla1121_inLineOfSight(uint32_t object0, uint32_t object1);
// Return true for in-combat; false for not-in-combat or unchecked
bool vanilla1121_inCombat(uint32_t object);
// Get in-game object type
int vanilla1121_getType(uint64_t targetGUID);

