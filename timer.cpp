#include "pch.h"

#include <unordered_map>
#include <mutex>

#include "cpptime.h"
#include "timer.h"

using namespace std;
using namespace CppTime;

CppTime::Timer gTimer{};

extern std::list< std::pair<CppTime::timer_id, std::string> > gTimerTriggeredFunctions{};
extern timed_mutex gLockTrigger{};
static unordered_map<timer_id, string> gFunctionMap{};
static timed_mutex gLockTimer{};



static void timerCallback(timer_id id) {
	gLockTrigger.lock();
	gTimerTriggeredFunctions.push_back(make_pair(id, gFunctionMap.at(id)));
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


