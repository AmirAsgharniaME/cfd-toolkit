# `Domain1D`

`Domain1D` is a small geometry class for a one-dimensional CFD domain. It
represents one finite, strictly increasing interval on the X axis. Its
implementation is declared in
[`include/cfd/geometry/domain/domain1d.hpp`](../../../../include/cfd/geometry/domain/domain1d.hpp)
and defined in
[`src/geometry/domain/domain1d.cpp`](../../../../src/geometry/domain/domain1d.cpp).

The object stores two values: `x_start` and `x_end`. `length_x()` returns their
difference, so the length remains consistent after any successful update.

## Constructing a domain

```cpp
#include <cfd/geometry/domain/domain1d.hpp>

// A length-only domain starts at zero: [0, 4].
cfd::geometry::Domain1D from_origin(4.0);

// The two-argument constructor takes the start and end coordinates: [-2, 3].
cfd::geometry::Domain1D from_bounds(-2.0, 3.0);

// Use the named factory when the second value is a length: [5, 7.5].
auto shifted = cfd::geometry::Domain1D::from_start_and_length(5.0, 2.5);
```

The named factory avoids ambiguity between an interval's end coordinate and
its length.

Stored endpoints are authoritative. The factory and `set_length_x()` compute
the end by floating-point addition, and `length_x()` reports the represented
endpoint difference. Rounding can make that difference depart from the requested
length, especially at large coordinate magnitudes. An endpoint that overflows
or rounds back to the start is rejected.

## Reading and changing bounds

The accessors `x_start()`, `x_end()`, and `length_x()` read the current
interval. The setters update it:

```cpp
cfd::geometry::Domain1D domain(-2.0, 3.0);
domain.set_x_start(-3.0);  // Keep the end at 3.0.
domain.set_x_end(4.0);     // Keep the start at -3.0.
domain.set_length_x(2.0);  // Keep the start; set the end to start + 2.
domain.set_bounds(1.0, 6.0);
```

Bounds must be finite and strictly increasing (`x_end > x_start`), and their
difference must also be finite. Lengths must be finite and greater than zero.
Invalid constructor arguments or updates throw `std::invalid_argument`.
Updates validate the proposed values before changing either stored bound, so
an invalid update leaves the domain unchanged.

## Role in a finite-volume solver

`Domain1D` defines where the physical interval begins and ends. It does not
discretize that interval. A mesh uses these bounds to create finite-volume
faces, cells, centers, and widths. For example, the project's
`cfd::mesh::UniformMesh1D` accepts a `Domain1D` and a cell count:

```cpp
#include <cfd/geometry/domain/domain1d.hpp>
#include <cfd/mesh/mesh1d/uniform_fv_mesh1d.hpp>

cfd::geometry::Domain1D domain(0.0, 1.0);
cfd::mesh::UniformMesh1D mesh(domain, 10); // Ten cells, eleven faces.
```

The mesh takes an independent snapshot of the bounds at construction. Later
changes to `domain` neither synchronize nor invalidate that mesh. Construct a
new mesh to discretize the updated interval.

The domain also does not hold boundary conditions, field values, material
properties, or cross-sectional area. Those are provided by other geometry,
mesh, field, and solver components as required by the problem.

## Running the example

The companion [`test_domain1d.cpp`](test_domain1d.cpp) demonstrates the
constructors, setters, rejected updates, and the class's scope. With examples
enabled, build it from the project root with:

```sh
cmake -S . -B build -DBUILD_EXAMPLES=ON
cmake --build build --target test_domain1d
```

The executable is written to `build/bin/test_domain1d`.
