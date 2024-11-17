#pragma once

#include <string>
#include <cstdint>

using namespace std;

int UnitXP_inSight(void * unit0, void * unit1);
int UnitXP_inSight(uint64_t guid0, uint64_t guid1);
int UnitXP_inSight(string unit0, string unit1);
int camera_inSight(void * unit);

// Return true if position is in camera viewing frustum without checking line of sight. When checkCone is 2.0f, the cone is same as game FoV
bool inViewingFrustum(C3Vector posObject, float checkCone);
