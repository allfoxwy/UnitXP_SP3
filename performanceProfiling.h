#pragma once

#include <string>

void perfMarkStart(int perfPoint);
void perfMarkEnd(int perfPoint);
void perfReset();
void perfSetSlotName(int slot, std::string name);
std::string perfSummary();
