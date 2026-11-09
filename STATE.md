# Project State: CFD Toolkit (1D FVM)

## 1. Project Overview & Architecture

- Language standard: C++20, declared as a public requirement via
  `target_compile_features(cfd_toolkit PUBLIC cxx_std_20)` in `CMakeLists.txt`.
  CMake 3.20 or newer is required. There are no external dependencies.
- Build product: the static library `cfd_toolkit`, exported as the in-tree alias
  `cfd::toolkit` and as the installed package target `cfd::toolkit`
  (`find_package(cfd-toolkit CONFIG REQUIRED)`, `SameMajorVersion`).
- Design philosophy: finite-volume quantities are represented by explicit value
  types (geometry, mesh, field, matrix) with checked access, and discretization
  is kept in separate free functions rather than fused into a solver object.
  Matrix assembly operators only accumulate into an existing
  `FvMatrix1D`; they never allocate, size, or solve a system.
- Source list: explicit in the root `CMakeLists.txt`, grouped by subsystem.
  Files are not discovered by globbing, so a new `.cpp` under `src/` is not
  compiled until it is added to that list.
- Folder organization:
  - `include/cfd/` — public headers; `src/` — matching implementations, kept in
    the same module subdirectory.
  - `geometry/domain/` — `Domain1D`, `Domain2D`, `Domain3D`.
  - `mesh/` — dimension-agnostic `GradingType`; `mesh/mesh1d/` — `FvMesh1D` and
    `grading1d`.
  - `field/field1d/` — templated `Field1D` plus non-member operators.
  - `fv_matrix/` — discretized-equation container `FvMatrix1D`.
  - `finite_volume/fvm/fvm1d/` — implicit operators (`Laplacian` only).
    `finite_volume/fvc/fvc1d/` — explicit operators (empty placeholders).
  - `schemes/gradient/`, `schemes/interpolation/` — free-function discretization
    utilities.
  - `boundary/boundary1d/` — Dirichlet, Neumann, patch (placeholder).
  - `linear_algebra/` — `TridiagonalMatrix`, `TDMASolver`, sparse/iterative
    placeholders.
  - `io/`, `time/`, `transport_model/`, `solvers/`, `vector/` — mostly reserved;
    `vector/vector1d.hpp` is a real one-component value type.
  - `examples/` — standalone driver/contract executables (registered under
    `BUILD_EXAMPLES`, output to `${CMAKE_BINARY_DIR}/bin`). There is no
    `applications/` directory in this repository.
- Namespaces: `cfd::geometry`, `cfd::mesh`, `cfd::mesh::grading`, `cfd::field`,
  `cfd::fv_matrix`, `cfd::fvm`, `cfd::boundary`, `cfd::schemes::gradient`,
  `cfd::schemes::interpolation`, `cfd::linear_algebra`, `cfd::vector`.
- Compiler diagnostics for the library target (GNU/Clang only, private):
  `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`. No formatter or linter
  configuration is present.
- Markdown policy in force: only `FILEPATH.md` may be maintained as project
  inventory; README files are not to be read or modified.

## 2. Mesh & Geometric Entities (`FvMesh1D`)

- Header `include/cfd/mesh/mesh1d/fv_mesh1d.hpp`, source
  `src/mesh/mesh1d/fv_mesh1d.cpp`. `CellId` and `FaceId` are both
  `std::size_t`.
- Storage: three owning vectors — `face_coords_` (`N_Faces()` = `N_Cells() + 1`),
  `cell_center_coords_` (`N_Cells()`), and `cell_volumes_` (`N_Cells()`).
  Cell volume equals cell length under a unit cross-sectional area; face area is
  fixed at `1.0` by `Face_Area(f)`.
- Construction: uniform `FvMesh1D(domain, n_cells)` or graded
  `FvMesh1D(domain, n_cells, GradingType, ratio)`. `n_cells >= 1`.
  `FvMesh1D` snapshots the domain endpoints exactly at construction; later
  `Domain1D` mutation does not alter the mesh. Faces must be finite and strictly
  increasing; centers must be finite, strictly inside their faces, and strictly
  increasing; excessive counts throw `std::length_error`.
- Copy/move construction is available; assignment is deleted. A moved-from mesh
  is empty (`N_Cells() == 0`), and `Start_Coord()`, `End_Coord()`, `Length()`,
  and `Print_Details()` throw `std::logic_error`.
- Cell indexing: cell `p` is bounded by face `p` (west) and face `p + 1` (east).
  `wFace_Coord(p)`/`eFace_Coord(p)` return those coordinates;
  `wFaceDx(p)`/`eFaceDx(p)` return the center-to-face distances;
  `wCellDx(p)`/`eCellDx(p)` return center-to-center distances to the neighbouring
  cells and throw `std::out_of_range` when that neighbour does not exist.
- Face indexing: face `0` is the west boundary, face `N_Faces() - 1`
  (equivalently `N_Cells()`) is the east boundary. Internal faces are
  `1 <= f < N_Cells()`.
- Face topology: `Owner(f)` returns `0` for face `0` and `f - 1` otherwise, so
  the owner is always the cell on the west (lower-index) side. `Neighbour(f)`
  returns `std::nullopt` for faces `0` and `N_Cells()`, and `CellId{f}` for
  internal faces, so the neighbour is the cell on the east (higher-index) side.
  Invalid face indices throw `std::out_of_range`.
- Unit face normals: `Face_Normal(f)` returns `-1.0` at face `0` and `+1.0` at
  every other face, i.e. the normal always points out of the owner. Face area is
  `1.0`, so the normal is a unit normal in the 1D sense.
- Cell outward normal: `Outward_Normal(p, f)` returns `-1.0` when `f == p`
  (west face of cell `p`) and `+1.0` when `f == p + 1` (east face). It throws
  `std::out_of_range` for an invalid index or a non-incident cell-face pair.
  Consequently the neighbour sees the opposite sign from the owner on a shared
  internal face.
- Internal-face stencil distances: `Dx_W(f)` = `Face_Coord(f) -
  Cell_Center_Coord(Owner(f))`, `Dx_E(f)` = `Cell_Center_Coord(*Neighbour(f)) -
  Face_Coord(f)`, `Dx_f(f)` = their sum. All three require an internal face and
  throw `std::out_of_range` for `f == 0` or `f >= N_Cells()`.
- Read-only geometry views: `Face_Coords()`, `Cell_Center_Coords()`,
  `Cell_Volumes()` (spans). Indexed access uses checked `.at()`
  (`Face_Coord`, `Cell_Center_Coord`, `Cell_Volume`). There is no unchecked
  cell `operator[]` on the mesh.
- Grading (`include/cfd/mesh/grading_type.hpp`, `mesh1d/grading1d.hpp`):
  `GradingType` = `Uniform`, `Forward`, `Backward`, `BothEnds`, `Center`;
  `grading::Make_Faces(domain, n_cells, type, r)` and
  `grading::Convert_R_To_r(n_cells, type, R)` are free functions. Graded modes
  require `r >= 1.0`; `r == 1.0` gives uniform widths. Intermediate exponent and
  weight arithmetic uses `long double`; returned faces are `double` and
  endpoints are anchored exactly.

## 3. Matrix Assembly API (`FvMatrix1D`)

- Header `include/cfd/fv_matrix/fv_matrix1d.hpp`, source
  `src/fv_matrix/fv_matrix1d.cpp`. `FvMatrix1D` is a cell-centered wrapper
  around `cfd::linear_algebra::TridiagonalMatrix`; it holds a
  `const FvMesh1D&` and sizes the system from `mesh.N_Cells()` (empty mesh
  rejected with `std::invalid_argument`). The mesh must outlive the matrix and
  must not be moved.
- Storage model (`TridiagonalMatrix`, `include/cfd/linear_algebra/matrix/
  tridiagonal_matrix.hpp`): four independent `std::vector<double>` of exactly
  `size()` entries, all row-indexed by cell `p`:
  - `diag_` — central coefficient `a_P`, main diagonal (`Diag(p)`).
  - `lower_` — west coefficient `a_W`, sub-diagonal (`Lower(p)`).
  - `upper_` — east coefficient `a_E`, super-diagonal (`Upper(p)`).
  - `source_` — right-hand side `b` (`Source(p)`).
  No sparsity pattern, face indexing, or block structure is stored.
- Row semantics: row `p` is
  `a_W(p) * phi[p-1] + a_P(p) * phi[p] + a_E(p) * phi[p+1] = b(p)`,
  with the out-of-system off-diagonals unused.
- Coefficient API on `FvMatrix1D`: read `a_W(p)`, `a_P(p)`, `a_E(p)`, `b(p)`;
  write `Set_a_W`, `Set_a_P`, `Set_a_E`, `Set_b`; accumulate `Add_a_W`,
  `Add_a_P`, `Add_a_E`, `Add_b`, and `Add_OffDiagonal(row, col, value)`;
  clear `Reset()`. Also `Mesh()`, `size()`,
  and `Matrix()` (mutable and const access to the underlying
  `TridiagonalMatrix`). Construction is copyable/movable; assignment is
  deleted.
- The underlying `TridiagonalMatrix` also exposes indexed and whole-vector
  overloads of `Diag`/`Lower`/`Upper`/`Source` and `Set_*`/`Add_*`.
- `Add_OffDiagonal(row, col, value)` routes an adjacent column below the row
  to lower storage and an adjacent column above it to upper storage. Equal
  indices throw `std::invalid_argument` before bounds are checked; out-of-range
  or nonadjacent indices throw `std::out_of_range`. Diagonal accumulation uses
  `Add_a_P`. This row-and-column seam lets face assembly avoid choosing a
  directional off-diagonal slot, while retaining tridiagonal storage.
- Known storage/abstraction limitations:
  - Directional coefficient accessors remain available, and there is no
    face-indexed `Add_Face(f, ...)` entry point.
  - Only one sub- and one super-diagonal exist. Second neighbours, 2D/3D
    connectivity, and reordered cell numbering are not representable.
  - The row-0 lower and last-row upper entries exist in the vectors but are
    dead storage: `Set_Lower(0, ...)` and `Add_Lower(0, ...)` return without
    writing, and `Set_Upper(n-1, ...)` and `Add_Upper(n-1, ...)` do the same.
    They read back as `0.0`.
  - There is no per-row clear, no equation-relaxation or under-relaxation
    support, no symmetry or diagonal-dominance check, and no enforcement of a
    sign convention — assembly code is responsible for the sign of every
    coefficient it adds.
  - `Matrix()` hands out the raw system, so callers can bypass the wrapper's
    named accessors.
- `linear_algebra::TDMASolver` solves the system: `solve(TridiagonalMatrix&,
  std::vector<double>&)` performs forward elimination in place (mutating the
  matrix), while `solve(TridiagonalMatrix)` takes the matrix by value and
  returns the solution. It rejects an empty system, inconsistent coefficient
  vector sizes, and pivots with absolute value below `1.0e-14`
  (`std::runtime_error`). No convergence control or iteration count is
  involved.

## 4. Current Operators & Discretization (`Laplacian1D`)

- Entry point: `cfd::fvm::Laplacian(const VolScalarField1D& gamma,
  FvMatrix1D& matrix)` in
  `include/cfd/finite_volume/fvm/fvm1d/laplacian1d.hpp` and
  `src/finite_volume/fvm/fvm1d/laplacian1d.cpp`. It is the only implemented
  implicit FVM operator; `ddt1d`, `fvm::div1d`, `fvm::source1d`, and the
  `fvc1d` explicit operators are headers containing only `#pragma once` plus
  empty translation units.
- Operator assembled: the internal-face contributions of the negative
  diffusion operator `-div(gamma grad(phi))`, integrated over each cell.
  The code applies the face conductance directly and does not divide by
  `Cell_Volume`.
- Validation: the coefficient field and the matrix must reference the same mesh
  object (pointer identity, not equal geometry); field and matrix sizes must
  equal `mesh.N_Cells()`; every cell diffusivity must be finite and
  nonnegative. Violations throw `std::invalid_argument`. A non-finite
  assembled conductance throws `std::overflow_error`.
- Flux definition and sign convention: for internal face `f` with owner
  `p_W = Owner(f)` and neighbour `p_E = *Neighbour(f)`, the owner-side outward
  flux is defined as
  `D_f * (phi[p_W] - phi[p_E])`, where
  `D_f = gamma_f * A_f / (Dx_W(f) + Dx_E(f))` and `A_f = Face_Area(f) = 1.0`.
  The neighbour receives the equal and opposite flux `-D_f * (phi[p_W] -
  phi[p_E])`. Summing over faces therefore produces a positive diagonal and
  negative off-diagonals.
- Scheme composition inside the face loop:
  - Diffusivity interpolation uses
    `schemes::interpolation::harmonic_mean(gamma[p_W], gamma[p_E],
    Dx_W(f), Dx_E(f))`, i.e.
    `gamma_f = (d_P + d_N) / (d_P/gamma_P + d_N/gamma_N)` with distances
    measured from each center to the shared face.
  - Because `harmonic_mean` requires strictly positive diffusivities, a zero
    diffusivity on either side is short-circuited to `gamma_f = 0.0` before the
    call; no face contribution is added in that case.
  - The geometric factor is obtained from
    `schemes::gradient::central_difference(0.0, 1.0, Dx_W(f), Dx_E(f))`,
    i.e. `1 / (Dx_W(f) + Dx_E(f))`, and multiplied by `gamma_f * face_area`.
  - `arithmetic_mean` (distance-weighted linear interpolation) exists and is
    tested, but is not used by `Laplacian`. `upwind` is an empty placeholder
    (header and source contain no declaration or definition).
- Matrix stencil written per internal face `f` in the loop
  `for (FaceId f = 1; f + 1 < n_face; ++f)`:
  - `matrix.Add_a_P(p_W, +D_f)`
  - `matrix.Add_a_P(p_E, +D_f)`
  - `matrix.Add_OffDiagonal(p_W, p_E, -D_f)`
  - `matrix.Add_OffDiagonal(p_E, p_W, -D_f)`

  With unit mesh spacing and unit diffusivity this gives `a_P = 1` for a
  one-sided interior cell, `a_P = 2` for a cell between two faces, and
  `a_W = a_E = -1`, matching the 3-point central-difference stencil.
- Boundary condition strategy — boundary faces are not touched by
  `Laplacian`; they are applied by a separate API:
  - Dirichlet: `DirichletBoundary1D::Apply(gamma, matrix)` selects cell `0`
    for `BoundarySide1D::Start` with distance `wFaceDx(0)`, or cell
    `N_Cells()-1` for `BoundarySide1D::End` with distance `eFaceDx(...)`, and
    adds `D = gamma[p] / distance` to `a_P(p)` (unit face area) and
    `D * value` to `b(p)`. This is consistent with the negative diffusion
    operator's sign convention. It validates the boundary value, the side, the
    boundary-cell diffusivity, and the distance, and throws
    `std::overflow_error` if the contribution or the accumulated row is not
    finite. It requires the coefficient field and matrix to share a mesh
    object.
  - Neumann: `NeumannBoundary1D` stores an outward normal gradient and exposes
    `outward_normal()` (`-1.0` at `Start`, `+1.0` at `End`) and
    `coordinate_gradient()` (= `normal_gradient * outward_normal`, so the
    coordinate derivative is negated at the start). Its header explicitly
    states matrix assembly is not provided, and no `Apply` method exists.
    Assembly of a Neumann flux into the RHS is therefore unimplemented.
  - Patch: `patch_boundary1d.hpp` contains only `#pragma once`, and its source
    only includes that header.
  - Robin is declared in `BoundaryType` but has no implementation.
- Consequence of the current coverage: interior rows are fully assembled by
  `Laplacian`, and boundary rows receive a diagonal/source contribution only
  from a Dirichlet object. Without a Dirichlet condition on a side, that side's
  boundary face contributes nothing, so the system can become singular (the
  pure-Neumann case is not handled).

## 5. Working Applications & Drivers

- Drivers live in `examples/` (there is no `applications/` directory) and are
  standalone executables, not a CTest suite. Six targets are registered in
  `CMakeLists.txt` under `BUILD_EXAMPLES` (default `ON`) and mirrored in
  `.vscode/launch.json`:
  - `test_domain1d` — `examples/test_geometry/test_domain/test_domain1d/
    test_domain1d.cpp`; `Domain1D` contract checks; waits on `std::cin`.
  - `test_uniform_mesh1d` — `examples/test_mesh/test_mesh1d/
    test_uniform_mesh1d/test_uniform_mesh1d.cpp`; mesh topology, normals,
    distances, and diffusion/Dirichlet assembly including a linear TDMA
    solution; waits on `std::cin`.
  - `test_non_uniform_mesh1d` — same directory tree; graded/explicit geometry
    contract checks; exits directly.
  - `test_grading1d` — `examples/test_mesh/test_mesh1d/test_grading1d/`; grading
    distribution and ratio-conversion checks; exits directly.
  - `test_field1d` — `examples/test_field/test_field1d/test_field1d.cpp`;
    scalar/vector field allocation, access, assignment, and arithmetic; waits on
    `std::cin`.
  - `steady_heat_conduction_rod1d` — `examples/test_solvers/test_solvers1d/
    test_diffusion1d/steady_heat_conduction_rod1d.cpp`; the working end-to-end
    demonstration driver; waits on `std::cin`.
- Working driver description: 5 uniform cells on `[0, 1]` m, `gamma = 1.0`,
  initial `T = 300`, Dirichlet `400` at `Start` and `200` at `End`. It composes
  `fvm::Laplacian`, two `DirichletBoundary1D::Apply` calls, and the by-value
  `TDMASolver::solve(matrix.Matrix())`, copies the result into the field, prints
  the solution and the assembled tridiagonal coefficients.
- Verification status — reported in the completed `TASK_PLAN.md` execution log:
  - The library-only build with `BUILD_EXAMPLES=OFF` passed after the matrix
    API declaration and implementation. The all-target build passed after the
    Laplacian refactor; all six existing executables exited with status `0` in
    the regression sanity check, including the mesh and field contract checks.
  - The `steady_heat_conduction_rod1d` CMake target rebuilt successfully. Its
    output after the refactor was compared with a freshly built pre-refactor
    `HEAD` baseline: stdout was byte-identical (1,148 bytes), and both runs
    produced empty stderr. This verifies parity for the printed coefficients
    and temperature profile for that benchmark.
  - The previously recorded rod profile has cell centers `0.1, 0.3, 0.5, 0.7, 0.9` and
    temperatures `380, 340, 300, 260, 220` K. Its assembled rows are
    `(0, 15, -5, 4000)`, `(-5, 10, -5, 0)` for rows 1–3, and
    `(-5, 15, 0, 2000)`, in `(a_W, a_P, a_E, b)` order.
- Remaining architectural priorities follow the existing scope: extend boundary
  assembly beyond Dirichlet, implement the reserved operators and solver
  workflows, and address the tridiagonal storage limits before supporting
  multidimensional connectivity. These capabilities are not implemented by the
  off-diagonal routing seam.
- Reserved / not usable despite file presence: `fvc1d` explicit gradient and
  divergence, `fvm1d` ddt/div/source, `fv_matrix` 2D/3D, uniform 2D/3D meshes,
  `solvers/solvers1d` workflow classes, conjugate gradient, Gauss–Seidel,
  sparse matrix, `io` writers, `time`, `transport_model`, and 2D/3D vector
  types all exist only as empty or placeholder files.
