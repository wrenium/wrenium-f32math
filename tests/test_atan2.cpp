// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "doctest/doctest.h"

#include "wrenium/f32math/atan2.h"

namespace f32math = wrenium::f32math;

namespace {

constexpr float kPi = 3.14159265358979f;
// Measured max error over a dense sweep (see the loops below) plus a
// small margin -- not the tightest bound that happens to pass, a bound
// that documents what this library actually guarantees.
constexpr float kTolerance = 8e-4f;

} // namespace

TEST_CASE("atan2 matches <cmath> across all four quadrants")
{
    float maxErr = 0.0f;
    for (int iy = -100; iy <= 100; ++iy) {
        for (int ix = -100; ix <= 100; ++ix) {
            if (ix == 0 && iy == 0) {
                continue;
            }
            const float y = static_cast<float>(iy) * 0.01f;
            const float x = static_cast<float>(ix) * 0.01f;
            maxErr = std::max(maxErr, std::fabs(f32math::atan2(y, x) - std::atan2(y, x)));
        }
    }
    CHECK(maxErr < kTolerance);
}

TEST_CASE("atan2 accuracy is scale-invariant, not just accurate near unit magnitude")
{
    // atan2's own reduction is a ratio (min(|x|,|y|)/max(|x|,|y|)), so
    // accuracy should depend only on that ratio, never on the absolute
    // magnitude of x/y themselves -- exercise inputs spanning several
    // orders of magnitude (both very large and very small) to confirm
    // that scale-invariance claim rather than just assume it from the
    // formula.
    std::srand(11);
    float maxErr = 0.0f;
    const float scales[] = {1e-6f, 1e-3f, 1.0f, 1e3f, 1e6f};
    for (float scale : scales) {
        for (int i = 0; i < 2000; ++i) {
            const float rx = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
            const float ry = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
            if (rx == 0.0f && ry == 0.0f) {
                continue;
            }
            const float x = rx * scale;
            const float y = ry * scale;
            const float err = static_cast<float>(std::fabs(static_cast<double>(f32math::atan2(y, x)) - std::atan2(static_cast<double>(y), static_cast<double>(x))));
            maxErr = std::max(maxErr, err);
        }
    }
    CHECK(maxErr < kTolerance);
}

TEST_CASE("atan2 known values")
{
    CHECK(f32math::atan2(0.0f, 1.0f) == doctest::Approx(0.0f).epsilon(1e-3));
    CHECK(f32math::atan2(1.0f, 0.0f) == doctest::Approx(kPi / 2.0f).epsilon(1e-3));
    CHECK(f32math::atan2(0.0f, -1.0f) == doctest::Approx(kPi).epsilon(1e-3));
    CHECK(f32math::atan2(-1.0f, 0.0f) == doctest::Approx(-kPi / 2.0f).epsilon(1e-3));
}

TEST_CASE("atan2 at axis-aligned and degenerate inputs")
{
    // All 4 axis directions, both signs of the other argument at zero,
    // and the (0,0) degenerate case this implementation defines as 0
    // rather than leaving as an unspecified/NaN result.
    CHECK(f32math::atan2(0.0f, 0.0f) == 0.0f);
    CHECK(f32math::atan2(0.0f, 5.0f) == doctest::Approx(0.0f).epsilon(1e-3));
    CHECK(f32math::atan2(0.0f, -5.0f) == doctest::Approx(kPi).epsilon(1e-3));
    CHECK(f32math::atan2(5.0f, 0.0f) == doctest::Approx(kPi / 2.0f).epsilon(1e-3));
    CHECK(f32math::atan2(-5.0f, 0.0f) == doctest::Approx(-kPi / 2.0f).epsilon(1e-3));
}

TEST_CASE("atan2 on the |y|==|x| diagonal, all four quadrants")
{
    // r = min(|x|,|y|)/max(|x|,|y|) == 1.0 exactly here -- the upper edge
    // of the fitted domain [0, 1], and the boundary between the
    // "ay > ax" branch taking effect or not.
    const float mags[] = {0.001f, 1.0f, 1000.0f};
    for (float m : mags) {
        CAPTURE(m);
        CHECK(f32math::atan2(m, m) == doctest::Approx(kPi / 4.0f).epsilon(1e-3));
        CHECK(f32math::atan2(m, -m) == doctest::Approx(3.0f * kPi / 4.0f).epsilon(1e-3));
        CHECK(f32math::atan2(-m, m) == doctest::Approx(-kPi / 4.0f).epsilon(1e-3));
        CHECK(f32math::atan2(-m, -m) == doctest::Approx(-3.0f * kPi / 4.0f).epsilon(1e-3));
    }
}
