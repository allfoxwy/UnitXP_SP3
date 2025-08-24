#pragma once

#include <cstdint>
#include <string>

typedef int(__fastcall* LUA_CFUNCTION)(void* L);

typedef struct {
    const char* name;
    LUA_CFUNCTION func;
}lua_func_reg; // Struct paired with luaL_openlib

typedef struct structC3Vector {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
} C3Vector;

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

// This struct is from https://github.com/vmangos/core/blob/2dae0ef85799fc34c2f9b62a1df3008cc069ebd0/src/game/Objects/MovementInfo.h#L27
enum MovementFlags
{
    MOVEFLAG_NONE = 0x00000000, // 0
    MOVEFLAG_FORWARD = 0x00000001, // 1
    MOVEFLAG_BACKWARD = 0x00000002, // 2
    MOVEFLAG_STRAFE_LEFT = 0x00000004, // 3
    MOVEFLAG_STRAFE_RIGHT = 0x00000008, // 4
    MOVEFLAG_TURN_LEFT = 0x00000010, // 5
    MOVEFLAG_TURN_RIGHT = 0x00000020, // 6
    MOVEFLAG_PITCH_UP = 0x00000040, // 7
    MOVEFLAG_PITCH_DOWN = 0x00000080, // 8
    MOVEFLAG_WALK_MODE = 0x00000100, // 9 Walking
    MOVEFLAG_UNUSED10 = 0x00000200, // 10 ??
    MOVEFLAG_LEVITATING = 0x00000400, // 11 ?? Seems not to work
    MOVEFLAG_FIXED_Z = 0x00000800, // 12 Fixed height. Jump => Glide across the entire map
    MOVEFLAG_ROOT = 0x00001000, // 13
    MOVEFLAG_JUMPING = 0x00002000, // 14
    MOVEFLAG_FALLINGFAR = 0x00004000, // 15
    MOVEFLAG_PENDING_STOP = 0x00008000, // 16 Only used in older client versions
    MOVEFLAG_PENDING_UNSTRAFE = 0x00010000, // 17 Only used in older client versions
    MOVEFLAG_PENDING_FORWARD = 0x00020000, // 18 Only used in older client versions
    MOVEFLAG_PENDING_BACKWARD = 0x00040000, // 19 Only used in older client versions
    MOVEFLAG_PENDING_STR_LEFT = 0x00080000, // 20 Only used in older client versions
    MOVEFLAG_PENDING_STR_RGHT = 0x00100000, // 21 Only used in older client versions
    MOVEFLAG_SWIMMING = 0x00200000, // 22 Ok
    MOVEFLAG_SPLINE_ENABLED = 0x00400000, // 23 Ok
    MOVEFLAG_MOVED = 0x00800000, // 24 Only used in older client versions
    MOVEFLAG_FLYING = 0x01000000, // 25 [-ZERO] is it really need and correct value
    MOVEFLAG_ONTRANSPORT = 0x02000000, // 26 Used for flying on some creatures
    MOVEFLAG_SPLINE_ELEVATION = 0x04000000, // 27 Used for flight paths
    MOVEFLAG_UNUSED28 = 0x08000000, // 28
    MOVEFLAG_WATERWALKING = 0x10000000, // 29 Prevent unit from falling through water
    MOVEFLAG_SAFE_FALL = 0x20000000, // 30 Active rogue safe fall spell (passive)
    MOVEFLAG_HOVER = 0x40000000, // 31
    MOVEFLAG_UNUSED32 = 0x80000000, // 32

    // Can not be present with MOVEFLAG_ROOT (otherwise client freeze)
    MOVEFLAG_MASK_MOVING =
    MOVEFLAG_FORWARD | MOVEFLAG_BACKWARD | MOVEFLAG_STRAFE_LEFT | MOVEFLAG_STRAFE_RIGHT |
    MOVEFLAG_PITCH_UP | MOVEFLAG_PITCH_DOWN | MOVEFLAG_JUMPING | MOVEFLAG_FALLINGFAR |
    MOVEFLAG_SPLINE_ELEVATION,
    MOVEFLAG_MASK_MOVING_OR_TURN = MOVEFLAG_MASK_MOVING | MOVEFLAG_TURN_LEFT | MOVEFLAG_TURN_RIGHT,

    // MovementFlags mask that only contains flags for x/z translations
    // this is to avoid that a jumping character that stands still triggers melee-leeway
    MOVEFLAG_MASK_XZ = MOVEFLAG_FORWARD | MOVEFLAG_BACKWARD | MOVEFLAG_STRAFE_LEFT | MOVEFLAG_STRAFE_RIGHT
};

// When player jump onto transports (boat/zeppelin) their coordinates system would change.
// If we pass coordinates from different system into vanilla1121_unitInLineOfSight(), game crashes
// To workaround, we test the distance. If they are too far away, we judge that situation as error
// 
// As of https://github.com/allfoxwy/UnitXP_SP3/commit/3d742e756d5d0e24d70248300b44f9bfa94cac3a
// The new implementation of unitPosition should play well with transport coordinates.
// We no longer need this workaround for transports.
// However I withnessed 1 crash without any distance guard. So there still be a guard in CWorld_Intersect
//extern const float guardAgainstTransportsCoordinates;

// We blur the intersect position a little bit,
// so that when camera hits ceiling, it doesn't cancel nameplates
extern const float cameraIntersectBlur;

// LUA language
// basic types
#define LUA_TNONE (-1)
#define LUA_TNIL (0)
#define LUA_TBOOLEAN (1)
#define LUA_TLIGHTUSERDATA (2)
#define LUA_TNUMBER (3)
#define LUA_TSTRING (4)
#define LUA_TTABLE (5)
#define LUA_TFUNCTION (6)
#define LUA_TUSERDATA (7)
#define LUA_TTHREAD (8)
// pseudo-indices
#define LUA_REGISTRYINDEX (-10000)
#define LUA_GLOBALSINDEX (-10001)
#define lua_upvalueindex(i) (LUA_GLOBALSINDEX-(i))

// To get lua_State pointer
void* GetContext(void);

void lua_pushstring(void* L, std::string str);
void luaL_openlib(void* L, std::string name_space, lua_func_reg function_list[], int upvalues);
std::string lua_tostring(void* L, int index);
double lua_tonumber(void* L, int index);
int lua_toboolean(void* L, int index);
int lua_gettop(void* L);
void lua_pushnil(void* L);
void lua_pushboolean(void* L, int boolean_value);
void lua_pushnumber(void* L, double n);
int lua_isnumber(void* L, int index);
int lua_isstring(void* L, int index);
int lua_type(void* L, int index);
std::string lua_typename(void* L, int type);
void lua_settop(void* L, int index);
#define lua_pop(L,n) lua_settop(L, -(n)-1)
void lua_pushvalue(void* L, int index);
void lua_remove(void* L, int index);
void lua_insert(void* L, int index);
void lua_newtable(void* L);
void lua_gettable(void* L, int index);
void lua_settable(void* L, int index);
int lua_next(void* L, int index);
double luaL_checknumber(void* L, int index);




// WoW C function
// Get GUID from UnitID
uint64_t UnitGUID(const char* unitID);
// Test intersect in world, p1 and p2 for input, intersectPoint and distance for output.
bool CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, C3Vector* intersectPoint, float* distance, uint32_t intersectFlag = 0x100111);
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
uint32_t vanilla1121_unitMovementFlags(uint32_t unit);
bool vanilla1121_unitIsMoving(uint32_t unit);
// Return true for "in sight"; false for "not in sight";
bool vanilla1121_unitInLineOfSight(uint32_t unit0, uint32_t unit1);
// Return true for in-combat; false for not-in-combat or unchecked
bool vanilla1121_unitInCombat(uint32_t unit);
// Return -1 for error
double vanilla1121_unitCurrentHP(uint32_t unit);
// Return -1 for error
double vanilla1121_unitMaximumHP(uint32_t unit);
// Get in-game object type
int vanilla1121_objectType(uint32_t targetObject);
// Get unit's target. This function has a delay when switching target. I suspect its data reqires network commnication to server.
uint64_t vanilla1121_unitTargetGUID(uint32_t unit);
// Get unit's creature type. Returns -1 when unit is 0 or return original function's result.
int vanilla1121_unitCreatureType(uint32_t unit);
uint32_t vanilla1121_unitMountDisplayID(uint32_t unit);
bool vanilla1121_unitIsMounted(uint32_t unit);
// Get active camera pointer
uint32_t vanilla1121_getCamera();
// Get camera position
C3Vector vanilla1121_getCameraPosition(uint32_t camera);
// Get Field of View value
float vanilla1121_getCameraFoV(uint32_t camera);
// Get the GUID whom the camera is looking at
uint64_t vanilla1121_getCameraLookingAtGUID(uint32_t camera);
// This distance value could contain animation process, it could be not the final value.
float vanilla1121_getCameraCurrentDistance(uint32_t camera);
// This should be the final distance value after animation.
float vanilla1121_getCameraDesiredDistance(uint32_t camera);
float vanilla1121_getCameraNearClip(uint32_t camera);
float vanilla1121_getCameraFarClip(uint32_t camera);
float vanilla1121_getCameraAspectRatio(uint32_t camera);
uint32_t vanilla1121_getCameraIntersectFlag();
C3Vector vanilla1121_getCameraForwardVector(uint32_t camera);
C3Vector vanilla1121_getCameraRightVector(uint32_t camera);
C3Vector vanilla1121_getCameraUpVector(uint32_t camera);
void vanilla1121_setCameraForwardVector(uint32_t camera, const C3Vector& v);
void vanilla1121_setCameraRightVector(uint32_t camera, const C3Vector& v);
void vanilla1121_setCameraUpVector(uint32_t camera, const C3Vector& v);
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
// Run a piece of Lua script
void vanilla1121_runScript(std::string luaScript);
// Get CMovement struct address from unit
uint32_t vanilla1121_unitCMovement(uint32_t unit);
// Seems to be a time value
uint32_t vanilla1121_gameTick();
// The collision box is not exactly same size as on-display.
float vanilla1121_unitCollisionBoxHeight(uint32_t unit);
float vectorLength(C3Vector& vec);
C3Vector vectorCrossProduct(const C3Vector& a, const C3Vector& b);
float vectorDotProduct(const C3Vector& a, const C3Vector& b);
void vectorNormalize(C3Vector& vec);
// Return 4 * M_PI for error
float angleBetweenVectors(C3Vector& a, C3Vector& b);
bool vectorsAreNear(C3Vector& a, C3Vector& b);
bool vectorAlmostZero(C3Vector& vec);
