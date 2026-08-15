// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

/// @file
/// atan2 minimax polynomial approximation -- max error ~6e-4 rad over the
/// full range. tests/test_atan2.cpp checks the resulting max error.

namespace wrenium::f32math {

namespace detail {

constexpr float kAtan2PiO2 = 1.5707963267948966f;
constexpr float kAtan2Pi = 3.14159265358979f;

// Fit on [0, 1] (atan2's own range reduction keeps its argument there
// via the min/max ratio below), degree 5.
constexpr float kAtan1 = 0.99535796f;
constexpr float kAtan3 = -0.28869024f;
constexpr float kAtan5 = 0.07933903f;

constexpr float atanPoly(float r)
{
    return r * (kAtan1 + r * r * (kAtan3 + r * r * kAtan5));
}

} // namespace detail

/// atan2(y, x) -- max error ~6e-4 rad over the full range.
// (y, x) deliberately matches <cmath>'s std::atan2 argument order -- this
// function is meant as a drop-in-shaped replacement, so reordering to
// dodge a swap-risk lint would work against the one thing callers can
// already rely on without reading this header.
constexpr float atan2(float y, float x) // NOLINT(bugprone-easily-swappable-parameters)
{
    const float ax = (x < 0.0f) ? -x : x;
    const float ay = (y < 0.0f) ? -y : y;
    const float m = (ax > ay) ? ax : ay;
    const float r = (m != 0.0f) ? ((ax < ay ? ax : ay) / m) : 0.0f;
    float res = detail::atanPoly(r);
    if (ay > ax) {
        res = detail::kAtan2PiO2 - res;
    }
    if (x < 0.0f) {
        res = detail::kAtan2Pi - res;
    }
    if (y < 0.0f) {
        res = -res;
    }
    return res;
}

} // namespace wrenium::f32math
