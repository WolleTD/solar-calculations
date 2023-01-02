use chrono::{DateTime, Duration, NaiveDate, NaiveDateTime, NaiveTime, TimeZone, Utc};
use angle::Angle;
use astro::{time_of_solar_elevation, time_of_solar_noon};
use julian::{JulianCentury, JulianDay};

mod angle;
mod astro;
mod julian;

enum SunTime {
    Noon,
    Midnight,
    AstroDawn,
    NautDawn,
    CivilDawn,
    Sunrise,
    Sunset,
    CivilDusk,
    NautDusk,
    AstroDusk,
}

pub struct SunTimes {
    pub noon: DateTime<Utc>,
    pub midnight: DateTime<Utc>,
    pub astro_dawn: Option<DateTime<Utc>>,
    pub naut_dawn: Option<DateTime<Utc>>,
    pub civil_dawn: Option<DateTime<Utc>>,
    pub sunrise: Option<DateTime<Utc>>,
    pub sunset: Option<DateTime<Utc>>,
    pub civil_dusk: Option<DateTime<Utc>>,
    pub naut_dusk: Option<DateTime<Utc>>,
    pub astro_dusk: Option<DateTime<Utc>>,
}

#[repr(C)]
pub struct SunTimesC {
    pub noon: i64,
    pub midnight: i64,
    pub astro_dawn: i64,
    pub naut_dawn: i64,
    pub civil_dawn: i64,
    pub sunrise: i64,
    pub sunset: i64,
    pub civil_dusk: i64,
    pub naut_dusk: i64,
    pub astro_dusk: i64,
}

const ASTRO_TWILIGHT_ELEV: f64 = -18.0;
const NAUT_TWILIGHT_ELEV: f64 = -12.0;
const CIVIL_TWILIGHT_ELEV: f64 = -6.0;
const DAYTIME_ELEV: f64 = -0.833;

fn time_angle(time_type: SunTime) -> Angle {
    match time_type {
        SunTime::AstroDawn => Angle::from_deg(-90.0 + ASTRO_TWILIGHT_ELEV),
        SunTime::NautDawn => Angle::from_deg(-90.0 + NAUT_TWILIGHT_ELEV),
        SunTime::CivilDawn => return Angle::from_deg(-90.0 + CIVIL_TWILIGHT_ELEV),
        SunTime::Sunrise => return Angle::from_deg(-90.0 + DAYTIME_ELEV),
        SunTime::Sunset => return Angle::from_deg(90.0 - DAYTIME_ELEV),
        SunTime::CivilDusk => return Angle::from_deg(90.0 - CIVIL_TWILIGHT_ELEV),
        SunTime::NautDusk => return Angle::from_deg(90.0 - NAUT_TWILIGHT_ELEV),
        SunTime::AstroDusk => return Angle::from_deg(90.0 - ASTRO_TWILIGHT_ELEV),
        _ => panic!("Invalid time angle"),
    }
}

fn get_sun_time(
    latitude: Angle,
    longitude: Angle,
    date: NaiveDate,
    ty: SunTime,
) -> Option<DateTime<Utc>> {
    // The requested midnight UTC time point in julian days. This is the mathematical baseline for all the
    // hour angles we will calculate. We have to cast to seconds first to keep the midnight part.
    let start_of_julian_century = JulianDay(2451545.0);
    let j_day = JulianCentury::from_date(date) - start_of_julian_century;

    let date = date.and_time(NaiveTime::default());
    let date = Utc.from_utc_datetime(&date);
    let a_noon = time_of_solar_noon(j_day, longitude);
    let j_noon = j_day + a_noon;
    let t_noon = date + Duration::from(a_noon);

    match ty {
        SunTime::Noon => Some(t_noon),
        SunTime::Midnight => Some(t_noon + Duration::hours(12)),
        ty => {
            let angle = time_of_solar_elevation(j_noon, latitude, longitude, time_angle(ty));
            if !angle.0.is_nan() {
                Some(date + Duration::from(angle))
            } else {
                None
            }
        }
    }
}

pub fn get_sun_times2(latitude: f64, longitude: f64, date: NaiveDate) -> SunTimes {
    let lat = Angle::from_deg(latitude);
    let lon = Angle::from_deg(longitude);
    SunTimes {
        noon: get_sun_time(lat, lon, date, SunTime::Noon).unwrap(),
        midnight: get_sun_time(lat, lon, date, SunTime::Midnight).unwrap(),
        astro_dawn: get_sun_time(lat, lon, date, SunTime::AstroDawn),
        naut_dawn: get_sun_time(lat, lon, date, SunTime::NautDawn),
        civil_dawn: get_sun_time(lat, lon, date, SunTime::CivilDawn),
        sunrise: get_sun_time(lat, lon, date, SunTime::Sunrise),
        sunset: get_sun_time(lat, lon, date, SunTime::Sunset),
        civil_dusk: get_sun_time(lat, lon, date, SunTime::CivilDusk),
        naut_dusk: get_sun_time(lat, lon, date, SunTime::NautDusk),
        astro_dusk: get_sun_time(lat, lon, date, SunTime::AstroDusk),
    }
}

#[no_mangle]
pub extern "C" fn get_sun_times_r(latitude: f64, longitude: f64, tp: i64) -> SunTimesC {
    let date = NaiveDateTime::from_timestamp_opt(tp, 0).unwrap().date();
    let res = get_sun_times2(latitude, longitude, date);

    SunTimesC {
        noon: res.noon.timestamp(),
        midnight: res.midnight.timestamp(),
        astro_dawn: res.astro_dawn.unwrap_or_default().timestamp(),
        naut_dawn: res.naut_dawn.unwrap_or_default().timestamp(),
        civil_dawn: res.civil_dawn.unwrap_or_default().timestamp(),
        sunrise: res.sunrise.unwrap_or_default().timestamp(),
        sunset: res.sunset.unwrap_or_default().timestamp(),
        civil_dusk: res.civil_dusk.unwrap_or_default().timestamp(),
        naut_dusk: res.naut_dusk.unwrap_or_default().timestamp(),
        astro_dusk: res.astro_dusk.unwrap_or_default().timestamp(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use chrono::{
        naive::{NaiveDate, NaiveDateTime, NaiveTime},
        DateTime, FixedOffset, Local, TimeZone, Utc,
    };

    fn make_tp(year: i32, month: u32, day: u32, hour: u32) -> NaiveDateTime {
        let date = NaiveDate::from_ymd_opt(year, month, day).unwrap();
        let time = NaiveTime::from_hms_opt(hour, 0, 0).unwrap();
        NaiveDateTime::new(date, time)
    }

    #[test]
    fn it_works() {
        let dates = vec![
            make_tp(2022, 10, 15, 4),
            make_tp(2022, 10, 15, 12),
            make_tp(2022, 10, 15, 20),
            make_tp(2022, 10, 15, 22),
            make_tp(2022, 10, 15, 23),
            make_tp(2022, 10, 16, 0),
            make_tp(2022, 10, 17, 0),
            make_tp(2022, 10, 18, 0),
            make_tp(2022, 10, 19, 0),
            make_tp(2022, 10, 20, 0),
            make_tp(2022, 10, 21, 0),
            make_tp(2022, 10, 22, 0),
        ];

        struct Location {
            latitude: f64,
            longitude: f64,
            zone: FixedOffset,
        }

        let locations = vec![Location {
            latitude: -78.463889,
            longitude: 106.83757,
            zone: FixedOffset::east_opt(6 * 60 * 60).unwrap(),
        }];

        fn print_time<T: TimeZone, U: TimeZone>(
            s: &str,
            time: Option<DateTime<T>>,
            rs_time: Option<DateTime<U>>,
        ) where
            T::Offset: std::fmt::Display,
            U::Offset: std::fmt::Display,
        {
            let my_str = if let Some(time) = time {
                format!("{}", time.format("%c %z"))
            } else {
                String::from("       does not happen        ")
            };
            let rs_str = if let Some(time) = rs_time {
                format!("{}", time.format("%c %z"))
            } else {
                String::from("       does not happen        ")
            };
            println!("{}: {} | {} UTC", s, my_str, rs_str);
        }

        for l in &locations {
            let lat = l.latitude;
            let lon = l.longitude;
            for d in &dates {
                let tp = l.zone.from_local_datetime(d).unwrap();
                let utc = Utc.from_utc_datetime(&tp.naive_local());

                let times = get_sun_times2(lat, lon, utc.date_naive());
                let times2 = &times;

                let map_zone = |opt: Option<DateTime<_>>| {
                    opt.map(|dt| l.zone.from_utc_datetime(&dt.naive_utc()))
                };

                println!("==== check at {} ====", tp.format("%c %z"));
                print_time(
                    " a. dawn",
                    map_zone(times.astro_dawn),
                    map_zone(times2.astro_dawn),
                );
                print_time(
                    " n. dawn",
                    map_zone(times.naut_dawn),
                    map_zone(times2.naut_dawn),
                );
                print_time(
                    " c. dawn",
                    map_zone(times.civil_dawn),
                    map_zone(times2.civil_dawn),
                );
                print_time(
                    " sunrise",
                    map_zone(times.sunrise),
                    map_zone(times2.sunrise),
                );
                print_time(
                    "    noon",
                    map_zone(Some(times.noon)),
                    map_zone(Some(times2.noon)),
                );
                print_time("  sunset", map_zone(times.sunset), map_zone(times2.sunset));
                print_time(
                    " c. dusk",
                    map_zone(times.civil_dusk),
                    map_zone(times2.civil_dusk),
                );
                print_time(
                    " n. dusk",
                    map_zone(times.naut_dusk),
                    map_zone(times2.naut_dusk),
                );
                print_time(
                    " a. dusk",
                    map_zone(times.astro_dusk),
                    map_zone(times2.astro_dusk),
                );
                print_time(
                    "midnight",
                    map_zone(Some(times.midnight)),
                    map_zone(Some(times2.midnight)),
                );
            }
        }

        let lat = 52.02182;
        let lon = 8.53509;
        let now = Local::now();
        let utc = Utc.from_utc_datetime(&now.naive_local());

        let times = get_sun_times2(lat, lon, utc.date_naive());
        let times2 = &times;

        let map_zone =
            |opt: Option<DateTime<_>>| opt.map(|dt| Local.from_utc_datetime(&dt.naive_utc()));

        println!("==== check at {} ====", now.format("%c %z"));
        print_time(
            " a. dawn",
            map_zone(times.astro_dawn),
            map_zone(times2.astro_dawn),
        );
        print_time(
            " n. dawn",
            map_zone(times.naut_dawn),
            map_zone(times2.naut_dawn),
        );
        print_time(
            " c. dawn",
            map_zone(times.civil_dawn),
            map_zone(times2.civil_dawn),
        );
        print_time(
            " sunrise",
            map_zone(times.sunrise),
            map_zone(times2.sunrise),
        );
        print_time(
            "    noon",
            map_zone(Some(times.noon)),
            map_zone(Some(times2.noon)),
        );
        print_time("  sunset", map_zone(times.sunset), map_zone(times2.sunset));
        print_time(
            " c. dusk",
            map_zone(times.civil_dusk),
            map_zone(times2.civil_dusk),
        );
        print_time(
            " n. dusk",
            map_zone(times.naut_dusk),
            map_zone(times2.naut_dusk),
        );
        print_time(
            " a. dusk",
            map_zone(times.astro_dusk),
            map_zone(times2.astro_dusk),
        );
        print_time(
            "midnight",
            map_zone(Some(times.midnight)),
            map_zone(Some(times2.midnight)),
        );
    }
}
