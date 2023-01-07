# Sunrise and sunset calculations

This repository contains four implementations of sunrise calculations.

- `cpp/redshift_solar.c`: copied from the [redshift](https://github.com/jonls/redshift/blob/master/src/solar.c)
  project for reference. It actually contained a bug, though. This file also is the reason why
  the test code is GPL licensed. My implementations of the calculation are MIT licensed though.
- `cpp/wiki_sun.cpp`: is the algorithm described on [Wikipedia](https://en.wikipedia.org/wiki/Sunrise_equation).
  It appears to be an approximation. It's results are slightly off in moderate latitudes and
  unusable in polar latitudes (the sun sets for a polar day). It's faster though, but the other
  calculations aren't "slow" in practice, either.
- `cpp/noaa_sun.cpp`: the result of the situation rendered by the above. I downloaded the NOAA spreadsheets,
  was able to comprehend the two-pass calculation done by existing implementations and could even
  match it with the output of [timeanddate.com](https://timeanddate.com/). So, this apparently is
  what everybody does and I reimplemented the spreadsheet cell by cell and now have my own
  MIT-licensed code. :)
- `rust/src/lib.rs`: this is my implementation, but ported to Rust. Just to compare it to C++ and do
  some FFI hacking.

See the `cpp/sun.h` header for available public functions. The CMake project currently is dumb, so the
easiest way to use this in a project is to copy the required files (`angle.h`, `julian_date.h`, `sun.h`
and whatever implementation you choose) and integrate them with your build system.

You need [`date`](https://github.com/HowardHinnant/date) for the library until we have C++20 chrono.
For the benchmark and test code, you'll also need [`fmt`](https://github.com/fmtlib/fmt) and
[`benchmark`](https://github.com/google/benchmark) installed on your system, as well as Rust.

I wrote these implementations for the fun of doing it. And maybe for the fun of building products
with stupidly precise astronomic calculations. _I know we'll probably never sell to the Arctic, but
if we do, at least my sunrise calculation is correct!_ Therefore, personal code.

Feel absolutely free to use this for your projects. And please let me know if this makes it into
some actual POS product. ;)
