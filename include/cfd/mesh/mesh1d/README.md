# One-Dimensional Finite-Volume Meshes

The `cfd::mesh` namespace provides `FvMesh1D`, the common read-only geometry
interface, `UniformFvMesh1D`, a uniform mesh implementation, and
`NonUniformFvMesh1D`, which accepts explicit faces. A mesh has
`N` cells and `N + 1` faces. Cell indices run from `0` to `N - 1`; face indices
run from `0` to `N`.

## Generating geometrically graded faces

`cfd::mesh::grading::geometric(domain, n_cells, ratio)` returns `n_cells + 1`
face coordinates spanning the exact domain endpoints. Cell widths follow a
geometric progression where each width is the preceding width multiplied by
`ratio`; `ratio == 1.0` gives uniform spacing. At least two cells and a
positive finite ratio are required. The returned faces can be passed directly
to `NonUniformFvMesh1D`.

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

`UniformFvMesh1D` can also be constructed directly from `start`, `end`,
and a cell count, using `Domain1D(start, end)` for bounds validation.
Constructing from a `Domain1D` snapshots its current authoritative endpoints;
subsequent domain changes do not change the mesh. Construct a new mesh to use
updated bounds. When using `Domain1D::from_start_and_length` or `set_length`,
nominal spacing uses `domain.length()` (the represented endpoint difference),
which can differ from the requested length due to rounding.

The bounds must be finite and strictly increasing, and the represented interval
length must be finite. At least one cell is required. Invalid bounds, a zero
cell count, invalid spacing, or coordinates that cannot be represented as
strictly increasing faces and distinct interior cell centers throw
`std::invalid_argument`. A valid domain can therefore be too small to mesh at
the requested resolution. A cell count too large
to allocate the required faces throws `std::length_error`.

`dx()`, `spacing()`, and `nominal_spacing()` are aliases for the nominal
spacing `(end() - start()) / num_cells`. Floating-point rounding can make
individual face-to-face distances differ from this target; use `cell_width(P)`
or `cell_volumes()[P]` as the actual finite-volume metric.

`UniformMesh1D` and `Mesh1D` are aliases for `UniformFvMesh1D`.

## Constructing a non-uniform mesh

```cpp
#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/mesh1d/non_uniform_fv_mesh1d.hpp"

cfd::geometry::Domain1D domain(0.0, 4.0);
cfd::mesh::NonUniformFvMesh1D mesh(domain, {0.0, 0.5, 2.0, 4.0});
```

The face vector must have at least two finite, strictly increasing values.
Its endpoints must exactly match `domain.start()` and `domain.end()`; no
tolerance is applied, and the supplied boundary coordinates are preserved.
The constructor snapshots both bounds and face geometry. Later changes to the
domain or the input vector do not change the mesh. As with the base class,
construction also rejects intervals where a distinct interior cell center
cannot be represented.

## Geometry queries

`FvMesh1D` exposes the geometry shared by mesh implementations:

- `num_cells()` / `cell_count()` and `num_faces()` / `face_count()` return the
  cell and face counts.
- `start()` and `end()` return the snapshotted boundary coordinates;
  `length()` returns their finite, positive represented difference, matching
  `Domain1D`. `x_start()` and `x_end()` remain compatibility aliases.
  The base constructor validates the complete interval with `Domain1D`,
  including for meshes constructed from explicit face arrays.
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
