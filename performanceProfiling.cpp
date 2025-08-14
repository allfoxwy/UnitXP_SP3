#include "pch.h"

#include <sstream>
#include <unordered_map>
#include <cstdint>
#include <profileapi.h>

#include "performanceProfiling.h"
#include "coffTimeDateStamp.h"
#include "gameSocket.h"
#include "polyfill.h"

static const int perfSlotsTotal = 16;

static LARGE_INTEGER perfStart[perfSlotsTotal] = {};
static LARGE_INTEGER perfEnd[perfSlotsTotal] = {};
static LARGE_INTEGER perfSampleCount[perfSlotsTotal] = {};
static LARGE_INTEGER perfTotalTime[perfSlotsTotal] = {};
static LARGE_INTEGER perfWorst[perfSlotsTotal] = {};
static LARGE_INTEGER perfLast[perfSlotsTotal] = {};

static LARGE_INTEGER performanceCounterFrequency = {};
static std::unordered_map<int, std::string> perfSlotsName{};

void perfMarkStart(int perfPoint) {
    if (perfPoint >= perfSlotsTotal) {
        // Not enough perf slots
        return;
    }

    QueryPerformanceCounter(&perfStart[perfPoint]);
}

void perfMarkEnd(int perfPoint) {
    if (perfPoint >= perfSlotsTotal) {
        // Not enough perf slots
        return;
    }

    QueryPerformanceCounter(&perfEnd[perfPoint]);

    LARGE_INTEGER delta = {};
    delta.QuadPart = perfEnd[perfPoint].QuadPart - perfStart[perfPoint].QuadPart;

    if (delta.QuadPart > perfWorst[perfPoint].QuadPart) {
        perfWorst[perfPoint].QuadPart = delta.QuadPart;
    }

    perfTotalTime[perfPoint].QuadPart += delta.QuadPart;

    perfSampleCount[perfPoint].QuadPart++;
}

void perfMarkLast(int perfPoint) {
    if (perfPoint >= perfSlotsTotal) {
        // Not enough perf slots
        return;
    }
    QueryPerformanceCounter(&perfLast[perfPoint]);
}

void perfReset() {
    QueryPerformanceFrequency(&performanceCounterFrequency);

    for (int i = 0; i < perfSlotsTotal; ++i) {
        perfStart[i].QuadPart = 0;
        perfEnd[i].QuadPart = 0;
        perfSampleCount[i].QuadPart = 0;
        perfTotalTime[i].QuadPart = 0;
        perfWorst[i].QuadPart = 0;
        perfLast[i].QuadPart = 0;
    }
}

void perfSetSlotName(int slot, std::string name) {
    if (slot >= perfSlotsTotal) {
        // Not enough perf slots
        return;
    }

    // the "if" check could avoid 1 copy
    if (perfSlotsName.find(slot) == perfSlotsName.end()) {
        perfSlotsName[slot] = name;
    }
}

LARGE_INTEGER getPerformanceCounterFrequency() {
    return performanceCounterFrequency;
}

static std::string timeString(const LARGE_INTEGER& t) {
    LARGE_INTEGER temp = {};
    temp.QuadPart = (t.QuadPart * 1000000) / performanceCounterFrequency.QuadPart;

    std::string unit{ "us" };

    if (temp.QuadPart / 1000 / 1000 >= 1) {
        temp.QuadPart = t.QuadPart / performanceCounterFrequency.QuadPart;
        unit = "s";
    }
    else if (temp.QuadPart / 1000 >= 10) {
        temp.QuadPart = (t.QuadPart * 1000) / performanceCounterFrequency.QuadPart;
        unit = "ms";
    }

    std::stringstream ss;
    ss << temp.QuadPart << unit;
    return ss.str();
}

std::string perfSummary() {
    std::stringstream ss;
    ss << "UnitXP_SP3 built at " << static_cast<uint32_t>(coffTimeDateStamp()) << std::endl;
    ss << "TCP quick ACK: " << gameSocket_isQuickACK() << std::endl;
    ss << "TCP fixed receiving window: " << gameSocket_hasBiggerWindow() << std::endl;
    ss << "IP smaller MTU: " << gameSocket_hasSmallerMTU() << std::endl;
    ss << getPolyfillDebug() << std::endl;
    for (int i = 0; i < perfSlotsTotal; ++i) {
        if (perfSampleCount[i].QuadPart == 0) {
            continue;
        }

        LARGE_INTEGER average = {};
        average.QuadPart = perfTotalTime[i].QuadPart / perfSampleCount[i].QuadPart;

        auto ni = perfSlotsName.find(i);
        if (ni != perfSlotsName.end()) {
            ss << "=== Profiling: " << ni->second << " ===" << std::endl;
        }
        else {
            ss << "=== Profiling point " << i << " ===" << std::endl;
        }
        ss << "samples: " << perfSampleCount[i].QuadPart << std::endl;
        ss << "worst: " << timeString(perfWorst[i]) << std::endl;
        ss << "average: " << timeString(average) << std::endl;
        ss << "total: " << timeString(perfTotalTime[i]) << std::endl;

        if (perfLast[i].QuadPart > 0) {
            LARGE_INTEGER delta = {}, now = {};

            QueryPerformanceCounter(&now);
            delta.QuadPart = now.QuadPart - perfLast[i].QuadPart;
            ss << "last: " << timeString(delta) << " ago" << std::endl;
        }
    }
    return ss.str();
}
