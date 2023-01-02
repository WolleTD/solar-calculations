#ifndef SOLAR_CALCULATIONS_FFI_H
#define SOLAR_CALCULATIONS_FFI_H

extern "C" {
#include <stdint.h>

struct sun_times_r {
    int64_t noon;
    int64_t midnight;
    int64_t astro_dawn;
    int64_t naut_dawn;
    int64_t civil_dawn;
    int64_t sunrise;
    int64_t sunset;
    int64_t civil_dusk;
    int64_t naut_dusk;
    int64_t astro_dusk;
};

sun_times_r get_sun_times_r(double latitude, double longitude, int64_t date);
}

#endif//SOLAR_CALCULATIONS_FFI_H
