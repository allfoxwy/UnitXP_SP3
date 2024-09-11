#include "pch.h"

#include <string>
#include <sstream>

#include "Vanilla1121_functions.h"

using namespace std;

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

	uint32_t obj0, obj1;

	obj0 = vanilla1121_getVisiableObject(guid0);
	if (obj0 == 0) {
		return -1;
	}

	obj1 = vanilla1121_getVisiableObject(guid1);
	if (obj1 == 0) {
		return -1;
	}

	if (vanilla1121_inLineOfSight(obj0, obj1)) {
		return 1;
	}
	else {
		return 0;
	}
}
