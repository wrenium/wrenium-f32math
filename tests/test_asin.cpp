// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include <algorithm>
#include <cmath>

#include "doctest/doctest.h"

#include "wrenium/f32math/asin.h"

namespace f32math = wrenium::f32math;

namespace {

constexpr float kPi = 3.14159265358979f;
// Dominated by atan2()'s own error budget -- see atan2.h.
constexpr float kTolerance = 8e-4f;

} // namespace

TEST_CASE("asin matches <cmath> across [-1, 1]")
{
    float maxErr = 0.0f;
    for (int i = -1000; i <= 1000; ++i) {
        const float x = static_cast<float>(i) * 0.001f;
        maxErr = std::max(maxErr, std::fabs(f32math::asin(x) - std::asin(x)));
    }
    CHECK(maxErr < kTolerance);
}

TEST_CASE("asin at its domain boundary")
{
    // sqrt(1 - x*x) == 0 exactly at x == +-1 -- the edge of asin's valid
    // domain, and where atan2's second argument hits zero.
    CHECK(f32math::asin(1.0f) == doctest::Approx(kPi / 2.0f).epsilon(1e-3));
    CHECK(f32math::asin(-1.0f) == doctest::Approx(-kPi / 2.0f).epsilon(1e-3));
    CHECK(f32math::asin(0.0f) == doctest::Approx(0.0f).epsilon(1e-3));
}

TEST_CASE("asin is odd: asin(-x) == -asin(x)")
{
    for (int i = 0; i <= 1000; ++i) {
        const float x = static_cast<float>(i) * 0.001f;
        CAPTURE(x);
        CHECK(f32math::asin(-x) == doctest::Approx(-f32math::asin(x)).epsilon(1e-4));
    }
}
