#pragma once

#include <string>
#include <cstdint>

#include "Vanilla1121_functions.h"

using namespace std;

// -1 for error
int UnitXP_inSight(const void* unit0, const void* unit1);
int UnitXP_inSight(const uint64_t guid0, const uint64_t guid1);
int UnitXP_inSight(const string unit0, const string unit1);
int camera_inSight(const void* unit);

// Return true if position is in camera viewing frustum without checking line of sight. When checkCone is 2.0f, the cone is same as game FoV
bool inViewingFrustum(const C3Vector& posObject, float checkCone);

// Check if I am behind a mob, -1 for error
int UnitXP_behind(const void* me, const void* mob);
int UnitXP_behind(const uint64_t guidMe, const uint64_t guidMob);
int UnitXP_behind(const string me, const string mob);

extern float behind_threshold;
