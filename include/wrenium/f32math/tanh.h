// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

#ifdef WRENIUM_F32MATH_USE_STD
#include <cmath>
#endif

/// @file
/// tanh minimax polynomial approximation, restricted to [-pi, pi] -- the
/// domain an inverse Web Mercator projection actually needs (y == pi is
/// exactly the standard pole-latitude limit, see atanh.h), not general
/// (-inf, inf). A plain odd polynomial fit (unlike atanh.h's rational
/// construction): tanh has no singularity anywhere, so it converges
/// cleanly without needing a Padé-style construction.
///
/// See trig.h's own top comment for what WRENIUM_F32MATH_USE_STD does
/// (swaps every function in this library for the real `<cmath>`
/// implementation) and its one real cost (drops `constexpr`).

namespace wrenium::f32math {

namespace detail {

// tanh(x) = x * P(u), u = x*x, degree 15 (fit over [-pi, pi]) -- degree 13
// was tried first and was accurate enough on its own (~1e-3), but a
// caller that composes this with asin() (asin.h) -- e.g. to invert an
// atanh-based projection -- feeds it a derivative that grows steeply as
// its argument approaches +-1 (tanh(pi) ~= 0.996), amplifying a small
// tanh error into a much larger error right at that domain edge --
// measured ~0.0125 rad with the degree-13 fit, ~0.005 rad with this one
// (see this composition's own worst-case check in test_tanh.cpp).
// Measured max |P(u)| over the fitted domain stays below 1 (see
// tests/test_tanh.cpp's domain-edge check) -- a minimax polynomial fit
// isn't guaranteed to respect tanh's own |tanh(x)| < 1 bound the way the
// true function does, and an overshoot past +-1 here would feed asin()
// an out-of-domain input, so this is verified, not assumed.
constexpr float kTanhC0 = 0.997685724f;
constexpr float kTanhC1 = -0.318482435f;
constexpr float kTanhC2 = 0.104223892f;
constexpr float kTanhC3 = -0.0255790029f;
constexpr float kTanhC4 = 0.00419843264f;
constexpr float kTanhC5 = -0.000425050047f;
constexpr float kTanhC6 = 0.0000237678298f;
constexpr float kTanhC7 = -0.000000558867743f;

} // namespace detail

#ifdef WRENIUM_F32MATH_USE_STD

/// tanh(x) -- see trig.h's own top comment for what
/// WRENIUM_F32MATH_USE_STD does and costs. @p x must be in [-pi, pi];
/// behavior outside that range is not defined (same contract as the
/// default build's own fit, even though std::tanh itself is defined on
/// the wider (-inf, inf)).
inline float tanh(float x)
{
    return std::tanh(x);
}

#else

/// tanh(x) -- max error ~4e-4 over the fitted domain (see tests/test_tanh.cpp).
/// @p x must be in [-pi, pi]; behavior outside that range is not defined.
constexpr float tanh(float x)
{
    const float u = x * x;
    const float p = detail::kTanhC0 + u * (detail::kTanhC1 + u * (detail::kTanhC2 + u * (detail::kTanhC3 + u * (detail::kTanhC4 + u * (detail::kTanhC5 + u * (detail::kTanhC6 + u * detail::kTanhC7))))));
    return x * p;
}

#endif // WRENIUM_F32MATH_USE_STD

} // namespace wrenium::f32math
