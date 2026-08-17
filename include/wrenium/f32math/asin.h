// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

#include <cmath>

#include "wrenium/f32math/atan2.h"

/// @file
/// asin, composed from atan2() (atan2.h) rather than its own polynomial
/// fit.

namespace wrenium::f32math {

/// asin(x) = atan2(x, sqrt(1 - x*x)) -- deliberately not a separate
/// polynomial fit: sqrt is already a single hardware instruction under a
/// hard-float ABI, so this costs one atan2() call plus one hardware sqrt,
/// with no separate coefficient set to maintain. Max error ~6e-4 rad
/// (dominated by atan2()'s own error) -- and inherits
/// WRENIUM_F32MATH_USE_STD (trig.h's own top comment) automatically for
/// the same reason, with no change needed here. @p x must be in [-1, 1].
inline float asin(float x)
{
    return atan2(x, std::sqrt(1.0f - x * x));
}

} // namespace wrenium::f32math
