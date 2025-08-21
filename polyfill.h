#pragma once

#include <cstdint>
#include <string>
#include <sstream>

/* These are some hot spots that I think might help if I rewrite them.
* 
* Matrix/Vector calculation is a must for 3D program. The game did it using x87 instructions. Replace them should result in
* better CPU utilization and more parallelism.
* 
* Blit functions transfer large data from memory to memory. The game's original logic is to use a REP MOVSQ to move large data,
* then switch to REP MOVSB to finish the tail. This was the optimal solution during 1996 - 2013.
* However in 2013, Intel deployed Enhanced REP MOVSB. Now REP MOVSB is faster, and it now requires a fixed heating up
* when it starts. So the original design is flipped. Replace them with std::memcpy and compile the mod with a modern compiler
* should fix it.
*/

extern uint64_t polyfill_debugCounter;
std::string getPolyfillDebug();

extern bool ERMS;
extern bool AVX;
void polyfill_checkCPU();

typedef float* (__fastcall* OPERATOR_MULTIPLY_1)(float*, float*, float*);
extern OPERATOR_MULTIPLY_1 p_operator_multiply_1;
extern OPERATOR_MULTIPLY_1 p_original_operator_multiply_1;
float* __fastcall detoured_operator_multiply_1(float* vecResult, float* vecA, float* matB);

typedef float* (__fastcall* OPERATOR_MULTIPLY_2)(float*, float*, float*);
extern OPERATOR_MULTIPLY_2 p_operator_multiply_2;
extern OPERATOR_MULTIPLY_2 p_original_operator_multiply_2;
float* __fastcall detoured_operator_multiply_2(float* vecResult, float* matA, float* vecB);

typedef float* (__fastcall* OPERATOR_MULTIPLY_3)(float*, float*, float*);
extern OPERATOR_MULTIPLY_3 p_operator_multiply_3;
extern OPERATOR_MULTIPLY_3 p_original_operator_multiply_3;
float* __fastcall detoured_operator_multiply_3(float* quaternionResult, float* quaternionA, float* matB);

typedef float* (__fastcall* OPERATOR_MULTIPLY_4)(float*, float*, float*);
extern OPERATOR_MULTIPLY_4 p_operator_multiply_4;
extern OPERATOR_MULTIPLY_4 p_original_operator_multiply_4;
float* __fastcall detoured_operator_multiply_4(float* matA, float* matB, float* matC);

typedef float* (__fastcall* OPERATOR_MULTIPLY_6)(float*, float*, float);
extern OPERATOR_MULTIPLY_6 p_operator_multiply_6;
extern OPERATOR_MULTIPLY_6 p_original_operator_multiply_6;
float* __fastcall detoured_operator_multiply_6(float* result, float* vecA, float factor);

typedef float* (__thiscall* OPERATOR_MULTIPLY_ASSIGN_1)(float*, float);
extern OPERATOR_MULTIPLY_ASSIGN_1 p_operator_multiply_assign_1;
extern OPERATOR_MULTIPLY_ASSIGN_1 p_original_operator_multiply_assign_1;
float* __fastcall detoured_operator_multiply_assign_1(float* self, void* ignored, float factor);

// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED
typedef float* (__thiscall* MATRIX_TRANSLATE_1)(float*, float*);
extern MATRIX_TRANSLATE_1 p_matrix_translate_1;
extern MATRIX_TRANSLATE_1 p_original_matrix_translate_1;
float* __fastcall detoured_matrix_translate_1(float* matSelf, void* ignored, float* matB);

typedef float* (__thiscall* MATRIX_SCALE_1)(float*, float*);
extern MATRIX_SCALE_1 p_matrix_scale_1;
extern MATRIX_SCALE_1 p_original_matrix_scale_1;
float* __fastcall detoured_matrix_scale_1(float* matSelf, void* ignored, float* vecB);

typedef void(__thiscall* MATRIX_SCALE_2)(float*, float);
extern MATRIX_SCALE_2 p_matrix_scale_2;
extern MATRIX_SCALE_2 p_original_matrix_scale_2;
void __fastcall detoured_matrix_scale_2(float* matSelf, void* ignored, float factor);

typedef float* (__fastcall* FUNTYPE_0x7be490)(float*, float*, float, bool);
extern FUNTYPE_0x7be490 p_fun_0x7be490;
extern FUNTYPE_0x7be490 p_original_fun_0x7be490;
float* __fastcall detoured_fun_0x7be490(float* matA, float* vecB, float angle, bool skipVectorNormalization);

typedef float* (__fastcall* FUNTYPE_0x7bdfc0)(float*, float*, float*);
extern FUNTYPE_0x7bdfc0 p_fun_0x7bdfc0;
extern FUNTYPE_0x7bdfc0 p_original_fun_0x7bdfc0;
float* __fastcall detoured_fun_0x7bdfc0(float* matSelf, float* matA, float* matB);

typedef float* (__fastcall* MATRIX_ROTATE_1)(float*, float*, float, bool);
extern MATRIX_ROTATE_1 p_matrix_rotate_1;
extern MATRIX_ROTATE_1 p_original_matrix_rotate_1;
float* __fastcall detoured_matrix_rotate_1(float* matA, float* vecB, float angle, bool skipVectorNormalization);

typedef double(__fastcall* SQUAREDMAGNITUDE)(float* vec);
extern SQUAREDMAGNITUDE p_squaredMagnitude;
extern SQUAREDMAGNITUDE p_original_squaredMagnitude;
double __fastcall detoured_squaredMagnitude(float* vec);

typedef void(__thiscall* CALPLANENORMAL)(float*, float*, float*, float*);
extern CALPLANENORMAL p_calculatePlaneNormal;
extern CALPLANENORMAL p_original_calculatePlaneNormal;
void __fastcall detoured_calculatePlaneNormal(float* self, void* ignored, float* p1, float* p2, float* p3);

typedef void(__fastcall* TRANSFORMAABOX)(float*, float*, float*, float*, float*);
extern TRANSFORMAABOX p_transformAABox;
extern TRANSFORMAABOX p_original_transformAABox;
void __fastcall detoured_transformAABox(float* C33Mat, float* C3Vec_A, float* C3Vec_B, float* CAAbox_A, float* CAAbox_B);

typedef int(__fastcall* LUA_SQRT)(void*);
extern LUA_SQRT p_lua_sqrt;
extern LUA_SQRT p_original_lua_sqrt;
int __fastcall detoured_lua_sqrt(void* L);

typedef void(__fastcall* BLIT_HUB)(int*, int, uint32_t, uint32_t, int, uint32_t, uint32_t, int);
extern BLIT_HUB p_blit_hub;
extern BLIT_HUB p_original_blit_hub;
void __fastcall detoured_blit_hub(int* vec2size, int unknownFuncIndex, uint32_t srcAddr, uint32_t srcStep, int srcFormat, uint32_t dstAddr, uint32_t dstStep, int dstFormat);
