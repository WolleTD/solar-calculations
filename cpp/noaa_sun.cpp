// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2022-2023 Eicke Herbertz

#include "angle.h"
#include "julian_date.h"
#include "rust_sun_ffi.h"
#include "sun.h"

using date::sys_days;
using date::sys_seconds;
using julian_date::julian_century;
using julian_date::julian_day;
using julian_date::julian_days;
using std::optional;
using std::chrono::floor;
using std::chrono::seconds;

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

Angle elevation_from_hour_angle(julian_century tp, Angle latitude, Angle hour_angle) {
    auto decli = sun_declination(tp);
    auto elev = acos(cos(hour_angle) * cos(latitude) * cos(decli) + sin(latitude) * sin(decli));
    return Angle::from_rad(elev);
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

Angle sun::noaa::get_sun_elevation(Angle latitude, Angle longitude, date::sys_seconds time_point) {
    constexpr auto start_of_julian_century = julian_days{2451545.0};
    const auto j_tp = julian_day(julian_date::sys_to_julian(time_point)) - start_of_julian_century;

    auto date = floor<date::days>(time_point);
    auto minutes_from_midnight = Angle(julian_days(time_point - date));
    auto eq_of_time = equation_of_time(j_tp);

    auto angle = longitude + eq_of_time + minutes_from_midnight - Noon;
    return elevation_from_hour_angle(j_tp, latitude, angle);
}

optional<sys_seconds> sun::noaa::get_sun_time(Angle latitude, Angle longitude, sys_days date, Angle elevation) {
    // The requested midnight UTC time point in julian days. This is the mathematical baseline for all the
    // hour angles we will calculate. We have to cast to seconds first to keep the midnight part.
    constexpr auto start_of_julian_century = julian_days{2451545.0};
    const auto j_day = julian_day(julian_date::sys_to_julian(sys_seconds(date))) - start_of_julian_century;

    auto a_noon = time_of_solar_noon(j_day, longitude);
    auto j_noon = j_day + a_noon;
    auto t_noon = date + a_noon;

    if (elevation == SunTime::Noon) {
        return floor<seconds>(t_noon);
    } else if (elevation == SunTime::Midnight) {
        return floor<seconds>(t_noon + julian_days(0.5));
    } else if (auto angle = time_of_solar_elevation(j_noon, latitude, longitude, elevation);
               !std::isnan(angle.count())) {
        return floor<seconds>(date + angle);
    } else {
        return std::nullopt;
    }
}

auto sun::noaa::get_sun_times(Angle lat, Angle lon, date::sys_days date) -> sun_times {
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

auto sun::noaa::get_sun_times_opt(Angle lat, Angle lon, date::sys_days date) -> sun_times {
    // The requested midnight UTC time point in julian days. This is the mathematical baseline for all the
    // hour angles we will calculate. We have to cast to seconds first to keep the midnight part.
    constexpr auto start_of_julian_century = julian_days{2451545.0};
    const auto j_day = julian_day(julian_date::sys_to_julian(sys_seconds(date))) - start_of_julian_century;

    sun_times res{};

    auto a_noon = time_of_solar_noon(j_day, lon);
    auto j_noon = j_day + a_noon;
    auto t_noon = date + a_noon;

    res.noon = floor<seconds>(t_noon);
    res.midnight = floor<seconds>(t_noon + julian_days(0.5));

    auto get_time = [&](Angle elevation) -> optional<sys_seconds> {
        auto angle = time_of_solar_elevation(j_noon, lat, lon, elevation);
        if (!std::isnan(angle.count())) {
            return floor<seconds>(date + angle);
        } else {
            return std::nullopt;
        }
    };

    res.astro_dawn = get_time(SunTime::AstroDawn);
    res.naut_dawn = get_time(SunTime::NautDawn);
    res.civil_dawn = get_time(SunTime::CivilDawn);
    res.sunrise = get_time(SunTime::Sunrise);
    res.sunset = get_time(SunTime::Sunset);
    res.civil_dusk = get_time(SunTime::CivilDusk);
    res.naut_dusk = get_time(SunTime::NautDusk);
    res.astro_dusk = get_time(SunTime::AstroDusk);

    return res;
}

auto sun::get_sun_times_rust(Angle latitude, Angle longitude, date::sys_days date) -> sun_times {
    auto tp = sys_seconds(date).time_since_epoch().count();
    auto res = get_sun_times_r(latitude.deg(), longitude.deg(), tp);
    auto map = [](int64_t tp) -> optional<sys_seconds> {
        if (tp) return sys_seconds(seconds(tp));
        else
            return std::nullopt;
    };
    return {
            sys_seconds(seconds(res.noon)),
            sys_seconds(seconds(res.midnight)),
            map(res.astro_dawn),
            map(res.naut_dawn),
            map(res.civil_dawn),
            map(res.sunrise),
            map(res.sunset),
            map(res.civil_dusk),
            map(res.naut_dusk),
            map(res.astro_dusk),
    };
}
