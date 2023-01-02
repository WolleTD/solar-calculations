// basically by Howard Hinnant, from here: https://stackoverflow.com/a/33964462

#ifndef SOLAR_CALCULATIONS_JULIAN_DATE_H
#define SOLAR_CALCULATIONS_JULIAN_DATE_H

#include <date/date.h>

namespace julian_date {
constexpr auto jdiff() {
    using namespace date;
    using namespace std::chrono_literals;
    return sys_days{January / 1 / 1970} - (sys_days{November / 24 / -4713} + 12h);
}

struct julian_clock {
    using rep = double;
    using period = std::ratio<86400>;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<julian_clock>;

    static constexpr bool is_steady = false;

    static time_point now() noexcept {
        auto sys_now = std::chrono::system_clock::now();
        return time_point{duration{sys_now.time_since_epoch()} + jdiff()};
    }
};

template<class Duration>
using julian_time = std::chrono::time_point<julian_clock, Duration>;

using julian_days = julian_clock::duration;
using julian_centuries = std::chrono::duration<double, std::ratio<86400 * 36525L>>;

using julian_day = julian_clock::time_point;
using julian_century = julian_time<julian_centuries>;

template<class Duration>
constexpr auto sys_to_julian(date::sys_time<Duration> tp) noexcept {
    static_assert(julian_clock::duration{jdiff()} < Duration::max(), "Overflow in sys_to_julian");
    const auto d = tp.time_since_epoch() + jdiff();
    return julian_time<std::remove_cv_t<decltype(d)>>{d};
}

static_assert(std::is_same_v<std::chrono::hours, decltype(sys_to_julian(date::sys_days{}))::duration>);

template<class Duration>
constexpr auto julian_to_sys(julian_time<Duration> tp) noexcept {
    static_assert(julian_clock::duration{-jdiff()} > Duration::min(), "Overflow in julian_to_sys");
    const auto d = tp.time_since_epoch() - jdiff();
    return date::sys_time<std::remove_cv_t<decltype(d)>>{d};
}
}// namespace julian_date

#endif//SOLAR_CALCULATIONS_JULIAN_DATE_H
