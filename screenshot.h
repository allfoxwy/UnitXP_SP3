#pragma once

#include <cstdint>

// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
typedef int(__thiscall* CTGAFILE_WRITE_0x5a4810)(uint32_t, char*);
int __fastcall detoured_CTgaFile_Write_0x5a4810(uint32_t self, void* ignored, char* TGAfilename);

extern CTGAFILE_WRITE_0x5a4810 p_CTgaFile_Write_0x5a4810;
extern CTGAFILE_WRITE_0x5a4810 p_original_CTgaFile_Write_0x5a4810;
