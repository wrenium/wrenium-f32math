// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include <algorithm>
#include <cmath>
#include <initializer_list>

#include "doctest/doctest.h"

#include "wrenium/f32math/trig.h"

namespace f32math = wrenium::f32math;

namespace {

constexpr float kPi = 3.14159265358979f;
// Measured max error over a dense sweep (see the loops below) plus a
// small margin -- not the tightest bound that happens to pass, a bound
// that documents what this library actually guarantees. Accuracy degrades
// slowly as |x| grows past a handful of full turns (the single-subtraction
// range reduction loses precision at large multiples of pi/2) -- fine for
// the bounded angles (never beyond [-pi, pi]) this library's actual
// callers use, which is what the sweeps below cover.
constexpr float kTolerance = 3e-6f;

} // namespace

TEST_CASE("sin/cos match <cmath> across the full angular range")
{
    float maxSinErr = 0.0f;
    float maxCosErr = 0.0f;
    for (int i = -10000; i <= 10000; ++i) {
        const float x = static_cast<float>(i) * 0.001f * kPi;
        maxSinErr = std::max(maxSinErr, std::fabs(f32math::sin(x) - std::sin(x)));
        maxCosErr = std::max(maxCosErr, std::fabs(f32math::cos(x) - std::cos(x)));
    }
    CHECK(maxSinErr < kTolerance);
    CHECK(maxCosErr < kTolerance);
}

TEST_CASE("sin^2 + cos^2 == 1 across the full angular range")
{
    // An invariant independent of comparing against <cmath> directly --
    // catches a class of bug (sin and cos individually plausible but
    // mutually inconsistent) a pointwise comparison to std::sin/cos alone
    // wouldn't necessarily surface.
    float maxErr = 0.0f;
    for (int i = -10000; i <= 10000; ++i) {
        const float x = static_cast<float>(i) * 0.001f * kPi;
        const float s = f32math::sin(x);
        const float c = f32math::cos(x);
        maxErr = std::max(maxErr, std::fabs(s * s + c * c - 1.0f));
    }
    CHECK(maxErr < 2e-5f);
}

TEST_CASE("sincos agrees with sin()/cos() called separately")
{
    for (int i = -1000; i <= 1000; ++i) {
        const float x = static_cast<float>(i) * 0.01f * kPi;
        float s, c;
        f32math::sincos(x, s, c);
        CHECK(s == f32math::sin(x));
        CHECK(c == f32math::cos(x));
    }
}

TEST_CASE("sin/cos known values")
{
    CHECK(f32math::sin(0.0f) == doctest::Approx(0.0f).epsilon(1e-5));
    CHECK(f32math::cos(0.0f) == doctest::Approx(1.0f).epsilon(1e-5));
    CHECK(f32math::sin(kPi / 2.0f) == doctest::Approx(1.0f).epsilon(1e-5));
    CHECK(f32math::cos(kPi / 2.0f) == doctest::Approx(0.0f).epsilon(1e-5));
    CHECK(f32math::sin(kPi) == doctest::Approx(0.0f).epsilon(1e-5));
    CHECK(f32math::cos(kPi) == doctest::Approx(-1.0f).epsilon(1e-5));
}

// The rest of this file targets the specific failure modes that actually
// bit prior attempts at this same technique during development: a wrong
// range-reduction sign flip at quadrant boundaries produced exactly-
// sign-flipped output (error ~2, not a small numerical error) rather than
// a gradual accuracy loss -- easy to miss if only spot-checking "nice"
// angles like 0 or pi/2 themselves, since round-to-nearest can land
// either side of the boundary depending on floating-point rounding.

TEST_CASE("sin/cos are continuous across every quadrant boundary, not just correct at the boundary itself")
{
    // Multiples of pi/2 are exactly where the quadrant index changes -- a
    // sign/swap bug here would show up as a jump, not a gradual error,
    // right at these points.
    const float boundaries[] = {-2.0f * kPi, -1.5f * kPi, -kPi, -0.5f * kPi, 0.0f, 0.5f * kPi, kPi, 1.5f * kPi, 2.0f * kPi};
    for (float b : boundaries) {
        for (float delta : {-1e-3f, -1e-5f, 0.0f, 1e-5f, 1e-3f}) {
            const float x = b + delta;
            CAPTURE(x);
            CHECK(std::fabs(f32math::sin(x) - std::sin(x)) < 1e-3f);
            CHECK(std::fabs(f32math::cos(x) - std::cos(x)) < 1e-3f);
            float s, c;
            f32math::sincos(x, s, c);
            CHECK(s == f32math::sin(x));
            CHECK(c == f32math::cos(x));
        }
    }
}

TEST_CASE("sin/cos handle negative angles through every quadrant, not just the positive-angle path")
{
    // The reduction's quadrant-index wraparound only fires for negative x
    // -- exercise it explicitly rather than relying on the wide sweep
    // above to happen to cover it.
    for (int i = -20; i <= 0; ++i) {
        const float x = static_cast<float>(i) * 0.31f; // steps through all 4 quadrants, both signs
        CAPTURE(x);
        CHECK(std::fabs(f32math::sin(x) - std::sin(x)) < kTolerance);
        CHECK(std::fabs(f32math::cos(x) - std::cos(x)) < kTolerance);
    }
}
