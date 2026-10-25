# Domain1D

`cfd::geometry::Domain1D` represents a one-dimensional, axis-aligned interval
with finite, strictly increasing X bounds. The stored endpoints are authoritative;
`length_x()` returns their represented difference, `x_end() - x_start()`.

## Construction

```cpp
#include "cfd/geometry/domain/domain1d.hpp"

using cfd::geometry::Domain1D;

Domain1D origin_domain(4.0);                    // [0, 4]
Domain1D bounded_domain(-2.0, 3.0);             // [-2, 3]
Domain1D shifted = Domain1D::from_start_and_length(10.0, 2.5); // [10, 12.5]
```

The one-argument constructor takes a length and starts at zero. The two-argument
constructor takes the start and end coordinates. Use
`from_start_and_length(start, length)` when the desired inputs are a start and a
length; it computes the end as `start + length` using floating-point arithmetic.

All constructors reject invalid intervals with `std::invalid_argument`. Bounds
must be finite and strictly increasing, and their difference must also be finite.
Length inputs must be finite and greater than zero. A computed endpoint that
overflows or rounds back to the start is rejected.

## Bounds and queries

- `x_start()` and `x_end()` return the stored bounds.
- `length_x()` returns `x_end() - x_start()`; it can differ from a requested
  length because of floating-point rounding, especially for large start values.
- `contains(x)` returns `true` when `x` is in the closed interval
  `[x_start(), x_end()]`.
- `set_bounds(start, end)` replaces both bounds after validation.
- `set_x_start(start)` and `set_x_end(end)` update one bound while retaining the
  other, and enforce the same interval rules.
- `set_length_x(length)` preserves the current start and sets the end to
  `start + length`, with the same rounding behavior as
  `from_start_and_length()`.

If a setter receives invalid values, it throws `std::invalid_argument` and
leaves the domain unchanged. `operator==` and `operator!=` compare both bounds
using exact floating-point equality.

## Mesh interaction

Finite-volume meshes constructed from a `Domain1D` take their own snapshot of its
bounds. Changing the domain later does not update or invalidate an existing
mesh; construct a new mesh to use changed bounds.
