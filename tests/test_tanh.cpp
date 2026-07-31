// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include <algorithm>
#include <cmath>

#include "doctest/doctest.h"

#include "wrenium/f32math/tanh.h"

namespace f32math = wrenium::f32math;

namespace {

constexpr float kPi = 3.14159265358979f;
// Measured max error over the full fitted domain [-pi, pi] (see tanh.h)
// plus a small margin -- not the tightest bound that happens to pass, a
// bound that documents what this library actually guarantees.
constexpr float kTolerance = 5e-4f;

} // namespace

TEST_CASE("tanh matches <cmath> across its fitted domain")
{
    float maxErr = 0.0f;
    for (int i = -2000; i <= 2000; ++i) {
        const float x = kPi * static_cast<float>(i) / 2000.0f;
        const float err = static_cast<float>(std::fabs(static_cast<double>(f32math::tanh(x)) - std::tanh(static_cast<double>(x))));
        maxErr = std::max(maxErr, err);
    }
    CHECK(maxErr < kTolerance);
}

TEST_CASE("tanh known values")
{
    CHECK(f32math::tanh(0.0f) == doctest::Approx(0.0f).epsilon(1e-6));
    CHECK(f32math::tanh(0.5f) == doctest::Approx(std::tanh(0.5)).epsilon(1e-3));
    CHECK(f32math::tanh(1.0f) == doctest::Approx(std::tanh(1.0)).epsilon(1e-3));
    CHECK(f32math::tanh(-1.0f) == doctest::Approx(std::tanh(-1.0)).epsilon(1e-3));
    CHECK(f32math::tanh(2.0f) == doctest::Approx(std::tanh(2.0)).epsilon(1e-3));
}

TEST_CASE("tanh is odd: tanh(-x) == -tanh(x)")
{
    for (int i = 0; i <= 2000; ++i) {
        const float x = kPi * static_cast<float>(i) / 2000.0f;
        CAPTURE(x);
        CHECK(f32math::tanh(-x) == doctest::Approx(-f32math::tanh(x)).epsilon(1e-4));
    }
}

TEST_CASE("tanh never overshoots +-1 across its fitted domain")
{
    // Load-bearing, not a nicety: a caller inverting an atanh-based
    // projection typically feeds this straight into asin() (asin.h),
    // whose sqrt(1 - x*x) becomes NaN for any |x| > 1. A minimax
    // polynomial fit isn't guaranteed to respect the true function's
    // |tanh(x)| < 1 bound the way tanh() itself is -- this is measured,
    // not assumed. Measured max |tanh(x)| here is ~0.9973 (a ~0.0027
    // safety margin below 1.0); check strictly below 1.0 with headroom,
    // not exactly at the measured value.
    float maxAbs = 0.0f;
    for (int i = -2000; i <= 2000; ++i) {
        const float x = kPi * static_cast<float>(i) / 2000.0f;
        maxAbs = std::max(maxAbs, std::fabs(f32math::tanh(x)));
    }
    CHECK(maxAbs < 0.999f);
}

TEST_CASE("tanh at the domain edge is close to but strictly below 1")
{
    // pi is the exact edge of the fitted domain (see tanh.h's own @file
    // comment for why -- it's the bound an inverse Web Mercator
    // projection needs).
    CHECK(f32math::tanh(kPi) == doctest::Approx(std::tanh(static_cast<double>(kPi))).epsilon(2e-3));
    CHECK(f32math::tanh(kPi) < 1.0f);
    CHECK(f32math::tanh(-kPi) > -1.0f);
}
