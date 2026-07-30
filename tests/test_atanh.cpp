// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include <algorithm>
#include <cmath>

#include "doctest/doctest.h"

#include "wrenium/f32math/atanh.h"

namespace f32math = wrenium::f32math;

namespace {

// atanh(sin(85.0511287798 deg)) -- the fitted domain's edge (see
// atanh.h's own @file comment for why this specific bound).
constexpr float kXMax = 0.99627207622074f;
// Measured max error over the full fitted domain (see atanh.h) plus a
// small margin -- not the tightest bound that happens to pass, a bound
// that documents what this library actually guarantees.
constexpr float kTolerance = 1.4e-3f;
// Measured max error up to 83 degrees latitude, away from the last ~2
// degrees before the domain edge where atanh's own steepness dominates
// -- see atanh.h.
constexpr float kToleranceAwayFromEdge = 7e-4f;

} // namespace

TEST_CASE("atanh matches <cmath> across its fitted domain")
{
    float maxErr = 0.0f;
    for (int i = -2000; i <= 2000; ++i) {
        const float x = kXMax * static_cast<float>(i) / 2000.0f;
        const float err = static_cast<float>(std::fabs(static_cast<double>(f32math::atanh(x)) - std::atanh(static_cast<double>(x))));
        maxErr = std::max(maxErr, err);
    }
    CHECK(maxErr < kTolerance);
}

TEST_CASE("atanh is accurate well away from the domain edge")
{
    // sin(83 deg) -- comfortably inside the fitted domain, away from the
    // last ~2 degrees where accuracy tapers off (see atanh.h).
    const float xAway = std::sin(83.0f * 3.14159265358979f / 180.0f);
    float maxErr = 0.0f;
    for (int i = -2000; i <= 2000; ++i) {
        const float x = xAway * static_cast<float>(i) / 2000.0f;
        const float err = static_cast<float>(std::fabs(static_cast<double>(f32math::atanh(x)) - std::atanh(static_cast<double>(x))));
        maxErr = std::max(maxErr, err);
    }
    CHECK(maxErr < kToleranceAwayFromEdge);
}

TEST_CASE("atanh known values")
{
    CHECK(f32math::atanh(0.0f) == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(f32math::atanh(0.5f) == doctest::Approx(std::atanh(0.5)).epsilon(1e-3));
    CHECK(f32math::atanh(-0.5f) == doctest::Approx(std::atanh(-0.5)).epsilon(1e-3));
}

TEST_CASE("atanh at the domain edge matches pi (the Web Mercator y == pi convention)")
{
    // This is the actual defining property of the standard Web Mercator
    // 85.0511... degree pole-latitude limit: it's the latitude where
    // atanh(sin(lat)) == pi exactly, not an arbitrary round number.
    constexpr float kPi = 3.14159265358979f;
    CHECK(f32math::atanh(kXMax) == doctest::Approx(kPi).epsilon(1e-3));
}

TEST_CASE("atanh is odd: atanh(-x) == -atanh(x)")
{
    for (int i = 0; i <= 2000; ++i) {
        const float x = kXMax * static_cast<float>(i) / 2000.0f;
        CAPTURE(x);
        CHECK(f32math::atanh(-x) == doctest::Approx(-f32math::atanh(x)).epsilon(1e-4));
    }
}
