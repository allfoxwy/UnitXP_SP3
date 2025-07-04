#include "pch.h"

#include <Windows.h>

#include "edit_CWorld_Intersect.h"
#include "utf8_to_utf16.h"
#include "MinHook.h"

auto p_CWorld_Intersect = reinterpret_cast<CWORLD__INTERSECT>(0x672170);
auto p_original_CWorld_Intersect = reinterpret_cast<CWORLD__INTERSECT>(NULL);

bool __fastcall detoured_CWorld_Intersect(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance, uint32_t queryFlags) {
    bool intersect = p_original_CWorld_Intersect(p1, p2, ignored, intersectPoint, distance, queryFlags);

    // These two flags are unique to camera collision detection
    if (queryFlags == 0x1F0171 || queryFlags == 0x100171) {
        if (intersect) {
            if (*distance >= 0 && *distance <= 1) {
                if (*distance > cameraIntersectBlur) {
                    *distance -= cameraIntersectBlur;

                    C3Vector vec = {};
                    vec.x = p2->x - p1->x;
                    vec.y = p2->y - p1->y;
                    vec.z = p2->z - p1->z;

                    intersectPoint->x = p1->x + vec.x * *distance;
                    intersectPoint->y = p1->y + vec.y * *distance;
                    intersectPoint->z = p1->z + vec.z * *distance;
                }
            }
        }
    }

    return intersect;
}

bool edit_CWorld_Intersect_init() {
    if (MH_CreateHook(p_CWorld_Intersect, &detoured_CWorld_Intersect, reinterpret_cast<LPVOID*>(&p_original_CWorld_Intersect)) != MH_OK) {
        MessageBoxW(NULL, utf8_to_utf16(u8"Failed to hook into CWorld_Intersect function.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
        return false;
    }
    return true;
}

bool edit_CWorld_Intersect_end() {
    if (MH_RemoveHook(p_CWorld_Intersect) != MH_OK) {
        MessageBoxW(NULL, utf8_to_utf16(u8"Failed when to remove hook for CWorld_Intersect function. Game might crash later.").data(), utf8_to_utf16(u8"UnitXP Service Pack 3").data(), MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
        return false;
    }
    return true;
}
