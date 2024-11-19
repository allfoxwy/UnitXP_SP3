#pragma once

#include <string>
#include <mutex>
#include <utility>
#include <list>

#include "cpptime.h"

extern std::list< std::pair<CppTime::timer_id, std::string> > gTimerTriggeredFunctions;
extern std::timed_mutex gLockTrigger;

// Arm a new timer which would firstly trigger at "when", then repeat at "interval". Would call Lua function "handlerName" on trigger. Expecting time in millseconds(ms).
CppTime::timer_id armTimer(uint64_t when, uint64_t interval, std::string handlerName);
// Disarm a timer
void disarmTimer(int timerID);
