#include "pch.h"

#include <string>
#include <sstream>
#include <cmath>

#include "Vanilla1121_functions.h"

using namespace std;

// return -1 for error
// This function is using void* to prevent implicit conversion from uint32_t to uint64_t
float UnitXP_distanceBetween(void* obj0, void* obj1) {
	if (!obj0 || !obj1) {
		return -1;
	}

	C3Vector pos0 = vanilla1121_getObjectPosition(reinterpret_cast<uint32_t>(obj0));
	C3Vector pos1 = vanilla1121_getObjectPosition(reinterpret_cast<uint32_t>(obj1));

	return hypot(pos0.x - pos1.x, pos0.y - pos1.y, pos0.z - pos1.z);
}

// return -1 for error
float UnitXP_distanceBetween(uint64_t guid0, uint64_t guid1) {
	return UnitXP_distanceBetween(
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid0)),
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid1))
	);
}

// return -1 for error
float UnitXP_distanceBetween(string unit0, string unit1) {
	uint64_t guid0, guid1;

	if (unit0.find(u8"0x") != unit0.npos) {
		stringstream ss{ unit0 };
		ss >> hex >> guid0;
		if (ss.fail()) {
			return -1;
		}
	}
	else {
		guid0 = UnitGUID(unit0.data());
		if (guid0 == 0) {
			return -1;
		}
	}

	if (unit1.find(u8"0x") != unit1.npos) {
		stringstream ss{ unit1 };
		ss >> hex >> guid1;
		if (ss.fail()) {
			return -1;
		}
	}
	else {
		guid1 = UnitGUID(unit1.data());
		if (guid1 == 0) {
			return -1;
		}
	}

	return UnitXP_distanceBetween(guid0, guid1);
}
