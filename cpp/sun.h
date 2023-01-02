// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2022 Eicke Herbertz

#include "date/date.h"
#include "date/tz.h"
#include <chrono>
#include <optional>

struct Angle;

namespace sun {

enum class SunTime {
    Noon,
    Midnight,
    AstroDawn,
    NautDawn,
    CivilDawn,
    Sunrise,
    Sunset,
    CivilDusk,
    NautDusk,
    AstroDusk,
};

Angle time_angle(SunTime time_type);

struct sun_times {
    date::sys_seconds noon;
    date::sys_seconds midnight;
    std::optional<date::sys_seconds> astro_dawn;
    std::optional<date::sys_seconds> naut_dawn;
    std::optional<date::sys_seconds> civil_dawn;
    std::optional<date::sys_seconds> sunrise;
    std::optional<date::sys_seconds> sunset;
    std::optional<date::sys_seconds> civil_dusk;
    std::optional<date::sys_seconds> naut_dusk;
    std::optional<date::sys_seconds> astro_dusk;
};

sun_times get_sun_times(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times2(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times3(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times_c(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times_rust(double latitude, double longitude, date::sys_days date);
}// namespace sun
