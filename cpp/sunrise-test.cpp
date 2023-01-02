// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Eicke Herbertz

#include "sun.h"
#include <chrono>
#include <date/date.h>
#include <fmt/format.h>

using date::days;
using date::local_days;
using date::local_seconds;
using date::sys_seconds;
using date::zoned_seconds;
using std::optional;
using std::chrono::duration_cast;
using std::chrono::floor;
using std::chrono::seconds;
using std::chrono::system_clock;

using date::operator""_y;
using namespace std::chrono_literals;

int main() {
    const std::vector<local_seconds> dates = {
            local_days(2022_y / 10 / 15) + 4h,  local_days(2022_y / 10 / 15) + 12h, local_days(2022_y / 10 / 15) + 20h,
            local_days(2022_y / 10 / 15) + 22h, local_days(2022_y / 10 / 15) + 23h, local_days(2022_y / 10 / 16),
            local_days(2022_y / 10 / 17),       local_days(2022_y / 10 / 18),       local_days(2022_y / 10 / 19),
            local_days(2022_y / 10 / 20),       local_days(2022_y / 10 / 21),       local_days(2022_y / 10 / 22)};

    struct Location {
        double latitude;
        double longitude;
        std::string_view zone;
    };
    const std::vector<Location> locations = {
            {-78.463889, 106.83757, "Antarctica/Vostok"},// +0600
    };

    auto print_time = [](auto str, optional<zoned_seconds> time, optional<zoned_seconds> rs_time) {
        std::string my_str;
        std::string rs_str;
        if (time) {
            my_str = date::format("%c %z", *time);
        } else {
            my_str = "       does not happen        ";
        }
        if (rs_time) {
            rs_str = date::format("%c %z", *rs_time);
        } else {
            rs_str = "       does not happen        ";
        }
        fmt::print("{}: {} | {} UTC\n", str, my_str, rs_str);
    };

    for (auto loc: locations) {
        fmt::print("Zone: {}\n", loc.zone);
        for (auto local_date: dates) {
            auto lat = loc.latitude;
            auto lon = loc.longitude;
            auto date = zoned_seconds{loc.zone, local_date};

            // To get the correct UTC day for a given local time, we have to add the timezone offset
            // before flooring for days. This is purely for humans though.
            auto offset = (date.get_info().offset > 12h) ? date.get_info().offset - 24h : date.get_info().offset;
            auto utc_date = floor<days>(date.get_sys_time() + offset);

            auto times = sun::get_sun_times2(lat, lon, utc_date);
            auto times2 = sun::get_sun_times_c(lat, lon, utc_date);

            auto map_zone = [&loc](optional<sys_seconds> tp) -> optional<zoned_seconds> {
                if (tp.has_value()) return zoned_seconds(loc.zone, tp.value());
                else
                    return std::nullopt;
            };

            fmt::print("==== check at {} ====\n", date::format("%c %z", date));
            print_time(" a. dawn", map_zone(times.astro_dawn), map_zone(times2.astro_dawn));
            print_time(" n. dawn", map_zone(times.naut_dawn), map_zone(times2.naut_dawn));
            print_time(" c. dawn", map_zone(times.civil_dawn), map_zone(times2.civil_dawn));
            print_time(" sunrise", map_zone(times.sunrise), map_zone(times2.sunrise));
            print_time("    noon", map_zone(times.noon), map_zone(times2.noon));
            print_time("  sunset", map_zone(times.sunset), map_zone(times2.sunset));
            print_time(" c. dusk", map_zone(times.civil_dusk), map_zone(times2.civil_dusk));
            print_time(" n. dusk", map_zone(times.naut_dusk), map_zone(times2.naut_dusk));
            print_time(" a. dusk", map_zone(times.astro_dusk), map_zone(times2.astro_dusk));
            print_time("midnight", map_zone(times.midnight), map_zone(times2.midnight));
        }
    }

    auto lat = 52.02182;
    auto lon = 8.53509;
    auto date = date::zoned_time(date::current_zone(), floor<seconds>(system_clock::now()));
    auto utc_date = floor<days>(date.get_sys_time());

    auto times = sun::get_sun_times2(lat, lon, utc_date);
    auto times2 = sun::get_sun_times_c(lat, lon, utc_date);

    auto map_zone = [](optional<sys_seconds> tp) -> optional<zoned_seconds> {
        if (tp.has_value()) return zoned_seconds(date::current_zone(), tp.value());
        else
            return std::nullopt;
    };

    fmt::print("Bielefeld, today\n");
    fmt::print("==== check at {} ====\n", date::format("%c %z", date));
    print_time(" a. dawn", map_zone(times.astro_dawn), map_zone(times2.astro_dawn));
    print_time(" n. dawn", map_zone(times.naut_dawn), map_zone(times2.naut_dawn));
    print_time(" c. dawn", map_zone(times.civil_dawn), map_zone(times2.civil_dawn));
    print_time(" sunrise", map_zone(times.sunrise), map_zone(times2.sunrise));
    print_time("    noon", map_zone(times.noon), map_zone(times2.noon));
    print_time("  sunset", map_zone(times.sunset), map_zone(times2.sunset));
    print_time(" c. dusk", map_zone(times.civil_dusk), map_zone(times2.civil_dusk));
    print_time(" n. dusk", map_zone(times.naut_dusk), map_zone(times2.naut_dusk));
    print_time(" a. dusk", map_zone(times.astro_dusk), map_zone(times2.astro_dusk));
    print_time("midnight", map_zone(times.midnight), map_zone(times2.midnight));
}
