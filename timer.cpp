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
static mutex gLockTimer{};

static void timerCallback(timer_id id) {
	lock_guard lg{ gLockTrigger };

	// We only accept 1 callback per timer per frame, discard the rest.
	if (gAlreadyTriggeredTimers.insert(id).second == true) {
		gTimerTriggeredFunctions.push_back({ id, gFunctionMap.at(id) });
	}
}

timer_id armTimer(uint64_t when, uint64_t interval, string handlerName) {
	lock_guard lg{ gLockTimer };

	timer_id newTimer = gTimer.add(when * 1000, timerCallback, interval * 1000); // We expecting time in millseconds(ms)
	gFunctionMap.insert({ newTimer, handlerName });

	return newTimer;
}

void disarmTimer(int timerID) {
	lock_guard lg{ gLockTimer };

	gTimer.remove(timerID);
	gFunctionMap.erase(timerID);

	// As we could not check gTimerTriggeredFunctions because of deadlock. Those "already triggered, but not yet on screen" timers would still goes off later
}


