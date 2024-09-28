#pragma once

#include <string>
#include <cstdint>

using namespace std;


float UnitXP_distanceBetween(C3Vector& pos0, C3Vector& pos1);
float UnitXP_distanceBetween(void* obj0, void* obj1);
float UnitXP_distanceBetween(uint64_t guid0, uint64_t guid1);
float UnitXP_distanceBetween(string unit0, string unit1);
