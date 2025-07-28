#include "pch.h"

#include <intrin.h>
#include <cmath>

#include "x87polyfill.h"
#include "Vanilla1121_functions.h"

OPERATOR_MULTIPLY_1 p_operator_multiply_1 = reinterpret_cast<OPERATOR_MULTIPLY_1>(0x7bca80);
OPERATOR_MULTIPLY_1 p_original_operator_multiply_1 = NULL;
float* __fastcall detoured_operator_multiply_1(float* vecResult, float* vecA, float* matB)
{
    vecResult[0] = vecA[0] * matB[0] + matB[4] * vecA[1] + matB[8] * vecA[2] + matB[0xc];
    vecResult[1] = matB[5] * vecA[1] + matB[1] * vecA[0] + matB[9] * vecA[2] + matB[0xd];
    vecResult[2] = matB[6] * vecA[1] + matB[2] * vecA[0] + matB[10] * vecA[2] + matB[0xe];
    return vecResult;
}

OPERATOR_MULTIPLY_2 p_operator_multiply_2 = reinterpret_cast<OPERATOR_MULTIPLY_2>(0x7bcae0);
OPERATOR_MULTIPLY_2 p_original_operator_multiply_2 = NULL;
float* __fastcall detoured_operator_multiply_2(float* vecResult, float* matA, float* vecB)
{
    vecResult[0] = matA[0] * vecB[0] + matA[1] * vecB[1] + matA[2] * vecB[2] + matA[3];
    vecResult[1] = matA[5] * vecB[1] + matA[4] * vecB[0] + matA[6] * vecB[2] + matA[7];
    vecResult[2] = matA[9] * vecB[1] + matA[8] * vecB[0] + matA[10] * vecB[2] + matA[0xb];
    return vecResult;
}

OPERATOR_MULTIPLY_3 p_operator_multiply_3 = reinterpret_cast<OPERATOR_MULTIPLY_3>(0x7bcb40);
OPERATOR_MULTIPLY_3 p_original_operator_multiply_3 = NULL;
float* __fastcall detoured_operator_multiply_3(float* quaternionResult, float* quaternionA, float* matB)
{
    quaternionResult[0] = quaternionA[0] * matB[0] + matB[4] * quaternionA[1] + matB[8] * quaternionA[2] + matB[0xc] * quaternionA[3];
    quaternionResult[1] = matB[5] * quaternionA[1] + matB[9] * quaternionA[2] + matB[1] * quaternionA[0] + matB[0xd] * quaternionA[3];
    quaternionResult[2] = matB[6] * quaternionA[1] + matB[10] * quaternionA[2] + matB[2] * quaternionA[0] + matB[0xe] * quaternionA[3];
    quaternionResult[3] = matB[7] * quaternionA[1] + matB[0xb] * quaternionA[2] + matB[3] * quaternionA[0] + matB[0xf] * quaternionA[3];
    return quaternionResult;
}

OPERATOR_MULTIPLY_4 p_operator_multiply_4 = reinterpret_cast<OPERATOR_MULTIPLY_4>(0x7bc6a0);
OPERATOR_MULTIPLY_4 p_original_operator_multiply_4 = NULL;
// SSE implementation from https://gist.github.com/rygorous/4172889
union SSE_mat44 {
    float m[4][4];
    __m128 row[4];
};

static inline __m128 linearCombination_SSE(const __m128& a, const SSE_mat44& B)
{
    __m128 result;
    result = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x00), B.row[0]);
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0x55), B.row[1]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xaa), B.row[2]));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xff), B.row[3]));
    return result;
}

float* __fastcall detoured_operator_multiply_4(float* matA, float* matB, float* matC)
{
    __m128 out0x = linearCombination_SSE(reinterpret_cast<SSE_mat44*>(matB)->row[0], *reinterpret_cast<SSE_mat44*>(matC));
    __m128 out1x = linearCombination_SSE(reinterpret_cast<SSE_mat44*>(matB)->row[1], *reinterpret_cast<SSE_mat44*>(matC));
    __m128 out2x = linearCombination_SSE(reinterpret_cast<SSE_mat44*>(matB)->row[2], *reinterpret_cast<SSE_mat44*>(matC));
    __m128 out3x = linearCombination_SSE(reinterpret_cast<SSE_mat44*>(matB)->row[3], *reinterpret_cast<SSE_mat44*>(matC));

    reinterpret_cast<SSE_mat44*>(matA)->row[0] = out0x;
    reinterpret_cast<SSE_mat44*>(matA)->row[1] = out1x;
    reinterpret_cast<SSE_mat44*>(matA)->row[2] = out2x;
    reinterpret_cast<SSE_mat44*>(matA)->row[3] = out3x;

    return matA;
}
/* Original implementation
float* __fastcall detoured_operator_multiply_4(float* matA, float* matB, float* matC)
{
    if (false == enableX87polyfill) {
        return p_original_operator_multiply_4(matA, matB, matC);
    }

    matA[0] = matC[12] * matB[3] + matC[8] * matB[2] + matB[1] * matC[4] + matB[0] * matC[0];
    matA[1] = matB[3] * matC[13] + matB[2] * matC[9] + matC[5] * matB[1] + matC[1] * matB[0];
    matA[2] = matB[3] * matC[14] + matB[2] * matC[10] + matC[6] * matB[1] + matC[2] * matB[0];
    matA[3] = matB[2] * matC[11] + matB[3] * matC[15] + matB[0] * matC[3] + matC[7] * matB[1];
    matA[4] = matB[7] * matC[12] + matB[6] * matC[8] + matB[4] * matC[0] + matC[4] * matB[5];
    matA[5] = matB[7] * matC[13] + matB[6] * matC[9] + matC[5] * matB[5] + matC[1] * matB[4];
    matA[6] = matB[7] * matC[14] + matB[6] * matC[10] + matC[6] * matB[5] + matC[2] * matB[4];
    matA[7] = matB[6] * matC[11] + matB[7] * matC[15] + matB[4] * matC[3] + matB[5] * matC[7];
    matA[8] = matB[11] * matC[12] + matB[10] * matC[8] + matB[8] * matC[0] + matC[4] * matB[9];
    matA[9] = matB[11] * matC[13] + matB[10] * matC[9] + matC[5] * matB[9] + matC[1] * matB[8];
    matA[10] = matB[11] * matC[14] + matB[10] * matC[10] + matC[6] * matB[9] + matC[2] * matB[8];
    matA[11] = matB[10] * matC[11] + matB[11] * matC[15] + matB[8] * matC[3] + matB[9] * matC[7];
    matA[12] = matB[14] * matC[8] + matB[13] * matC[4] + matB[15] * matC[12] + matB[12] * matC[0];
    matA[13] = matB[14] * matC[9] + matB[12] * matC[1] + matB[13] * matC[5] + matB[15] * matC[13];
    matA[14] = matB[14] * matC[10] + matB[12] * matC[2] + matB[13] * matC[6] + matB[15] * matC[14];
    matA[15] = matB[14] * matC[11] + matB[13] * matC[7] + matB[12] * matC[3] + matC[15] * matB[15];

    return matA;
}
*/

/* Seems not used
* typedef float* (__fastcall* OPERATOR_MULTIPLY_5)(float*, float*, float*);
OPERATOR_MULTIPLY_5 p_operator_multiply_5 = reinterpret_cast<OPERATOR_MULTIPLY_5>(0x7bae60);
OPERATOR_MULTIPLY_5 p_original_operator_multiply_5 = NULL;
float* __fastcall detoured_operator_multiply_5(float* matA, float* matB, float* matC)
{
    if (false == enableX87polyfill) {
        return p_original_operator_multiply_5(matA, matB, matC);
    }
    // not tested
    matA[1] = matB[2] * matC[7] + matB[1] * matC[4] + matC[1] * matB[0];
    matA[2] = matB[1] * matC[5] + matB[2] * matC[8] + matB[0] * matC[2];
    matA[3] = matC[6] * matB[5] + matC[3] * matB[4] + matB[3] * matC[0];
    matA[0] = matB[0] * matC[0] + matB[2] * matC[6] + matB[1] * matC[3];
    matA[4] = matB[3] * matC[1] + matB[5] * matC[7] + matB[4] * matC[4];
    matA[6] = matB[6] * matC[0] + matC[6] * matB[8] + matC[3] * matB[7];
    matA[5] = matC[8] * matB[5] + matB[3] * matC[2] + matB[4] * matC[5];
    matA[7] = matB[8] * matC[7] + matB[7] * matC[4] + matB[6] * matC[1];
    matA[8] = matB[8] * matC[8] + matB[6] * matC[2] + matB[7] * matC[5];
    matA[9] = matC[6] * matB[11] + matB[10] * matC[3] + matB[9] * matC[0] + matC[9];
    matA[10] = matB[10] * matC[4] + matB[9] * matC[1] + matB[11] * matC[7] + matC[10];
    matA[11] = matB[10] * matC[5] + matB[9] * matC[2] + matC[8] * matB[11] + matC[11];

    return matA;
}
*/


MATRIX_TRANSLATE_1 p_matrix_translate_1 = reinterpret_cast<MATRIX_TRANSLATE_1>(0x7bdc40);
MATRIX_TRANSLATE_1 p_original_matrix_translate_1 = NULL;
float* __fastcall detoured_matrix_translate_1(float* matSelf, void* ignored, float* matB)
{
    matSelf[0xc] = matB[0] * matSelf[0] + matSelf[4] * matB[1] + matSelf[8] * matB[2] + matSelf[0xc];
    matSelf[0xd] = matSelf[1] * matB[0] + matSelf[5] * matB[1] + matSelf[9] * matB[2] + matSelf[0xd];
    matSelf[0xe] = matSelf[2] * matB[0] + matSelf[6] * matB[1] + matSelf[10] * matB[2] + matSelf[0xe];
    return matB;
}

MATRIX_SCALE_1 p_matrix_scale_1 = reinterpret_cast<MATRIX_SCALE_1>(0x7bdca0);
MATRIX_SCALE_1 p_original_matrix_scale_1 = NULL;
float* __fastcall detoured_matrix_scale_1(float* matSelf, void* ignored, float* vecB)
{
    matSelf[0] *= vecB[0];
    matSelf[1] *= vecB[0];
    matSelf[2] *= vecB[0];
    matSelf[4] *= vecB[1];
    matSelf[5] *= vecB[1];
    matSelf[6] *= vecB[1];
    matSelf[8] *= vecB[2];
    matSelf[9] *= vecB[2];
    matSelf[10] *= vecB[2];
    return vecB;
}

MATRIX_SCALE_2 p_matrix_scale_2 = reinterpret_cast<MATRIX_SCALE_2>(0x7bdd00);
MATRIX_SCALE_2 p_original_matrix_scale_2 = NULL;
void __fastcall detoured_matrix_scale_2(float* matSelf, void* ignored, float factor)
{
    matSelf[0] *= factor;
    matSelf[1] *= factor;
    matSelf[2] *= factor;
    matSelf[4] *= factor;
    matSelf[5] *= factor;
    matSelf[6] *= factor;
    matSelf[8] *= factor;
    matSelf[9] *= factor;
    matSelf[10] *= factor;
}

FUNTYPE_0x7be490 p_fun_0x7be490 = reinterpret_cast<FUNTYPE_0x7be490>(0x7be490);
FUNTYPE_0x7be490 p_original_fun_0x7be490 = NULL;
float* __fastcall detoured_fun_0x7be490(float* matA, float* vecB, float angle, bool skipSQRT)
{
    if (skipSQRT == false) {
        float sqrtResult = 1.0f / vectorLength(vecB);
        vecB[0] *= sqrtResult;
        vecB[1] *= sqrtResult;
        vecB[2] *= sqrtResult;
    }
    float cosResult = std::cos(angle);
    float sinResult = std::sin(angle);
    float rcosResult = 1.0f - cosResult;
    float rVecXY = rcosResult * vecB[1] * vecB[0];
    float rVecXZ = rcosResult * vecB[2] * vecB[0];
    float rVecYZ = rcosResult * vecB[2] * vecB[1];
    matA[0] = std::pow(vecB[0], 2.0f) * rcosResult + cosResult;
    matA[1] = rVecXY + vecB[2] * sinResult;
    matA[2] = rVecXZ - vecB[1] * sinResult;
    matA[3] = rVecXY - vecB[2] * sinResult;
    matA[4] = std::pow(vecB[1], 2.0f) * rcosResult + cosResult;
    matA[5] = rVecYZ + vecB[0] * sinResult;
    matA[6] = rVecXZ + vecB[1] * sinResult;
    matA[7] = rVecYZ - vecB[0] * sinResult;
    matA[8] = std::pow(vecB[2], 2.0f) * rcosResult + cosResult;
    return matA;
}

/* Seems not used
* typedef float* (__thiscall* FUNTYPE_0x7bf860)(float*, float*, float*, float*);
FUNTYPE_0x7bf860 p_fun_0x7bf860 = reinterpret_cast<FUNTYPE_0x7bf860>(0x7bf860);
FUNTYPE_0x7bf860 p_original_fun_0x7bf860 = NULL;
float* __fastcall detoured_fun_0x7bf860(float* matSelf, void* ignored, float* vecB, float* vecC, float* vecD)
{
    if (false == enableX87polyfill) {
        return p_original_fun_0x7bf860(matSelf, vecB, vecC, vecD);
    }
    // not tested
    float rsqrtResult = 1.0f / vectorLength(matSelf);
    matSelf[0] = (vecC[1] - vecB[1]) * (vecD[2] - vecB[2]) - (vecC[2] - vecB[2]) * (vecD[1] - vecB[1]);
    matSelf[0] *= rsqrtResult;
    matSelf[1] = (vecD[0] - vecB[0]) * (vecC[2] - vecB[2]) - (vecC[0] - vecB[0]) * (vecD[2] - vecB[2]);
    matSelf[1] *= rsqrtResult;
    matSelf[2] = (vecC[0] - vecB[0]) * (vecD[1] - vecB[1]) - (vecD[0] - vecB[0]) * (vecC[1] - vecB[1]);
    matSelf[2] *= rsqrtResult;
    matSelf[3] = -(matSelf[0] * vecB[0] + matSelf[1] * vecB[1] + matSelf[2] * vecB[2]);
    matSelf[3] *= rsqrtResult;
    matSelf[4] = vecB[0];
    matSelf[5] = vecB[1];
    matSelf[6] = vecB[2];
    matSelf[7] = vecC[0];
    matSelf[8] = vecC[1];
    matSelf[9] = vecC[2];
    matSelf[10] = vecD[0];
    matSelf[0xb] = vecD[1];
    matSelf[0xc] = vecD[2];
    return matSelf;
}
*/

MATRIX_ROTATE_1 p_matrix_rotate_1 = reinterpret_cast<MATRIX_ROTATE_1>(0x7bdb00);
MATRIX_ROTATE_1 p_original_matrix_rotate_1 = NULL;
float* __fastcall detoured_matrix_rotate_1(float* matA, float* vecB, float angle, bool skipSQRT)
{
    if (skipSQRT == false) {
        float sqrtResult = 1.0f / vectorLength(vecB);
        vecB[0] *= sqrtResult;
        vecB[1] *= sqrtResult;
        vecB[2] *= sqrtResult;
    }

    float cosResult = std::cos(angle);
    float sinResult = std::sin(angle);
    float rcosResult = 1.0f - cosResult;

    matA[0] = std::pow(vecB[0], 2.0f) * rcosResult + cosResult;
    matA[1] = rcosResult * vecB[1] * vecB[0] + vecB[2] * sinResult;
    matA[2] = rcosResult * vecB[2] * vecB[0] - vecB[1] * sinResult;
    matA[3] = 0.0f;
    matA[4] = rcosResult * vecB[1] * vecB[0] - vecB[2] * sinResult;
    matA[5] = std::pow(vecB[1], 2.0f) * rcosResult + cosResult;
    matA[6] = vecB[0] * sinResult + rcosResult * vecB[2] * vecB[1];
    matA[7] = 0.0f;
    matA[8] = rcosResult * vecB[2] * vecB[0] + vecB[1] * sinResult;
    matA[9] = rcosResult * vecB[2] * vecB[1] - vecB[0] * sinResult;
    matA[0xa] = std::pow(vecB[2], 2.0f) * rcosResult + cosResult;
    matA[0xb] = 0.0f;
    matA[0xc] = 0.0f;
    matA[0xd] = 0.0f;
    matA[0xe] = 0.0f;
    matA[0xf] = 1.0f;

    return matA;
}

LUA_SQRT p_lua_sqrt = reinterpret_cast<LUA_SQRT>(0x7fb020);
LUA_SQRT p_original_lua_sqrt = NULL;
int __fastcall detoured_lua_sqrt(void* L) {
    double v = luaL_checknumber(L, 1);
    lua_pushnumber(L, std::sqrt(v));
    return 1;
}
