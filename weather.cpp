#include "pch.h"

#include "weather.h"

WEATHER_SETTYPE p_weather_setType = reinterpret_cast<WEATHER_SETTYPE>(0x67baf0);
WEATHER_SETTYPE p_original_weather_setType = NULL;

bool weather_alwaysClear = false;

void __fastcall detoured_weather_setType(void* self, void* ignored, int type, float intensity, bool unknown) {
    if (weather_alwaysClear && type > 0 && type <= 3) {
        p_original_weather_setType(self, 0, intensity, unknown);
    }
    else {
        p_original_weather_setType(self, type, intensity, unknown);
    }
}
