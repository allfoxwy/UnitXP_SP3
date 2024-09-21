#pragma once

#include <vector>


struct mob_entity {
    uint64_t GUID;
    float distance;
};

typedef uint64_t(*MOB_SELECTFUNCTION)(uint64_t current, std::vector<mob_entity>& list);

// Return true when found a target
bool targetNearestEnemy(float distanceLimit);
// This function ignore line of sight.
bool targetWorldBoss(float distanceLimit);

// Return true when found a target
bool targetEnemyConsideringDistance(MOB_SELECTFUNCTION selectFunction);
bool targetEnemyInCycle(MOB_SELECTFUNCTION selectFunction);

// Select in list sorted by GUID
uint64_t selectNext(uint64_t current, vector<struct mob_entity>& list);
uint64_t selectPrevious(uint64_t current, vector<struct mob_entity>& list);
