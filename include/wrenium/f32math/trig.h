// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

/// @file
/// sin/cos minimax polynomial approximations -- max error ~9e-7 over the
/// full angular range. tests/test_sincos.cpp checks the resulting max
/// error.

namespace wrenium::f32math {

namespace detail {

constexpr float kTrigInvPiO2 = 0.6366197723675814f;

// sinPoly/cosPoly are fit as sin(r*pi/2)/cos(r*pi/2), r in [-0.5, 0.5]
// -- the pi/2 scaling is folded into the coefficients so range
// reduction only needs one subtract (see reduce() below), not a
// second multiply to convert r back to radians.
constexpr float kSin1 = 1.5707886332107643f;
constexpr float kSin3 = -0.6457116839620999f;
constexpr float kSin5 = 0.07766293226199035f;
constexpr float kCos2 = -1.2336952317352528f;
constexpr float kCos4 = 0.25357698228453524f;
constexpr float kCos6 = -0.020350594550262963f;

constexpr float sinPoly(float r, float s)
{
    return r * (kSin1 + s * (kSin3 + s * kSin5));
}

constexpr float cosPoly(float s)
{
    return 1.0f + s * (kCos2 + s * (kCos4 + s * kCos6));
}

// Range-reduces x to the nearest multiple of pi/2 -- r comes out in
// dimensionless quarter-turn units (matching sinPoly/cosPoly's own
// scaling above), q is that quadrant index mod 4.
// r/q are distinct types (float&/int&) that can't actually bind to a
// swapped call-site argument by accident -- neither is a const reference,
// so passing a float where an int& is expected (or vice versa) is a hard
// compile error, not a silent swap.
constexpr void reduce(float x, float &r, int &q) // NOLINT(bugprone-easily-swappable-parameters)
{
    const float t = x * kTrigInvPiO2;
    // Symmetric round-half-away-from-zero, manually inlined: this is the
    // single hottest function in the library (called by every sin/cos/
    // sincos), and lround()/round() would add a real function-call cost
    // here for no correctness difference over what this already computes
    // correctly for both signs of t.
    // NOLINTNEXTLINE(bugprone-incorrect-roundings)
    const float qf = (t >= 0.0f) ? static_cast<float>(static_cast<int>(t + 0.5f)) : static_cast<float>(static_cast<int>(t - 0.5f));
    r = t - qf;
    q = static_cast<int>(qf) & 3;
    if (q < 0) {
        q += 4;
    }
}

} // namespace detail

/// sin(x) -- max error ~9e-7 over the full angular range. Branches on
/// quadrant parity before evaluating either polynomial, so a standalone
/// sin() call only pays for the one it actually needs -- prefer sincos()
/// instead when both sin and cos of the same angle are needed (that
/// shares one range reduction between both, cheaper than two independent
/// sin()/cos() calls).
constexpr float sin(float x)
{
    float r = 0.0f;
    int q = 0;
    detail::reduce(x, r, q);
    const float s = r * r;
    if (q & 1) {
        const float v = detail::cosPoly(s);
        return (q & 2) ? -v : v;
    }
    const float v = detail::sinPoly(r, s);
    return (q & 2) ? -v : v;
}

/// cos(x) -- see sin()'s own doc comment; prefer sincos() if both are needed.
constexpr float cos(float x)
{
    float r = 0.0f;
    int q = 0;
    detail::reduce(x, r, q);
    const float s = r * r;
    if (q & 1) {
        const float v = -detail::sinPoly(r, s);
        return (q & 2) ? -v : v;
    }
    const float v = detail::cosPoly(s);
    return (q & 2) ? -v : v;
}

/// sin(x) and cos(x) together, one range reduction and both polynomials
/// evaluated once each -- the preferred entry point over calling
/// sin()/cos() separately whenever both are needed for the same angle.
// (s, c) deliberately matches POSIX/GNU sincos()'s output-argument order --
// see atan2()'s identical rationale above.
constexpr void sincos(float x, float &s, float &c) // NOLINT(bugprone-easily-swappable-parameters)
{
    float r = 0.0f;
    int q = 0;
    detail::reduce(x, r, q);
    const float sq = r * r;
    const float sv = detail::sinPoly(r, sq);
    const float cv = detail::cosPoly(sq);
    switch (q) {
    case 0:
        s = sv;
        c = cv;
        break;
    case 1:
        s = cv;
        c = -sv;
        break;
    case 2:
        s = -sv;
        c = -cv;
        break;
    default:
        s = -cv;
        c = sv;
        break;
    }
}

} // namespace wrenium::f32math
