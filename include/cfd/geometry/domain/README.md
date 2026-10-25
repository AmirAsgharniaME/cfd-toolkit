# Geometry domains

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

## Rectangular and cuboid domains

`Domain2D` and `Domain3D` compose two or three validated `Domain1D` values.
Include `domain2d.hpp` or `domain3d.hpp` from this directory and construct them
with an interval for each axis. The endpoint and finite-extent rules apply
independently to every axis; no area or volume representability is guaranteed.

Construction and axis replacement copy their arguments. Axis getters return
`Domain1D` values: they are independent snapshots, remain valid after the parent
is destroyed, and do not track later parent mutations. To change an axis, use
`set_x_domain`, `set_y_domain`, or (in 3D) `set_z_domain`. `set_domains` replaces
all axes together. Invalid interval arguments throw during argument construction,
before any parent state changes. Once arguments exist, these constructors and
setters perform only nonthrowing value copies and are declared `noexcept`.

Containment is the conjunction of exact closed-interval checks on all axes.
NaN and infinite coordinates are outside. Closed domains can share boundary
points; containment does not implement mesh cell ownership. Equality compares
stored endpoints exactly (positive and negative zero compare equal); it does
not apply a tolerance.

Only the 1D mesh currently consumes a domain in this repository. A valid domain
need not support an arbitrary cell count: mesh construction must still check
spacing and coordinate representability. Multidimensional mesh integration
should likewise copy endpoint values when constructing a mesh snapshot.
