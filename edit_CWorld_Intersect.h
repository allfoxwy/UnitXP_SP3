#pragma once

#include <cstdint>

#include "Vanilla1121_functions.h"

typedef bool(__fastcall* CWORLD__INTERSECT)(const C3Vector* p1, const C3Vector* p2, int ignored, C3Vector* intersectPoint, float* distance, uint32_t queryFlags);
extern CWORLD__INTERSECT p_CWorld_Intersect;
extern CWORLD__INTERSECT p_original_CWorld_Intersect;

bool edit_CWorld_Intersect_init();
bool edit_CWorld_Intersect_end();
