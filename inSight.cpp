#include "pch.h"

#include <string>
#include <sstream>
#include <cmath>

#include "Vanilla1121_functions.h"
#include "distanceBetween.h"

using namespace std;

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
// This function is using void* to prevent implicit conversion from uint32_t to uint64_t
int camera_inSight(void* obj) {
	if (!obj) {
		return -1;
	}

	C3Vector pos0 = vanilla1121_getCameraPosition();
	C3Vector pos1 = vanilla1121_getObjectPosition(reinterpret_cast<uint32_t>(obj));

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_inLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	float testDistance = UnitXP_distanceBetween(pos0, pos1);
	if (testDistance > guardAgainstTransportsCoordinates) {
		return -1;
	}

	//TODO: I can't find height of object
	pos1.z += 2.4f;

	C3Vector intersectPoint = { 0,0,0 };
	float distance = 1.0f;

	bool result = CWorld_Intersect(&pos0, &pos1, 0, &intersectPoint, &distance, 0x100171);

	if (result) {
		if (distance <= 1 && distance >= 0) {
			// intersect between points, loss sight
			return false;
		}
		else {
			// intersect after points, still in sight
			return true;
		}
	}
	else {
		// no intersect, in sight
		return true;
	}
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
// This function is using void* to prevent implicit conversion from uint32_t to uint64_t
int UnitXP_inSight(void* obj0, void* obj1) {
	if (!obj0 || !obj1) {
		return -1;
	}

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_inLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	float distance = UnitXP_distanceBetween(obj0, obj1);
	if (distance > guardAgainstTransportsCoordinates) {
		return -1;
	}


	if (vanilla1121_inLineOfSight(
		reinterpret_cast<uint32_t>(obj0),
		reinterpret_cast<uint32_t>(obj1)
	)) {
		return 1;
	}
	else {
		return 0;
	}
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
int UnitXP_inSight(uint64_t guid0, uint64_t guid1) {
	return UnitXP_inSight(
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid0)),
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid1))
	);
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
