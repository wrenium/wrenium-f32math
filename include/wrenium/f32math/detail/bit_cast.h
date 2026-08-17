// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#pragma once

#include <cstdint>
#include <cstring>

// Internal helper (wrenium::f32math::detail -- not public API). Reinterprets
// a float's bit pattern as a uint32_t and back via memcpy, the strict-
// aliasing-safe, portable way to do this in C++17 (no std::bit_cast until
// C++20, and a reinterpret_cast through pointers would violate strict
// aliasing). Used by log.h/exp.h's own default-build range reduction
// (IEEE-754 exponent/mantissa manipulation) -- WRENIUM_F32MATH_USE_STD
// (see trig.h's own top comment) calls std::log/std::exp directly instead
// and never reaches this.

namespace wrenium::f32math {
namespace detail {

inline std::uint32_t floatToBits(float value)
{
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

inline float bitsToFloat(std::uint32_t bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

} // namespace detail
} // namespace wrenium::f32math
