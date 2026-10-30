# 1D Mesh Construction and Data Flow

The files in this directory divide 1D mesh work into face generation, mesh
ownership, and shared geometry calculations. There is no central mesh factory
or input reader: the calling code creates a `Domain1D`, chooses a construction
route, and supplies the cell count or face coordinates.

## Files and responsibilities

- `uniform_fv_mesh1d.hpp` / `src/mesh/mesh1d/uniform_fv_mesh1d.cpp` implement
  `UniformFvMesh1D`. The caller supplies a `Domain1D` and cell count, or the
  start and end coordinates and cell count. The coordinate overload first
  constructs a `Domain1D`. The class computes nominal spacing as the represented
  domain length divided by the cell count and generates the face coordinates.
  Boundary faces retain the domain endpoints exactly. `dx()`, `spacing()`, and
  `nominal_spacing()` report the target spacing; actual widths come from the
  stored faces. `UniformMesh1D` and `Mesh1D` are aliases for this class.
- `grading1d.hpp` / `src/mesh/mesh1d/grading1d.cpp` implement the free function
  `cfd::mesh::grading::make_1d_faces`. The caller supplies a domain, at least
  two cells, a `GradingType`, and a ratio. The function builds normalized cell
  width weights, converts cumulative weights to faces, and returns a vector of
  `N + 1` coordinates. It preserves the exact domain endpoints and validates
  that faces and interior centers are representable. It does not construct or
  own a mesh. The grading enum itself is declared in
  `include/cfd/mesh/grading_type.hpp`.
- `non_uniform_fv_mesh1d.hpp` /
  `src/mesh/mesh1d/non_uniform_fv_mesh1d.cpp` implement
  `NonUniformFvMesh1D`. The caller supplies a domain and explicit face vector.
  This is also the consumer for faces returned by `make_1d_faces`; the caller
  connects the generator to this constructor.
- `fv_mesh1d.hpp` / `src/mesh/mesh1d/fv_mesh1d.cpp` define the common base
  `FvMesh1D`. Its protected constructor takes ownership of a face vector,
  validates the full interval and face sequence, then derives each cell width
  and center. Derived mesh classes therefore share the same stored geometry
  and query API.

## Input flow and termination

All construction starts at the caller. A `Domain1D` supplies authoritative,
finite, increasing bounds; mesh constructors use those values as a snapshot, so
later domain changes do not change an existing mesh.

```text
Caller
  ├─ Domain1D + cell count ──> UniformFvMesh1D
  │                              └─ computes nominal spacing and faces
  ├─ Domain1D + explicit faces ─> NonUniformFvMesh1D
  └─ Domain1D + cell count + GradingType + ratio
                                 └─ grading::make_1d_faces -> face vector
                                      └─ caller passes vector to NonUniformFvMesh1D

UniformFvMesh1D / NonUniformFvMesh1D
  └─ FvMesh1D validates faces and derives widths and centers
       └─ read-only geometry queries and spans exposed to downstream code
```

For `UniformFvMesh1D`, the input terminates as an owned mesh containing its
faces, widths, and centers. For explicit nonuniform input, validation happens
first in `NonUniformFvMesh1D` (finite, strictly increasing faces whose first
and last values exactly match the domain), then shared validation and
derivation happen in `FvMesh1D`. The grading route terminates first at the
returned face vector. It reaches a mesh only if the caller passes that vector
to `NonUniformFvMesh1D`, which applies the same checks.

After construction, consumers read geometry through `FvMesh1D` methods such as
`face()`, `cell_center()`, `cell_width()`, and `cell_volume()`, or through its
read-only spans. Volumes assume unit cross-sectional area, so their numerical
values equal cell widths. The mesh layer does not itself advance a simulation
or read field values; it ends by providing validated, immutable-through-this-API
geometry for finite-volume and field code.

The examples under `examples/test_mesh/test_mesh1d/` demonstrate these paths:
the uniform example constructs a uniform mesh, and the grading example
generates faces and passes them to `NonUniformFvMesh1D`.
