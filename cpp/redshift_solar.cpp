// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Eicke Herbertz

#include "sun.h"

extern "C" {
#include "redshift_solar.h"
}

using date::sys_seconds;
using std::chrono::seconds;
using std::optional;

auto sun::get_sun_times_c(double latitude, double longitude, date::sys_days date) -> sun_times {
    double res[SOLAR_TIME_MAX];
    auto d_epoch = static_cast<double>(std::chrono::duration_cast<seconds>(date.time_since_epoch()).count());
    solar_table_fill(d_epoch, latitude, longitude, res);
    auto map = [](double tp) -> optional<sys_seconds> {
        if (!std::isnan(tp)) return sys_seconds(seconds(static_cast<size_t>(tp)));
        else
            return std::nullopt;
    };
    return {
            sys_seconds(seconds(static_cast<size_t>(res[SOLAR_TIME_NOON]))),
            sys_seconds(seconds(static_cast<size_t>(res[SOLAR_TIME_MIDNIGHT]))),
            map(res[SOLAR_TIME_ASTRO_DAWN]),
            map(res[SOLAR_TIME_NAUT_DAWN]),
            map(res[SOLAR_TIME_CIVIL_DAWN]),
            map(res[SOLAR_TIME_SUNRISE]),
            map(res[SOLAR_TIME_SUNSET]),
            map(res[SOLAR_TIME_CIVIL_DUSK]),
            map(res[SOLAR_TIME_NAUT_DUSK]),
            map(res[SOLAR_TIME_ASTRO_DUSK]),
    };
}
