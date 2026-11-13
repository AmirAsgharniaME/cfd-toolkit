# Project State and Working Guide

**Repository state inspected: November 13, 2026.** This guide summarizes the current design, implemented public behavior, unfinished areas, project rules, and verification status so a contributor or AI assistant can investigate changes without first surveying the entire tree. The source headers and implementations remain authoritative when this guide conflicts with code. No build or executable was run for this inspection.

## 1. Project purpose and working approach

`cfd-toolkit` is an early-stage C++20 library for computational-fluid-dynamics research. The implemented path is primarily one-dimensional: validated domain bounds create an owned mesh; fields and a finite-volume matrix refer to that mesh; numerical operators accumulate system coefficients; and a tridiagonal solver solves the resulting system. Clients compose these pieces directly. There is not yet a complete simulation or equation-solver framework.

The project favors clearly separated responsibilities:

- Domain objects own validated coordinate bounds.
- Meshes own generated geometry and expose topology and geometric queries.
- Fields own their values and retain a non-owning reference to a mesh.
- The finite-volume matrix retains a non-owning reference to its mesh and wraps algebraic storage.
- Operators and boundary objects accumulate into an existing matrix; the caller controls assembly order, reset, and solve.
- Independent schemes (gradient and interpolation) are separate APIs, not hidden mesh behavior.

When addressing user changes or reported issues, first identify which layer owns the behavior, verify exact current names in its header, and preserve these seams. Treat presence of a header or source file as insufficient evidence that its API is implemented. The repository's standalone examples are contract checks or demonstrations, not a complete test framework.

## 2. Governing repository rules and contributor preferences

These rules are part of the project state and should guide future work:

- **Markdown scope:** `FILEPATH.md` is the only Markdown file agents may modify during routine work. This `STATE.md` rewrite is authorized by the user's explicit request. Do not read, search, reference, create, or modify any README file unless the user explicitly requests it. Keep all other Markdown files unchanged unless expressly authorized.
- **Inventory upkeep:** Update root `FILEPATH.md` when a task adds/removes/renames a project file or directory, adds a test executable, changes CMake configuration/build flags/dependencies, or establishes a new coding standard or architectural pattern. List implemented free functions separately from classes. Do not change it for edits that trigger none of these conditions.
- **C++ conventions:** C++20; four-space indentation; follow surrounding brace style; `snake_case` filenames and local variables; established mixed public naming is intentional. Use `#pragma once`, existing namespaces and module layout. Use `std::size_t` for nonnegative sizes/indices; check relevant container `max_size()` before allocation. Size vectors before indexed writes and do not index capacity beyond size. Address warnings introduced under GNU/Clang's `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`.
- **Build integration:** CMake source lists are explicit. Add each new `.cpp` deliberately. If creating, deleting, or renaming an executable, update `CMakeLists.txt` and `.vscode/launch.json` together. Build output belongs in ignored build directories, not commits.
- **Verification:** Standalone executables are used instead of CTest or a test framework. For numerical changes, cover regular values, invalid inputs, boundaries, and floating-point limits, and use nonzero exit on contract failure. Demonstration output alone is weaker evidence than assertions. Do not claim a check passed unless it was run and observed. The current task instructions may restrict running checks; follow those instructions.
- **Numerical care:** Mesh coordinates and public grading ratios are `double`; use `long double` for sensitive intermediate grading/scheme calculations where established. Preserve exact endpoints and reject invalid or unrepresentable geometry. Follow the repository's numeric guidance files when a relevant task calls for their use, but do not read README files.
- **History and review:** Keep changes focused. Commit subjects are short imperative phrases naming the affected component. PR descriptions should explain behavior, affected modules, validation results, and relevant issue links.

## 3. Build, package, and executable layout

- CMake minimum is 3.20; project version is 0.1.0. The static library target is `cfd_toolkit`, with alias `cfd::toolkit`; C++20 is a public target feature. No external dependencies are declared.
- `BUILD_EXAMPLES` defaults to ON. There are six standalone targets; executables are placed in `${CMAKE_BINARY_DIR}/bin`. There is no CTest registration.
- GNU/Clang warnings are private library compile options. Sources are enumerated explicitly and grouped in the root `CMakeLists.txt`.
- Installation uses `GNUInstallDirs`, installs the library and `include/`, and exports `cfd::toolkit`. The package config/version files install under `${CMAKE_INSTALL_LIBDIR}/cmake/cfd-toolkit` with `SameMajorVersion` compatibility. Installed placeholder headers are not proof of working functionality.
- Configure/build examples from repository root: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`, then `cmake --build build --parallel`. Use `-DBUILD_EXAMPLES=OFF` for a library-only build. Example programs that wait on stdin can be run unattended with `< /dev/null`.
- The six example sources are `test_domain1d`, `test_uniform_mesh1d`, `test_non_uniform_mesh1d`, `test_grading1d`, `test_field1d`, and `steady_heat_conduction_rod1d`; their paths and launch configurations are recorded in `FILEPATH.md` and `.vscode/launch.json`.

## 4. Implemented APIs and behavioral contracts

### Domains

`geometry::Domain1D` stores authoritative start/end `double` coordinates and requires finite bounds with finite positive represented extent. It offers length and endpoint constructors, `from_start_and_length`, `Start_Coord()`, `End_Coord()`, `Length()`, closed-bound `IsContain()`, exact endpoint equality, and validated updates (`Set_Bounds`, `Set_Start_Coord`, `set_End_Coord`, `Set_Length`). A rejected update preserves the prior value.

`Domain2D` and `Domain3D` compose validated `Domain1D` values. They provide axis snapshots (`X_Domain`, `Y_Domain`, and `Z_Domain`), closed-bound containment, and axis/domain setters. Returned axis values are independent snapshots. Representable area or volume is not guaranteed. Their current coverage is less complete than `Domain1D`'s example coverage.

### 1D mesh and grading

`mesh::FvMesh1D` is the concrete uniform and graded value type. Construct with `(domain, n_cells)` or `(domain, n_cells, GradingType, ratio)`. `n_cells` must be at least one; one cell is accepted for every grading profile. The mesh snapshots domain endpoints and owns face coordinates, center coordinates, and cell volumes (equal to cell lengths for unit cross-sectional area). Construction preserves endpoints exactly and validates finite strictly increasing faces, positive finite lengths, and distinct representable interior centers. Centers use `std::midpoint`. Invalid/unrepresentable geometry throws `std::invalid_argument`; excessive face counts throw `std::length_error`.

Copy/move construction is available; assignment is deleted. A moved-from mesh has zero cells/faces, and extent accessors throw `std::logic_error`. Counts are `N_Cells()` / `N_Faces()`, extents are `Start_Coord()` / `End_Coord()` / `Length()`, read-only spans are `Face_Coords()` / `Cell_Center_Coords()` / `Cell_Volumes()`, and checked indexed accessors are `Face_Coord()`, `Cell_Center_Coord()`, and `Cell_Volume()`. `Cell_Faces(p)` returns the two face IDs in coordinate order. `Cell_Face_Distance(p,f)` requires an incident face. There is no unchecked cell `operator[]`.

`CellId` and `FaceId` are `std::size_t`. Face 0 is west; face `N_Cells()` is east. `Owner(f)` is cell 0 on the west boundary and `f - 1` otherwise. `Neighbour(f)` is absent on boundaries and cell `f` internally. Internal face orientation is owner-to-neighbour along +x; boundary orientation is outward. `s_f(p,f)` is +1 for an owner (including boundary owners) and -1 for an internal neighbour. `e_ON(f)` is +1 for internal faces only. `Face_Area(f)` returns 1.0. Invalid indices, non-incident pairs, and internal-only queries on boundaries throw `std::out_of_range`.

**Mesh distance API alignment:** the mesh exposes `Distance_Owner_face(f)` and `Distance_Neighbour_face(f)`, and `d_ON(f)` returns their sum for internal faces. The Laplacian and uniform-mesh example use these names; boundary and invalid-index checks use the matching methods. The earlier source mismatch is resolved in the current tree.

`mesh::grading::Make_Faces(domain, n_cells, type, ratio)` separately generates face coordinates. `Convert_R_To_r(n_cells, type, R)` converts total ratio; for a pattern without exponent span, `R == 1.0` returns 1.0. Profiles are `Uniform`, `Forward`, `Backward`, `BothEnds`, and `Center`. The generator validates representability. Old separate uniform/non-uniform 1D mesh classes are removed. Grading types are dimension-agnostic; 1D face helpers live in `mesh/mesh1d/`.

### Fields and vector values

`field::Field1D<ValueType, LocationTag>` is implemented for `double` and one-component `vector::Vector1D`, at `CellLocation` and `FaceLocation`. Aliases: `VolScalarField1D`, `SurfaceScalarField1D`, `VolVectorField1D`, `SurfaceVectorField1D`. Include `field1d/field1d.hpp` for class/aliases and `field1d/field1d_operators.hpp` for non-member binary operators. `Vector1D` is in `include/cfd/vector/vector1d.hpp`.

Construction takes a name, mesh reference, and optional initial value. Storage size follows mesh cell or face count. Mesh must outlive the field and remain unmoved; non-const rvalue mesh binding is deleted. API includes `Name()`, `Mesh()`, `size()`, face-only `N_Faces()`, `values()` / `span()` mutable and read-only views, legacy `Values()`, `Fill()`, unchecked `operator[]`, and checked `at()`. Copy/move construction preserves the mesh reference; assignment and field arithmetic require mesh object identity and matching size, not merely equal geometry. Both value types support addition/subtraction and scalar scaling. Scalar fields additionally support elementwise multiply/divide; zero division throws `std::invalid_argument`. All four supported combinations are explicitly instantiated in `field1d.cpp`. `FieldLocation` in `field/types.hpp` is separate from these type tags.

### Boundary objects

`DirichletBoundary1D` stores a side and fixed value. `Apply(gamma, matrix)` validates the mesh identity, side, boundary-cell diffusivity, distance, boundary value and finite accumulated results, then adds `D = gamma_p / distance` to the boundary diagonal and `D * boundary_value` to the source, for unit face area and the negative diffusion operator. It is a separate accumulation step from `fvm::Laplacian`.

`NeumannBoundary1D` stores an outward normal derivative and exposes its normal, side, type, and coordinate gradient. Coordinate gradient is `-normal_gradient` at Start and `+normal_gradient` at End. It does not assemble matrix entries. Patch boundaries are placeholders. `BoundaryType::Robin` exists as an enum value only; no mixed-boundary assembly is implemented. Shared boundary enums are in `boundary/types.hpp`; field location tags are in `field/types.hpp`.

### Schemes and finite-volume operators

Implemented separate scheme functions:

- `schemes::gradient::central_difference(phi_owner, phi_neighbour, distance_owner, distance_neighbour)` computes `(phi_neighbour - phi_owner) / (distance_owner + distance_neighbour)`, the directional gradient along the owner-to-neighbour vector. Inputs must be finite and distances finite positive. It uses `long double` intermediates and throws `std::invalid_argument` for invalid input or `std::overflow_error` for an unrepresentable sum/result.
- `schemes::interpolation::arithmetic_mean(value_owner, value_neighbour, d_owner, d_neighbour)` computes `(d_neighbour*value_owner + d_owner*value_neighbour)/(d_owner+d_neighbour)` for finite values and positive finite distances. `harmonic_mean(gamma_owner, gamma_neighbour, d_owner, d_neighbour)` computes resistance-weighted diffusivity and requires positive finite diffusivities and distances. Both use scaled `long double` intermediate arithmetic and reject invalid or unrepresentable results with the documented exception categories. The parameter names describe the mesh's owner/neighbour orientation; they do not change the function signatures' types or arity.
- Upwind interpolation is a placeholder.

`fvm::Laplacian(gamma, matrix)` assembles internal faces only for the cell-integrated negative diffusion operator `-div(gamma grad(phi))`. It requires finite nonnegative diffusivity values and that field and matrix refer to the same mesh object. For an internal face it uses owner/neighbour center-to-face distances, `harmonic_mean(gamma_owner, gamma_neighbour, d_owner, d_neighbour)`, `d_ON(f)`, and the owner orientation factor `s_f(owner,f) * e_ON(f)`. A zero diffusivity on either side gives zero face diffusivity because the standalone harmonic scheme requires strictly positive inputs. It adds a positive diagonal and negative adjacent-cell coefficient in both neighboring rows, including boundary-cell rows. It accumulates into existing entries and does not reset the matrix or add boundary-face contributions. A one-cell mesh contributes nothing. The implementation no longer includes the separate central-difference scheme; conductance is calculated from face diffusivity and mesh geometry. Conductance must be finite; accumulated overflow is not separately validated in the documented contract.

No explicit gradient/divergence, transient derivative, implicit divergence, or source operator is implemented. Their 1D files are placeholders. The current mathematical stencil for an internal face with center-to-face distances `d_P`, `d_N` and positive cell coefficients is `gamma_f=(d_P+d_N)/(d_P/gamma_P+d_N/gamma_N)`, `D_f=gamma_f/d_ON`; owner and neighbour rows each receive `+D_f` on their diagonal and `-D_f` on their adjacent coefficient (unit area).

### Finite-volume matrix and linear algebra

`fv_matrix::FvMatrix1D(mesh)` wraps a `linear_algebra::TridiagonalMatrix` sized to the mesh cells and retains a mesh reference that must outlive it and remain unmoved. Copy/move construction is available; assignment is deleted. It exposes `Mesh()`, `size()`, `Matrix()`, `a_W(p)`, `a_P(p)`, `a_E(p)`, `b(p)`, corresponding `Set_*` and `Add_*` methods, `Add_OffDiagonal(row,col,value)`, and `Reset()`. `Add_OffDiagonal` maps adjacent row/column entries to lower/upper tridiagonal storage; equal indices throw `std::invalid_argument` before bounds checks, and out-of-range/nonadjacent indices throw `std::out_of_range`.

`TridiagonalMatrix` stores diagonal, lower, upper and source vectors, each indexed by row, with `Diag`, `Lower`, `Upper`, `Source` indexed/vector accessors, indexed `Set_*` / `Add_*`, `size()`, and `Reset()`. Boundary slots in lower/upper arrays exist but do not represent unknown neighbors. Mutable vector access means clients can change lengths; the solver checks consistency.

`TDMASolver::solve(matrix, solution)` performs forward elimination in place. `TDMASolver::solve(matrix)` takes by value and returns a solution, preserving an lvalue caller's system. It rejects an empty or inconsistent system and pivots whose absolute value is below `1.0e-14`; it does not pivot or precondition. The equation workflow is manual: create fields and matrix on the same mesh, accumulate Laplacian and boundary contributions, then pass `matrix.Matrix()` to TDMA.

## 5. Reserved or incomplete areas

- 2D/3D domain values are implemented; multidimensional meshes, fields, finite-volume matrices, operators, and equation workflows are not implemented. Mesh 2D/3D files are placeholders.
- The FVC/FVM 1D gradient, divergence, time derivative, and source modules are placeholders. Upwind is a placeholder.
- Neumann matrix assembly, patch boundary infrastructure, Robin/mixed assembly, full equation workflows, and a solver abstraction in `solvers/solvers1d/` are unimplemented.
- Sparse matrix, conjugate-gradient and Gauss-Seidel files are placeholders; TDMA is the implemented solver.
- I/O writers, simulation time, transport properties, multidimensional vectors, and transport/simulation workflows are reserved. Only `Vector1D` is currently implemented.
- Headers may be installed even when their modules are placeholders.

## 6. Examples and verification record

The examples are:

| Target | Role |
| --- | --- |
| `test_domain1d` | `Domain1D` construction, update, containment, invalid-input and floating-point contracts. |
| `test_uniform_mesh1d` | Uniform mesh, topology, orientation, distances, Neumann orientation, diffusion assembly, Dirichlet assembly, and linear TDMA checks. Its distance-query calls now match the mesh API; latest source change is not yet built or run. |
| `test_non_uniform_mesh1d` | Generated/nonuniform geometry contracts. |
| `test_grading1d` | Grading distributions, symmetry, ratio conversion, endpoint anchoring and representability. |
| `test_field1d` | Scalar/vector field allocation, access, assignment, arithmetic, views and mesh binding. |
| `steady_heat_conduction_rod1d` | Demonstration composing mesh, fields, matrix, Laplacian, two Dirichlet boundaries and TDMA; prints coefficients and solution and waits for Enter. It is not by itself a numerical assertion. |

Historical records in the previous state notes: library/all-target builds succeeded at earlier development steps; earlier runs of `test_domain1d`, `test_non_uniform_mesh1d`, `test_grading1d`, and `test_field1d` exited 0. On November 11, 2026, a `test_uniform_mesh1d` run exited 1 with `Internal diffusion stencil is incorrect`. A rod demo run on that date exited 0 and printed `[400, 350, 300, 250, 200]` for five cells on `[0,1]`, unit diffusivity and endpoint values 400/200, but this was before the current mesh/API refactor and was not an assertion-based validation. Earlier output parity against a prior revision was also recorded, but does not validate current operator behavior.

**Current verification status:** no build or executable was run after the current uncommitted source edits. Those edits clarify interpolation parameter names as owner/neighbour in declarations and implementations, update the Laplacian to use that terminology, and remove its unused central-difference include. The earlier mesh distance-query mismatch is resolved, but the current working tree has not been compiled or exercised. Do not treat historical results as validation of the current tree. Useful follow-up checks are a build of all targets and all six standalone executables, with stdin redirected for examples that pause.

## 7. Practical investigation priorities

1. Build the current tree and run the uniform-mesh target now that its distance-query calls match the declared mesh API; address any further compile or contract failures found.
2. Verify internal stencil signs and coefficients for both rows and on one-cell, two-cell, uniform, graded, discontinuous-diffusivity and zero-diffusivity cases. Check symmetry/conservation where applicable, and test the Dirichlet contributions plus the linear rod solution.
3. Run all configured examples after the API/build issue is fixed; record exact commands and outcomes. Avoid presenting a demo's successful exit as proof of numerical correctness.
4. Extend boundary conditions, operators, dimensions, and solver workflows only with explicit contracts and numerical checks alongside the implementation.

## 8. Key paths

- Public API: `include/cfd/`
- Implementations: `src/`
- Examples: `examples/`
- CMake build/package configuration: `CMakeLists.txt`, `cmake/`
- Root path inventory: `FILEPATH.md`
- Editor launch configurations: `.vscode/launch.json`

Use the current `finite_volume/fvc`, `finite_volume/fvm`, `field1d`, and `fv_mesh1d` layout and filenames when changing code. Keep domain values, mesh geometry, field storage, matrix assembly and solving separate.
