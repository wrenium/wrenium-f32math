// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include <algorithm>
#include <cmath>

#include "doctest/doctest.h"

#include "wrenium/f32math/exp.h"
#include "wrenium/f32math/log.h"

namespace f32math = wrenium::f32math;

namespace {

// Measured max relative error over the full +-85 clamped domain (see
// exp.h's own comment) plus a small margin -- not the tightest bound
// that happens to pass, a bound that documents what this library
// actually guarantees.
constexpr float kRelTolerance = 1e-5f;

} // namespace

TEST_CASE("exp matches <cmath> across its full +-85 domain")
{
    float maxRelErr = 0.0f;
    for (int i = -8500; i <= 8500; ++i) {
        const float x = static_cast<float>(i) * 0.01f;
        const double ref = std::exp(static_cast<double>(x));
        const double approx = static_cast<double>(f32math::exp(x));
        const double relErr = std::fabs(approx - ref) / ref;
        maxRelErr = std::max(maxRelErr, static_cast<float>(relErr));
    }
    CHECK(maxRelErr < kRelTolerance);
}

TEST_CASE("exp known values")
{
    CHECK(f32math::exp(0.0f) == doctest::Approx(1.0f).epsilon(1e-5));
    CHECK(f32math::exp(1.0f) == doctest::Approx(std::exp(1.0f)).epsilon(1e-4));
    CHECK(f32math::exp(-1.0f) == doctest::Approx(std::exp(-1.0f)).epsilon(1e-4));
    CHECK(f32math::exp(10.0f) == doctest::Approx(std::exp(10.0f)).epsilon(1e-4));
}

#ifndef WRENIUM_F32MATH_USE_STD
TEST_CASE("exp saturates rather than overflowing past its +-85 clamp")
{
    // See exp.h's own top comment: this clamps to the +-85 result rather
    // than reaching true IEEE infinity/zero the way std::exp does -- a
    // deliberate difference from std::exp, so unlike every other test in
    // this file it doesn't hold under WRENIUM_F32MATH_USE_STD (which calls
    // std::exp directly, with std::exp's own real overflow/underflow).
    CHECK(std::isfinite(f32math::exp(1000.0f)));
    CHECK(f32math::exp(1000.0f) == doctest::Approx(f32math::exp(85.0f)).epsilon(1e-6));
    CHECK(f32math::exp(-1000.0f) == doctest::Approx(f32math::exp(-85.0f)).epsilon(1e-6));
}
#endif

TEST_CASE("exp(log(x)) round-trips for a range of positive x")
{
    for (float x : {1e-3f, 0.1f, 0.5f, 1.0f, 2.0f, 10.0f, 1000.0f, 1e6f}) {
        CAPTURE(x);
        CHECK(f32math::exp(f32math::log(x)) == doctest::Approx(x).epsilon(2e-3));
    }
}
