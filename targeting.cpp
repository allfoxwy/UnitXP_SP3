#include "pch.h"

#include <vector>
#include <algorithm>
#include <random>

using namespace std;

#include "Vanilla1121_functions.h"
#include "distanceBetween.h"
#include "inSight.h"
#include "targeting.h"

random_device hwRng;
mt19937 randomGenerator(hwRng());

struct mob_entity {
    uint64_t GUID;
    float distance;
} ;

bool targetNearestEnemy() {
    vector<struct mob_entity> mobs;

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        if ((type == OBJECT_TYPE_Unit || type == OBJECT_TYPE_Player)
            && UnitXP_inSight(UnitGUID("player"), currentObjectGUID) == 1
            && vanilla1121_canAttack(currentObjectGUID) == 1
            && vanilla1121_isDead(i) == 0) {

            bool targetInCombat = vanilla1121_inCombat(i);
            bool selfInCombat = vanilla1121_inCombat(vanilla1121_getVisiableObject(UnitGUID("player")));

            if (selfInCombat) {
                if (targetInCombat) {
                    struct mob_entity new_mob;
                    new_mob.GUID = currentObjectGUID;
                    new_mob.distance = UnitXP_distanceBetween(UnitGUID("player"), currentObjectGUID);
                    mobs.push_back(new_mob);
                }
            }
            else {
                struct mob_entity new_mob;
                new_mob.GUID = currentObjectGUID;
                new_mob.distance = UnitXP_distanceBetween(UnitGUID("player"), currentObjectGUID);
                mobs.push_back(new_mob);
            }

        }
        i = *reinterpret_cast<uint32_t*>(i + 0x3c);
    }

    if (mobs.size() > 0) {
        auto compareFunction = [](struct mob_entity a, struct mob_entity b) {
            return a.distance < b.distance;
            };
        sort(mobs.begin(), mobs.end(), compareFunction);

        vanilla1121_target(mobs.front().GUID);
        return true;
    }

    return false;
}

bool targetRandomEnemy() {
    vector<struct mob_entity> meleeRange;
    vector<struct mob_entity> chargeRange;
    vector<struct mob_entity> farRange;

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        if ((type == OBJECT_TYPE_Unit || type == OBJECT_TYPE_Player)
            && UnitXP_inSight(UnitGUID("player"), currentObjectGUID) == 1
            && vanilla1121_canAttack(currentObjectGUID) == 1
            && vanilla1121_isDead(i) == 0) {

            bool targetInCombat = vanilla1121_inCombat(i);
            bool selfInCombat = vanilla1121_inCombat(vanilla1121_getVisiableObject(UnitGUID("player")));

            if (selfInCombat) {
                if (targetInCombat) {
                    struct mob_entity new_mob;
                    new_mob.GUID = currentObjectGUID;
                    new_mob.distance = UnitXP_distanceBetween(UnitGUID("player"), currentObjectGUID);

                    if (new_mob.distance <= 5.0f) {
                        meleeRange.push_back(new_mob);
                    }
                    else if (new_mob.distance <= 25.0f) {
                        chargeRange.push_back(new_mob);
                    }
                    else if (new_mob.distance < 41.0f) {
                        farRange.push_back(new_mob);
                    }
                }
            }
            else {
                struct mob_entity new_mob;
                new_mob.GUID = currentObjectGUID;
                new_mob.distance = UnitXP_distanceBetween(UnitGUID("player"), currentObjectGUID);
                if (new_mob.distance < 5.0f) {
                    meleeRange.push_back(new_mob);
                }
                else if (new_mob.distance < 25.0f) {
                    chargeRange.push_back(new_mob);
                }
                else if (new_mob.distance < 41.0f) {
                    farRange.push_back(new_mob);
                }
            }
        }
        i = *reinterpret_cast<uint32_t*>(i + 0x3c);
    }

    static uint64_t lastTarget = 0;

    if (meleeRange.size() > 0) {
        uniform_int_distribution<int> dist(0, meleeRange.size() - 1);
        uint64_t choice = meleeRange.at(dist(randomGenerator)).GUID;
        while (meleeRange.size() > 1 && choice == lastTarget) {
            choice = meleeRange.at(dist(randomGenerator)).GUID;
        }
        lastTarget = choice;
        vanilla1121_target(choice);
        return true;
    }
    if (chargeRange.size() > 0) {
        const int limit = 3;
        if (chargeRange.size() > limit) {
            auto compareFunction = [](struct mob_entity a, struct mob_entity b) {
                return a.distance < b.distance;
                };
            sort(chargeRange.begin(), chargeRange.end(), compareFunction);
            chargeRange.erase(chargeRange.begin() + limit, chargeRange.end());
        }
        uniform_int_distribution<int> dist(0, chargeRange.size() - 1);
        uint64_t choice = chargeRange.at(dist(randomGenerator)).GUID;
        while (chargeRange.size() > 1 && choice == lastTarget) {
            choice = chargeRange.at(dist(randomGenerator)).GUID;
        }
        lastTarget = choice;
        vanilla1121_target(choice);
        return true;
    }
    if (farRange.size() > 0) {
        const int limit = 5;
        if (farRange.size() > limit) {
            auto compareFunction = [](struct mob_entity a, struct mob_entity b) {
                return a.distance < b.distance;
                };
            sort(farRange.begin(), farRange.end(), compareFunction);
            farRange.erase(farRange.begin() + limit, farRange.end());
        }

        uniform_int_distribution<int> dist(0, farRange.size() - 1);
        uint64_t choice = farRange.at(dist(randomGenerator)).GUID;
        while (farRange.size() > 1 && choice == lastTarget) {
            choice = farRange.at(dist(randomGenerator)).GUID;
        }
        lastTarget = choice;
        vanilla1121_target(choice);
        return true;
    }
    return false;
}
