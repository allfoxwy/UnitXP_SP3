#include "pch.h"

#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace std;

#include "Vanilla1121_functions.h"
#include "distanceBetween.h"
#include "inSight.h"
#include "targeting.h"

extern float targetingRangeCone = 2.2f;

// Return true if position is in camera viewing frustum
bool inViewingFrustum(C3Vector posObject) {
    C3Vector posCamera = vanilla1121_getCameraPosition();
    C3Vector posPlayer = vanilla1121_getObjectPosition(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

    // When player jump onto transports (boat/zeppelin) their coordinates system would change.
    // If we pass coordinates from different system into vanilla1121_inLineOfSight(), game crashes
    // TODO: I don't have a way to find out what the current system is
    // To workaround, we test the distance. If they are too far away, we judge that situation as error
    float testDistance0 = UnitXP_distanceBetween(posCamera, posPlayer);
    float testDistance1 = UnitXP_distanceBetween(posCamera, posObject);
    if (testDistance0 > guardAgainstTransportsCoordinates || testDistance1 > guardAgainstTransportsCoordinates) {
        return false;
    }

    C3Vector vecObject;
    vecObject.x = posObject.x - posCamera.x;
    vecObject.y = posObject.y - posCamera.y;
    vecObject.z = posObject.z - posCamera.z;

    C3Vector vecPlayer;
    vecPlayer.x = posPlayer.x - posCamera.x;
    vecPlayer.y = posPlayer.y - posCamera.y;
    vecPlayer.z = posPlayer.z - posCamera.z;
    
    float dotProduct = vecObject.x * vecPlayer.x + vecObject.y * vecPlayer.y + vecObject.z * vecPlayer.z;
    float lenVecObject = hypot(vecObject.x, vecObject.y, vecObject.z);
    float lenVecPlayer = hypot(vecPlayer.x, vecPlayer.y, vecPlayer.z);

    // I tested in game and find out that even Vanilla Tweaks change this value, the screen border of objects still follow original FoV somehow
    // I suspect game has additional transformation before Direct X FoV, or Vanilla Tweaks did on a wrong address...
    //float fov = vanilla1121_getCameraFoV();
    const float fov = 1.5708f;

    float angleBetweenPlayerAndObject = acos(dotProduct / (lenVecObject * lenVecPlayer));

    if (angleBetweenPlayerAndObject > fov / targetingRangeCone) {
        return false;
    }
    else {
        return true;
    }
}

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
            && inViewingFrustum(vanilla1121_getObjectPosition(i))
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


bool targetWorldBoss(float distanceLimit) {
    vector<struct mob_entity> mobs;

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    void* player = reinterpret_cast<void*>(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        if (((type == OBJECT_TYPE_Unit && vanilla1121_objIsControlledByPlayer(i) == 0) || type == OBJECT_TYPE_Player)
            && UnitXP_inSight(player, reinterpret_cast<void*>(i)) == 1
            && inViewingFrustum(vanilla1121_getObjectPosition(i))
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

// Return the mark's order in list
unsigned int markInList(const int mark, const vector<int>& list) {
    for (unsigned int i = 0; i < list.size(); ++i) {
        if (list[i] == mark) {
            return i;
        }
    }
    return UINT_MAX;
}

uint64_t selectNext(uint64_t current, vector<struct mob_entity>& list) {
    if (list.size() == 0) {
        return 0;
    }

    auto compareFunction = [](const struct mob_entity& a, const struct mob_entity& b) {
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

    auto compareFunction = [](const struct mob_entity& a, const struct mob_entity& b) {
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

uint64_t selectNextMark(const uint64_t current, vector<struct mob_entity>& list, const vector<int>& priority) {
    if (list.size() == 0) {
        return 0;
    }

    auto compareFunction = [&priority](const struct mob_entity& a, const struct mob_entity& b) {
        return markInList(a.targetMark, priority) < markInList(b.targetMark, priority);
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

uint64_t selectPreviousMark(const uint64_t current, vector<struct mob_entity>& list, const vector<int>& priority) {
    if (list.size() == 0) {
        return 0;
    }

    auto compareFunction = [&priority](const struct mob_entity& a, const struct mob_entity& b) {
        return markInList(a.targetMark, priority) > markInList(b.targetMark, priority);
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
            && inViewingFrustum(vanilla1121_getObjectPosition(i))
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

bool targetMarkedEnemyInCycle(MOB_SELECTFUNCTION_WITH_MARK selectFunction, string priority) {
    if (!selectFunction) {
        return false;
    }

    vector <struct mob_entity> list;
    vector <int> priorityList;

    for (unsigned int i = 0; i < min(8u, priority.length()); ++i) {
        if (priority[i] >= '1' && priority[i] <= '8') {
            priorityList.push_back(priority[i] - 48);
        }
    }

    if (priorityList.size() == 0) {
        for (int i = 8; i >= 1; --i) {
            priorityList.push_back(i);
        }
    }

    uint32_t objects = *reinterpret_cast<uint32_t*>(0x00b41414);
    uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xac);

    void* player = reinterpret_cast<void*>(vanilla1121_getVisiableObject(UnitGUID(u8"player")));

    // TODO: Maybe we could find last target without lagging
    //static uint64_t lastTarget = 0;
    uint64_t lastTarget = vanilla1121_getObject_s_targetGUID(vanilla1121_getVisiableObject(UnitGUID("player")));

    while (i != 0 && (i & 1) == 0) {
        uint64_t currentObjectGUID = *reinterpret_cast<uint64_t*>(i + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(i + 0x14);

        int mark = vanilla1121_getTargetMark(currentObjectGUID);

        if (((type == OBJECT_TYPE_Unit && vanilla1121_objIsControlledByPlayer(i) == 0) || type == OBJECT_TYPE_Player)
            && mark > 0
            && UnitXP_inSight(player, reinterpret_cast<void*>(i)) == 1
            && inViewingFrustum(vanilla1121_getObjectPosition(i))
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
                    new_mob.targetMark = mark;
                    if (markInList(new_mob.targetMark, priorityList) < priorityList.size()) {
                        list.push_back(new_mob);
                    }
                }
            }
            else {
                struct mob_entity new_mob;
                new_mob.GUID = currentObjectGUID;
                new_mob.distance = UnitXP_distanceBetween(player, reinterpret_cast<void*>(i));
                new_mob.targetMark = mark;
                if (markInList(new_mob.targetMark, priorityList) < priorityList.size()) {
                    list.push_back(new_mob);
                }
            }
        }
        i = *reinterpret_cast<uint32_t*>(i + 0x3c);
    }

    if (list.size() > 0) {
        uint64_t choice = selectFunction(lastTarget, list, priorityList);
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
            && inViewingFrustum(vanilla1121_getObjectPosition(i))
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
