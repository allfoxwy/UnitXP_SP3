#pragma once

#include <string>
#include <cstdint>

#include "Vanilla1121_functions.h"

using namespace std;

enum distanceMeters {
    METER_GAUSSIAN,			// Raw distance. Calculations like camera frustum should be using this meter
    METER_RANGED,			// Ranged, Targeted spells. Like bolts, heals and charge.
    METER_MELEE_AUTOATTACK,	// Melee auto attack. Note this meter isn`t exactly fit into melee spells because of ignoring Z-axis. We could find a spot where mobs could melee us but we out of taunt range
    METER_AOE,				// AoE spells. Like novas and whirlwind.
    METER_CHAINS,			// To tell if spell would chain from one to another. Like cleave, multishot. Vmangos CHAIN_SPELL_JUMP_RADIUS is 10
};

float UnitXP_distanceBetween(const C3Vector& pos0, const C3Vector& pos1);
float UnitXP_distanceBetween(const void* unit0, const void* unit1, distanceMeters meter = METER_GAUSSIAN);
float UnitXP_distanceBetween(const uint64_t guid0, const uint64_t guid1, distanceMeters meter = METER_GAUSSIAN);
float UnitXP_distanceBetween(const string unit0, const string unit1, distanceMeters meter = METER_GAUSSIAN);
