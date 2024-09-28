#pragma once

#include <vector>


struct mob_entity {
    uint64_t GUID;
    float distance;
    int targetMark;
};

typedef uint64_t(*MOB_SELECTFUNCTION)(uint64_t current, std::vector<mob_entity>& list);
typedef uint64_t(*MOB_SELECTFUNCTION_WITH_MARK)(uint64_t current, std::vector<mob_entity>& list, const std::vector<int>& markPriority);

// Factor which define targeting range cone in front of camera. Minimum 2 is exactly same as FoV. Default is 2.2. Bigger value narrows the cone.
extern float targetingRangeCone;
// Return true if position is in camera viewing frustum
bool inViewingFrustum(C3Vector posObject);

// Return true when found a target
bool targetNearestEnemy(float distanceLimit);
bool targetWorldBoss(float distanceLimit);
bool targetEnemyConsideringDistance(MOB_SELECTFUNCTION selectFunction);
bool targetEnemyInCycle(MOB_SELECTFUNCTION selectFunction);
bool targetMarkedEnemyInCycle(MOB_SELECTFUNCTION_WITH_MARK selectFunction, string priority);

// Select mobs sorted by GUID
uint64_t selectNext(uint64_t current, vector<struct mob_entity>& list);
uint64_t selectPrevious(uint64_t current, vector<struct mob_entity>& list);

// Select mobs with mark;
uint64_t selectNextMark(const uint64_t current, vector<struct mob_entity>& list, const vector<int>& priority);
uint64_t selectPreviousMark(const uint64_t current, vector<struct mob_entity>& list, const vector<int>& priority);
