# wrenium-f32math

[![CI](https://github.com/wrenium/wrenium-f32math/actions/workflows/ci.yml/badge.svg)](https://github.com/wrenium/wrenium-f32math/actions/workflows/ci.yml)
[![REUSE status](https://api.reuse.software/badge/github.com/wrenium/wrenium-f32math)](https://api.reuse.software/info/github.com/wrenium/wrenium-f32math)

[API documentation](https://wrenium.github.io/wrenium-f32math/)

A C++17 header-only library of float-only math approximations for
single-precision-FPU embedded targets: `sin`/`cos`/`sincos`/`atan2`/`asin`.

## Accuracy

| Function | Max error |
|---|---|
| `sin`/`cos` | ~9e-7 rad |
| `atan2` | ~6e-4 rad |
| `asin` | ~6e-4 rad (dominated by `atan2`'s own error) |

Each figure is checked by the test suite against `<cmath>` over a dense
sweep, not just asserted. `atan2`/`asin` trade precision for speed
deliberately; neither is survey-grade or scientific-computing precision.

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

`sincos()` computes both sin and cos of the same angle from one shared
range reduction -- prefer it over calling `sin()` then `cos()` separately
whenever both are needed for the same angle (the common case), which
redoes the reduction and half the polynomial work for nothing.

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
