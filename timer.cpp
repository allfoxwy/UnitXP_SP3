#include "pch.h"

#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <mutex>

#include "cpptime.h"
#include "timer.h"

using namespace std;
using namespace CppTime;

CppTime::Timer gTimer{};

extern std::deque< std::pair<CppTime::timer_id, std::string> > gTimerTriggeredFunctions{};
extern std::unordered_set<CppTime::timer_id> gAlreadyTriggeredTimers{};
extern timed_mutex gLockTrigger{};
static unordered_map<timer_id, string> gFunctionMap{};
static timed_mutex gLockTimer{};

static void timerCallback(timer_id id) {
	gLockTrigger.lock();

	// We only accept 1 callback per timer per frame, discard the rest.
	if (gAlreadyTriggeredTimers.find(id) == gAlreadyTriggeredTimers.end()) {
		gTimerTriggeredFunctions.push_back(make_pair(id, gFunctionMap.at(id)));
		gAlreadyTriggeredTimers.insert(id);
	}

	gLockTrigger.unlock();
}

timer_id armTimer(uint64_t when, uint64_t interval, string handlerName) {
	gLockTimer.lock();
	timer_id newTimer = gTimer.add(when * 1000, timerCallback, interval * 1000); // We expecting time in millseconds(ms)
	gFunctionMap.insert(make_pair(newTimer, handlerName));
	gLockTimer.unlock();

	return newTimer;
}

void disarmTimer(int timerID) {
	gLockTimer.lock();
	gFunctionMap.erase(timerID);
	gTimer.remove(timerID);
	gLockTimer.unlock();
}


