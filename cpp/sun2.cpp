// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2022 Eicke Herbertz

#include "angle.h"
#include "julian_date.h"
#include "sun.h"

using date::sys_days;
using date::sys_seconds;
using julian_date::julian_century;
using julian_date::julian_day;
using julian_date::julian_days;
using std::optional;
using std::chrono::floor;
using std::chrono::seconds;
using sun::SunTime;

Angle sun_geometric_mean_longitude(julian_century tp) {
    auto t = tp.time_since_epoch().count();
    return Angle::from_deg(fmod(280.46646 + t * (36000.76983 + t * 0.0003032), 360.0));
}

Angle sun_geometric_mean_anomaly(julian_century tp) {
    auto t = tp.time_since_epoch().count();
    return Angle::from_deg(357.52911 + t * (35999.05029 - 0.0001537 * t));
}

double earth_orbit_eccentricity(julian_century tp) {
    auto t = tp.time_since_epoch().count();
    return 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
}

Angle sun_equation_of_center(julian_century tp) {
    auto an = sun_geometric_mean_anomaly(tp);
    auto t = tp.time_since_epoch().count();

    return Angle::from_deg(sin(an) * (1.914602 - t * (0.004817 + 0.000014 * t)) +
                           sin(2 * an) * (0.019993 - 0.000101 * t) + sin(3 * an) * 0.000289);
}

Angle sun_apparent_longitude(julian_century tp) {
    auto sun_true_longitude = sun_geometric_mean_longitude(tp) + sun_equation_of_center(tp);
    auto t = tp.time_since_epoch().count();
    auto a = Angle::from_deg(125.04 - 1934.136 * t);
    return sun_true_longitude - Angle::from_deg(0.00569 + 0.00478 * sin(a));
}

Angle mean_ecliptic_obliquity(julian_century tp) {
    auto t = tp.time_since_epoch().count();
    return Angle::from_deg(23 + (26 + ((21.448 - t * (46.815 + t * (0.00059 - t * 0.001813)))) / 60) / 60);
}

Angle obliquity_correction(julian_century tp) {
    auto mean_obliq = mean_ecliptic_obliquity(tp);
    auto t = tp.time_since_epoch().count();
    auto a = Angle::from_deg(125.04 - 1934.136 * t);
    return mean_obliq + Angle::from_deg(0.00256 * cos(a));
}

Angle sun_declination(julian_century tp) {
    auto al = sun_apparent_longitude(tp);
    auto oc = obliquity_correction(tp);

    return Angle::from_rad(asin(sin(oc) * sin(al)));
}

Angle equation_of_time(julian_century tp) {
    auto oc = obliquity_correction(tp);
    auto I2 = sun_geometric_mean_longitude(tp);
    auto J2 = sun_geometric_mean_anomaly(tp);
    auto K2 = earth_orbit_eccentricity(tp);
    auto y = tan(oc / 2) * tan(oc / 2);

    auto V2 = y * sin(2 * I2) - 2 * K2 * sin(J2) + 4 * K2 * y * sin(J2) * cos(2 * I2) - 0.5 * y * y * sin(4 * I2) -
              1.25 * K2 * K2 * sin(2 * J2);
    return Angle::from_rad(V2);
}

Angle hour_angle(julian_century tp, Angle latitude, Angle elevation) {
    // The original JavaScript code just comments to negate the return value for sunset, which is ugly, so we use
    // copysign() and negated elevation inputs to do that. Inspired by redshift/solar.c.
    auto decli = sun_declination(tp);
    auto omega = acos(cos(elevation) / (cos(latitude) * cos(decli)) - tan(latitude) * tan(decli));
    return Angle::from_rad(copysign(omega, elevation.rad()));
}

static constexpr auto Noon = Angle::from_deg(180);

julian_days time_of_solar_noon(julian_century day, Angle longitude) {
    // First, we approximate the time of local noon via the longitude...
    const auto approx_noon_offset = julian_days{Noon - longitude};
    auto tp = day + approx_noon_offset;

    // ...and calculate the equation of time for that
    auto eq_of_time = equation_of_time(tp);
    tp = day + julian_days{Noon - longitude - eq_of_time};

    // with the new time point, we do a second pass to get the exact result
    eq_of_time = equation_of_time(tp);
    return julian_days{Noon - longitude - eq_of_time};
}

julian_days time_of_solar_elevation(julian_century noon, Angle latitude, Angle longitude, Angle elevation) {
    // We can reuse the computation of actual noon and apply the hour angle from there like the sheet does.
    auto angle = hour_angle(noon, latitude, elevation);
    auto tp = noon + julian_days{angle};

    // Then, with the new time point, we do a second pass to get exact equation of time and hour angle and return
    // the angle as julian days from midnight like we do for noon.
    auto eq_of_time = equation_of_time(tp);
    angle = hour_angle(tp, latitude, elevation);
    return julian_days{Noon - longitude - eq_of_time + angle};
}

static optional<sys_seconds> get_sun_time(Angle latitude, Angle longitude, const sys_days date, const SunTime type) {
    // The requested midnight UTC time point in julian days. This is the mathematical baseline for all the
    // hour angles we will calculate. We have to cast to seconds first to keep the midnight part.
    constexpr auto start_of_julian_century = julian_days{2451545.0};
    const auto j_day = julian_day(julian_date::sys_to_julian(sys_seconds(date))) - start_of_julian_century;

    auto a_noon = time_of_solar_noon(j_day, longitude);
    auto j_noon = j_day + a_noon;
    auto t_noon = date + a_noon;

    if (type == SunTime::Noon) {
        return floor<seconds>(t_noon);
    } else if (type == SunTime::Midnight) {
        return floor<seconds>(t_noon + julian_days(0.5));
    } else if (auto angle = time_of_solar_elevation(j_noon, latitude, longitude, time_angle(type));
               !std::isnan(angle.count())) {
        return floor<seconds>(date + angle);
    } else {
        return std::nullopt;
    }
}

auto sun::get_sun_times2(double latitude, double longitude, date::sys_days date) -> sun_times {
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
