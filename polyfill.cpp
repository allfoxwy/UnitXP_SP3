#include "pch.h"

#include <cmath>
#include <map>
#include <tuple>

#include <intrin.h>

#include "polyfill.h"
#include "Vanilla1121_functions.h"

bool ERMS = false;
bool AVX = false;
void polyfill_checkCPU() {
    // By Chat GPT

    int cpuInfo[4] = { 0 };

    __cpuid(cpuInfo, 0);
    if (cpuInfo[0] < 7) {
        ERMS = false;
    }

    __cpuidex(cpuInfo, 0x7, 0);
    ERMS = (cpuInfo[1] & (1 << 9)) != 0;

    __cpuid(cpuInfo, 1);

    bool osUsesXSAVE_XRSTORE = (cpuInfo[2] & (1 << 27)) != 0;
    bool cpuAVXSupport = (cpuInfo[2] & (1 << 28)) != 0;

    if (osUsesXSAVE_XRSTORE && cpuAVXSupport) {
        unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        AVX = (xcrFeatureMask & 0x6) == 0x6;
    }
    else {
        AVX = false;
    }
}

OPERATOR_MULTIPLY_1 p_operator_multiply_1 = reinterpret_cast<OPERATOR_MULTIPLY_1>(0x7bca80);
OPERATOR_MULTIPLY_1 p_original_operator_multiply_1 = NULL;
float* __fastcall detoured_operator_multiply_1(float* vecResult, float* vecA, float* matB)
{
    vecResult[0] = static_cast<float>(static_cast<double>(vecA[0]) * static_cast<double>(matB[0]) + static_cast<double>(matB[4]) * static_cast<double>(vecA[1]) + static_cast<double>(matB[8]) * static_cast<double>(vecA[2]) + static_cast<double>(matB[0xc]));
    vecResult[1] = static_cast<float>(static_cast<double>(matB[5]) * static_cast<double>(vecA[1]) + static_cast<double>(matB[1]) * static_cast<double>(vecA[0]) + static_cast<double>(matB[9]) * static_cast<double>(vecA[2]) + static_cast<double>(matB[0xd]));
    vecResult[2] = static_cast<float>(static_cast<double>(matB[6]) * static_cast<double>(vecA[1]) + static_cast<double>(matB[2]) * static_cast<double>(vecA[0]) + static_cast<double>(matB[10]) * static_cast<double>(vecA[2]) + static_cast<double>(matB[0xe]));
    return vecResult;
}

OPERATOR_MULTIPLY_2 p_operator_multiply_2 = reinterpret_cast<OPERATOR_MULTIPLY_2>(0x7bcae0);
OPERATOR_MULTIPLY_2 p_original_operator_multiply_2 = NULL;
float* __fastcall detoured_operator_multiply_2(float* vecResult, float* matA, float* vecB)
{
    vecResult[0] = static_cast<float>(static_cast<double>(matA[0]) * static_cast<double>(vecB[0]) + static_cast<double>(matA[1]) * static_cast<double>(vecB[1]) + static_cast<double>(matA[2]) * static_cast<double>(vecB[2]) + static_cast<double>(matA[3]));
    vecResult[1] = static_cast<float>(static_cast<double>(matA[5]) * static_cast<double>(vecB[1]) + static_cast<double>(matA[4]) * static_cast<double>(vecB[0]) + static_cast<double>(matA[6]) * static_cast<double>(vecB[2]) + static_cast<double>(matA[7]));
    vecResult[2] = static_cast<float>(static_cast<double>(matA[9]) * static_cast<double>(vecB[1]) + static_cast<double>(matA[8]) * static_cast<double>(vecB[0]) + static_cast<double>(matA[10]) * static_cast<double>(vecB[2]) + static_cast<double>(matA[0xb]));
    return vecResult;
}

OPERATOR_MULTIPLY_3 p_operator_multiply_3 = reinterpret_cast<OPERATOR_MULTIPLY_3>(0x7bcb40);
OPERATOR_MULTIPLY_3 p_original_operator_multiply_3 = NULL;
float* __fastcall detoured_operator_multiply_3(float* quaternionResult, float* quaternionA, float* matB)
{
    quaternionResult[0] = static_cast<float>(static_cast<double>(matB[0]) * static_cast<double>(quaternionA[0]) + static_cast<double>(matB[4]) * static_cast<double>(quaternionA[1]) + static_cast<double>(matB[8]) * static_cast<double>(quaternionA[2]) + static_cast<double>(matB[0xc]) * static_cast<double>(quaternionA[3]));
    quaternionResult[1] = static_cast<float>(static_cast<double>(matB[5]) * static_cast<double>(quaternionA[1]) + static_cast<double>(matB[9]) * static_cast<double>(quaternionA[2]) + static_cast<double>(matB[1]) * static_cast<double>(quaternionA[0]) + static_cast<double>(matB[0xd]) * static_cast<double>(quaternionA[3]));
    quaternionResult[2] = static_cast<float>(static_cast<double>(matB[6]) * static_cast<double>(quaternionA[1]) + static_cast<double>(matB[10]) * static_cast<double>(quaternionA[2]) + static_cast<double>(matB[2]) * static_cast<double>(quaternionA[0]) + static_cast<double>(matB[0xe]) * static_cast<double>(quaternionA[3]));
    quaternionResult[3] = static_cast<float>(static_cast<double>(matB[7]) * static_cast<double>(quaternionA[1]) + static_cast<double>(matB[0xb]) * static_cast<double>(quaternionA[2]) + static_cast<double>(matB[3]) * static_cast<double>(quaternionA[0]) + static_cast<double>(matB[0xf]) * static_cast<double>(quaternionA[3]));
    return quaternionResult;
}

OPERATOR_MULTIPLY_4 p_operator_multiply_4 = reinterpret_cast<OPERATOR_MULTIPLY_4>(0x7bc6a0);
OPERATOR_MULTIPLY_4 p_original_operator_multiply_4 = NULL;
float* __fastcall detoured_operator_multiply_4(float* matA, float* matB, float* matC)
{
    matA[0] = static_cast<float>(static_cast<double>(matC[12]) * static_cast<double>(matB[3]) + static_cast<double>(matC[8]) * static_cast<double>(matB[2]) + static_cast<double>(matB[1]) * static_cast<double>(matC[4]) + static_cast<double>(matB[0]) * static_cast<double>(matC[0]));
    matA[1] = static_cast<float>(static_cast<double>(matB[3]) * static_cast<double>(matC[13]) + static_cast<double>(matB[2]) * static_cast<double>(matC[9]) + static_cast<double>(matC[5]) * static_cast<double>(matB[1]) + static_cast<double>(matC[1]) * static_cast<double>(matB[0]));
    matA[2] = static_cast<float>(static_cast<double>(matB[3]) * static_cast<double>(matC[14]) + static_cast<double>(matB[2]) * static_cast<double>(matC[10]) + static_cast<double>(matC[6]) * static_cast<double>(matB[1]) + static_cast<double>(matC[2]) * static_cast<double>(matB[0]));
    matA[3] = static_cast<float>(static_cast<double>(matB[2]) * static_cast<double>(matC[11]) + static_cast<double>(matB[3]) * static_cast<double>(matC[15]) + static_cast<double>(matB[0]) * static_cast<double>(matC[3]) + static_cast<double>(matC[7]) * static_cast<double>(matB[1]));
    matA[4] = static_cast<float>(static_cast<double>(matB[7]) * static_cast<double>(matC[12]) + static_cast<double>(matB[6]) * static_cast<double>(matC[8]) + static_cast<double>(matB[4]) * static_cast<double>(matC[0]) + static_cast<double>(matC[4]) * static_cast<double>(matB[5]));
    matA[5] = static_cast<float>(static_cast<double>(matB[7]) * static_cast<double>(matC[13]) + static_cast<double>(matB[6]) * static_cast<double>(matC[9]) + static_cast<double>(matC[5]) * static_cast<double>(matB[5]) + static_cast<double>(matC[1]) * static_cast<double>(matB[4]));
    matA[6] = static_cast<float>(static_cast<double>(matB[7]) * static_cast<double>(matC[14]) + static_cast<double>(matB[6]) * static_cast<double>(matC[10]) + static_cast<double>(matC[6]) * static_cast<double>(matB[5]) + static_cast<double>(matC[2]) * static_cast<double>(matB[4]));
    matA[7] = static_cast<float>(static_cast<double>(matB[6]) * static_cast<double>(matC[11]) + static_cast<double>(matB[7]) * static_cast<double>(matC[15]) + static_cast<double>(matB[4]) * static_cast<double>(matC[3]) + static_cast<double>(matB[5]) * static_cast<double>(matC[7]));
    matA[8] = static_cast<float>(static_cast<double>(matB[11]) * static_cast<double>(matC[12]) + static_cast<double>(matB[10]) * static_cast<double>(matC[8]) + static_cast<double>(matB[8]) * static_cast<double>(matC[0]) + static_cast<double>(matC[4]) * static_cast<double>(matB[9]));
    matA[9] = static_cast<float>(static_cast<double>(matB[11]) * static_cast<double>(matC[13]) + static_cast<double>(matB[10]) * static_cast<double>(matC[9]) + static_cast<double>(matC[5]) * static_cast<double>(matB[9]) + static_cast<double>(matC[1]) * static_cast<double>(matB[8]));
    matA[10] = static_cast<float>(static_cast<double>(matB[11]) * static_cast<double>(matC[14]) + static_cast<double>(matB[10]) * static_cast<double>(matC[10]) + static_cast<double>(matC[6]) * static_cast<double>(matB[9]) + static_cast<double>(matC[2]) * static_cast<double>(matB[8]));
    matA[11] = static_cast<float>(static_cast<double>(matB[10]) * static_cast<double>(matC[11]) + static_cast<double>(matB[11]) * static_cast<double>(matC[15]) + static_cast<double>(matB[8]) * static_cast<double>(matC[3]) + static_cast<double>(matB[9]) * static_cast<double>(matC[7]));
    matA[12] = static_cast<float>(static_cast<double>(matB[14]) * static_cast<double>(matC[8]) + static_cast<double>(matB[13]) * static_cast<double>(matC[4]) + static_cast<double>(matB[15]) * static_cast<double>(matC[12]) + static_cast<double>(matB[12]) * static_cast<double>(matC[0]));
    matA[13] = static_cast<float>(static_cast<double>(matB[14]) * static_cast<double>(matC[9]) + static_cast<double>(matB[12]) * static_cast<double>(matC[1]) + static_cast<double>(matB[13]) * static_cast<double>(matC[5]) + static_cast<double>(matB[15]) * static_cast<double>(matC[13]));
    matA[14] = static_cast<float>(static_cast<double>(matB[14]) * static_cast<double>(matC[10]) + static_cast<double>(matB[12]) * static_cast<double>(matC[2]) + static_cast<double>(matB[13]) * static_cast<double>(matC[6]) + static_cast<double>(matB[15]) * static_cast<double>(matC[14]));
    matA[15] = static_cast<float>(static_cast<double>(matB[14]) * static_cast<double>(matC[11]) + static_cast<double>(matB[13]) * static_cast<double>(matC[7]) + static_cast<double>(matB[12]) * static_cast<double>(matC[3]) + static_cast<double>(matC[15]) * static_cast<double>(matB[15]));

    return matA;
}

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

OPERATOR_MULTIPLY_6 p_operator_multiply_6 = reinterpret_cast<OPERATOR_MULTIPLY_6>(0x5f8cf0);
OPERATOR_MULTIPLY_6 p_original_operator_multiply_6 = NULL;
float* __fastcall detoured_operator_multiply_6(float* result, float* vecA, float factor) {
    result[0] = static_cast<float>(static_cast<double>(vecA[0]) * static_cast<double>(factor));
    result[1] = static_cast<float>(static_cast<double>(vecA[1]) * static_cast<double>(factor));
    result[2] = static_cast<float>(static_cast<double>(vecA[2]) * static_cast<double>(factor));

    return result;
}

OPERATOR_MULTIPLY_ASSIGN_1 p_operator_multiply_assign_1 = reinterpret_cast<OPERATOR_MULTIPLY_ASSIGN_1>(0x5132f0);
OPERATOR_MULTIPLY_ASSIGN_1 p_original_operator_multiply_assign_1 = NULL;
float* __fastcall detoured_operator_multiply_assign_1(float* self, void* ignored, float factor) {
    self[0] = static_cast<float>(static_cast<double>(self[0]) * static_cast<double>(factor));
    self[1] = static_cast<float>(static_cast<double>(self[1]) * static_cast<double>(factor));
    self[2] = static_cast<float>(static_cast<double>(self[2]) * static_cast<double>(factor));
    return self;
}

MATRIX_TRANSLATE_1 p_matrix_translate_1 = reinterpret_cast<MATRIX_TRANSLATE_1>(0x7bdc40);
MATRIX_TRANSLATE_1 p_original_matrix_translate_1 = NULL;
float* __fastcall detoured_matrix_translate_1(float* matSelf, void* ignored, float* matB)
{
    matSelf[0xc] = static_cast<float>(static_cast<double>(matSelf[0]) * static_cast<double>(matB[0]) + static_cast<double>(matSelf[4]) * static_cast<double>(matB[1]) + static_cast<double>(matSelf[8]) * static_cast<double>(matB[2]) + static_cast<double>(matSelf[0xc]));
    matSelf[0xd] = static_cast<float>(static_cast<double>(matSelf[1]) * static_cast<double>(matB[0]) + static_cast<double>(matSelf[5]) * static_cast<double>(matB[1]) + static_cast<double>(matSelf[9]) * static_cast<double>(matB[2]) + static_cast<double>(matSelf[0xd]));
    matSelf[0xe] = static_cast<float>(static_cast<double>(matSelf[2]) * static_cast<double>(matB[0]) + static_cast<double>(matSelf[6]) * static_cast<double>(matB[1]) + static_cast<double>(matSelf[10]) * static_cast<double>(matB[2]) + static_cast<double>(matSelf[0xe]));
    return matB;
}

MATRIX_SCALE_1 p_matrix_scale_1 = reinterpret_cast<MATRIX_SCALE_1>(0x7bdca0);
MATRIX_SCALE_1 p_original_matrix_scale_1 = NULL;
float* __fastcall detoured_matrix_scale_1(float* matSelf, void* ignored, float* vecB)
{
    matSelf[0] = static_cast<float>(static_cast<double>(matSelf[0]) * static_cast<double>(vecB[0]));
    matSelf[1] = static_cast<float>(static_cast<double>(matSelf[1]) * static_cast<double>(vecB[0]));
    matSelf[2] = static_cast<float>(static_cast<double>(matSelf[2]) * static_cast<double>(vecB[0]));
    matSelf[4] = static_cast<float>(static_cast<double>(matSelf[4]) * static_cast<double>(vecB[1]));
    matSelf[5] = static_cast<float>(static_cast<double>(matSelf[5]) * static_cast<double>(vecB[1]));
    matSelf[6] = static_cast<float>(static_cast<double>(matSelf[6]) * static_cast<double>(vecB[1]));
    matSelf[8] = static_cast<float>(static_cast<double>(matSelf[8]) * static_cast<double>(vecB[2]));
    matSelf[9] = static_cast<float>(static_cast<double>(matSelf[9]) * static_cast<double>(vecB[2]));
    matSelf[10] = static_cast<float>(static_cast<double>(matSelf[10]) * static_cast<double>(vecB[2]));
    return vecB;
}

MATRIX_SCALE_2 p_matrix_scale_2 = reinterpret_cast<MATRIX_SCALE_2>(0x7bdd00);
MATRIX_SCALE_2 p_original_matrix_scale_2 = NULL;
void __fastcall detoured_matrix_scale_2(float* matSelf, void* ignored, float factor)
{
    matSelf[0] = static_cast<float>(static_cast<double>(matSelf[0]) * static_cast<double>(factor));
    matSelf[1] = static_cast<float>(static_cast<double>(matSelf[1]) * static_cast<double>(factor));
    matSelf[2] = static_cast<float>(static_cast<double>(matSelf[2]) * static_cast<double>(factor));
    matSelf[4] = static_cast<float>(static_cast<double>(matSelf[4]) * static_cast<double>(factor));
    matSelf[5] = static_cast<float>(static_cast<double>(matSelf[5]) * static_cast<double>(factor));
    matSelf[6] = static_cast<float>(static_cast<double>(matSelf[6]) * static_cast<double>(factor));
    matSelf[8] = static_cast<float>(static_cast<double>(matSelf[8]) * static_cast<double>(factor));
    matSelf[9] = static_cast<float>(static_cast<double>(matSelf[9]) * static_cast<double>(factor));
    matSelf[10] = static_cast<float>(static_cast<double>(matSelf[10]) * static_cast<double>(factor));
}

FUNTYPE_0x7be490 p_fun_0x7be490 = reinterpret_cast<FUNTYPE_0x7be490>(0x7be490);
FUNTYPE_0x7be490 p_original_fun_0x7be490 = NULL;
float* __fastcall detoured_fun_0x7be490(float* matA, float* vecB, float angle, bool skipVectorNormalization)
{
    if (skipVectorNormalization == false) {
        double sqrtResult = std::sqrt(std::pow(static_cast<double>(vecB[0]), 2.0) + std::pow(static_cast<double>(vecB[1]), 2.0) + std::pow(static_cast<double>(vecB[2]), 2.0));
        vecB[0] = static_cast<float>(vecB[0] / sqrtResult);
        vecB[1] = static_cast<float>(vecB[1] / sqrtResult);
        vecB[2] = static_cast<float>(vecB[2] / sqrtResult);
    }
    double cosResult = std::cos(static_cast<double>(angle));
    double sinResult = std::sin(static_cast<double>(angle));
    double rcosResult = 1.0 - cosResult;
    double rVecXY = rcosResult * static_cast<double>(vecB[1]) * static_cast<double>(vecB[0]);
    double rVecXZ = rcosResult * static_cast<double>(vecB[2]) * static_cast<double>(vecB[0]);
    double rVecYZ = rcosResult * static_cast<double>(vecB[2]) * static_cast<double>(vecB[1]);

    matA[0] = static_cast<float>(std::pow(static_cast<double>(vecB[0]), 2.0) * rcosResult + cosResult);
    matA[1] = static_cast<float>(rVecXY + static_cast<double>(vecB[2]) * sinResult);
    matA[2] = static_cast<float>(rVecXZ - static_cast<double>(vecB[1]) * sinResult);
    matA[3] = static_cast<float>(rVecXY - static_cast<double>(vecB[2]) * sinResult);
    matA[4] = static_cast<float>(std::pow(static_cast<double>(vecB[1]), 2.0) * rcosResult + cosResult);
    matA[5] = static_cast<float>(rVecYZ + static_cast<double>(vecB[0]) * sinResult);
    matA[6] = static_cast<float>(rVecXZ + static_cast<double>(vecB[1]) * sinResult);
    matA[7] = static_cast<float>(rVecYZ - static_cast<double>(vecB[0]) * sinResult);
    matA[8] = static_cast<float>(std::pow(static_cast<double>(vecB[2]), 2.0) * rcosResult + cosResult);

    return matA;
}

FUNTYPE_0x7bdfc0 p_fun_0x7bdfc0 = reinterpret_cast<FUNTYPE_0x7bdfc0>(0x7bdfc0);
FUNTYPE_0x7bdfc0 p_original_fun_0x7bdfc0 = NULL;
float* __fastcall detoured_fun_0x7bdfc0(float* matSelf, float* matA, float* matB) {
    matSelf[0] = static_cast<float>(static_cast<double>(matA[0]) * static_cast<double>(matB[0]) + static_cast<double>(matB[3]) * static_cast<double>(matA[1]) + static_cast<double>(matB[6]) * static_cast<double>(matA[2]));
    matSelf[1] = static_cast<float>(static_cast<double>(matA[1]) * static_cast<double>(matB[4]) + static_cast<double>(matB[7]) * static_cast<double>(matA[2]) + static_cast<double>(matB[1]) * static_cast<double>(matA[0]));
    matSelf[2] = static_cast<float>(static_cast<double>(matA[1]) * static_cast<double>(matB[5]) + static_cast<double>(matA[0]) * static_cast<double>(matB[2]) + static_cast<double>(matB[8]) * static_cast<double>(matA[2]));
    matSelf[3] = static_cast<float>(static_cast<double>(matB[3]) * static_cast<double>(matA[4]) + static_cast<double>(matB[6]) * static_cast<double>(matA[5]) + static_cast<double>(matA[3]) * static_cast<double>(matB[0]));
    matSelf[4] = static_cast<float>(static_cast<double>(matA[3]) * static_cast<double>(matB[1]) + static_cast<double>(matA[4]) * static_cast<double>(matB[4]) + static_cast<double>(matA[5]) * static_cast<double>(matB[7]));
    matSelf[5] = static_cast<float>(static_cast<double>(matA[4]) * static_cast<double>(matB[5]) + static_cast<double>(matA[3]) * static_cast<double>(matB[2]) + static_cast<double>(matA[5]) * static_cast<double>(matB[8]));
    matSelf[6] = static_cast<float>(static_cast<double>(matA[6]) * static_cast<double>(matB[0]) + static_cast<double>(matA[7]) * static_cast<double>(matB[3]) + static_cast<double>(matB[6]) * static_cast<double>(matA[8]));
    matSelf[7] = static_cast<float>(static_cast<double>(matA[8]) * static_cast<double>(matB[7]) + static_cast<double>(matA[6]) * static_cast<double>(matB[1]) + static_cast<double>(matA[7]) * static_cast<double>(matB[4]));
    matSelf[8] = static_cast<float>(static_cast<double>(matB[8]) * static_cast<double>(matA[8]) + static_cast<double>(matA[6]) * static_cast<double>(matB[2]) + static_cast<double>(matA[7]) * static_cast<double>(matB[5]));
    return matSelf;
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
float* __fastcall detoured_matrix_rotate_1(float* matA, float* vecB, float angle, bool skipVectorNormalization)
{
    if (skipVectorNormalization == false) {
        double sqrtResult = std::sqrt(std::pow(static_cast<double>(vecB[0]), 2.0) + std::pow(static_cast<double>(vecB[1]), 2.0) + std::pow(static_cast<double>(vecB[2]), 2.0));
        vecB[0] = static_cast<float>(vecB[0] / sqrtResult);
        vecB[1] = static_cast<float>(vecB[1] / sqrtResult);
        vecB[2] = static_cast<float>(vecB[2] / sqrtResult);
    }

    double cosResult = std::cos(static_cast<double>(angle));
    double sinResult = std::sin(static_cast<double>(angle));
    double rcosResult = 1.0 - cosResult;

    matA[0] = static_cast<float>(std::pow(static_cast<double>(vecB[0]), 2.0) * rcosResult + cosResult);
    matA[1] = static_cast<float>(rcosResult * static_cast<double>(vecB[1]) * static_cast<double>(vecB[0]) + static_cast<double>(vecB[2]) * sinResult);
    matA[2] = static_cast<float>(rcosResult * static_cast<double>(vecB[2]) * static_cast<double>(vecB[0]) - static_cast<double>(vecB[1]) * sinResult);
    matA[3] = 0.0f;
    matA[4] = static_cast<float>(rcosResult * static_cast<double>(vecB[1]) * static_cast<double>(vecB[0]) - static_cast<double>(vecB[2]) * sinResult);
    matA[5] = static_cast<float>(std::pow(static_cast<double>(vecB[1]), 2.0) * rcosResult + cosResult);
    matA[6] = static_cast<float>(static_cast<double>(vecB[0]) * sinResult + rcosResult * static_cast<double>(vecB[2]) * static_cast<double>(vecB[1]));
    matA[7] = 0.0f;
    matA[8] = static_cast<float>(rcosResult * static_cast<double>(vecB[2]) * static_cast<double>(vecB[0]) + static_cast<double>(vecB[1]) * sinResult);
    matA[9] = static_cast<float>(rcosResult * static_cast<double>(vecB[2]) * static_cast<double>(vecB[1]) - static_cast<double>(vecB[0]) * sinResult);
    matA[0xa] = static_cast<float>(std::pow(static_cast<double>(vecB[2]), 2.0) * rcosResult + cosResult);
    matA[0xb] = 0.0f;
    matA[0xc] = 0.0f;
    matA[0xd] = 0.0f;
    matA[0xe] = 0.0f;
    matA[0xf] = 1.0f;

    return matA;
}

SQUAREDMAGNITUDE p_squaredMagnitude = reinterpret_cast<SQUAREDMAGNITUDE>(0x4549f0);
SQUAREDMAGNITUDE p_original_squaredMagnitude = NULL;
double __fastcall detoured_squaredMagnitude(float* src) {
    return std::pow(static_cast<double>(src[0]), 2)
        + std::pow(static_cast<double>(src[1]), 2)
        + std::pow(static_cast<double>(src[2]), 2);
}

CALPLANENORMAL p_calculatePlaneNormal = reinterpret_cast<CALPLANENORMAL>(0x637480);
CALPLANENORMAL p_original_calculatePlaneNormal = NULL;
void __fastcall detoured_calculatePlaneNormal(float* self, void* ignored, float* p1, float* p2, float* p3) {
    double selfHD[4];
    selfHD[0] = (static_cast<double>(p2[1]) - static_cast<double>(p1[1])) * (static_cast<double>(p3[2]) - static_cast<double>(p1[2])) - (static_cast<double>(p2[2]) - static_cast<double>(p1[2])) * (static_cast<double>(p3[1]) - static_cast<double>(p1[1]));
    selfHD[1] = (static_cast<double>(p2[2]) - static_cast<double>(p1[2])) * (static_cast<double>(p3[0]) - static_cast<double>(p1[0])) - (static_cast<double>(p3[2]) - static_cast<double>(p1[2])) * (static_cast<double>(p2[0]) - static_cast<double>(p1[0]));
    selfHD[2] = (static_cast<double>(p3[1]) - static_cast<double>(p1[1])) * (static_cast<double>(p2[0]) - static_cast<double>(p1[0])) - (static_cast<double>(p2[1]) - static_cast<double>(p1[1])) * (static_cast<double>(p3[0]) - static_cast<double>(p1[0]));

    double sqrtResult = std::sqrt(std::pow(selfHD[2], 2.0) + std::pow(selfHD[1], 2.0) + std::pow(selfHD[0], 2.0));
    self[0] = static_cast<float>(selfHD[0] / sqrtResult);
    self[1] = static_cast<float>(selfHD[1] / sqrtResult);
    self[2] = static_cast<float>(selfHD[2] / sqrtResult);
    self[3] = static_cast<float>(-(selfHD[0] * static_cast<double>(p1[0]) + selfHD[1] * static_cast<double>(p1[1]) + selfHD[2] * static_cast<double>(p1[2])) / sqrtResult);
}

TRANSFORMAABOX p_transformAABox = reinterpret_cast<TRANSFORMAABOX>(0x6dc470);
TRANSFORMAABOX p_original_transformAABox = NULL;
void __fastcall detoured_transformAABox(float* C33Mat, float* C3Vec_A, float* C3Vec_B, float* CAAbox_A, float* CAAbox_B) {
    float* ptr[3];
    ptr[0] = C33Mat;
    ptr[1] = C3Vec_A;
    ptr[2] = C3Vec_B;
    
    for (uint32_t outer_i = 0; outer_i < 3; outer_i++, CAAbox_B++) {
        for (uint32_t inner_i = 0; inner_i < 3; inner_i++) {
            double test1 = static_cast<double>(ptr[inner_i][outer_i]) * static_cast<double>(CAAbox_A[inner_i]);
            double test2 = static_cast<double>(CAAbox_A[inner_i + 3]) * static_cast<double>(ptr[inner_i][outer_i]);
            if (test2 <= test1) {
                CAAbox_B[0] = static_cast<float>(test2 + static_cast<double>(CAAbox_B[0]));
            }
            else {
                CAAbox_B[0] = static_cast<float>(test1 + static_cast<double>(CAAbox_B[0]));
                test1 = test2;
            }
            CAAbox_B[3] = static_cast<float>(test1 + static_cast<double>(CAAbox_B[3]));
        }
    }

    return;
}

LUA_SQRT p_lua_sqrt = reinterpret_cast<LUA_SQRT>(0x7fb020);
LUA_SQRT p_original_lua_sqrt = NULL;
int __fastcall detoured_lua_sqrt(void* L) {
    double v = luaL_checknumber(L, 1);
    lua_pushnumber(L, std::sqrt(v));
    return 1;
}

static void blit_noPitch_1(int w, int h, uint32_t src, uint32_t dst) {
    if (w < 4) {
        w = 4;
    }
    if (h < 4) {
        h = 4;
    }
    uint32_t len = w * h;
    std::memcpy(reinterpret_cast<void*>(dst), reinterpret_cast<void*>(src), len);
}

static void blit_noPitch_2(int w, int h, uint32_t src, uint32_t dst) {
    if (w < 4) {
        w = 4;
    }
    if (h < 4) {
        h = 4;
    }
    uint32_t len = w * h * 4 / 8;
    std::memcpy(reinterpret_cast<void*>(dst), reinterpret_cast<void*>(src), len);
}

static void blit_withPitch(int w, int h, uint32_t src, uint32_t srcPitch, uint32_t dst, uint32_t dstPitch, int pixelSize) {
    if (srcPitch == dstPitch && w * pixelSize == srcPitch) {
        uint32_t len = w * h * pixelSize;
        std::memcpy(reinterpret_cast<void*>(dst), reinterpret_cast<void*>(src), len);
    }
    else {
        for (int y = 0; y < h; ++y) {
            uint32_t len = w * pixelSize;
            std::memcpy(reinterpret_cast<void*>(dst), reinterpret_cast<void*>(src), len);

            src += srcPitch;
            dst += dstPitch;
        }
    }
}


static std::map<std::tuple<int, int, int>, uint64_t> blitCounters{};

extern BLIT_HUB p_blit_hub = reinterpret_cast<BLIT_HUB>(0x5a4f60);
extern BLIT_HUB p_original_blit_hub = NULL;
void __fastcall detoured_blit_hub(int* vec2size, int unknownFuncIndex, uint32_t srcAddr, uint32_t srcStep, int srcFormat, uint32_t dstAddr, uint32_t dstStep, int dstFormat) {
    int* gameBlitInitialized = reinterpret_cast<int*>(0xc0f558);
    if (0 == *gameBlitInitialized) {
        typedef void(__fastcall* INITBLIT)();
        auto p_initBlit = reinterpret_cast<INITBLIT>(0x5a4fc0);
        p_initBlit();

        *gameBlitInitialized = 1;
    }

    if (unknownFuncIndex == 0) {
        if (srcFormat == 1 && dstFormat == 1) {
            blit_withPitch(vec2size[0], vec2size[1], srcAddr, srcStep, dstAddr, dstStep, 4);
            return;
        }
        if (srcFormat == 2 && dstFormat == 2) {
            blit_withPitch(vec2size[0], vec2size[1], srcAddr, srcStep, dstAddr, dstStep, 2);
            return;
        }
        if (srcFormat == 4 && dstFormat == 4) {
            blit_withPitch(vec2size[0], vec2size[1], srcAddr, srcStep, dstAddr, dstStep, 2);
            return;
        }
        if (srcFormat == 5 && dstFormat == 5) {
            blit_noPitch_2(vec2size[0], vec2size[1], srcAddr, dstAddr);
            return;
        }
        if (srcFormat == 6 && dstFormat == 6) {
            blit_noPitch_1(vec2size[0], vec2size[1], srcAddr, dstAddr);
            return;
        }
        if (srcFormat == 7 && dstFormat == 7) {
            blit_noPitch_1(vec2size[0], vec2size[1], srcAddr, dstAddr);
            return;
        }
    }

    auto op = std::make_tuple(unknownFuncIndex, srcFormat, dstFormat);
    auto i = blitCounters.find(op);
    if (i != blitCounters.end()) {
        i->second++;
    }
    else {
        blitCounters.insert({ op, 1 });
    }

    p_original_blit_hub(vec2size, unknownFuncIndex, srcAddr, srcStep, srcFormat, dstAddr, dstStep, dstFormat);
    return;
}

uint64_t polyfill_debugCounter = 0;
std::string getPolyfillDebug() {
    std::stringstream ss{};
    ss << "Enhanced REP MOVSB: " << ERMS << std::endl;
    //ss << "Polyfill debug counter: " << polyfill_debugCounter << std::endl;
    ss << "Unimplemented Blit history: " << std::endl;
    for (auto& i : blitCounters) {
        ss << "blit" << std::get<0>(i.first) << "(" << std::get<1>(i.first) << ", " << std::get<2>(i.first) << ") = " << i.second << std::endl;
    }
    return ss.str();
}
