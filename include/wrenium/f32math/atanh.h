// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

#ifdef WRENIUM_F32MATH_USE_STD
#include <cmath>
#endif

/// @file
/// atanh minimax rational approximation, restricted to |x| <=
/// sin(85.0511 deg) (~0.99627) -- the standard "Web Mercator" pole-
/// latitude limit, the actual point where atanh(sin(lat)) == pi. Not a
/// general (-1, 1) atanh: a plain polynomial fit converges far too
/// slowly approaching atanh's true singularity at +-1 to be practical,
/// so this is a rational (Padé-style) fit instead, restricted to the one
/// bounded domain this library actually needs it for.
///
/// See trig.h's own top comment for what WRENIUM_F32MATH_USE_STD does
/// (swaps every function in this library for the real `<cmath>`
/// implementation) and its one real cost (drops `constexpr`).

namespace wrenium::f32math {

namespace detail {

// atanh(x) = x * N(v) / D(v), v = 2*(x*x/kAtanhUMax) - 1 mapping the
// fitted domain to [-1, 1]. N/D are held as Chebyshev coefficients (not
// plain monomial/power-basis coefficients) and evaluated via Clenshaw's
// recurrence, hand-unrolled below -- a monomial-basis fit of the same
// degree is numerically unstable in float32 here (individual N/D terms
// reach magnitude ~1-3 that nearly cancel near the domain edge, losing
// most of float32's precision); Chebyshev evaluation avoids that by
// construction (every intermediate term stays bounded).
constexpr float kAtanhUMax = 0.992558049857184f; // sin(85.0511287798 deg)^2

constexpr float kAtanhN0 = 1.059074637765608f;
constexpr float kAtanhN1 = -1.486308774443876f;
constexpr float kAtanhN2 = 0.48314281636163325f;
constexpr float kAtanhN3 = -0.054830479116242954f;

constexpr float kAtanhD1 = -1.4571155448261535f;
constexpr float kAtanhD2 = 0.5399319292684692f;
constexpr float kAtanhD3 = -0.08549550377456618f;
constexpr float kAtanhD4 = 0.003021052862125718f;

} // namespace detail

#ifdef WRENIUM_F32MATH_USE_STD

/// atanh(x) -- see trig.h's own top comment for what
/// WRENIUM_F32MATH_USE_STD does and costs. @p x must be in
/// [-0.99627, 0.99627]; behavior outside that range is not defined (same
/// contract as the default build's own fit, even though std::atanh
/// itself is defined on the wider (-1, 1)).
inline float atanh(float x)
{
    return std::atanh(x);
}

#else

/// atanh(x) -- max error ~1e-4 over most of the domain, ~1.2e-3 in the
/// last degree before the limit (|x| approaching ~0.996, i.e. |lat|
/// approaching 85 deg -- see this file's own top comment). @p x must be
/// in [-0.99627, 0.99627]; behavior outside that range is not defined.
constexpr float atanh(float x)
{
    const float u = x * x;
    const float v = 2.0f * (u / detail::kAtanhUMax) - 1.0f;

    const float nb3 = detail::kAtanhN3;
    const float nb2 = detail::kAtanhN2 + 2.0f * v * nb3;
    const float nb1 = detail::kAtanhN1 + 2.0f * v * nb2 - nb3;
    const float n = detail::kAtanhN0 + v * nb1 - nb2;

    const float dc4 = detail::kAtanhD4;
    const float dc3 = detail::kAtanhD3 + 2.0f * v * dc4;
    const float dc2 = detail::kAtanhD2 + 2.0f * v * dc3 - dc4;
    const float dc1 = detail::kAtanhD1 + 2.0f * v * dc2 - dc3;
    const float d = 1.0f + (v * dc1 - dc2);

    return x * n / d;
}

#endif // WRENIUM_F32MATH_USE_STD

} // namespace wrenium::f32math
