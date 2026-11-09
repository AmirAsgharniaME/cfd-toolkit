# AGENT_REPORT.md

## Status
- **Current Task:** 1D FVM Laplacian Refactoring.
- **Last Completed Step:** Architecture Audit completed.
- **Current Status:** Ready for implementation.

## Progress Log
- [x] Step 1: Technical Audit of current codebase.
- [ ] Step 2: Implementation of row/column off-diagonal matrix API.
- [ ] Step 3: Refactor Laplacian assembly.

## Next Immediate Step
Implement a storage-neutral `Add_OffDiagonal(row, col, value)` API in `FvMatrix1D`.

---

# Technical Audit Report: Systematic Face Flux Assembly in 1D FVM

## Scope and conclusion

This audit covers the current 1D mesh topology, `FvMatrix1D` storage API, the
1D implicit Laplacian, and the gradient/interpolation schemes it calls. The
mesh already exposes enough topology and orientation information to drive a
face-based assembly loop. The remaining architectural constraint is the
matrix wrapper: it stores a tridiagonal system and names its off-diagonal
entries `a_W` and `a_E`, so the operator currently has to translate a cell pair
into those slots itself.

The least disruptive design is to keep the face loop and geometric sign
calculation in `Laplacian`, while adding a cell-pair or face-coupling insertion
operation to `FvMatrix1D`. That operation can validate the pair and route an
off-diagonal coefficient to the lower or upper diagonal internally. This
removes East/West knowledge from the operator without prematurely replacing
the 1D tridiagonal storage. A later unstructured-ready matrix can replace that
implementation with general owner/neighbour addressing behind the same
conceptual operation.

## 1. Current state

### `FvMesh1D` topology and geometry

`include/cfd/mesh/mesh1d/fv_mesh1d.hpp` defines `CellId` and `FaceId` as
`std::size_t`. A mesh owns explicit face coordinates, cell-center coordinates,
and cell volumes. Its topology is currently the canonical 1D line topology:

- `N_Cells()` is the number of cells and `N_Faces()` is `N_Cells() + 1`.
- `Owner(f)` returns cell `0` for the west/start boundary face and `f - 1`
  for every other face.
- `Neighbour(f)` returns `std::nullopt` for face `0` and face `N_Cells()`;
  for an internal face it returns cell `f`.
- `Face_Normal(f)` returns `-1` at the start boundary and `+1` for every
  other face. The documented convention is that this is directed outward from
  the owner.
- `Outward_Normal(p, f)` validates that `p` is incident on `f` and returns
  `-1` for the cell's west face and `+1` for its east face. On an internal
  face, the owner has relative sign `+1` and the neighbour has `-1`.
- `Face_Area(f)` is `1.0` for the unit cross-section convention.

The checked topology methods reject invalid indices and invalid cell/face
pairs. `Dx_W(f)`, `Dx_E(f)`, and `Dx_f(f)` provide the two center-to-face
distances and their sum for internal faces. These names still encode the
current 1D orientation, but the owner/neighbour methods and
`Outward_Normal` do not require an operator to infer west/east from a face
number.

The mesh has no general incident-cell range or face-connectivity object. For
an internal face, callers therefore obtain a two-cell pair through
`Owner(f)` and `Neighbour(f)`, then query each cell's relative outward sign.
Boundary faces have no neighbour and must remain a separate boundary-condition
path, as the current Laplacian contract specifies.

### `FvMatrix1D` coefficient insertion

`include/cfd/fv_matrix/fv_matrix1d.hpp` binds a matrix to a particular
`FvMesh1D` and owns a `linear_algebra::TridiagonalMatrix`. It exposes:

- `a_P(p)`, `a_W(p)`, `a_E(p)`, and `b(p)` accessors;
- `Set_a_W`, `Set_a_P`, `Set_a_E`, and `Set_b`;
- `Add_a_W`, `Add_a_P`, `Add_a_E`, and `Add_b`;
- `Mesh()`, `size()`, `Matrix()`, and `Reset()`.

The wrapper documents that west is the lower diagonal and east is the upper
diagonal. Consequently, it has no API for “add a coefficient from row `p` to
arbitrary neighbouring cell `q`.” The operator must know that the lower slot
means `q = p - 1` and the upper slot means `q = p + 1`. `Matrix()` also exposes
the underlying tridiagonal object, so callers can bypass the wrapper entirely,
although the current Laplacian does not do so.

The underlying `TridiagonalMatrix` likewise only provides indexed lower,
diagonal, upper, and source vectors. It has no owner/neighbour addressing or
generic sparse row/column insertion.

### Current Laplacian assembly

`src/finite_volume/fvm/fvm1d/laplacian1d.cpp` validates that `gamma`, the
matrix, and the mesh are compatible by object identity and size, then loops
over internal faces using numeric bounds (`1 <= f < N_Faces() - 1`). For each
face it obtains:

1. `p_W = mesh.Owner(f)` and `p_E = *mesh.Neighbour(f)`;
2. `Dx_W(f)`, `Dx_E(f)`, and `Face_Area(f)`;
3. a resistance-weighted harmonic face diffusivity, with a zero on either
   side treated as zero face diffusivity;
4. a central-difference unit-jump gradient and the resulting conductance.

It then inserts the conserved two-point diffusion stencil explicitly:

```text
row owner:    +D on diagonal, -D in east slot
row neighbour:+D on diagonal, -D in west slot
```

The comments express the physical rule as owner outward flux
`D * (phi_owner - phi_neighbour)` and the neighbour receiving the opposite
flux. The implementation is mathematically correct for this line mesh, but
the operator itself performs both translations that should eventually belong
to topology/matrix infrastructure: it treats the owner as west, the neighbour
as east, and selects `a_E`/`a_W` manually.

Boundary contributions are intentionally outside this function. The current
Dirichlet boundary object adds a diagonal conductance and source term for the
start or end cell. The Neumann object exposes normal and coordinate-gradient
orientation but does not assemble a matrix term.

### Related schemes

`central_difference(phi_p, phi_n, distance_p, distance_n)` computes the
coordinate gradient `(phi_n - phi_p) / (distance_p + distance_n)` and requires
finite field values and positive finite distances. `harmonic_mean` computes
the resistance-weighted face diffusivity from positive diffusivities and
distances. Both are deliberately pair-oriented and do not know about mesh
orientation or matrix rows. That separation is appropriate: geometry should
provide the ordered pair and distances, while the operator determines the
conservative flux contribution.

## 2. Architectural approaches

### A. Sign-driven face loop with existing matrix API

The operator can retain the current face loop and use `Owner`, `Neighbour`,
`Face_Normal`, and `Outward_Normal` to derive the contribution for each
incident cell. For an internal face with owner `o`, neighbour `n`, intrinsic
face-normal direction, and conductance `D`, the negative-diffusion outward
flux for incident cell `c` can be expressed as:

```text
q_c = -D * s_c * (phi_n - phi_o)
```

where `s_c = Outward_Normal(c, f)` relative to the owner-directed face normal,
and the owner/neighbour value ordering is retained for the face gradient. This
gives `+D` on the row diagonal and `-D` on the other cell in both rows. It
removes hardcoded sign arithmetic from the physical flux calculation, but the
operator would still need a local mapping from the other cell's id to
`a_W` or `a_E`. It is therefore an incremental improvement, not a complete
removal of directional knowledge.

### B. Add a generic adjacent-cell insertion helper to `FvMatrix1D`

Add a matrix-level operation conceptually equivalent to one of these forms:

```text
Add_OffDiagonal(row, column, value)
Add_Face_Coupling(owner, neighbour, conductance)
```

The implementation can use the current tridiagonal storage internally. It
would validate row/column bounds and adjacency, add the diagonal terms, and
route an off-diagonal coefficient to lower or upper storage based on the two
cell ids. The Laplacian would then loop over owner/neighbour pairs and request
the conservative face coupling without mentioning `a_W` or `a_E`.

`Add_OffDiagonal` is the more general matrix primitive; a face-coupling helper
is more expressive for this operator and can centralize the equal-and-opposite
diffusion stencil. Either form can use `Outward_Normal` in the operator (or in
a lower-level flux helper) while keeping storage-specific routing in the
matrix wrapper.

### C. Introduce a small topology/assembly helper separate from the matrix

Create a mesh-side or finite-volume helper that returns a face connectivity
record: owner id, optional neighbour id, intrinsic normal, each incident
cell's outward sign, area, and center-to-face distances. `Laplacian` would
consume that record and call a matrix pair-insertion API or, temporarily, a
small adapter that maps the pair to lower/upper slots.

This makes the operator readable and gives future 2D/3D meshes a place to
provide the same concepts. It adds a new abstraction before there is a second
consumer, so its shape should be kept small and based on stable concepts rather
than 1D names such as west/east.

### D. Replace tridiagonal storage with general owner/neighbour addressing

Adopt an OpenFOAM-like ldu matrix representation: faces provide owner and
neighbour addressing, and coefficient arrays are assembled by face connectivity
rather than by directional slots. The operator contributes per-face
coefficients and the matrix handles row accumulation. This is the strongest
long-term fit for unstructured and dimension-agnostic meshes, but it is a
larger change: the TDMA solver, `FvMatrix1D` public API, boundary assembly, and
existing examples/tests all currently depend on three diagonals.

## 3. Trade-offs and minimal-impact recommendation

Approach A has almost no API cost and can demonstrate the correct geometric
sign convention, but it leaves the operator coupled to the tridiagonal naming
scheme. It is useful as an intermediate verification step, yet it does not
fully meet the stated goal.

Approach B preserves the existing C++20 value types, TDMA storage, solver, and
boundary code. It moves the row/column-to-diagonal translation to the object
that owns matrix storage and lets `Laplacian` express a face coupling in terms
of mesh topology. A pair-based helper can reject non-adjacent cells and make
the current 1D limitation explicit. This is the best fit for the requested
minimal friction.

Approach C improves separation further and establishes reusable topology
vocabulary, but it should follow or accompany the matrix helper rather than
become a speculative generalized mesh framework. The helper should expose
owner/neighbour and orientation as data, not assume that all future meshes are
ordered Cartesian grids.

Approach D is the cleanest endpoint for unstructured FVM and most closely
resembles OpenFOAM's owner/neighbour addressing. It should be treated as a
planned matrix-generation step after the 1D API has stabilized, because doing
it now would expand the change into solver and compatibility work unrelated to
the requested operator audit.

Recommended path:

1. Keep the existing face-based loop and scheme calls.
2. Add a storage-neutral matrix operation for an adjacent owner/neighbour
   pair, with routing implemented inside `FvMatrix1D`.
3. Use `Outward_Normal` and `Face_Normal` (or an equivalent face-sign helper)
   to derive the flux sign, and use owner/neighbour ids only as topology
   identities.
4. Preserve `a_W`/`a_E` as compatibility accessors for current users while
   making them implementation details of pair routing.
5. Record the eventual generalization point: a future matrix can implement the
   same pair/face assembly concept with arbitrary owner/neighbour addressing.

The key design boundary is that geometry supplies orientation and adjacency,
the discretization schemes supply gradient/interpolation values, and the
matrix supplies row/column coefficient storage. The Laplacian should compose
those responsibilities rather than translate all three itself.

## 4. Recommended next steps before implementation

1. **Agree on the matrix seam.** Choose whether the first API is a generic
   `Add_OffDiagonal(row, column, value)` plus diagonal insertion, or a more
   semantic `Add_Face_Coupling(owner, neighbour, conductance)`. The latter is
   lower risk for the current diffusion operator; the former is a better seed
   for convection and source operators.
2. **Write the sign and conservation contract.** Document that the intrinsic
   face normal is owner-directed, that `Outward_Normal(owner, f)` and
   `Outward_Normal(neighbour, f)` are opposites on an internal face, and that
   the negative diffusion operator contributes `+D` to each incident diagonal
   and `-D` to the opposite-cell column.
3. **Define validation behavior.** Decide how pair insertion handles boundary
   faces, non-adjacent cells, reversed pairs, self-pairs, invalid ids, and
   conductances that are non-finite. Keep these checks in the matrix or a
   shared assembly helper so every operator gets the same behavior.
4. **Add focused contract checks before changing the operator.** Cover owner
   and neighbour rows on uniform and graded meshes, both orientations of the
   relative normal, conservation of the two assembled rows, and the single-cell
   boundary-only case. Existing numerical checks should remain unchanged.
5. **Refactor the Laplacian only after the seam is tested.** Replace the four
   explicit `Add_a_*` calls with the chosen pair/face operation, keeping the
   existing harmonic interpolation, zero-diffusivity handling, overflow checks,
   and boundary separation.
6. **Plan the 2D/3D follow-up separately.** Once the 1D helper is stable,
   compare its concepts with a general owner/neighbour addressing structure.
   Avoid naming a new abstraction around west/east or assuming that every
   future matrix is tridiagonal.

## Files inspected

- `include/cfd/mesh/mesh1d/fv_mesh1d.hpp`
- `src/mesh/mesh1d/fv_mesh1d.cpp`
- `include/cfd/fv_matrix/fv_matrix1d.hpp`
- `src/fv_matrix/fv_matrix1d.cpp`
- `include/cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp`
- `src/finite_volume/fvm/fvm1d/laplacian1d.cpp`
- `include/cfd/linear_algebra/matrix/tridiagonal_matrix.hpp`
- `include/cfd/schemes/gradient/central_difference.hpp`
- `src/schemes/gradient/central_difference.cpp`
- `include/cfd/schemes/interpolation/harmonic_mean.hpp`
- `src/schemes/interpolation/harmonic_mean.cpp`
- `include/cfd/boundary/boundary1d/dirichlet_boundary1d.hpp`
- `src/boundary/boundary1d/dirichlet_boundary1d.cpp`
- `include/cfd/boundary/boundary1d/neumann_boundary1d.hpp`
- `src/boundary/boundary1d/neumann_boundary1d.cpp`
- `examples/test_mesh/test_mesh1d/test_uniform_mesh1d/test_uniform_mesh1d.cpp`

