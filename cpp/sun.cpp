// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2022 Eicke Herbertz

#include "sun.h"
#include "angle.h"
#include "julian_date.h"

using date::days;
using date::sys_days;
using date::sys_seconds;
using julian_date::julian_day;
using julian_date::julian_days;
using std::optional;
using std::chrono::ceil;
using std::chrono::floor;
using std::chrono::seconds;
using sun::SunTime;

static constexpr auto astroTwilightElev = -18.0;
static constexpr auto nautTwilightElev = -12.0;
static constexpr auto civilTwilightElev = -6.0;
static constexpr auto daytimeElev = -0.833;

Angle sun::time_angle(SunTime time_type) {
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

// This is basically a step-by-step implementation of the calculations described in this Wikipedia article:
// https://en.wikipedia.org/wiki/Sunrise_equation#Complete_calculation_on_Earth

constexpr auto start_of_jul_century = julian_days{2451545.0};

static julian_day to_julian_day(date::sys_seconds date) {
    return ceil<days>(julian_date::sys_to_julian(date) - start_of_jul_century + julian_days(0.0008));
}

static julian_day mean_solar_time(julian_day days, Angle longitude) {
    return days - julian_days(longitude);
}

static Angle solar_mean_anomaly(julian_day mean_solar_time) {
    return Angle::from_deg(fmod((357.5291 + 0.98560028 * mean_solar_time.time_since_epoch().count()), 360.0));
}

static Angle equation_of_the_center(Angle mean_anomaly) {
    auto C1 = 1.9148 * sin(mean_anomaly);
    auto C2 = 0.0200 * sin(2 * mean_anomaly);
    auto C3 = 0.0003 * sin(3 * mean_anomaly);
    return Angle::from_deg(C1 + C2 + C3);
}

static Angle ecliptic_longitude(Angle mean_anomaly) {
    auto eqc = equation_of_the_center(mean_anomaly);
    return Angle::from_deg(fmod(mean_anomaly.deg() + eqc.deg() + 180 + 102.9372, 360.0));
}

static julian_day solar_transit(julian_day mean_solar_time, Angle mean_anomaly, Angle ecliptic_longitude) {
    auto eq_time = julian_days(0.0053 * sin(mean_anomaly) - 0.0069 * sin(2 * ecliptic_longitude));
    return start_of_jul_century + mean_solar_time + eq_time;
}

static Angle declination_of_the_sun(Angle ecliptic_longitude) {
    constexpr auto axial_tilt = Angle::from_deg(23.44);
    return Angle::from_rad(asin(sin(ecliptic_longitude) * sin(axial_tilt)));
}

// This is the only one not exactly matching the article, because I already extended it to work
// with different angles for the twilight times.
static Angle hour_angle(Angle latitude, Angle declination, Angle time_angle) {
    auto num = cos(time_angle) - sin(latitude) * sin(declination);
    auto den = cos(latitude) * cos(declination);

    auto omega = acos(num / den);
    return Angle::from_rad(copysign(omega, time_angle.rad()));
}

static optional<sys_seconds> get_sun_time(Angle latitude, Angle longitude, sys_days date, SunTime type) {
    auto j_day = to_julian_day(date);
    auto mst = mean_solar_time(j_day, longitude);
    auto sma = solar_mean_anomaly(mst);
    auto el = ecliptic_longitude(sma);
    auto declination = declination_of_the_sun(el);

    auto true_noon = solar_transit(mst, sma, el);

    julian_day result = true_noon;

    if (type == SunTime::Midnight) {
        result += julian_days(0.5);
    } else if (type != SunTime::Noon) {
        auto hourAngle = hour_angle(latitude, declination, time_angle(type));
        result += julian_days(hourAngle.deg() / 360.0);
    }

    if (std::isnan(result.time_since_epoch().count())) {
        return std::nullopt;
    } else {
        return floor<seconds>(julian_to_sys(result));
    }
}

auto sun::get_sun_times(double latitude, double longitude, date::sys_days date) -> sun_times {
    auto lat = Angle::from_deg(latitude);
    auto lon = Angle::from_deg(longitude);
    return {
            get_sun_time(lat, lon, date, SunTime::Noon).value(),
            get_sun_time(lat, lon, date, SunTime::Midnight).value(),
            get_sun_time(lat, lon, date, SunTime::AstroDawn),
            get_sun_time(lat, lon, date, SunTime::NautDawn),
            get_sun_time(lat, lon, date, SunTime::CivilDawn),
            get_sun_time(lat, lon, date, SunTime::Sunrise),
            get_sun_time(lat, lon, date, SunTime::Sunset),
            get_sun_time(lat, lon, date, SunTime::CivilDusk),
            get_sun_time(lat, lon, date, SunTime::NautDusk),
            get_sun_time(lat, lon, date, SunTime::AstroDusk),
    };
}
