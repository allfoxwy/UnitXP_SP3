#include "pch.h"

#include <algorithm>
#include <cmath>

#include "editCamera.h"
#include "Vanilla1121_functions.h"
#include "distanceBetween.h"
#include "inSight.h"
#include "performanceProfiling.h"

CGCAMERA_UPDATECALLBACK_0x511bc0 p_CGCamera_updateCallback_0x511bc0 = reinterpret_cast<CGCAMERA_UPDATECALLBACK_0x511bc0>(0x511bc0);
CGCAMERA_UPDATECALLBACK_0x511bc0 p_original_CGCamera_updateCallback_0x511bc0 = NULL;

ORGANICSMOOTH_0x5b7bb0 p_OrganicSmooth = reinterpret_cast<ORGANICSMOOTH_0x5b7bb0>(0x5b7bb0);
ORGANICSMOOTH_0x5b7bb0 p_original_OrganicSmooth = NULL;

float cameraHorizontalAddend = 0.0f;
float cameraVerticalAddend = 0.0f;
float cameraPitchAddend = 0.0f;
bool cameraFollowTarget = false;
bool cameraOrganicSmooth = true;
bool cameraPinHeight = false;

static const float keepDistanceFromWall = 0.2f;
static C3Vector cameraOriginalPosition = {};
static C3Vector cameraTranslatedPosition = {};
static C3Vector cameraOriginalForwardVector = {};
static C3Vector cameraRotatedForwardVector = {};

static C3Vector lastCameraTranslatedPosition = {};
static float lastCameraPitchAddend = 0.0f;
static C3Vector lastVerticalNearClipCorrection = {};
static C3Vector lastHorizontalNearClipCorrection = {};
static C3Vector lastVerticalCameraCollisionCorrection = {};
static C3Vector lastHorizontalCameraCollisionCorrection = {};
static LARGE_INTEGER lastCameraCollisionTime = {};
static LARGE_INTEGER cameraCollisionRefreshInterval = {};

void editCamera_init() {
    lastCameraCollisionTime.QuadPart = 0;
    cameraCollisionRefreshInterval.QuadPart = getPerformanceCounterFrequency().QuadPart / 60;
}

C3Vector editCamera_originalPosition() {
    if (cameraOriginalPosition.x == 0.0f && cameraOriginalPosition.y == 0.0f && cameraOriginalPosition.z == 0.0f) {
        return vanilla1121_getCameraPosition(vanilla1121_getCamera());
    }
    return cameraOriginalPosition;
}

C3Vector editCamera_originalForword() {
    if (cameraOriginalForwardVector.x == 0.0f && cameraOriginalForwardVector.y == 0.0f && cameraOriginalForwardVector.z == 0.0f) {
        return vanilla1121_getCameraForwardVector(vanilla1121_getCamera());
    }
    return cameraOriginalForwardVector;
}

C3Vector editCamera_translatedPosition() {
    if (cameraTranslatedPosition.x == 0.0f && cameraTranslatedPosition.y == 0.0f && cameraTranslatedPosition.z == 0.0f) {
        return vanilla1121_getCameraPosition(vanilla1121_getCamera());
    }
    return cameraTranslatedPosition;
}

C3Vector editCamera_rotatedForword() {
    if (cameraRotatedForwardVector.x == 0.0f && cameraRotatedForwardVector.y == 0.0f && cameraRotatedForwardVector.z == 0.0f) {
        return vanilla1121_getCameraForwardVector(vanilla1121_getCamera());
    }
    return cameraRotatedForwardVector;
}

double __fastcall detoured_OrganicSmooth(float start, float end, float step) {
    if (cameraOrganicSmooth) {
        return p_original_OrganicSmooth(start, end, step);
    }
    else {
        return end;
    }
}

// Vanilla camera point at player's eye. But this height could change as druids shapeshift
// As https://github.com/allfoxwy/UnitXP_SP3/pull/19
// KinTheInfinite found a way to know this eye height
// So that we could pin camera position at a fixed height
// Return -1.0f for error
static float cameraUnitEyeHeight(const uint32_t camera) {
    float cameraTargetZ = *reinterpret_cast<float*>(camera + 0x17c);
    if (cameraTargetZ < 0) {
        return -1.0f;
    }

    uint32_t lookingAtUnit = vanilla1121_getVisiableObject(vanilla1121_getCameraLookingAtGUID(camera));
    if (lookingAtUnit == 0) {
        return -1.0f;
    }
    if (vanilla1121_objectType(lookingAtUnit) != OBJECT_TYPE_Player && vanilla1121_objectType(lookingAtUnit) != OBJECT_TYPE_Unit) {
        return -1.0f;
    }

    C3Vector unitPos = vanilla1121_unitPosition(lookingAtUnit);

    float result = cameraTargetZ - unitPos.z;
    if (result < 0) {
        return -1.0f;
    }

    return result;
}

static void cameraFollowPosition(const uint32_t camera, const C3Vector& targetPosition) {
    C3Vector cameraPosition = vanilla1121_getCameraPosition(camera);

    // Fundamental method of calculating look-at matrix is from https://medium.com/@carmencincotti/lets-look-at-magic-lookat-matrices-c77e53ebdf78
    // However the WoW internal data structures are not the same.

    C3Vector vecForward = {};
    vecForward.x = -(cameraPosition.x - targetPosition.x);
    vecForward.y = -(cameraPosition.y - targetPosition.y);
    vecForward.z = -(cameraPosition.z - targetPosition.z);
    if (vectorAlmostZero(vecForward)) {
        return;
    }
    vectorNormalize(vecForward);
    

    C3Vector vecTemp = {};
    vecTemp.x = 0.0f;
    vecTemp.y = 0.0f;
    vecTemp.z = 1.0f;

    C3Vector vecRight = vectorCrossProduct(vecTemp, vecForward);
    vectorNormalize(vecRight);

    C3Vector vecUp = vectorCrossProduct(vecForward, vecRight);
    vectorNormalize(vecUp);

    vanilla1121_setCameraForwardVector(camera, vecForward);
    vanilla1121_setCameraRightVector(camera, vecRight);
    vanilla1121_setCameraUpVector(camera, vecUp);
}

static void cameraAddPitch(uint32_t camera, float delta) {
    C3Vector vecForward = vanilla1121_getCameraForwardVector(camera);
    vecForward.z += delta;

    if (vectorAlmostZero(vecForward)) {
        return;
    }
    vectorNormalize(vecForward);

    C3Vector vecTemp = {};
    vecTemp.x = 0.0f;
    vecTemp.y = 0.0f;
    vecTemp.z = 1.0f;

    C3Vector vecRight = vectorCrossProduct(vecTemp, vecForward);
    vectorNormalize(vecRight);

    C3Vector vecUp = vectorCrossProduct(vecForward, vecRight);
    vectorNormalize(vecUp);

    vanilla1121_setCameraForwardVector(camera, vecForward);
    vanilla1121_setCameraRightVector(camera, vecRight);
    vanilla1121_setCameraUpVector(camera, vecUp);
}

static C3Vector cameraTranslate(const uint32_t camera, float horizontalDelta, float verticalDelta) {
    C3Vector a = vanilla1121_getCameraPosition(camera);
    C3Vector result = a;

    uint32_t lookingAtUnit = vanilla1121_getVisiableObject(vanilla1121_getCameraLookingAtGUID(camera));
    if (lookingAtUnit == 0) {
        return result;
    }
    C3Vector b = vanilla1121_unitPosition(lookingAtUnit);

    C3Vector vecCameraToTarget = {};
    vecCameraToTarget.x = b.x - a.x;
    vecCameraToTarget.y = b.y - a.y;
    vecCameraToTarget.z = 0.0f;

    float distanceCameraToTarget = vectorLength(vecCameraToTarget);

    if (distanceCameraToTarget < 0.5f) {
        // Don't translate for first person camera
        return result;
    }

    // Pin camera at a fixed height instead of player's eye height
    // so that camera does not change when druids shapeshift
    if (cameraPinHeight) {
        float eyeHeight = cameraUnitEyeHeight(camera);
        if (false == vanilla1121_unitIsMounted(lookingAtUnit) && eyeHeight >= 0) {
            result.z -= eyeHeight;
            result.z += vanilla1121_unitCollisionBoxHeight(lookingAtUnit);
        }
    }

    if (std::abs(horizontalDelta) > std::numeric_limits<float>::epsilon()) {
        if (horizontalDelta > 0.0f) {
            // Translating towards right
            result.x = std::abs(horizontalDelta) * (b.y - a.y) / distanceCameraToTarget + a.x;
            result.y = std::abs(horizontalDelta) * (a.x - b.x) / distanceCameraToTarget + a.y;
        }
        else {
            // Translating towards left
            result.x = std::abs(horizontalDelta) * (a.y - b.y) / distanceCameraToTarget + a.x;
            result.y = std::abs(horizontalDelta) * (b.x - a.x) / distanceCameraToTarget + a.y;
        }
    }

    if (std::abs(verticalDelta) > std::numeric_limits<float>::epsilon()) {
        result.z += verticalDelta;
    }

    return result;
}

static C3Vector nearClipCollisionCheckForHorizontalTranslation(const uint32_t camera, const float horizontalDelta, const C3Vector& noCollisionCameraPosition, const C3Vector& couldCollideCameraPosition) {
    float nearClip = vanilla1121_getCameraNearClip(camera);
    C3Vector vecCameraRight = vanilla1121_getCameraRightVector(camera);
    C3Vector vecCameraForward = vanilla1121_getCameraForwardVector(camera);
    vectorNormalize(vecCameraRight);
    vectorNormalize(vecCameraForward);
    const float fov = vanilla1121_getCameraFoV(camera);
    const float frontPlaneHalfLength = std::tan(fov / 2.0f) * nearClip;

    bool translatedRightward = true;
    if (horizontalDelta > 0.0f) {
        translatedRightward = true;
    }
    else {
        translatedRightward = false;
    }

    C3Vector vecCheckPosition = couldCollideCameraPosition;
    vecCheckPosition.x += vecCameraForward.x * nearClip;
    vecCheckPosition.y += vecCameraForward.y * nearClip;
    vecCheckPosition.z += vecCameraForward.z * nearClip;

    C3Vector vecOriginalPosition = noCollisionCameraPosition;
    vecOriginalPosition.x += vecCameraForward.x * nearClip;
    vecOriginalPosition.y += vecCameraForward.y * nearClip;
    vecOriginalPosition.z += vecCameraForward.z * nearClip;

    if (translatedRightward) {
        vecCheckPosition.x += vecCameraRight.x * frontPlaneHalfLength;
        vecCheckPosition.y += vecCameraRight.y * frontPlaneHalfLength;
        vecCheckPosition.z += vecCameraRight.z * frontPlaneHalfLength;

        vecOriginalPosition.x += vecCameraRight.x * frontPlaneHalfLength;
        vecOriginalPosition.y += vecCameraRight.y * frontPlaneHalfLength;
        vecOriginalPosition.z += vecCameraRight.z * frontPlaneHalfLength;
    }
    else {
        vecCheckPosition.x -= vecCameraRight.x * frontPlaneHalfLength;
        vecCheckPosition.y -= vecCameraRight.y * frontPlaneHalfLength;
        vecCheckPosition.z -= vecCameraRight.z * frontPlaneHalfLength;

        vecOriginalPosition.x -= vecCameraRight.x * frontPlaneHalfLength;
        vecOriginalPosition.y -= vecCameraRight.y * frontPlaneHalfLength;
        vecOriginalPosition.z -= vecCameraRight.z * frontPlaneHalfLength;
    }

    // Test the front/near clip plane
    C3Vector result = {};
    float correction_nearClip = 0.0f;
    C3Vector intersectPosition = {};
    float intersectDistance = 1.0f;
    if (CWorld_Intersect(&vecOriginalPosition, &vecCheckPosition, &intersectPosition, &intersectDistance, vanilla1121_getCameraIntersectFlag())
        && intersectDistance <= 1.0f && intersectDistance >= 0.0f) {
        result.x = -((vecCheckPosition.x - vecOriginalPosition.x) * (1.0f - intersectDistance));
        result.y = -((vecCheckPosition.y - vecOriginalPosition.y) * (1.0f - intersectDistance));
        result.z = 0.0f;

        // Try to keep distance from the wall, else revert the translation
        C3Vector vecIntersectDelta = {};
        vecIntersectDelta.x = (vecCheckPosition.x - vecOriginalPosition.x) * intersectDistance;
        vecIntersectDelta.y = (vecCheckPosition.y - vecOriginalPosition.y) * intersectDistance;
        vecIntersectDelta.z = 0.0f;
        if (vectorLength(vecIntersectDelta) >= keepDistanceFromWall) {
            C3Vector vecResultUnit = result;
            vectorNormalize(vecResultUnit);

            result.x += vecResultUnit.x * keepDistanceFromWall;
            result.y += vecResultUnit.y * keepDistanceFromWall;
            result.z += vecResultUnit.z * keepDistanceFromWall;
        }
        else {
            result.x = noCollisionCameraPosition.x - couldCollideCameraPosition.x;
            result.y = noCollisionCameraPosition.y - couldCollideCameraPosition.y;
            result.z = 0.0f;
        }
    }

    return result;
}

static C3Vector nearClipCollisionCheckForVerticalTranslation(const uint32_t camera, const C3Vector& noCollisionCameraPosition, const C3Vector& couldCollideCameraPosition) {
    float nearClip = vanilla1121_getCameraNearClip(camera);
    C3Vector vecCameraUp = vanilla1121_getCameraUpVector(camera);
    C3Vector vecCameraForward = vanilla1121_getCameraForwardVector(camera);
    vectorNormalize(vecCameraUp);
    vectorNormalize(vecCameraForward);
    const float fov = vanilla1121_getCameraFoV(camera) / vanilla1121_getCameraAspectRatio(camera);
    const float frontPlaneHalfLength = std::tan(fov / 2.0f) * nearClip;

    bool translatedUpward = true;
    if (couldCollideCameraPosition.z > noCollisionCameraPosition.z) {
        translatedUpward = true;
    }
    else {
        translatedUpward = false;
    }

    C3Vector vecCheckPosition = couldCollideCameraPosition;
    vecCheckPosition.x += vecCameraForward.x * nearClip;
    vecCheckPosition.y += vecCameraForward.y * nearClip;
    vecCheckPosition.z += vecCameraForward.z * nearClip;

    C3Vector vecOriginalPosition = noCollisionCameraPosition;
    vecOriginalPosition.x += vecCameraForward.x * nearClip;
    vecOriginalPosition.y += vecCameraForward.y * nearClip;
    vecOriginalPosition.z += vecCameraForward.z * nearClip;

    if (translatedUpward) {
        vecCheckPosition.x += vecCameraUp.x * frontPlaneHalfLength;
        vecCheckPosition.y += vecCameraUp.y * frontPlaneHalfLength;
        vecCheckPosition.z += vecCameraUp.z * frontPlaneHalfLength;

        vecOriginalPosition.x += vecCameraUp.x * frontPlaneHalfLength;
        vecOriginalPosition.y += vecCameraUp.y * frontPlaneHalfLength;
        vecOriginalPosition.z += vecCameraUp.z * frontPlaneHalfLength;
    }
    else {
        vecCheckPosition.x -= vecCameraUp.x * frontPlaneHalfLength;
        vecCheckPosition.y -= vecCameraUp.y * frontPlaneHalfLength;
        vecCheckPosition.z -= vecCameraUp.z * frontPlaneHalfLength;

        vecOriginalPosition.x -= vecCameraUp.x * frontPlaneHalfLength;
        vecOriginalPosition.y -= vecCameraUp.y * frontPlaneHalfLength;
        vecOriginalPosition.z -= vecCameraUp.z * frontPlaneHalfLength;
    }

    // Test the front/near clip plane
    C3Vector result = {};
    float correction_nearClip = 0.0f;
    C3Vector intersectPosition = {};
    float intersectDistance = 1.0f;
    if (CWorld_Intersect(&vecOriginalPosition, &vecCheckPosition, &intersectPosition, &intersectDistance, vanilla1121_getCameraIntersectFlag())
        && intersectDistance <= 1.0f && intersectDistance >= 0.0f) {
        result.x = 0.0f;
        result.y = 0.0f;
        result.z = -((vecCheckPosition.z - vecOriginalPosition.z) * (1.0f - intersectDistance));

        // Try to keep distance from the wall, else revert the translation
        C3Vector vecIntersectDelta = {};
        vecIntersectDelta.x = 0.0f;
        vecIntersectDelta.y = 0.0f;
        vecIntersectDelta.z = (vecCheckPosition.z - vecOriginalPosition.z) * intersectDistance;
        if (vectorLength(vecIntersectDelta) >= keepDistanceFromWall) {
            C3Vector vecResultUnit = result;
            vectorNormalize(vecResultUnit);

            result.x += vecResultUnit.x * keepDistanceFromWall;
            result.y += vecResultUnit.y * keepDistanceFromWall;
            result.z += vecResultUnit.z * keepDistanceFromWall;
        }
        else {
            result.x = 0.0f;
            result.y = 0.0f;
            result.z = noCollisionCameraPosition.z - couldCollideCameraPosition.z;
        }
    }

    return result;
}

int __fastcall detoured_CGCamera_updateCallback_0x511bc0(void* unknown1, uint32_t camera) {
    int result = p_original_CGCamera_updateCallback_0x511bc0(unknown1, camera);

    if (camera > 0 && (camera & 1) == 0) {
        cameraOriginalPosition = vanilla1121_getCameraPosition(camera);
        cameraOriginalForwardVector = vanilla1121_getCameraForwardVector(camera);

        uint32_t u = vanilla1121_getVisiableObject(vanilla1121_getCameraLookingAtGUID(camera));
        if (u > 0 &&
            (vanilla1121_objectType(u) == OBJECT_TYPE_Player || vanilla1121_objectType(u) == OBJECT_TYPE_Unit)) {

            cameraTranslatedPosition = cameraTranslate(camera, cameraHorizontalAddend, cameraVerticalAddend);

            if (std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                cameraAddPitch(camera, cameraPitchAddend);
            }

            // Collision test
            if (cameraPinHeight || std::abs(cameraVerticalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraHorizontalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                C3Vector verticalNearClipCorrection = {};
                C3Vector horizontalNearClipCorrection = {};
                C3Vector verticalCameraCollisionCorrection = {};
                C3Vector horizontalCameraCollisionCorrection = {};

                LARGE_INTEGER now = {};
                QueryPerformanceCounter(&now);

                if (now.QuadPart - lastCameraCollisionTime.QuadPart > cameraCollisionRefreshInterval.QuadPart) {
                    if (false == vectorsAreNear(cameraTranslatedPosition, lastCameraTranslatedPosition) || (std::abs(std::abs(lastCameraPitchAddend) - std::abs(cameraPitchAddend)) > std::numeric_limits<float>::epsilon())) {
                        QueryPerformanceCounter(&lastCameraCollisionTime);
                        lastCameraTranslatedPosition = cameraTranslatedPosition;
                        lastCameraPitchAddend = cameraPitchAddend;

                        if (cameraPinHeight || std::abs(cameraVerticalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                            verticalNearClipCorrection = nearClipCollisionCheckForVerticalTranslation(camera, cameraOriginalPosition, cameraTranslatedPosition);
                        }
                        lastVerticalNearClipCorrection = verticalNearClipCorrection;

                        if (std::abs(cameraHorizontalAddend) > std::numeric_limits<float>::epsilon()) {
                            horizontalNearClipCorrection = nearClipCollisionCheckForHorizontalTranslation(camera, cameraHorizontalAddend, cameraOriginalPosition, cameraTranslatedPosition);
                        }
                        lastHorizontalNearClipCorrection = horizontalNearClipCorrection;

                        if (cameraPinHeight || std::abs(cameraVerticalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraHorizontalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                            // Check if camera hits a wall
                            float cameraIntersectDistance = 1.0f;
                            C3Vector cameraIntersectPoint = {};
                            if (CWorld_Intersect(&cameraOriginalPosition, &cameraTranslatedPosition, &cameraIntersectPoint, &cameraIntersectDistance, vanilla1121_getCameraIntersectFlag())
                                && cameraIntersectDistance >= 0.0f && cameraIntersectDistance <= 1.0f) {
                                horizontalCameraCollisionCorrection.x = -((cameraTranslatedPosition.x - cameraOriginalPosition.x) * (1.0f - cameraIntersectDistance));
                                horizontalCameraCollisionCorrection.y = -((cameraTranslatedPosition.y - cameraOriginalPosition.y) * (1.0f - cameraIntersectDistance));
                                verticalCameraCollisionCorrection.z = -((cameraTranslatedPosition.z - cameraOriginalPosition.z) * (1.0f - cameraIntersectDistance));
                            }
                        }
                        lastVerticalCameraCollisionCorrection = verticalCameraCollisionCorrection;
                        lastHorizontalCameraCollisionCorrection = horizontalCameraCollisionCorrection;
                    }
                    else {
                        if (cameraPinHeight || std::abs(cameraVerticalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                            verticalNearClipCorrection = lastVerticalNearClipCorrection;
                        }
                        if (std::abs(cameraHorizontalAddend) > std::numeric_limits<float>::epsilon()) {
                            horizontalNearClipCorrection = lastHorizontalNearClipCorrection;
                        }
                        if (cameraPinHeight || std::abs(cameraVerticalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraHorizontalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                            horizontalCameraCollisionCorrection = lastHorizontalCameraCollisionCorrection;
                            verticalCameraCollisionCorrection = lastVerticalCameraCollisionCorrection;
                        }
                    }
                }
                else {
                    if (cameraPinHeight || std::abs(cameraVerticalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                        verticalNearClipCorrection = lastVerticalNearClipCorrection;
                    }
                    if (std::abs(cameraHorizontalAddend) > std::numeric_limits<float>::epsilon()) {
                        horizontalNearClipCorrection = lastHorizontalNearClipCorrection;
                    }
                    if (cameraPinHeight || std::abs(cameraVerticalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraHorizontalAddend) > std::numeric_limits<float>::epsilon() || std::abs(cameraPitchAddend) > std::numeric_limits<float>::epsilon()) {
                        horizontalCameraCollisionCorrection = lastHorizontalCameraCollisionCorrection;
                        verticalCameraCollisionCorrection = lastVerticalCameraCollisionCorrection;
                    }
                }


                // Final result of collision check
                if (vectorLength(verticalCameraCollisionCorrection) > vectorLength(verticalNearClipCorrection)) {
                    cameraTranslatedPosition.z += verticalCameraCollisionCorrection.z;
                }
                else {
                    cameraTranslatedPosition.z += verticalNearClipCorrection.z;
                }
                if (vectorLength(horizontalCameraCollisionCorrection) > vectorLength(horizontalNearClipCorrection)) {
                    cameraTranslatedPosition.x += horizontalCameraCollisionCorrection.x;
                    cameraTranslatedPosition.y += horizontalCameraCollisionCorrection.y;
                }
                else {
                    cameraTranslatedPosition.x += horizontalNearClipCorrection.x;
                    cameraTranslatedPosition.y += horizontalNearClipCorrection.y;
                }
            }

            float* editPtr = reinterpret_cast<float*>(camera + 0x8);
            editPtr[0] = cameraTranslatedPosition.x;
            editPtr[1] = cameraTranslatedPosition.y;
            editPtr[2] = cameraTranslatedPosition.z;

            if (cameraFollowTarget) {
                uint64_t targetGUID = UnitGUID("target");
                if (targetGUID > 0) {
                    uint32_t t = vanilla1121_getVisiableObject(targetGUID);
                    if (t > 0 &&
                        ((vanilla1121_objectType(t) == OBJECT_TYPE_Player && vanilla1121_unitCanBeAttacked(t) == 0) || (vanilla1121_objectType(t) == OBJECT_TYPE_Unit && vanilla1121_unitIsControlledByPlayer(t) == 0))) {
                        if (UnitXP_distanceBetween("player", "target", METER_RANGED) < 50.0f
                            && UnitXP_inSight("player", "target")) {
                            C3Vector targetPosition = vanilla1121_unitPosition(t);
                            targetPosition.z += vanilla1121_unitCollisionBoxHeight(t);
                            cameraFollowPosition(camera, targetPosition);
                        }
                    }
                }
            }

            cameraRotatedForwardVector = vanilla1121_getCameraForwardVector(camera);
        }
    }
    return result;
}
