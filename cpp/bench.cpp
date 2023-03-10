// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022-2023 Eicke Herbertz

#include "sun.h"
#include <benchmark/benchmark.h>

using date::days;
using std::chrono::floor;
using std::chrono::system_clock;

static const auto lat = Angle::from_deg(52.02182);
static const auto lon = Angle::from_deg(8.53509);

static void BM_sun_times_wiki(benchmark::State &state) {
    // Perform setup here
    auto tp = floor<days>(system_clock::now());
    for (auto _: state) {
        // This code gets timed
        sun::wiki::get_sun_times(lat, lon, tp);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_sun_times_wiki);

static void BM_sun_times_c(benchmark::State &state) {
    // Perform setup here
    auto tp = floor<days>(system_clock::now());
    for (auto _: state) {
        // This code gets timed
        sun::get_sun_times_c(lat, lon, tp);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_sun_times_c);

static void BM_sun_times_noaa(benchmark::State &state) {
    // Perform setup here
    auto tp = floor<days>(system_clock::now());
    for (auto _: state) {
        // This code gets timed
        sun::noaa::get_sun_times(lat, lon, tp);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_sun_times_noaa);

static void BM_sun_times_noaa_opt(benchmark::State &state) {
    // Perform setup here
    auto tp = floor<days>(system_clock::now());
    for (auto _: state) {
        // This code gets timed
        sun::noaa::get_sun_times_opt(lat, lon, tp);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_sun_times_noaa_opt);

static void BM_sun_times_rust(benchmark::State &state) {
    // Perform setup here
    auto tp = floor<days>(system_clock::now());
    for (auto _: state) {
        // This code gets timed
        sun::get_sun_times_rust(lat, lon, tp);
    }
}
// Register the function as a benchmark
BENCHMARK(BM_sun_times_rust);

// Run the benchmark
BENCHMARK_MAIN();
