use chrono::{Duration, NaiveDate, NaiveTime};
use crate::angle::Angle;
use std::ops::{Add, Sub};

#[derive(Copy, Clone)]
pub(crate) struct JulianCentury(pub f64);
#[derive(Copy, Clone)]
pub(crate) struct JulianDay(pub f64);

impl JulianDay {
    pub fn from_date(date: NaiveDate) -> Self {
        let tp = date.and_time(NaiveTime::default()).timestamp() as f64;
        return Self((tp / 86400.0) + 2440587.5);
    }
}

impl JulianCentury {
    pub fn from_date(date: NaiveDate) -> Self {
        JulianDay::from_date(date).into()
    }
}

impl From<JulianDay> for JulianCentury {
    fn from(d: JulianDay) -> Self {
        Self(d.0 / 36525.0)
    }
}

impl From<JulianCentury> for JulianDay {
    fn from(c: JulianCentury) -> Self {
        Self(c.0 * 36525.0)
    }
}

impl From<JulianDay> for Duration {
    fn from(d: JulianDay) -> Self {
        let sec = (d.0 * 86400.0).floor() as i64;
        Duration::seconds(sec)
    }
}

impl From<Angle> for JulianDay {
    fn from(a: Angle) -> Self {
        Self(a.deg() / 360.0)
    }
}

impl Add<JulianCentury> for JulianCentury {
    type Output = Self;

    fn add(self, rhs: JulianCentury) -> Self::Output {
        Self(self.0 + rhs.0)
    }
}

impl Add<JulianDay> for JulianCentury {
    type Output = Self;

    fn add(self, rhs: JulianDay) -> Self::Output {
        Self(self.0 + JulianCentury::from(rhs).0)
    }
}

impl Sub<JulianCentury> for JulianCentury {
    type Output = Self;

    fn sub(self, rhs: JulianCentury) -> Self::Output {
        Self(self.0 - rhs.0)
    }
}

impl Sub<JulianDay> for JulianCentury {
    type Output = Self;

    fn sub(self, rhs: JulianDay) -> Self::Output {
        Self(self.0 - JulianCentury::from(rhs).0)
    }
}
