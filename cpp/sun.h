// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2022-2023 Eicke Herbertz

#include "angle.h"
#include <chrono>
#include <date/date.h>
#include <optional>

namespace sun {

static constexpr auto astroTwilightElev = -18.0;
static constexpr auto nautTwilightElev = -12.0;
static constexpr auto civilTwilightElev = -6.0;
static constexpr auto daytimeElev = -0.833;

// Predefined sun elevation angles for useful time points. Noon and Midnight aren't real,
// they are special values used by the code.
namespace SunTime {
    static constexpr auto Noon = Angle::from_deg(0.0);
    static constexpr auto Midnight = Angle::from_deg(180.0);
    static constexpr auto AstroDawn = Angle::from_deg(-90.0 + astroTwilightElev);
    static constexpr auto NautDawn = Angle::from_deg(-90.0 + nautTwilightElev);
    static constexpr auto CivilDawn = Angle::from_deg(-90.0 + civilTwilightElev);
    static constexpr auto Sunrise = Angle::from_deg(-90.0 + daytimeElev);
    static constexpr auto Sunset = Angle::from_deg(90.0 - daytimeElev);
    static constexpr auto CivilDusk = Angle::from_deg(90.0 - civilTwilightElev);
    static constexpr auto NautDusk = Angle::from_deg(90.0 - nautTwilightElev);
    static constexpr auto AstroDusk = Angle::from_deg(90.0 - astroTwilightElev);
}// namespace SunTime

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

namespace wiki {
    std::optional<date::sys_seconds> get_sun_time(Angle latitude, Angle longitude, date::sys_days date,
                                                  Angle sun_elevation);
    sun_times get_sun_times(Angle latitude, Angle longitude, date::sys_days date);
}// namespace wiki

namespace noaa {
    std::optional<date::sys_seconds> get_sun_time(Angle latitude, Angle longitude, date::sys_days date,
                                                  Angle sun_elevation);
    sun_times get_sun_times(Angle latitude, Angle longitude, date::sys_days date);
    sun_times get_sun_times_opt(Angle latitude, Angle longitude, date::sys_days date);
}// namespace noaa

sun_times get_sun_times_c(Angle latitude, Angle longitude, date::sys_days date);
sun_times get_sun_times_rust(Angle latitude, Angle longitude, date::sys_days date);
}// namespace sun
