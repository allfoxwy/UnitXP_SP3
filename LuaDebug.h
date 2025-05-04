#pragma once

#include <string>

#define LUA_IDSIZE      (60)

struct lua_Debug {
    int event;
    const char* name;     /* (n) */
    const char* namewhat; /* (n) `global', `local', `field', `method' */
    const char* what;     /* (S) `Lua', `C', `main', `tail' */
    const char* source;   /* (S) */
    int currentline;      /* (l) */
    int nups;             /* (u) number of upvalues */
    int linedefined;      /* (S) */
    char short_src[LUA_IDSIZE]; /* (S) */
    /* private part */
    int i_ci;  /* active function */
};

typedef struct lua_Debug lua_Debug;  /* activation record */

/*
** Event codes
*/
#define LUA_HOOKCALL    (0)
#define LUA_HOOKRET     (1)
#define LUA_HOOKLINE    (2)
#define LUA_HOOKCOUNT   (3)
#define LUA_HOOKTAILRET (4)

/*
** Event masks
*/
#define LUA_MASKCALL    (1 << LUA_HOOKCALL)
#define LUA_MASKRET     (1 << LUA_HOOKRET)
#define LUA_MASKLINE    (1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT   (1 << LUA_HOOKCOUNT)

int lua_getstack(void* L, int level, lua_Debug* ar);
int lua_getinfo(void* L, std::string what, lua_Debug* ar);
std::string lua_getlocal(void* L, const lua_Debug* ar, int n);
std::string lua_setlocal(void* L, const lua_Debug* ar, int n);
std::string lua_getupvalue(void* L, int funcindex, int n);
std::string lua_setupvalue(void* L, int funcindex, int n);
void lua_getfenv(void* L, int index);
int lua_setfenv(void* L, int index);

// Custom function, return a string representation of a Lua value for debug purpose
std::string lua_todebugstring(void* L, int index);

typedef void(__fastcall* lua_Hook) (void* L, lua_Debug* ar);
int lua_sethook(void* L, lua_Hook func, int mask, int count);
lua_Hook lua_gethook(void* L);
int lua_gethookmask(void* L);

int LuaDebug_breakpoint();
void __fastcall LuaDebug_hook(void* L, lua_Debug* ar);

// L is default to GetContext(), it would be set inside LuaDebug_end()
void LuaDebug_end(void* L = NULL);
