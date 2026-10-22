# Domain1D

`Domain1D` represents a finite, increasing interval on the X axis. It stores
the interval bounds and provides its length; it does not contain a mesh or a
finite-volume discretization.

## Construction

```cpp
#include "cfd/geometry/domain/domain1d.hpp"

cfd::geometry::Domain1D from_origin(2.0);       // [0, 2]
cfd::geometry::Domain1D interval(-1.0, 1.0);    // bounds: [-1, 1]
cfd::geometry::Domain1D shifted =
    cfd::geometry::Domain1D::from_start_and_length(-1.0, 2.0); // [-1, 1]
```

The one-argument constructor takes a length and starts the interval at `0.0`.
The two-argument constructor takes the start and end coordinates. To specify a
start and a length, use `from_start_and_length`; this named factory avoids
confusing those two meanings. All construction paths require finite,
strictly increasing bounds.

## Bounds and length

Read the interval with `x_start()`, `x_end()`, and `length_x()`. Bounds can be
updated with `set_bounds(start, end)`, `set_x_start(start)`, or
`set_x_end(end)`. `set_length_x(length)` changes the length while preserving
the current start coordinate.

Every update enforces finite, strictly increasing bounds. Lengths must be
finite and greater than zero. Invalid input throws `std::invalid_argument`;
the current bounds remain unchanged when an update is rejected.

## Use with a finite-volume mesh

Use `Domain1D` to define the physical interval, then construct a mesh over it.
For example, `cfd::mesh::UniformMesh1D` accepts a domain and a cell count:

```cpp
#include "cfd/mesh/mesh1d/uniform_mesh1d.hpp"

cfd::geometry::Domain1D domain(0.0, 1.0);
cfd::mesh::UniformMesh1D mesh(domain, 10);
```

The mesh supplies the finite-volume faces, cell centers, and cell widths;
`Domain1D` only supplies the geometric bounds.
