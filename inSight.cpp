#include "pch.h"

#include <string>
#include <sstream>

#include "Vanilla1121_functions.h"
#include "distanceBetween.h"

using namespace std;

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
int UnitXP_inSight(uint64_t guid0, uint64_t guid1) {
	uint32_t obj0, obj1;

	obj0 = vanilla1121_getVisiableObject(guid0);
	if (obj0 == 0) {
		return -1;
	}

	obj1 = vanilla1121_getVisiableObject(guid1);
	if (obj1 == 0) {
		return -1;
	}

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_inLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	float distance = UnitXP_distanceBetween(guid0, guid1);
	if (distance > 150.0f) {
		return -1;
	}


	if (vanilla1121_inLineOfSight(obj0, obj1)) {
		return 1;
	}
	else {
		return 0;
	}
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
int UnitXP_inSight(string unit0, string unit1) {
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

	return UnitXP_inSight(guid0, guid1);
}
