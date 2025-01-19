#pragma once

#include <string>
#include <cstdint>

using namespace std;


float UnitXP_distanceBetween(const C3Vector& pos0, const C3Vector& pos1);
float UnitXP_distanceBetween(void * unit0, void * unit1);
float UnitXP_distanceBetween(uint64_t guid0, uint64_t guid1);
float UnitXP_distanceBetween(string unit0, string unit1);
