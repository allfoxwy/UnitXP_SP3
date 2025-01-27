#include "pch.h"

#include <cmath>

#include "editCamera.h"
#include "Vanilla1121_functions.h"
#include "distanceBetween.h"
#include "performanceProfiling.h"

extern CGCAMERA_UPDATECALLBACK_0x511bc0 p_CGCamera_updateCallback_0x511bc0 = reinterpret_cast<CGCAMERA_UPDATECALLBACK_0x511bc0>(0x511bc0);
extern CGCAMERA_UPDATECALLBACK_0x511bc0 p_original_CGCamera_updateCallback_0x511bc0 = NULL;

extern float cameraHorizontalAddend = 0.0f;
extern float cameraVerticalAddend = 0.0f;

static C3Vector cameraTranslate(const C3Vector& a, const C3Vector& b, float horizontalDelta, float verticalDelta) {
	C3Vector result = {};
	result.x = a.x;
	result.y = a.y;
	result.z = a.z;

	float temp = std::hypot(b.x - a.x, b.y - a.y);

	if (temp < 0.5f) {
		// Don't translate for first person camera
		return result;
	}

	// When player jump onto transports (boat/zeppelin) their coordinates system would change.
	// If we pass coordinates from different system into vanilla1121_unitInLineOfSight(), game crashes
	// TODO: I don't have a way to find out what the current system is
	// To workaround, we test the distance. If they are too far away, we judge that situation as error
	if (UnitXP_distanceBetween(a, b) > guardAgainstTransportsCoordinates) {
		return result;
	}

	bool needCollisionCheck = false;

	if (std::abs(horizontalDelta) > 0.1f) {
		needCollisionCheck = true;
		if (horizontalDelta > 0.0f) {
			// Translate toward right
			result.x = std::abs(horizontalDelta) * (b.y - a.y) / temp + a.x;
			result.y = std::abs(horizontalDelta) * (a.x - b.x) / temp + a.y;
		}
		else {
			// Translate toward left
			result.x = std::abs(horizontalDelta) * (a.y - b.y) / temp + a.x;
			result.y = std::abs(horizontalDelta) * (b.x - a.x) / temp + a.y;
		}
	}

	if (std::abs(verticalDelta) > 0.1f) {
		needCollisionCheck = true;
		result.z += verticalDelta;
	}

	if (needCollisionCheck) {
		C3Vector intersectPoint = {};
		float distance = 1.0f;

		bool collide = CWorld_Intersect(&a, &result, &intersectPoint, &distance);

		if (collide) {
			if (distance <= 1 && distance >= 0) {
				// intersect between points, collide

				// 0.7f is a small fix to make collide distance less accurate,
				// so that camera would not flash when collide on walls
				result.x = 0.7f * (intersectPoint.x - a.x) + a.x;
				result.y = 0.7f * (intersectPoint.y - a.y) + a.y;
				result.z = 0.7f * (intersectPoint.z - a.z) + a.z;

			}
		}
	}

	return result;
}

int __fastcall detoured_CGCamera_updateCallback_0x511bc0(void* unknown1, uint32_t camera) {
	perfSetSlotName(5, "oringinal camera update callback");
	perfMarkStart(5);
	int result = p_original_CGCamera_updateCallback_0x511bc0(unknown1, camera);
	perfMarkEnd(5);

	perfSetSlotName(6, "camera translate");
	perfMarkStart(6);
	uint32_t u = vanilla1121_getVisiableObject(*reinterpret_cast<uint64_t*>(camera + 0x88));
	if (u > 0 &&
		(vanilla1121_objectType(u) == OBJECT_TYPE_Player || vanilla1121_objectType(u) == OBJECT_TYPE_Unit)) {

		C3Vector targetPosition = vanilla1121_unitPosition(u);

		float* editPtr = reinterpret_cast<float*>(camera + 0x8);

		C3Vector cameraPosition = {};
		cameraPosition.x = editPtr[0];
		cameraPosition.y = editPtr[1];
		cameraPosition.z = editPtr[2];

		C3Vector translated = cameraTranslate(cameraPosition, targetPosition, cameraHorizontalAddend, cameraVerticalAddend);

		editPtr[0] = translated.x;
		editPtr[1] = translated.y;
		editPtr[2] = translated.z;
	}
	perfMarkEnd(6);
	
	return result;
}

