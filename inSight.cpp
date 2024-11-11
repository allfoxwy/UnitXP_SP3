#include "pch.h"

#include <string>
#include <sstream>
#include <cmath>

#include "Vanilla1121_functions.h"
#include "distanceBetween.h"

using namespace std;


// Return true if position is in camera viewing frustum without checking line of sight. When checkCone is 2.0f, the cone is same as game FoV
bool inViewingFrustum(C3Vector posObject, float checkCone) {
	C3Vector posCamera = vanilla1121_getCameraPosition();
	C3Vector posPlayer = vanilla1121_getUnitPosition(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_inLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	float testDistance0 = UnitXP_distanceBetween(posCamera, posPlayer);
	float testDistance1 = UnitXP_distanceBetween(posCamera, posObject);
	if (testDistance0 > guardAgainstTransportsCoordinates || testDistance1 > guardAgainstTransportsCoordinates) {
		return false;
	}

	C3Vector vecObject;
	vecObject.x = posObject.x - posCamera.x;
	vecObject.y = posObject.y - posCamera.y;
	vecObject.z = posObject.z - posCamera.z;

	C3Vector vecPlayer;
	vecPlayer.x = posPlayer.x - posCamera.x;
	vecPlayer.y = posPlayer.y - posCamera.y;
	vecPlayer.z = posPlayer.z - posCamera.z;

	float dotProduct = vecObject.x * vecPlayer.x + vecObject.y * vecPlayer.y + vecObject.z * vecPlayer.z;
	float lenVecObject = hypot(vecObject.x, vecObject.y, vecObject.z);
	float lenVecPlayer = hypot(vecPlayer.x, vecPlayer.y, vecPlayer.z);

	// I tested in game and find out that even Vanilla Tweaks change this value, the screen border of objects still follow original FoV somehow
	// I suspect game has additional transformation before Direct X FoV, or Vanilla Tweaks did on a wrong address...
	//float fov = vanilla1121_getCameraFoV();
	const float fov = 1.5708f;

	float angleBetweenPlayerAndObject = acos(dotProduct / (lenVecObject * lenVecPlayer));

	if (angleBetweenPlayerAndObject > fov / checkCone) {
		return false;
	}
	else {
		return true;
	}
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
// This function is using void* to prevent implicit conversion from uint32_t to uint64_t
int camera_inSight(void* obj) {
	if (!obj) {
		return -1;
	}

	C3Vector pos0 = vanilla1121_getCameraPosition();
	C3Vector pos1 = vanilla1121_getUnitPosition(reinterpret_cast<uint32_t>(obj));

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_inLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	float testDistance = UnitXP_distanceBetween(pos0, pos1);
	if (testDistance > guardAgainstTransportsCoordinates) {
		return -1;
	}

	if (inViewingFrustum(pos1, 2.0f) == false) {
		return 0;
	}

	//TODO: I can't find height of object
	pos1.z += 2.4f;

	C3Vector intersectPoint = { 0,0,0 };
	float distance = 1.0f;

	bool result = CWorld_Intersect(&pos0, &pos1, 0, &intersectPoint, &distance);

	if (result) {
		if (distance <= 1 && distance >= 0) {
			// intersect between points, loss sight
			return 0;
		}
		else {
			// intersect after points, still in sight
			return 1;
		}
	}
	else {
		// no intersect, in sight
		return 1;
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
