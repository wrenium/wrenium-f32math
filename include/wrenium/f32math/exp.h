// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

#ifdef WRENIUM_F32MATH_USE_STD
#include <cmath>
#else
#include "wrenium/f32math/detail/bit_cast.h"
#endif

/// @file
/// exp, a minimax polynomial approximation of exp(r) on the single
/// reduced range r in [-ln(2)/2, ln(2)/2] (max error ~2e-7 there),
/// composed with an IEEE-754 range reduction (`x = k*ln(2) + r`, `exp(x)
/// == exp(r) * 2^k`, the standard technique -- see detail::floatToBits'
/// own comment for why manipulating the exponent bits directly is
/// preferred over an actual `2^k` multiplication loop) rather than
/// fitting the full domain directly, the same reduce-then-fit shape
/// trig.h's sin/cos use for their own quadrant reduction. `ln(2)` is
/// split into a high/low pair (`kExpLn2Hi` exactly representable enough
/// that `k * kExpLn2Hi` has no rounding error for any @p x this fits) so
/// the reduction itself doesn't dominate the polynomial's own tighter
/// error -- without that split, error grows with @p x instead of staying
/// flat across the whole domain (confirmed by direct comparison during
/// this fit's derivation). @p x is clamped internally to +-85 to keep
/// `2^k` representable as a normal float32; this saturates to the
/// clamped result rather than true IEEE overflow-to-infinity/underflow-
/// to-zero the way std::exp does.

namespace wrenium::f32math {

#ifdef WRENIUM_F32MATH_USE_STD

/// exp(x) -- see trig.h's own top comment for what
/// WRENIUM_F32MATH_USE_STD does and costs.
inline float exp(float x)
{
    return std::exp(x);
}

#else

namespace detail {

// ln(2), split so k*kExpLn2Hi is exact in float32 for every k this
// function's own +-85 clamp can produce (see this file's own top
// comment) -- kExpLn2Hi's low mantissa bits are zeroed for exactly that
// reason, with kExpLn2Lo holding the remainder.
constexpr float kExpLn2 = 0.6931471824645996f; // nearest float32 to true ln(2), for k's own rounding only
constexpr float kExpLn2Hi = 0.69140625f;
constexpr float kExpLn2Lo = 0.0017409306019544601f;

// exp(r) on r in [-ln(2)/2, ln(2)/2], minimax-fit (Remez exchange) --
// max error ~2e-7 there.
constexpr float kExpC0 = 1.0000001192092896f;
constexpr float kExpC1 = 1.0000001192092896f;
constexpr float kExpC2 = 0.4999886751174927f;
constexpr float kExpC3 = 0.166663259267807f;
constexpr float kExpC4 = 0.041917528957128525f;
constexpr float kExpC5 = 0.008381109684705734f;

constexpr float expPoly(float r)
{
    return kExpC0 + r * (kExpC1 + r * (kExpC2 + r * (kExpC3 + r * (kExpC4 + r * kExpC5))));
}

} // namespace detail

/// exp(x) -- see this file's own top comment for the fit's error and the
/// +-85 internal clamp.
inline float exp(float x)
{
    float clamped = x;
    if (clamped > 85.0f) {
        clamped = 85.0f;
    } else if (clamped < -85.0f) {
        clamped = -85.0f;
    }

    // Symmetric round-half-away-from-zero, same manually-inlined pattern
    // trig.h's own reduce() uses -- see that function's comment.
    // NOLINTNEXTLINE(bugprone-incorrect-roundings)
    const float kf = (clamped >= 0.0f) ? static_cast<float>(static_cast<int>(clamped / detail::kExpLn2 + 0.5f)) : static_cast<float>(static_cast<int>(clamped / detail::kExpLn2 - 0.5f));
    const int k = static_cast<int>(kf);

    const float r = (clamped - kf * detail::kExpLn2Hi) - kf * detail::kExpLn2Lo;
    const float poly = detail::expPoly(r);

    const std::uint32_t scaleBits = static_cast<std::uint32_t>(k + 127) << 23;
    const float scale = detail::bitsToFloat(scaleBits);
    return poly * scale;
}

#endif // WRENIUM_F32MATH_USE_STD

} // namespace wrenium::f32math
