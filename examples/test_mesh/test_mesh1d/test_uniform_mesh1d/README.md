# UniformMesh1D

`cfd::mesh::UniformMesh1D` defines a uniform, cell-centered one-dimensional
mesh for finite-volume calculations. For `N` cells, it stores `N` cell centers
and `N + 1` face coordinates. The first and last faces are the domain
boundaries, and each pair of adjacent faces bounds one control volume.

The class declaration is in
[`include/cfd/mesh/mesh1d/uniform_mesh1d.hpp`](../../../../include/cfd/mesh/mesh1d/uniform_mesh1d.hpp),
and its implementation is in
[`src/mesh/mesh1d/uniform_mesh1d.cpp`](../../../../src/mesh/mesh1d/uniform_mesh1d.cpp).

## Constructing a mesh

Pass either a `Domain1D` or explicit interval bounds, followed by the **number
of cells**:

```cpp
#include <iostream>

#include "cfd/geometry/domain/domain1d.hpp"
#include "cfd/mesh/mesh1d/uniform_mesh1d.hpp"

int main()
{
    const cfd::geometry::Domain1D domain(0.0, 1.0);
    const cfd::mesh::UniformMesh1D mesh(domain, 10);

    std::cout << "Cells: " << mesh.num_cells() << '\n';
    std::cout << "Faces: " << mesh.num_faces() << '\n';
    std::cout << "First center: " << mesh.cell_center(0) << '\n';
    std::cout << "First cell width: " << mesh.cell_width(0) << '\n';
}
```

The equivalent bounds constructor is:

```cpp
const cfd::mesh::UniformMesh1D mesh(-1.0, 1.0, 20);
```

At least one cell is required. Bounds must be finite and strictly increasing.
Invalid or unrepresentable geometry throws `std::invalid_argument`; a cell
count too large to form the face count can throw `std::length_error`.

## Finite-volume geometry

The mesh exposes read-only contiguous spans for its arrays and checked
single-coordinate accessors:

| Quantity | Accessors | Meaning |
| --- | --- | --- |
| Cell count | `num_cells()` | Number of control volumes, `N` |
| Face count | `num_faces()` | Number of faces, `N + 1` |
| Face coordinates | `faces()`, `face(f)`, `face_at(f)` | `x_f[f]`; checked accessors throw `std::out_of_range` |
| Cell-center coordinates | `cell_centers()`, `cell_center(P)`, `cell_center_at(P)`, `operator[](P)` | `x_P[P]`; `operator[]` is unchecked |
| Cell widths | `cell_widths()`, `cell_width(P)` | `x_f[P + 1] - x_f[P]` |
| Nominal spacing | `nominal_spacing()` | `(x_end - x_start) / N` |

Widths are computed from neighboring stored face coordinates, so calculations
can use the actual represented control-volume geometry rather than assuming
each floating-point width is exactly the nominal spacing. Centers are
midpoints of their respective faces.

For an interior cell `P`, `distance_to_east_neighbor(P)` returns
`x_center[P + 1] - x_center[P]`, and `distance_to_west_neighbor(P)` returns
`x_center[P] - x_center[P - 1]`. Each throws `std::out_of_range` when the
requested neighbor does not exist (including an invalid cell index).

## Geometry lifetime and compatibility

Mesh geometry cannot be resized or assigned after construction. This keeps
the cell and face arrays stable for fields that hold a reference to the mesh.
Construct a new mesh when a different resolution is needed.

The former node-named accessors remain as compatibility aliases: nodes map to
faces, `node_count()` maps to `num_faces()`, and `cell_count()` maps to
`num_cells()`. New code should use the explicit FVM names. The constructor's
count is now a **cell count**, whereas the earlier node-centered version
interpreted it as a node count.

## Example program

[`test_uniform_mesh1d.cpp`](test_uniform_mesh1d.cpp) constructs a mesh over a
`Domain1D` and prints its per-cell face coordinates, center, and width using
`print_details(std::cout)`.
