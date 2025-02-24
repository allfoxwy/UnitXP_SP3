#include "pch.h"

#define _USE_MATH_DEFINES

#include <string>
#include <sstream>
#include <unordered_map>
#include <map>
#include <random>
#include <profileapi.h>

#include "inSight.h"
#include "distanceBetween.h"
#include "performanceProfiling.h"

using namespace std;

// We implement 2 cache to workaround issue #10
// Idea is to call CWorld_Intersect less
// unitSightCache should be unordered_map too, however I don't know how to implement a hash function for std::pair
static unordered_map<uint64_t, pair<LARGE_INTEGER, int>> cameraSightCache{};
static map<pair<uint64_t, uint64_t>, pair<LARGE_INTEGER, int>> unitSightCache{};
static LARGE_INTEGER cacheTTL = {};
static random_device rDev{};
static mt19937 rnd(rDev());
static uniform_int_distribution<mt19937::result_type> rDist(0, 60);
static LARGE_INTEGER ditcherUnit = {};


// Use a dither to make cache expires more fluently
static LARGE_INTEGER cacheDither() {
	if (ditcherUnit.QuadPart == 0) {
		ditcherUnit.QuadPart = getPerformanceCounterFrequency().QuadPart / 1000;
	}

	LARGE_INTEGER result = {};
	result.QuadPart = ditcherUnit.QuadPart * static_cast<int64_t>(rDist(rnd));

	return result;
}

static void cameraCacheHousekeeping() {
	if (cacheTTL.QuadPart == 0) {
		// According to https://bionumbers.hms.harvard.edu/bionumber.aspx?s=n&v=4&id=110800
		// "mean auditory reaction times 140-160msec: touch 155msec: visual reaction times 180-200msec msec"
		// We are going to have 100msec + 60msec dither
		cacheTTL.QuadPart = getPerformanceCounterFrequency().QuadPart / 10;
	}

	static int delayCounter = 0;
	if (++delayCounter < 100) {
		return;
	}
	else {
		delayCounter = 0;
	}

	LARGE_INTEGER now = {};
	LARGE_INTEGER delta = {};
	QueryPerformanceCounter(&now);

	for (auto it = cameraSightCache.begin(); it != cameraSightCache.end(); ++it) {
		delta.QuadPart = now.QuadPart - it->second.first.QuadPart;
		if (delta.QuadPart > cacheTTL.QuadPart) {
			cameraSightCache.erase(it);
		}
	}
}

static void unitCacheHousekeeping() {
	if (cacheTTL.QuadPart == 0) {
		// According to https://bionumbers.hms.harvard.edu/bionumber.aspx?s=n&v=4&id=110800
		// "mean auditory reaction times 140-160msec: touch 155msec: visual reaction times 180-200msec msec"
		// We are going to have 100msec + 60msec dither
		cacheTTL.QuadPart = getPerformanceCounterFrequency().QuadPart / 10;
	}

	static int delayCounter = 0;
	if (++delayCounter < 100) {
		return;
	}
	else {
		delayCounter = 0;
	}

	LARGE_INTEGER now = {};
	LARGE_INTEGER delta = {};
	QueryPerformanceCounter(&now);

	for (auto it = unitSightCache.begin(); it != unitSightCache.end(); ++it) {
		delta.QuadPart = now.QuadPart - it->second.first.QuadPart;
		if (delta.QuadPart > cacheTTL.QuadPart) {
			unitSightCache.erase(it);
		}
	}
}

// Return true if position is in camera viewing frustum without checking line of sight. When checkCone is 2.0f, the cone is same as game FoV
bool inViewingFrustum(const C3Vector& posObject, float checkCone) {
	uint64_t playerGUID = UnitGUID("player");
	if (playerGUID == 0) {
		return false;
	}

	uint32_t player = vanilla1121_getVisiableObject(playerGUID);
	if (player == 0) {
		return false;
	}

	if (vanilla1121_objectType(player) != OBJECT_TYPE_Player && vanilla1121_objectType(player) != OBJECT_TYPE_Unit) {
		return false;
	}

	C3Vector posPlayer = vanilla1121_unitPosition(player);
	C3Vector posCamera = vanilla1121_getCameraPosition();

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_unitInLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	float testDistance0 = UnitXP_distanceBetween(posCamera, posPlayer);
	float testDistance1 = UnitXP_distanceBetween(posCamera, posObject);
	if (testDistance0 > guardAgainstTransportsCoordinates || testDistance1 > guardAgainstTransportsCoordinates) {
		return false;
	}

	C3Vector vecObject = {};
	vecObject.x = posObject.x - posCamera.x;
	vecObject.y = posObject.y - posCamera.y;
	vecObject.z = posObject.z - posCamera.z;

	C3Vector vecPlayer = {};
	vecPlayer.x = posPlayer.x - posCamera.x;
	vecPlayer.y = posPlayer.y - posCamera.y;
	vecPlayer.z = posPlayer.z - posCamera.z;

	// I tested in game and find out that even Vanilla Tweaks change this value, the screen border of objects still follow original FoV somehow
	// I suspect game has additional transformation before Direct X FoV
	//float fov = vanilla1121_getCameraFoV();
	const float fov = 1.5708f;

	float angle = angleBetweenVectors(vecObject, vecPlayer);

	if (angle > fov / checkCone) {
		return false;
	}
	else {
		return true;
	}
}

int UnitXP_behind(const void* mevoid, const void* mobvoid) {
	if (!mevoid || !mobvoid) {
		return -1;
	}

	uint32_t unitMe = reinterpret_cast<uint32_t>(mevoid);
	uint32_t unitMob = reinterpret_cast<uint32_t>(mobvoid);

	if ((unitMe & 1) != 0 || (unitMob & 1) != 0) {
		return -1;
	}

	if (vanilla1121_objectType(unitMe) != OBJECT_TYPE_Unit &&
		vanilla1121_objectType(unitMe) != OBJECT_TYPE_Player) {
		return -1;
	}
	if (vanilla1121_objectType(unitMob) != OBJECT_TYPE_Unit &&
		vanilla1121_objectType(unitMob) != OBJECT_TYPE_Player) {
		return -1;
	}

	C3Vector posMe = vanilla1121_unitPosition(unitMe);
	C3Vector posMob = vanilla1121_unitPosition(unitMob);

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_unitInLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	if (UnitXP_distanceBetween(posMe, posMob) > guardAgainstTransportsCoordinates) {
		return -1;
	}

	float facing = vanilla1121_unitFacing(unitMob);

	C3Vector vecLeft = {};
	vecLeft.x = -std::sin(facing);
	vecLeft.y = std::cos(facing);

	C3Vector vecForward = {};
	vecForward.x = vecLeft.x * std::cos(static_cast<float>(-M_PI_2)) - vecLeft.y * std::sin(static_cast<float>(-M_PI_2));
	vecForward.y = vecLeft.x * std::sin(static_cast<float>(-M_PI_2)) + vecLeft.y * std::cos(static_cast<float>(-M_PI_2));

	C3Vector vecCheck = {};
	vecCheck.x = posMe.x - posMob.x;
	vecCheck.y = posMe.y - posMob.y;

	float angle = angleBetweenVectors(vecForward, vecCheck);

	if (angle > 2 * M_PI) {
		return -1;
	}

	if (angle < M_PI_2) {
		return 0;
	}
	else {
		return 1;
	}
}

int UnitXP_behind(const uint64_t guidMe, const uint64_t guidMob) {
	if (guidMe == 0 || guidMob == 0) {
		return -1;
	}
	return UnitXP_behind(
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guidMe)),
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guidMob)));
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
int UnitXP_behind(const string unit0, const string unit1) {
	uint64_t guid0 = 0, guid1 = 0;

	if (unit0.empty() || unit1.empty()) {
		return -1;
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

	return UnitXP_behind(guid0, guid1);
}


static int test_camera_inSight(const uint32_t unit) {
	C3Vector pos0 = vanilla1121_getCameraPosition();
	C3Vector pos1 = vanilla1121_unitPosition(unit);

	if (inViewingFrustum(pos1, 2.0f) == false) {
		return 0;
	}

	//TODO: I can't find height of object
	pos1.z += 2.4f;

	C3Vector intersectPoint = {};
	float distance = 1.0f;

	bool result = CWorld_Intersect(&pos0, &pos1, &intersectPoint, &distance);

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
int camera_inSight(const void* unitvoid) {
	if (!unitvoid) {
		return -1;
	}

	uint32_t unit = reinterpret_cast<uint32_t>(unitvoid);

	if ((unit & 1) != 0) {
		return -1;
	}

	if (vanilla1121_objectType(unit) != OBJECT_TYPE_Player && vanilla1121_objectType(unit) != OBJECT_TYPE_Unit) {
		return -1;
	}

	cameraCacheHousekeeping();

	uint64_t guid = *reinterpret_cast<uint64_t*>(unit + 0x30);

	LARGE_INTEGER now = {};
	QueryPerformanceCounter(&now);

	auto it = cameraSightCache.find(guid);
	if (it != cameraSightCache.end()) {
		LARGE_INTEGER delta = {};
		delta.QuadPart = now.QuadPart - it->second.first.QuadPart;

		if (delta.QuadPart < cacheTTL.QuadPart) {
			return it->second.second;
		}
	}

	int result = test_camera_inSight(unit);

	QueryPerformanceCounter(&now);
	now.QuadPart += cacheDither().QuadPart;
	cameraSightCache[guid] = make_pair(now, result);

	return result;
}

int camera_inSight(uint64_t guid) {
	if (guid == 0) {
		return -1;
	}
	return camera_inSight(reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid)));
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
// This function is using void* to prevent implicit conversion from uint32_t to uint64_t
int UnitXP_inSight(const void* unit0void, const void* unit1void) {
	if (!unit0void || !unit1void) {
		return -1;
	}

	uint32_t unit0 = reinterpret_cast<uint32_t>(unit0void);
	uint32_t unit1 = reinterpret_cast<uint32_t>(unit1void);
	
	if ((unit0 & 1) != 0 || (unit1 & 1) != 0) {
		return -1;
	}

	if (vanilla1121_objectType(unit0) != OBJECT_TYPE_Unit &&
		vanilla1121_objectType(unit0) != OBJECT_TYPE_Player) {
		return -1;
	}
	if (vanilla1121_objectType(unit1) != OBJECT_TYPE_Unit &&
		vanilla1121_objectType(unit1) != OBJECT_TYPE_Player) {
		return -1;
	}

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_unitInLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	C3Vector posUnit0 = vanilla1121_unitPosition(unit0);
	C3Vector posUnit1 = vanilla1121_unitPosition(unit1);
	float distance = UnitXP_distanceBetween(posUnit0, posUnit1);
	if (distance > guardAgainstTransportsCoordinates) {
		return -1;
	}

	unitCacheHousekeeping();

	uint64_t guid0 = *reinterpret_cast<uint64_t*>(unit0 + 0x30);
	uint64_t guid1 = *reinterpret_cast<uint64_t*>(unit1 + 0x30);

	LARGE_INTEGER now = {};
	QueryPerformanceCounter(&now);

	auto it = unitSightCache.find(make_pair(guid0, guid1));
	if (it != unitSightCache.end()) {
		LARGE_INTEGER delta = {};
		delta.QuadPart = now.QuadPart - it->second.first.QuadPart;

		if (delta.QuadPart < cacheTTL.QuadPart) {
			return it->second.second;
		}
	}

	int result = vanilla1121_unitInLineOfSight(unit0, unit1);

	QueryPerformanceCounter(&now);
	now.QuadPart += cacheDither().QuadPart;
	unitSightCache[make_pair(guid0, guid1)] = make_pair(now, result);
	unitSightCache[make_pair(guid1, guid0)] = make_pair(now, result);

	return result;
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
int UnitXP_inSight(const uint64_t guid0, const uint64_t guid1) {
	if (guid0 == 0 || guid1 == 0) {
		return -1;
	}
	return UnitXP_inSight(
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid0)),
		reinterpret_cast<void*>(vanilla1121_getVisiableObject(guid1)));
}

// return 0 for "not in sight"; 1 for "in sight"; -1 for error
int UnitXP_inSight(const string unit0, const string unit1) {
	uint64_t guid0 = 0, guid1 = 0;

	if (unit0.empty() || unit1.empty()) {
		return -1;
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

	if (unit0.find(u8"camera") != unit0.npos) {
		return camera_inSight(guid1);
	}

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

	return UnitXP_inSight(guid0, guid1);
}
