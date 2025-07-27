#pragma once

#include <cstdint>

/* These are some hot spots that I think might help if recompile them in SSE.
* But to be frank, I see no big difference than x87.
*/

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
float* __fastcall detoured_fun_0x7be490(float* matA, float* vecB, float angle, bool skipSQRT);

typedef float* (__fastcall* MATRIX_ROTATE_1)(float*, float*, float, bool);
extern MATRIX_ROTATE_1 p_matrix_rotate_1;
extern MATRIX_ROTATE_1 p_original_matrix_rotate_1;
float* __fastcall detoured_matrix_rotate_1(float* matA, float* vecB, float angle, bool skipSQRT);
