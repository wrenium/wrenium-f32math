// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

#ifdef WRENIUM_F32MATH_USE_STD
#include <cmath>
#else
#include "wrenium/f32math/atanh.h"
#include "wrenium/f32math/detail/bit_cast.h"
#endif

/// @file
/// log (natural logarithm), valid for @p x > 0 -- behavior for @p x <= 0
/// is undefined. Composed from atanh() (atanh.h) plus an IEEE-754
/// exponent/mantissa range reduction, rather than its own polynomial fit
/// (unlike atanh itself, which needed one -- see that file's own
/// comment): `ln(m) == 2*atanh((m-1)/(m+1))` is an exact identity, and
/// splitting @p x into `m * 2^e` (`m` in `[sqrt(2)/2, sqrt(2))`, the
/// standard range-reduction choice that minimizes `(m-1)/(m+1)`'s own
/// magnitude) keeps that ratio within +-0.1716 -- well inside atanh's own
/// fitted domain (+-0.9962), and close enough to its domain's center that
/// this reuses atanh()'s existing fit at a tighter error than atanh()'s
/// own documented worst case (see atanh.h) rather than needing new
/// coefficients of its own. `ln(x) = ln(m) + e*ln(2)` finishes it.

namespace wrenium::f32math {

#ifdef WRENIUM_F32MATH_USE_STD

/// log(x) -- see trig.h's own top comment for what
/// WRENIUM_F32MATH_USE_STD does and costs. @p x must be > 0; behavior
/// otherwise is not defined (same contract as the default build's own
/// fit, even though std::log itself is defined down to any positive
/// subnormal).
inline float log(float x)
{
    return std::log(x);
}

#else

namespace detail {

constexpr float kLogSqrt2 = 1.4142135623730951f;
constexpr float kLogLn2 = 0.6931471805599453f;

} // namespace detail

/// log(x) -- max error ~2e-4 over the full positive float32 range (see
/// this file's own top comment for why it inherits atanh()'s own error
/// tier rather than needing a separate accuracy figure). @p x must be
/// > 0; behavior otherwise is not defined.
inline float log(float x)
{
    const std::uint32_t bits = detail::floatToBits(x);
    const int exponent = static_cast<int>((bits >> 23) & 0xFFu) - 127;
    float mantissa = detail::bitsToFloat((bits & 0x007FFFFFu) | (127u << 23)); // [1, 2)

    int e = exponent;
    if (mantissa > detail::kLogSqrt2) {
        mantissa *= 0.5f;
        e += 1;
    } // now in [sqrt(2)/2, sqrt(2))

    const float z = (mantissa - 1.0f) / (mantissa + 1.0f);
    return 2.0f * atanh(z) + static_cast<float>(e) * detail::kLogLn2;
}

#endif // WRENIUM_F32MATH_USE_STD

} // namespace wrenium::f32math
