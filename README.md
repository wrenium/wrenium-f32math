# wrenium-f32math

[![CI](https://github.com/wrenium/wrenium-f32math/actions/workflows/ci.yml/badge.svg)](https://github.com/wrenium/wrenium-f32math/actions/workflows/ci.yml)
[![REUSE status](https://api.reuse.software/badge/github.com/wrenium/wrenium-f32math)](https://api.reuse.software/info/github.com/wrenium/wrenium-f32math)

[API documentation](https://wrenium.github.io/wrenium-f32math/)

A C++17 header-only library of float-only math approximations for
single-precision-FPU embedded targets: `sin`/`cos`/`sincos`/`atan2`/
`asin`/`atanh`/`tanh`/`log`/`exp`.

This library has been created for [wrenium-geo](https://github.com/wrenium/wrenium-geo),
to replace `<cmath>` with faster approximations for its constrained (MCU)
targets -- `atanh`/`tanh`'s bounded domains in particular are derived
directly from what wrenium-geo's inverse Web Mercator projection needs,
which is why their own comments and tests reference pole-latitude
limits and composing with `asin()`. There's no actual dependency on
wrenium-geo anywhere in this repo, though, and the same functions fit
just as well in any other project willing to trade some precision for
speed (see Accuracy below).

## Accuracy

| Function | Max error |
|---|---|
| `sin`/`cos` | ~9e-7 rad |
| `atan2` | ~6e-4 rad |
| `asin` | ~6e-4 rad (dominated by `atan2`'s own error) |
| `atanh` | ~1e-4 over most of its domain, ~1.2e-3 in the last ~2 degrees before the domain edge (`\|x\| <= sin(85.0511 deg)`, not general (-1, 1) -- see `atanh.h`) |
| `tanh` | ~1e-3 over its fitted domain (`\|x\| <= pi`, not general (-inf, inf) -- see `tanh.h`) |
| `log` | ~2e-4 over the full positive `float32` range (dominated by `atanh`'s own error -- see `log.h`) |
| `exp` | ~2e-7 relative, over `x` in `[-85, 85]` (clamped outside that -- see `exp.h`) |

Each figure is checked by the test suite against `<cmath>` over a dense
sweep, not just asserted. `atan2`/`asin`/`atanh` trade precision for
speed deliberately; none of them are survey-grade or scientific-computing
precision.

## How

Every coefficient here was derived by fitting a short polynomial
against `<cmath>`'s own `sin`/`cos`/`atan` on a reduced range, via
minimax approximation computed by the Remez exchange algorithm (1934)
-- each function's "max error" figure above is that fit's guaranteed
uniform bound.

`sin`/`cos` reduce their argument to the nearest multiple of pi/2 and
evaluate one of two fitted polynomials depending on quadrant, using
trigonometric symmetry (sign flips, a sin/cos swap) to cover the full
circle from a single small-interval fit. `atan2` reduces via the ratio
`min(|x|,|y|)/max(|x|,|y|)`, evaluates a single fitted polynomial on
`[0, 1]`, then corrects for quadrant. `asin(x)` is `atan2(x, sqrt(1-x*x))`
rather than its own fit -- `sqrt` is already a single hardware instruction
under a hard-float ABI, so this costs one `atan2()` call plus one hardware
`sqrt`, with no extra coefficients to maintain.

`atanh` is the one exception to "polynomial fit on a reduced range": a
plain polynomial converges far too slowly approaching `atanh`'s true
singularity at +-1 to be practical, so it's a rational (Padé-style) fit
instead -- `x * N(v) / D(v)`, restricted to `\|x\| <= sin(85.0511 deg)`
(the standard "Web Mercator" pole-latitude limit; not a general (-1, 1)
`atanh`). `N`/`D` are held as Chebyshev coefficients and evaluated via
Clenshaw's recurrence rather than plain powers of `x` -- a monomial-basis
fit of the same degree is numerically unstable in `float32` here
(individual terms reach magnitude ~1-3 that nearly cancel near the
domain edge, losing most of `float32`'s precision); Chebyshev evaluation
avoids that by construction.

`tanh`, unlike `atanh`, has no singularity anywhere, so a plain polynomial
fit (mirroring `sin`/`cos`/`atan2`'s own construction, not `atanh`'s
rational one) converges cleanly -- restricted to `\|x\| <= pi`, the only
domain an inverse Web Mercator projection actually needs it for (`y ==
pi` is exactly the standard pole-latitude limit above), not a general
`(-inf, inf)` `tanh`.

`sincos()` computes both sin and cos of the same angle from one shared
range reduction -- prefer it over calling `sin()` then `cos()` separately
whenever both are needed for the same angle (the common case), which
redoes the reduction and half the polynomial work for nothing.

`log` reuses `atanh` rather than its own fit: `ln(m) == 2*atanh((m-1)/(m+1))`
is an exact identity, and splitting the input into `m * 2^e` (the standard
IEEE-754 range reduction, `m` in `[sqrt(2)/2, sqrt(2))`) keeps that ratio
within +-0.17 -- close enough to the middle of `atanh`'s own fitted domain
that its existing coefficients cover this at a tighter error than
`atanh`'s own worst case, with nothing new to fit.

`exp` reduces via `x = k*ln(2) + r` (`r` in `[-ln(2)/2, ln(2)/2]`), the
same reduce-then-fit shape `sin`/`cos` use for their own quadrant
reduction, then composes a fitted polynomial on `r` with `2^k` built
directly from `r`'s own IEEE-754 exponent bits rather than a multiplication
loop. `ln(2)` is split into a high/low pair so the reduction itself stays
exact for every `k` this needs, rather than dominating the polynomial's
own tighter error at large `x`.

## Trading speed for accuracy

Define `WRENIUM_F32MATH_USE_STD` project-wide and every function here
calls the real `<cmath>` implementation instead of its own
approximation -- still float32 in and out, no source changes needed
anywhere that already calls these by name.

Two costs: `sincos()` becomes two independent `<cmath>` calls instead of
one shared range reduction, and every function loses `constexpr` (`std`'s
own trig isn't usable in a constant expression until C++23).

## Using the library

Header-only. Either add `include/` to your own include path directly, or
consume it as a CMake target:

```cmake
add_subdirectory(path/to/wrenium-f32math) # or FetchContent_Declare + FetchContent_MakeAvailable
target_link_libraries(your_target PRIVATE Wrenium::f32math)
```

Each function has its own self-contained header, or include a single
umbrella header for everything at once:

```cpp
#include <wrenium/f32math/f32math.h>

float s, c;
wrenium::f32math::sincos(angle, s, c);

const float recovered = wrenium::f32math::atan2(s, c);
```

## Building

Requires CMake >= 3.21 and a C++17 compiler. The test suite's only
dependency (doctest) is fetched automatically at configure time.

```sh
git clone <repo-url>
cd wrenium-f32math
cmake -S . -B build
cmake --build build --target tests
ctest --test-dir build

doxygen Doxyfile   # API reference: docs/api/html/index.html
```

## Versioning

Releases follow [Semantic Versioning](https://semver.org/) -- see the
repository's tags and [releases](https://github.com/wrenium/wrenium-f32math/releases)
for what changed in each one.

## License

MIT (see `LICENSE.md`).
