#pragma once

#include <string>
#include <profileapi.h>

void perfMarkStart(int perfPoint);
void perfMarkEnd(int perfPoint);
void perfMarkLast(int perfPoint);
void perfReset();
void perfSetSlotName(int slot, std::string name);
LARGE_INTEGER getPerformanceCounterFrequency();
std::string perfSummary();
