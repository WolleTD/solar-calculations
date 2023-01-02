use std::ops::{Add, Div, Mul, Sub};

#[derive(Copy, Clone)]
pub(crate) struct Angle(f64);

impl Angle {
    pub fn deg(&self) -> f64 {
        self.0.to_degrees()
    }
    pub fn rad(&self) -> f64 {
        self.0
    }

    pub const fn from_rad(rad: f64) -> Self {
        Angle(rad)
    }
    pub fn from_deg(deg: f64) -> Self {
        Angle(deg.to_radians())
    }

    pub fn sin(&self) -> f64 {
        self.0.sin()
    }
    pub fn cos(&self) -> f64 {
        self.0.cos()
    }
    pub fn tan(&self) -> f64 {
        self.0.tan()
    }
}

impl Add<Angle> for Angle {
    type Output = Angle;
    fn add(self, rhs: Angle) -> Self::Output {
        Angle(self.0 + rhs.0)
    }
}

impl Sub<Angle> for Angle {
    type Output = Angle;
    fn sub(self, rhs: Angle) -> Self::Output {
        Angle(self.0 - rhs.0)
    }
}

impl Mul<f64> for Angle {
    type Output = Angle;
    fn mul(self, rhs: f64) -> Self::Output {
        Angle(self.0 * rhs)
    }
}

impl Mul<Angle> for f64 {
    type Output = Angle;
    fn mul(self, rhs: Angle) -> Self::Output {
        Angle(rhs.0 * self)
    }
}

impl Div<f64> for Angle {
    type Output = Angle;
    fn div(self, rhs: f64) -> Self::Output {
        Angle(self.0 / rhs)
    }
}
