#pragma once

#include <string>
#include <cstdint>

#include "Vanilla1121_functions.h"

using namespace std;

int UnitXP_inSight(void * unit0, void * unit1);
int UnitXP_inSight(uint64_t guid0, uint64_t guid1);
int UnitXP_inSight(string unit0, string unit1);
int camera_inSight(void * unit);

// Return true if position is in camera viewing frustum without checking line of sight. When checkCone is 2.0f, the cone is same as game FoV
bool inViewingFrustum(const C3Vector& posObject, float checkCone);


int UnitXP_inFrontOfPlayer(const C3Vector& pos);
int UnitXP_inFrontOfPlayer(uint64_t guid);
int UnitXP_inFrontOfPlayer(string unit);

