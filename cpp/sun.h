// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2022-2023 Eicke Herbertz

#include "angle.h"
#include <chrono>
#include <date/date.h>
#include <optional>

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

static constexpr auto astroTwilightElev = -18.0;
static constexpr auto nautTwilightElev = -12.0;
static constexpr auto civilTwilightElev = -6.0;
static constexpr auto daytimeElev = -0.833;

inline Angle time_angle(SunTime time_type) {
    switch (time_type) {
        case SunTime::AstroDawn:
            return Angle::from_deg(-90.0 + astroTwilightElev);
        case SunTime::NautDawn:
            return Angle::from_deg(-90.0 + nautTwilightElev);
        case SunTime::CivilDawn:
            return Angle::from_deg(-90.0 + civilTwilightElev);
        case SunTime::Sunrise:
            return Angle::from_deg(-90.0 + daytimeElev);
        case SunTime::Sunset:
            return Angle::from_deg(90.0 - daytimeElev);
        case SunTime::CivilDusk:
            return Angle::from_deg(90.0 - civilTwilightElev);
        case SunTime::NautDusk:
            return Angle::from_deg(90.0 - nautTwilightElev);
        case SunTime::AstroDusk:
            return Angle::from_deg(90.0 - astroTwilightElev);
        default:
            throw std::invalid_argument("Invalid SunTime");
    }
}

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

sun_times get_sun_times_wiki(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times_noaa(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times_noaa_opt(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times_c(double latitude, double longitude, date::sys_days date);
sun_times get_sun_times_rust(double latitude, double longitude, date::sys_days date);
}// namespace sun
