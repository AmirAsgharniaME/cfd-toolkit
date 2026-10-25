# One-Dimensional Finite-Volume Meshes

The `cfd::mesh` namespace provides `FvMesh1D`, the common read-only geometry
interface, and `UniformFvMesh1D`, a uniform mesh implementation. A mesh has
`N` cells and `N + 1` faces. Cell indices run from `0` to `N - 1`; face indices
run from `0` to `N`.

## Constructing a uniform mesh

```cpp
#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/mesh1d/uniform_fv_mesh1d.hpp"

cfd::geometry::Domain1D domain(-1.0, 1.0);
cfd::mesh::UniformFvMesh1D mesh(domain, 4);

double first_center = mesh.cell_center(0);
double first_width = mesh.cell_width(0);
double nominal_dx = mesh.dx();
```

`UniformFvMesh1D` can also be constructed directly from `x_start`, `x_end`,
and a cell count. Constructing from a `Domain1D` copies its current bounds;
subsequent domain changes do not change the mesh.

The bounds must be finite and strictly increasing, and the represented interval
length must be finite. At least one cell is required. Invalid bounds, a zero
cell count, invalid spacing, or coordinates that cannot be represented as
strictly increasing faces throw `std::invalid_argument`. A cell count too large
to allocate the required faces throws `std::length_error`.

`dx()`, `spacing()`, and `nominal_spacing()` are aliases for the nominal
spacing `(x_end - x_start) / num_cells`. Floating-point rounding can make
individual face-to-face distances differ from this target; use `cell_width(P)`
or `cell_volumes()[P]` as the actual finite-volume metric.

`UniformMesh1D` and `Mesh1D` are aliases for `UniformFvMesh1D`.

## Geometry queries

`FvMesh1D` exposes the geometry shared by mesh implementations:

- `num_cells()` / `cell_count()` and `num_faces()` / `face_count()` return the
  cell and face counts.
- `x_start()` and `x_end()` return the mesh boundary coordinates.
- `face(f)` / `face_at(f)` return a face coordinate.
- `cell_center(P)` / `cell_center_at(P)` return a cell center. The
  `operator[](P)` shorthand also returns the center, but does not check bounds.
- `cell_width(P)` returns the width derived from adjacent faces.
- `cell_volume(P)` and `cell_volumes()` return cell widths, assuming Cartesian
  geometry with unit cross-sectional area (`V = A * dx`, `A = 1`).
- `faces()`, `cell_centers()`, and `cell_widths()` return read-only
  `std::span<const double>` views of the corresponding arrays.
- `distance_to_east_neighbor(P)` and `distance_to_west_neighbor(P)` return
  center-to-center distances. They throw `std::out_of_range` if `P` is invalid
  or the requested neighbor does not exist.
- `print_details(output)` writes the bounds, counts, and per-cell face, center,
  and width information to the supplied output stream.

The checked indexed queries (`face`, `face_at`, `cell_center`,
`cell_center_at`, and `cell_width`) throw `std::out_of_range` for invalid
indices. `operator[]` is unchecked.

Meshes can be copied by construction. Copy assignment is disabled; move
construction and move assignment are supported. Moving from or move-assigning a
mesh replaces or transfers its geometry, so do not do so while fields or views
depend on it. A moved-from mesh may only be destroyed or assigned a new value
before making geometry queries.
