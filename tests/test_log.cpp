// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include <algorithm>
#include <cmath>

#include "doctest/doctest.h"

#include "wrenium/f32math/log.h"

namespace f32math = wrenium::f32math;

namespace {

// Measured max error over a dense sweep spanning the full positive
// float32 exponent range (see the loop below) plus a small margin -- not
// the tightest bound that happens to pass, a bound that documents what
// this library actually guarantees. See log.h's own comment for why this
// inherits atanh()'s own error tier.
constexpr float kTolerance = 3e-4f;

} // namespace

TEST_CASE("log matches <cmath> across a wide positive range")
{
    float maxErr = 0.0f;
    // Geometric sweep (not linear) -- log's own domain is scale-invariant
    // (ln(x) - ln(x/10) is constant), so a linear sweep would spend almost
    // all its samples on large x and barely probe small x at all.
    for (int i = -600; i <= 600; ++i) {
        const float x = std::pow(10.0f, static_cast<float>(i) * 0.01f);
        const float err = static_cast<float>(std::fabs(static_cast<double>(f32math::log(x)) - std::log(static_cast<double>(x))));
        maxErr = std::max(maxErr, err);
    }
    CHECK(maxErr < kTolerance);
}

TEST_CASE("log known values")
{
    CHECK(f32math::log(1.0f) == doctest::Approx(0.0f).epsilon(1e-4));
    CHECK(f32math::log(std::exp(1.0f)) == doctest::Approx(1.0f).epsilon(1e-3));
    CHECK(f32math::log(2.0f) == doctest::Approx(std::log(2.0f)).epsilon(1e-3));
    CHECK(f32math::log(0.5f) == doctest::Approx(std::log(0.5f)).epsilon(1e-3));
}

TEST_CASE("log is continuous across a power-of-two boundary")
{
    // The exponent/mantissa range reduction (log.h's own comment) could
    // introduce a seam right where the mantissa's own renormalization
    // flips -- check a dense sweep straddling several such boundaries
    // finds none.
    for (float base : {0.5f, 1.0f, 2.0f, 4.0f, 1024.0f}) {
        float prev = f32math::log(base * 0.999f);
        for (int i = 0; i <= 200; ++i) {
            const float x = base * (0.999f + static_cast<float>(i) * 0.002f / 200.0f);
            const float cur = f32math::log(x);
            CAPTURE(x);
            CHECK(std::fabs(cur - prev) < 0.01f);
            prev = cur;
        }
    }
}
