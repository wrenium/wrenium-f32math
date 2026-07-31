// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Tommi Tauriainen

#include "doctest/doctest.h"

#include "wrenium/f32math/f32math.h"

namespace f32math = wrenium::f32math;

// Regression check for the umbrella header itself, not the functions it
// pulls in (each has its own dedicated, precision-checked test file) --
// nothing else ever included f32math.h, so a future function header
// added without also being wired in here would otherwise go unnoticed.
// One call per function is enough: a missing #include is a compile
// error, not a wrong-value bug, so there's nothing to CHECK().
TEST_CASE("f32math.h pulls in every function this library provides")
{
    float s = 0.0f;
    float c = 0.0f;
    f32math::sincos(0.5f, s, c);
    (void)f32math::sin(0.5f);
    (void)f32math::cos(0.5f);
    (void)f32math::atan2(0.5f, 0.5f);
    (void)f32math::asin(0.5f);
    (void)f32math::atanh(0.5f);
    (void)f32math::tanh(0.5f);
}
