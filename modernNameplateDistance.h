#pragma once

// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
typedef void(__thiscall* ADDNAMEPLATE)(void*, void*, void*);
extern ADDNAMEPLATE p_addNameplate;
extern ADDNAMEPLATE p_original_addNameplate;

typedef int(__thiscall* RENDERWORLD)(void* self);
extern RENDERWORLD p_renderWorld;
extern RENDERWORLD p_original_renderWorld;

typedef void(__thiscall* REMOVENAMEPLATE)(uint32_t);
extern REMOVENAMEPLATE p_removeNameplate;

extern bool modernNameplateDistance;
extern bool prioritizeTargetNameplate;
extern bool prioritizeMarkedNameplate;
extern bool nameplateCombatFilter;
extern bool showInCombatNameplatesNearPlayer;
extern bool hideCritterNameplate;

int __fastcall detoured_renderWorld(void* self, void* ignored);
void __fastcall detoured_addNameplate(void* self, void* ignored, void* unknown1, void* unknown2);
