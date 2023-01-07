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
    // Returns the time of solar elevation at a given location and date, or nullopt if that elevation
    // isn't reached there and then. You can use the predefined angles from the SunTimes namespace for
    // the usual twilight angles. This variant may return an event that doesn't actually happen in
    // polar circles (e.g. a last sunset right before the polar day).
    std::optional<date::sys_seconds> get_sun_time(Angle latitude, Angle longitude, date::sys_days date,
                                                  Angle sun_elevation);

    // Returns a filled sun_times struct with all twilight elevation times at a given location and date.
    // Events that don't occur are nullopt. This variant may return an event that doesn't actually happen
    // in polar circles (e.g. a last sunset right before the polar day).
    sun_times get_sun_times(Angle latitude, Angle longitude, date::sys_days date);
}// namespace wiki

namespace noaa {
    // Returns the solar elevation at a given location and time. This is the reverse function of get_sun_time.
    // It has the huge benefit of always returning a value and is useful for applications like dimmers, that really
    // want to depend on this rather than any concrete elevation angles. Redshift also works like this.
    Angle get_sun_elevation(Angle latitude, Angle longitude, date::sys_seconds time_point);

    // Returns the time of solar elevation at a given location and date, or nullopt if that elevation
    // isn't reached there and then. You can use the predefined angles from the SunTimes namespace for
    // the usual twilight angles. This variant seems to be reliable even for polar regions.
    std::optional<date::sys_seconds> get_sun_time(Angle latitude, Angle longitude, date::sys_days date,
                                                  Angle sun_elevation);

    // Returns a filled sun_times struct with all twilight elevation times at a given location and date.
    // Events that don't occur are nullopt. This variant seems to be reliable even for polar regions.
    sun_times get_sun_times(Angle latitude, Angle longitude, date::sys_days date);

    // Returns a filled sun_times struct with all twilight elevation times at a given location and date.
    // Events that don't occur are nullopt. Differs from get_sun_times only in being optimized to reuse
    // some calculations and run slightly faster.
    sun_times get_sun_times_opt(Angle latitude, Angle longitude, date::sys_days date);
}// namespace noaa

// Returns a filled sun_times struct with all twilight elevation times at a given location and date.
// Events that don't occur are nullopt. This variant calls the solar code from redshift.
sun_times get_sun_times_c(Angle latitude, Angle longitude, date::sys_days date);

// Returns a filled sun_times struct with all twilight elevation times at a given location and date.
// Events that don't occur are nullopt. This variant calls the NOAA rust implementation.
sun_times get_sun_times_rust(Angle latitude, Angle longitude, date::sys_days date);
}// namespace sun
