#ifndef SOLAR_CALCULATIONS_ANGLE_H
#define SOLAR_CALCULATIONS_ANGLE_H

#include "julian_date.h"
#include <cmath>

struct Angle {
    [[nodiscard]] constexpr double deg() const { return value * (180.0 / M_PI); }
    [[nodiscard]] constexpr double rad() const { return value; }

    static constexpr Angle from_rad(double rad) { return Angle(rad); }
    static constexpr Angle from_deg(double deg) { return Angle(deg * (M_PI / 180.0)); }

    explicit operator julian_date::julian_days() const { return julian_date::julian_days{value / (2 * M_PI)}; }

private:
    explicit constexpr Angle(double rad) : value(rad) {}

    double value;
};

inline auto sin(Angle a) -> double { return ::sin(a.rad()); }
inline auto cos(Angle a) -> double { return ::cos(a.rad()); }
inline auto tan(Angle a) -> double { return ::tan(a.rad()); }

inline auto operator*(double lhs, Angle rhs) -> Angle { return Angle::from_rad(lhs * rhs.rad()); }

inline auto operator*(Angle lhs, double rhs) -> Angle { return Angle::from_rad(lhs.rad() * rhs); }

inline auto operator/(Angle lhs, double rhs) -> Angle { return Angle::from_rad(lhs.rad() / rhs); }

inline auto operator+(Angle lhs, Angle rhs) -> Angle { return Angle::from_rad(lhs.rad() + rhs.rad()); }

inline auto operator-(Angle lhs, Angle rhs) -> Angle { return Angle::from_rad(lhs.rad() - rhs.rad()); }

static_assert(Angle::from_rad(0).deg() == 0);
static_assert(Angle::from_deg(90).rad() == M_PI / 2);
static_assert(Angle::from_rad(M_PI).deg() == 180);
static_assert(Angle::from_deg(360).rad() == 2 * M_PI);

#endif//SOLAR_CALCULATIONS_ANGLE_H
