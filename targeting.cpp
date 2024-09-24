#include "pch.h"

#include <vector>
#include <algorithm>

using namespace std;

#include "Vanilla1121_functions.h"
#include "distanceBetween.h"
#include "inSight.h"
#include "targeting.h"



bool targetNearestEnemy(float distanceLimit) {
    vector<struct mob_entity> mobs;

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    void* player = reinterpret_cast<void*>(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        if (((type == OBJECT_TYPE_Unit && vanilla1121_objIsControlledByPlayer(i) == 0) || type == OBJECT_TYPE_Player)
            && UnitXP_inSight(player, reinterpret_cast<void*>(i)) == 1
            && vanilla1121_canAttack(i) == 1
            && vanilla1121_objIsDead(i) == 0
            && vanilla1121_getObject_s_creatureType(i) != 8) {

            bool targetInCombat = vanilla1121_inCombat(i);
            bool selfInCombat = vanilla1121_inCombat(vanilla1121_getVisiableObject(UnitGUID("player")));

            if (selfInCombat) {
                if (targetInCombat) {
                    struct mob_entity new_mob;
                    new_mob.GUID = currentObjectGUID;
                    new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));
                    mobs.push_back(new_mob);
                }
            }
            else {
                struct mob_entity new_mob;
                new_mob.GUID = currentObjectGUID;
                new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));
                mobs.push_back(new_mob);
            }

        }
        i = *reinterpret_cast<uint32_t*>(i + 0x3c);
    }

    if (mobs.size() > 0) {
        auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
            return a.distance < b.distance;
            };
        sort(mobs.begin(), mobs.end(), compareFunction);

        if (mobs.front().distance < distanceLimit) {
            vanilla1121_target(mobs.front().GUID);
            return true;
        }
    }

    return false;
}

// This function ignore line of sight.
bool targetWorldBoss(float distanceLimit) {
    vector<struct mob_entity> mobs;

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    void* player = reinterpret_cast<void*>(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        if (((type == OBJECT_TYPE_Unit && vanilla1121_objIsControlledByPlayer(i) == 0) || type == OBJECT_TYPE_Player)
            && vanilla1121_canAttack(i) == 1
            && vanilla1121_objIsDead(i) == 0
            && vanilla1121_getObject_s_classification(i) == CLASSIFICATION_WORLDBOSS
            && vanilla1121_getObject_s_creatureType(i) != 8) {

            bool targetInCombat = vanilla1121_inCombat(i);
            bool selfInCombat = vanilla1121_inCombat(vanilla1121_getVisiableObject(UnitGUID("player")));

            if (selfInCombat) {
                if (targetInCombat) {
                    struct mob_entity new_mob;
                    new_mob.GUID = currentObjectGUID;
                    new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));
                    if (new_mob.distance < distanceLimit) {
                        mobs.push_back(new_mob);
                    }
                }
            }
            else {
                struct mob_entity new_mob;
                new_mob.GUID = currentObjectGUID;
                new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));
                if (new_mob.distance < distanceLimit) {
                    mobs.push_back(new_mob);
                }
            }

        }
        i = *reinterpret_cast<uint32_t*>(i + 0x3c);
    }

    if (mobs.size() > 0) {
        auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
            return a.distance < b.distance;
            };
        sort(mobs.begin(), mobs.end(), compareFunction);

        // TODO: Maybe we could find last target without lagging
        //static uint64_t lastTarget = 0;
        uint64_t lastTarget = vanilla1121_getObject_s_targetGUID(vanilla1121_getVisiableObject(UnitGUID("player")));

        bool lastTargetIsBoss = false;
        for (auto const& m : mobs) {
            if (m.GUID == lastTarget) {
                lastTargetIsBoss = true;
                break;
            }
        }

        if (lastTarget == 0 || lastTargetIsBoss == false) {
            vanilla1121_target(mobs.front().GUID);
            return true;
        }
        else {
            vanilla1121_target(selectNext(lastTarget, mobs));
            return true;
        }
    }

    return false;
}

uint64_t selectNext(uint64_t current, vector<struct mob_entity>& list) {
    if (list.size() == 0) {
        return 0;
    }

    auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
        return a.GUID < b.GUID;
        };
    sort(list.begin(), list.end(), compareFunction);

    for (auto i = list.begin(); i < list.end(); ++i) {
        if ((*i).GUID == current) {
            if (i < list.end() - 1) {
                return (*(i + 1)).GUID;
            }
            else {
                break;
            }
        }
    }

    return list.front().GUID;
}

uint64_t selectPrevious(uint64_t current, vector<struct mob_entity>& list) {
    if (list.size() == 0) {
        return 0;
    }

    auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
        return a.GUID > b.GUID;
        };
    sort(list.begin(), list.end(), compareFunction);

    for (auto i = list.begin(); i < list.end(); ++i) {
        if ((*i).GUID == current) {
            if (i < list.end() - 1) {
                return (*(i + 1)).GUID;
            }
            else {
                break;
            }
        }
    }

    return list.front().GUID;
}

bool targetEnemyInCycle(MOB_SELECTFUNCTION selectFunction) {
    if (!selectFunction) {
        return false;
    }

    vector <struct mob_entity> list;

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    void* player = reinterpret_cast<void*>(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

    // TODO: Maybe we could find last target without lagging
    //static uint64_t lastTarget = 0;
    uint64_t lastTarget = vanilla1121_getObject_s_targetGUID(vanilla1121_getVisiableObject(UnitGUID("player")));

    if (lastTarget == 0) {
        return targetNearestEnemy(41.0f);
    }

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        if (((type == OBJECT_TYPE_Unit && vanilla1121_objIsControlledByPlayer(i) == 0) || type == OBJECT_TYPE_Player)
            && UnitXP_inSight(player, reinterpret_cast<void*>(i)) == 1
            && vanilla1121_canAttack(i) == 1
            && vanilla1121_objIsDead(i) == 0
            && vanilla1121_getObject_s_creatureType(i) != 8) {

            bool targetInCombat = vanilla1121_inCombat(i);
            bool selfInCombat = vanilla1121_inCombat(vanilla1121_getVisiableObject(UnitGUID("player")));

            if (selfInCombat) {
                if (targetInCombat) {
                    struct mob_entity new_mob;
                    new_mob.GUID = currentObjectGUID;
                    new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));

                    if (new_mob.distance < 41.0f) {
                        list.push_back(new_mob);
                    }
                }
            }
            else {
                struct mob_entity new_mob;
                new_mob.GUID = currentObjectGUID;
                new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));
                
                if (new_mob.distance < 41.0f) {
                    list.push_back(new_mob);
                }
            }
        }
        i = *reinterpret_cast<uint32_t*>(i + 0x3c);
    }

    if (list.size() > 0) {
        uint64_t choice = selectFunction(lastTarget, list);
        vanilla1121_target(choice);
        return true;
    }

    return false;
}

bool targetEnemyConsideringDistance(MOB_SELECTFUNCTION selectFunction) {
    if (!selectFunction) {
        return false;
    }

    vector<struct mob_entity> meleeRange;
    vector<struct mob_entity> chargeRange;
    vector<struct mob_entity> farRange;

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    void* player = reinterpret_cast<void*>(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

    // TODO: Maybe we could find last target without lagging
    //static uint64_t lastTarget = 0;
    uint64_t lastTarget = vanilla1121_getObject_s_targetGUID(vanilla1121_getVisiableObject(UnitGUID("player")));

    if (lastTarget == 0) {
        return targetNearestEnemy(41.0f);
    }

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        if (((type == OBJECT_TYPE_Unit && vanilla1121_objIsControlledByPlayer(i) == 0) || type == OBJECT_TYPE_Player)
            && UnitXP_inSight(player, reinterpret_cast<void*>(i)) == 1
            && vanilla1121_canAttack(i) == 1
            && vanilla1121_objIsDead(i) == 0
            && vanilla1121_getObject_s_creatureType(i) != 8) {

            bool targetInCombat = vanilla1121_inCombat(i);
            bool selfInCombat = vanilla1121_inCombat(vanilla1121_getVisiableObject(UnitGUID("player")));

            if (selfInCombat) {
                if (targetInCombat) {
                    struct mob_entity new_mob;
                    new_mob.GUID = currentObjectGUID;
                    new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));

                    if (new_mob.distance <= 8.0f) {
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
                new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));
                if (new_mob.distance < 8.0f) {
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

    if (meleeRange.size() > 0) {
        
        uint64_t choice = selectFunction(lastTarget, meleeRange);
        
        lastTarget = choice;
        vanilla1121_target(choice);
        return true;
    }
    if (chargeRange.size() > 0) {
        const int limit = 3;
        if (chargeRange.size() > limit) {
            auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
                return a.distance < b.distance;
                };
            sort(chargeRange.begin(), chargeRange.end(), compareFunction);
            chargeRange.erase(chargeRange.begin() + limit, chargeRange.end());
        }
        
        uint64_t choice = selectFunction(lastTarget, chargeRange);
        
        lastTarget = choice;
        vanilla1121_target(choice);
        return true;
    }
    if (farRange.size() > 0) {
        const int limit = 5;
        if (farRange.size() > limit) {
            auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
                return a.distance < b.distance;
                };
            sort(farRange.begin(), farRange.end(), compareFunction);
            farRange.erase(farRange.begin() + limit, farRange.end());
        }

        
        uint64_t choice = selectFunction(lastTarget, farRange);
        
        lastTarget = choice;
        vanilla1121_target(choice);
        return true;
    }
    return false;
}
