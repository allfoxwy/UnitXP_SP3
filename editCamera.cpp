#include "pch.h"

#include "editCamera.h"
#include "Vanilla1121_functions.h"
#include "distanceBetween.h"
#include "inSight.h"
#include "performanceProfiling.h"

#include <iostream>
#include <fstream>

CGCAMERA_UPDATECALLBACK_0x511bc0 p_CGCamera_updateCallback_0x511bc0 = reinterpret_cast<CGCAMERA_UPDATECALLBACK_0x511bc0>(0x511bc0);
CGCAMERA_UPDATECALLBACK_0x511bc0 p_original_CGCamera_updateCallback_0x511bc0 = NULL;

float cameraHorizontalAddend = 0.0f;
float cameraVerticalAddend = 0.0f;
bool cameraFollowTarget = false;
bool cameraFixedPosition = false;


static C3Vector cameraOriginalPosition = {};
static C3Vector cameraTranslatedPosition = {};

C3Vector editCamera_originalPosition() {
    if (cameraOriginalPosition.x == 0.0f && cameraOriginalPosition.y == 0.0f && cameraOriginalPosition.z == 0.0f) {
        return vanilla1121_getCameraPosition();
    }
    return cameraOriginalPosition;
}

C3Vector editCamera_translatedPosition() {
    if (cameraTranslatedPosition.x == 0.0f && cameraTranslatedPosition.y == 0.0f && cameraTranslatedPosition.z == 0.0f) {
        return vanilla1121_getCameraPosition();
    }
    return cameraTranslatedPosition;
}

static void cameraFollowPosition(const uint32_t camera, const C3Vector& targetPosition) {
    float* cameraPositionPtr = reinterpret_cast<float*>(camera + 0x8);
    C3Vector cameraPosition = {};
    cameraPosition.x = cameraPositionPtr[0];
    cameraPosition.y = cameraPositionPtr[1];
    cameraPosition.z = cameraPositionPtr[2];

    // When player jump onto transports (boat/zeppelin) their coordinates system would change.
    // If we pass coordinates from different system into vanilla1121_unitInLineOfSight(), game crashes
    // TODO: I don't have a way to find out what the current system is
    // To workaround, we test the distance. If they are too far away, we judge that situation as error
    if (UnitXP_distanceBetween(cameraPosition, targetPosition) > guardAgainstTransportsCoordinates) {
        return;
    }

    // Fundamental method of calculating look-at matrix is from https://medium.com/@carmencincotti/lets-look-at-magic-lookat-matrices-c77e53ebdf78
    // However the WoW internal data structures are not the same.

    C3Vector vecForward = {};
    vecForward.x = -(cameraPosition.x - targetPosition.x);
    vecForward.y = -(cameraPosition.y - targetPosition.y);
    vecForward.z = -(cameraPosition.z - targetPosition.z);
    vectorNormalize(vecForward);
    if (std::abs(vecForward.x) < 0.01f && std::abs(vecForward.y) < 0.01f) {
        return;
    }

    C3Vector vecTemp = {};
    vecTemp.x = 0.0f;
    vecTemp.y = 0.0f;
    vecTemp.z = 1.0f;

    C3Vector vecRight = vectorCrossProduct(vecTemp, vecForward);
    vectorNormalize(vecRight);

    C3Vector vecUp = vectorCrossProduct(vecForward, vecRight);
    vectorNormalize(vecUp);

    float* matCamera = reinterpret_cast<float*>(camera + 0x14);

    matCamera[0] = vecForward.x;
    matCamera[1] = vecForward.y;
    matCamera[2] = vecForward.z;
    matCamera[3] = vecRight.x;
    matCamera[4] = vecRight.y;
    matCamera[5] = vecRight.z;
    matCamera[6] = vecUp.x;
    matCamera[7] = vecUp.y;
    matCamera[8] = vecUp.z;
}

static C3Vector cameraTranslate(uint32_t camera, const C3Vector& a, const C3Vector& b, float horizontalDelta, float verticalDelta) {
    C3Vector result = {};
    result.x = a.x;
    result.y = a.y;
    result.z = a.z;

    C3Vector vecTemp = {};
    vecTemp.x = b.x - a.x;
    vecTemp.y = b.y - a.y;
    vecTemp.z = 0.0f;

    float temp = vectorLength(vecTemp);

    if (temp < 0.5f) {
        // Don't translate for first person camera
        return result;
    }

    if (cameraFixedPosition) {
        C3Vector playerEyePosition = {};
        // x = 0x174 and y = 0x178 are the same as the unit position
        playerEyePosition.x = b.x;
        playerEyePosition.y = b.y;
        // Will crash if done in first person
        playerEyePosition.z = *reinterpret_cast<float*>(camera + 0x17C);

        // Get the vector to the camera from the players current eye position
        C3Vector difference = {};
        difference.x = a.x - playerEyePosition.x;
        difference.y = a.y - playerEyePosition.y;
        difference.z = a.z - playerEyePosition.z;

        // Add vector from a fixed eye position to get our fixed camera position
        float height = 2.151306f;
        result.x = playerEyePosition.x + difference.x;
        result.y = playerEyePosition.y + difference.y;
        result.z = b.z + height + cameraVerticalAddend + difference.z;
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
    int result = p_original_CGCamera_updateCallback_0x511bc0(unknown1, camera);

    if (camera > 0 && (camera & 1) == 0) {
        float* editPtr = reinterpret_cast<float*>(camera + 0x8);

        cameraOriginalPosition.x = editPtr[0];
        cameraOriginalPosition.y = editPtr[1];
        cameraOriginalPosition.z = editPtr[2];

        uint32_t u = vanilla1121_getVisiableObject(*reinterpret_cast<uint64_t*>(camera + 0x88));
        if (u > 0 &&
            (vanilla1121_objectType(u) == OBJECT_TYPE_Player || vanilla1121_objectType(u) == OBJECT_TYPE_Unit)) {
            C3Vector playerPosition = vanilla1121_unitPosition(u);
            float height = vanilla1121_unitHeight(u);

            cameraTranslatedPosition = cameraTranslate(camera, cameraOriginalPosition, playerPosition, cameraHorizontalAddend, cameraVerticalAddend);

            editPtr[0] = cameraTranslatedPosition.x;
            editPtr[1] = cameraTranslatedPosition.y;
            editPtr[2] = cameraTranslatedPosition.z;

            if (cameraFollowTarget) {
                uint64_t targetGUID = UnitGUID("target");
                if (targetGUID > 0) {
                    uint32_t t = vanilla1121_getVisiableObject(targetGUID);
                    if (t > 0 &&
                        ((vanilla1121_objectType(t) == OBJECT_TYPE_Player && vanilla1121_unitCanBeAttacked(t) == 0) || (vanilla1121_objectType(t) == OBJECT_TYPE_Unit && vanilla1121_unitIsControlledByPlayer(t) == 0))) {
                        C3Vector targetPosition = vanilla1121_unitPosition(t);
                        targetPosition.z += vanilla1121_unitHeight(t);

                        if (UnitXP_distanceBetween("player", "target", METER_RANGED) < 50.0f
                            && UnitXP_inSight("player", "target")) {
                            cameraFollowPosition(camera, targetPosition);
                        }
                    }
                }
            }
        }
    }
    return result;
}

