#pragma once

#include <string>
#include <mutex>
#include <utility>
#include <deque>
#include <unordered_set>

#include "cpptime.h"

extern std::deque< std::pair<CppTime::timer_id, std::string> > gTimerTriggeredFunctions;
extern std::unordered_set<CppTime::timer_id> gAlreadyTriggeredTimers;
extern std::timed_mutex gLockTrigger;

// Arm a new timer which would firstly trigger at "when", then repeat at "interval". Would call Lua function "handlerName" on trigger. Expecting time in millseconds(ms).
CppTime::timer_id armTimer(uint64_t when, uint64_t interval, std::string handlerName);
// Disarm a timer
void disarmTimer(int timerID);
