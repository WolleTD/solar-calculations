use crate::{angle::Angle, julian::{JulianCentury, JulianDay}};

fn sun_geometric_mean_longitude(tp: JulianCentury) -> Angle {
    let t = tp.0;
    Angle::from_deg(280.46646 + t * (36000.76983 + t * 0.0003032) % 360.0)
}

fn sun_geometric_mean_anomaly(tp: JulianCentury) -> Angle {
    let t = tp.0;
    Angle::from_deg(357.52911 + t * (35999.05029 - 0.0001537 * t))
}

fn earth_orbit_eccentricity(tp: JulianCentury) -> f64 {
    let t = tp.0;
    0.016708634 - t * (0.000042037 + 0.0000001267 * t)
}

fn sun_equation_of_center(tp: JulianCentury) -> Angle {
    let an = sun_geometric_mean_anomaly(tp);
    let t = tp.0;

    Angle::from_deg(
        an.sin() * (1.914602 - t * (0.004817 + 0.000014 * t))
            + (2.0 * an).sin() * (0.019993 - 0.000101 * t)
            + (3.0 * an).sin() * 0.000289,
    )
}

fn sun_apparent_longitude(tp: JulianCentury) -> Angle {
    let sun_true_longitude = sun_geometric_mean_longitude(tp) + sun_equation_of_center(tp);
    let t = tp.0;
    let a = Angle::from_deg(125.04 - 1934.136 * t);
    sun_true_longitude - Angle::from_deg(0.00569 + 0.00478 * a.sin())
}

fn mean_ecliptic_obliquity(tp: JulianCentury) -> Angle {
    let t = tp.0;
    Angle::from_deg(
        23.0 + (26.0 + (21.448 - t * (46.815 + t * (0.00059 - t * 0.001813))) / 60.0) / 60.0,
    )
}

fn obliquity_correction(tp: JulianCentury) -> Angle {
    let mean_obliq = mean_ecliptic_obliquity(tp);
    let t = tp.0;
    let a = Angle::from_deg(125.04 - 1934.136 * t);
    mean_obliq + Angle::from_deg(0.00256 * a.cos())
}

fn sun_declination(tp: JulianCentury) -> Angle {
    let al = sun_apparent_longitude(tp);
    let oc = obliquity_correction(tp);

    return Angle::from_rad((oc.sin() * al.sin()).asin());
}

fn equation_of_time(tp: JulianCentury) -> Angle {
    let oc = obliquity_correction(tp);
    let ml = sun_geometric_mean_longitude(tp);
    let ma = sun_geometric_mean_anomaly(tp);
    let oe = earth_orbit_eccentricity(tp);
    let y = (oc / 2.0).tan() * (oc / 2.0).tan();

    let et = y * (2.0 * ml).sin() - 2.0 * oe * ma.sin()
        + 4.0 * oe * y * ma.sin() * (2.0 * ml).cos()
        - 0.5 * y * y * (4.0 * ml).sin()
        - 1.25 * oe * oe * (2.0 * ma).sin();

    Angle::from_rad(et)
}

fn hour_angle(tp: JulianCentury, latitude: Angle, elevation: Angle) -> Angle {
    // The original JavaScript code just comments to negate the return value for sunset, which is ugly, so we use
    // copysign() and negated elevation inputs to do that. Inspired by redshift/solar.c.
    let decli = sun_declination(tp);
    let omega =
        (elevation.cos() / (latitude.cos() * decli.cos()) - latitude.tan() * decli.tan()).acos();
    Angle::from_rad(omega.copysign(-elevation.rad()))
}

const NOON: Angle = Angle::from_rad(std::f64::consts::PI);

pub(crate) fn time_of_solar_noon(day: JulianCentury, longitude: Angle) -> JulianDay {
    // First, we approximate the time of local noon via the longitude...
    let approx_noon_offset = JulianDay::from(NOON - longitude);
    let tp = day + approx_noon_offset;

    // ...and calculate the equation of time for that
    let eq_of_time = equation_of_time(tp);
    let tp = day + JulianDay::from(NOON - longitude - eq_of_time);

    // with the new time point, we do a second pass to get the exact result
    let eq_of_time = equation_of_time(tp);
    JulianDay::from(NOON - longitude - eq_of_time)
}

pub(crate) fn time_of_solar_elevation(
    noon: JulianCentury,
    latitude: Angle,
    longitude: Angle,
    elevation: Angle,
) -> JulianDay {
    // We can reuse the computation of actual noon and apply the hour angle from there like the sheet does.
    let angle = hour_angle(noon, latitude, elevation);
    let tp = noon - JulianDay::from(angle);

    // Then, with the new time point, we do a second pass to get exact equation of time and hour angle and return
    // the angle as julian days from midnight like we do for noon.
    let eq_of_time = equation_of_time(tp);
    let angle = hour_angle(tp, latitude, elevation);
    JulianDay::from(NOON - longitude - eq_of_time - angle)
}
