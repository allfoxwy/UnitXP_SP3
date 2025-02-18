#pragma once

#include <cstdint>
#include <string>

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
typedef double(__fastcall* LUA_TONUMBER)(void* L, int index);
typedef int(__fastcall* LUA_ISNUMBER)(void* L, int index);
typedef const char* (__fastcall* LUA_TOSTRING)(void* L, int index);
typedef void(__fastcall* LUA_GETTABLE)(void* L, int index);
typedef int(__fastcall* LUA_PCALL)(void* L, int nArgs, int nResults, int errFunction);
typedef uint64_t(__fastcall* UNITGUID)(const char* unitID);
typedef struct {
    float x;
    float y;
    float z;
} C3Vector;
typedef bool(__fastcall* CWORLD__INTERSECT)(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance, uint32_t queryFlags);
typedef void(__fastcall* TARGET)(uint64_t* GUID);
typedef int(__thiscall* UNITREACTION)(uint32_t self, uint32_t targetObj);
typedef bool(__thiscall* CANATTACK)(uint32_t self, uint32_t targetObj);
typedef int(__fastcall* GETCREATURETYPE)(uint32_t obj);
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
enum InGameClassification {
    CLASSIFICATION_NORMAL,
    CLASSIFICATION_ELITE,
    CLASSIFICATION_RARE_ELITE,
    CLASSIFICATION_WORLDBOSS,
    CLASSIFICATION_RARE,
};

// When player jump onto transports (boat/zeppelin) their coordinates system would change.
// If we pass coordinates from different system into vanilla1121_unitInLineOfSight(), game crashes
// TODO: I don't have a way to find out what the current system is
// To workaround, we test the distance. If they are too far away, we judge that situation as error
extern float guardAgainstTransportsCoordinates;

// To get lua_State pointer
void* GetContext(void);


// LUA language
void lua_pushstring(void* L, std::string str);
void luaL_openlib(void* L, std::string name_space, lua_func_reg function_list[], int upvalues);
std::string lua_tostring(void* L, int index);
double lua_tonumber(void* L, int index);
int lua_gettop(void* L);
void lua_pushnil(void* L);
void lua_pushboolean(void* L, int boolean_value);
void lua_pushnumber(void* L, double n);
int lua_isnumber(void* L, int index);
int lua_isstring(void* L, int index);
void lua_gettable(void* L, int index);
int lua_pcall(void* L, int nArgs, int nResults, int errFunction);
#define LUA_GLOBALSINDEX (-10001)


// WoW C function
// Get GUID from UnitID
uint64_t UnitGUID(const char* unitID);
// Test intersect in world, p1 and p2 for input, intersectPoint and distance for output.
bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, C3Vector* intersectPoint, float* distance);
// Target the unit with GUID
void vanilla1121_target(uint64_t targetGUID);
// Get in-game unit reaction, return -1 for error
int vanilla1121_unitReaction(uint32_t unit);
// -1 for error. This function is different from getReaction because enemy player could turn off PvP
int vanilla1121_unitCanBeAttacked(uint32_t unit);
// Return 1 for dead, 0 for alive, -1 for error
int vanilla1121_unitIsDead(uint32_t unit);
// Return 1 for player controlling, 0 for not, -1 for error
int vanilla1121_unitIsControlledByPlayer(uint32_t unit);
// Get object's classification: normal, elite, rare elite, world boss, rare. Return -1 for error.
int vanilla1121_unitClassification(uint32_t unit);
// Search visiable objects for GUID and return its address as uint32_t (Because uint32_t is easier to do math than void*)
uint32_t vanilla1121_getVisiableObject(const uint64_t targetGUID);
// This function only work for Unit and Player type objects, or game would crash
C3Vector vanilla1121_unitPosition(uint32_t unit);
// This function only work for Unit and Player type objects, or game would crash
float vanilla1121_unitFacing(uint32_t unit);
// Return true for "in sight"; false for "not in sight";
bool vanilla1121_unitInLineOfSight(uint32_t unit0, uint32_t unit1);
// Return true for in-combat; false for not-in-combat or unchecked
bool vanilla1121_unitInCombat(uint32_t unit);
// Get in-game object type
int vanilla1121_objectType(uint32_t targetObject);
// Get unit's target. This function has a delay when switching target. I suspect its data reqires network commnication to server.
uint64_t vanilla1121_unitTargetGUID(uint32_t unit);
// Get unit's creature type. Returns -1 when unit is 0 or return original function's result.
int vanilla1121_unitCreatureType(uint32_t unit);
// Get camera pointer
uint32_t vanilla1121_getCamera();
// Get active camera position
C3Vector vanilla1121_getCameraPosition();
// Get Field of View value
float vanilla1121_getCameraFoV();
// Get the GUID whom the camera is looking at
uint64_t vanilla1121_getCameraLookingAtGUID();
// Get Raid/Party target mark.
// Return icon index as https://wowwiki-archive.fandom.com/wiki/API_GetRaidTargetIndex
// Return -1 for error
// In fact there is an official function with same capability at 0x4bb190, but I'm not sure about its calling convention
int vanilla1121_getTargetMark(uint64_t targetGUID);
// Return -1.0f for error.
float vanilla1121_unitBoundingRadius(uint32_t unit);
// Return -1.0f for error.
float vanilla1121_unitCombatReach(uint32_t unit);
// Return -1.0f for error.
float vanilla1121_unitScaleX(uint32_t unit);

float vectorLength(const C3Vector& vec);
C3Vector vectorCrossProduct(const C3Vector& a, const C3Vector& b);
float vectorDotProduct(const C3Vector& a, const C3Vector& b);
void vectorNormalize(C3Vector& vec);
// Return 4 * M_PI for error
float angleBetweenVectors(const C3Vector& a, const C3Vector& b);
