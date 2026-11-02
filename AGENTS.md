# Repository Guidelines

## Markdown and README Files

The root `FILEPATH.md` path inventory is the only Markdown file agents may modify unless the user explicitly instructs otherwise. Do not create or modify any other Markdown file, including `AGENTS.md`, as part of routine work.

Do not reference, read, create, modify, or otherwise write to any README file anywhere in the project unless the user explicitly instructs you to do so. This restriction applies throughout all tasks, including documentation updates, searches, and inventories.

## Project Structure & Module Organization

This repository provides a C++20 library for computational fluid dynamics (CFD) research, focusing on robust Finite Volume Method (FVM) implementations. Public headers live under `include/cfd/`; implementations live under `src/` in corresponding module directories. Keep headers (`.hpp`) and implementations (`.cpp`) together by module, and add implementation files to the explicit source list in `CMakeLists.txt`. File and directory presence does not imply a usable API: much of the architecture is scaffolding.

Current implementation status:

| Module paths under `include/cfd/` and `src/` | Implemented behavior or reserved scope |
| --- | --- |
| `geometry/domain/` | `Domain1D`, `Domain2D`, and `Domain3D` represent finite coordinate-agnostic intervals, Cartesian rectangles, and cuboids, respectively. `Domain1D` exposes `Start_Coord`, `End_Coord`, `Length`, `IsContain`, and its current `Set_*` coordinate API. `Domain2D` and `Domain3D` expose `X_Domain` / `Y_Domain` / `Z_Domain`, `IsContain`, and `Set_*_Domain(s)` APIs. Check each header for exact spelling. |
| `mesh/` and `mesh/mesh1d/` | `FvMesh1D` is the concrete value type owning explicit faces, derived centers, and cell volumes (equal to lengths for unit cross-sectional area). Construct it with `FvMesh1D(domain, n_cells)` (uniform, `n_cells >= 1`) or `FvMesh1D(domain, n_cells, GradingType, ratio)`; a single cell is valid for every grading type. It is copy/move-constructible but not assignable, and `Start_Coord()` / `End_Coord()` / `Length()` throw `std::logic_error` on a moved-from (empty) mesh. `GradingType` is dimension-agnostic; `grading::Make_Faces` and `grading::Convert_R_To_r` remain separate free functions (`Make_Faces` accepts `n_cells >= 1`; `Convert_R_To_r` returns 1.0 for `R == 1.0` when the pattern has no exponent span). The generator supports Uniform, Forward, Backward, BothEnds, and Center distributions and validates representability. |
| `field/field1d/` | `Field1D<Type, LocationTag>` in `field1d/field1d.hpp` stores `double` or the one-component field value `Vector1D` (defined in `field1d/vector1d.hpp`) at cells or faces of an `FvMesh1D`. Four aliases cover the scalar/vector and cell/face combinations. Non-member binary arithmetic operators are in the header-only `field1d/field1d_operators.hpp`. The former scalar-only class/source and empty per-vector-field files have been removed; implementations and explicit instantiations now live in `field1d.cpp`. |
| `boundary/boundary1d/` | Dirichlet and Neumann boundary objects are implemented; `DirichletBoundary1D::apply(gamma, matrix)` accumulates the fixed-value diffusion contribution into the boundary row and requires the coefficient field and matrix to reference the same mesh object. Patch boundaries are placeholders. Shared boundary enums live in `boundary/types.hpp`; field locations live in `field/types.hpp`. |
| `mesh/mesh2d/`, `mesh/mesh3d/` | The uniform 2D header/source are empty; the uniform 3D files are placeholders. There is no implemented multidimensional mesh API. |
| `finite_volume/fvc/`, `finite_volume/fvm/` | Explicit gradient/divergence and implicit time-derivative/divergence/source files are 1D placeholders. The 1D implicit Laplacian currently exposes only `fvm::laplacian(gamma, matrix)`: it assembles internal-face contributions for the integrated `div(gamma grad(phi))` operator, with no boundary-face contributions or boundary-condition overload. It accumulates into the existing `FvMatrix1D`, requires `gamma` and the matrix to reference the same mesh object, and validates finite nonnegative cell diffusivities. Internal-face diffusivity uses resistance-weighted harmonic interpolation. `DirichletBoundary1D::apply(gamma, matrix)` remains a separate API for adding a fixed-value boundary contribution. 2D/3D directories are reserved with `.gitkeep`. |
| `fv_matrix/`, `linear_algebra/`, `solvers/` | `fv_matrix::FvMatrix1D` is implemented as a cell-centered wrapper around `linear_algebra::TridiagonalMatrix`; the 2D/3D FV matrix files remain placeholders. The wrapper sizes its system from an `FvMesh1D`, retains a reference to that mesh (which must outlive it and remain unmoved), and exposes west/central/east coefficients and source values with `a_W(p)`, `a_P(p)`, `a_E(p)`, and `b(p)` accessors plus matching `set_*` / `add_*` methods. `Matrix()` exposes the underlying system, and `Reset()` clears it without changing its size. Copy/move construction is available; assignment is deleted. The wrapper does not itself assemble equations or solve them. `linear_algebra::TridiagonalMatrix` stores the three diagonals and source vector for a square tridiagonal system; `linear_algebra::TDMASolver` solves such systems. The mutable-reference solver overload performs forward elimination in place, while the by-value overload works on a copy. 2D/3D equation-solver directories are reserved. |
| `schemes/` | The reorganized layout uses `schemes/gradient/central_difference` and `schemes/interpolation/{arithmetic_mean,harmonic_mean,upwind}`. `cfd::schemes::gradient::central_difference(phi_p, phi_n, distance_pn)` estimates the gradient between neighboring cell centers as `(phi_n - phi_p) / distance_pn`; it requires finite field values and a finite positive distance, uses `long double` intermediates, and throws `std::invalid_argument` for invalid inputs or `std::overflow_error` when the result is not representable as a finite `double`. `cfd::schemes::interpolation::arithmetic_mean(phi_p, phi_n, distance_p, distance_n)` computes the distance-weighted linear face value for finite field values and finite positive distances; it throws `std::invalid_argument` for invalid inputs and `std::overflow_error` if the result is not a finite `double`. `harmonic_mean(gamma_p, gamma_n, distance_p, distance_n)` computes resistance-weighted face diffusivity for finite positive diffusivities and distances, with the same invalid-input and unrepresentable-result exception categories. Both interpolation functions use scaled, `long double` intermediate calculations. Upwind interpolation remains a placeholder. The explicit CMake source list includes all current scheme source paths. |
| `io/`, `time/`, `transport_model/`, `vector/` | Writer, simulation-time, transport, and geometric-vector modules remain reserved; these files do not yet provide functional APIs. Geometric-vector headers have no corresponding source files. |

Implemented gradient and interpolation functions are separate APIs in `include/cfd/schemes/` and `src/schemes/`. The central-difference gradient declaration and implementation are in `gradient/central_difference.hpp` and `.cpp`; arithmetic and harmonic interpolation declarations and implementations are in `interpolation/arithmetic_mean.hpp` / `.cpp` and `interpolation/harmonic_mean.hpp` / `.cpp`. Their implementation files are explicitly listed in the root `CMakeLists.txt`. The implicit 1D Laplacian currently has its own internal resistance-weighted face-diffusivity helper; it does not call the public `harmonic_mean` function. The harmonic interpolation function validates strictly positive cell values, while the Laplacian accepts finite nonnegative diffusivities and handles a zero neighbor diffusivity as zero face diffusivity.

Field directories for 2D/3D are also reserved with `.gitkeep`. Use the current `finite_volume/fvc` and `finite_volume/fvm` layout and the `field1d` / `fv_mesh1d` filenames when adding code or includes. Mesh grading strategy names are dimension-agnostic and belong in `mesh/`; concrete 1D face-generation helpers belong in `mesh/mesh1d/`.

The 1D mesh is constructed as `FvMesh1D(domain, n_cells)` for uniform faces or `FvMesh1D(domain, n_cells, grading_type, ratio)` for generated grading. Uniform and graded meshes share this value type; the old separate `UniformFvMesh1D` and `NonUniformFvMesh1D` classes and sources have been removed. `FvMesh1D` owns its generated faces and derives cell centers and cell volumes. `grading::Make_Faces` remains available as a separate helper for callers that need generated face coordinates. The mesh header is `include/cfd/mesh/mesh1d/fv_mesh1d.hpp`.

`FvMesh1D` requires at least one cell; generated faces must be finite and strictly increasing, with positive finite cell lengths and representable interior centers. The domain endpoints are preserved exactly. Cell volumes equal cell lengths under the unit cross-sectional area convention.

Preserve the separation between domain values, mesh geometry, and field storage. `Domain1D` stores authoritative endpoints with a finite, positive represented extent; it exposes `Start_Coord()`, `End_Coord()`, `Length()`, `IsContain()`, `Set_Bounds()`, `Set_Start_Coord()`, `set_End_Coord()`, and `Set_Length()`, plus the `from_start_and_length()` factory. `Domain2D` exposes axis snapshots through `X_Domain()` and `Y_Domain()`; `Domain3D` also exposes `Z_Domain()`. Both provide closed-bound `IsContain()` and axis updates through `Set_X_Domain()`, `Set_Y_Domain()`, and `Set_Domains()`; `Domain3D` additionally provides `Set_Z_Domain()`. The composed domains store validated `Domain1D` values, return independent axis snapshots, and do not guarantee representable area or volume. Domain containment uses closed bounds and equality compares endpoints exactly. Rejected `Domain1D` updates preserve the existing value; composed-domain setters take already-valid `Domain1D` values. Mesh construction from a domain snapshots its bounds, so later domain mutations do not alter the mesh. `FvMesh1D` exposes `N_Cells()`, `N_Faces()`, `Start_Coord()`, `End_Coord()`, and `Length()`; its read-only span accessors are `Face_Coords()`, `Cell_Center_Coords()`, and `Cell_Volumes()`. Checked indexed accessors are `Face_Coord()`, `Cell_Center_Coord()`, and `Cell_Volume()`; `wFace_Coord()` / `eFace_Coord()` and `wFaceDx()` / `eFaceDx()` / `wCellDx()` / `eCellDx()` provide stencil geometry. `Print_Details()` writes details to a supplied output stream. Face 0 is the west boundary, and face `N_Faces() - 1` (equivalently `N_Cells()`) is the east boundary. Mesh copy/move construction is available, but assignment is deleted; moving from a mesh empties the source and its extent queries throw `std::logic_error`. Scalar fields retain a reference to their `FvMesh1D`, which must outlive them; rvalue meshes cannot be bound, and the mesh should remain unmoved while dependent fields or views exist. Field arithmetic requires the same mesh object (pointer identity, not equal geometry), sampling location, and storage size.

Generated meshes preserve endpoints exactly. Zero cells, invalid geometry, and unrepresentable faces or centers are rejected; excessive face counts throw `std::length_error`. Indexed mesh geometry uses checked `.at()` accessors, while distance queries reject invalid cells or missing neighbors with `std::out_of_range`. The mesh has no unchecked cell `operator[]` API.

`Field1D<Type, LocationTag>` in `cfd::field` supports `double` and `Vector1D` at `CellLocation` and `FaceLocation`. The aliases are `VolScalarField1D`, `SurfaceScalarField1D`, `VolVectorField1D`, and `SurfaceVectorField1D`; include `field1d/field1d.hpp` for the field class and aliases, and `field1d/field1d_operators.hpp` for non-member binary arithmetic operators. `Vector1D` is a lightweight one-component value type in `cfd::field`, separate from the reserved geometric-vector headers in `cfd::vector`. The constructor takes a name, an `FvMesh1D` reference, and an optional initial value; it sizes storage with `N_Cells()` or `N_Faces()`. The mesh must outlive the field and remain unmoved while fields exist. The API provides `Name()`, `Mesh()`, `Size()`, `values()` / `span()` read-write spans, legacy `Values()`, `Fill()`, unchecked `operator[]`, checked `at()`, and arithmetic. Copy and move construction preserve the mesh reference; assignment and field arithmetic require mesh object identity and matching storage size. Field multiplication and division are available for scalar fields; both value types support addition, subtraction, and scalar scaling. Division by zero throws `std::invalid_argument`. All four combinations are explicitly instantiated in `field1d.cpp`. Non-const rvalue mesh binding is deleted. `FieldLocation` is separate and is not used by these classes.

`CMakeLists.txt` keeps the library source list explicit and grouped by subsystem;
new implementation files must be added there deliberately. Standalone example
executables are registered under `BUILD_EXAMPLES` and their files are nested
by component and dimension:

- `test_domain1d`: `examples/test_geometry/test_domain/test_domain1d/test_domain1d.cpp` (checks `Domain1D` contracts; `Domain2D` and `Domain3D` are implemented but are not covered by this example currently).
- `test_uniform_mesh1d`: `examples/test_mesh/test_mesh1d/test_uniform_mesh1d/test_uniform_mesh1d.cpp`.
- `test_non_uniform_mesh1d`: `examples/test_mesh/test_mesh1d/test_non_uniform_mesh1d/test_non_uniform_mesh1d.cpp`.
- `test_grading1d`: `examples/test_mesh/test_mesh1d/test_grading1d/test_grading1d.cpp`.
- `test_field1d`: `examples/test_field/test_field1d/test_field1d.cpp` (checks scalar and vector field allocation, access, assignment, and arithmetic).

`steady_heat_conduction_rod1d` is registered under `BUILD_EXAMPLES` from `examples/test_solvers/test_solvers1d/test_diffusion1d/steady_heat_conduction_rod1d.cpp`. It creates a uniform 1D mesh and temperature and diffusivity fields, a mesh-bound `FvMatrix1D`, and start/end Dirichlet boundary objects; it prints the initialized temperature field. Matrix assembly and boundary-condition application remain TODOs, so it does not yet demonstrate a heat-conduction solve. The library separately provides the internal-face 1D implicit Laplacian assembly API described above, while `DirichletBoundary1D::apply` adds a boundary contribution independently.

The root `FILEPATH.md` is the project path inventory, grouped into documentation, headers, sources, class pairs, free-function header/source pairs, and subsystem sections. Record implemented free-function APIs such as `grading::Make_Faces` under `Functions (header and source)`, separately from class pairs. Build directories such as `build/` and `build_release/` contain generated artifacts and are ignored by Git.

## Build, Test, and Development Commands

Use CMake 3.20 or newer and a compiler supporting C++20. Run commands from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/bin/test_domain1d < /dev/null
./build/bin/test_uniform_mesh1d < /dev/null
./build/bin/test_non_uniform_mesh1d
./build/bin/test_grading1d
./build/bin/test_field1d < /dev/null
./build/bin/steady_heat_conduction_rod1d
```

Configuration enables examples by default through `BUILD_EXAMPLES`; pass `-DBUILD_EXAMPLES=OFF` for a library-only build. Executables go to `${CMAKE_BINARY_DIR}/bin`. Build a single check with `cmake --build build --target test_field1d`. The domain, uniform-mesh, and field examples call `std::cin.get()`, so redirect stdin for unattended runs; the non-uniform mesh contract check exits directly.

The root CMake configuration builds the static `cfd_toolkit` library with the in-tree alias `cfd::toolkit`. C++20 is a public target requirement, and include paths use separate build/install interfaces. No external dependencies are currently declared. Installation uses `GNUInstallDirs` and exports `cfd::toolkit`; consumers call `find_package(cfd-toolkit CONFIG REQUIRED)` and link that target. The package config template is `cmake/cfd-toolkitConfig.cmake.in`; generated config, version, and target files are installed under `${CMAKE_INSTALL_LIBDIR}/cmake/cfd-toolkit`, with `SameMajorVersion` compatibility. To check installation, use `cmake --install build --prefix /tmp/cfd-toolkit-install` after building.

Sources are listed explicitly rather than discovered by globbing. Adding a `.cpp` under `src/` does not include it in the build. Installation copies the entire `include/` directory, including component documentation and placeholder headers; installed presence does not establish API completeness. Example executables are built only when `BUILD_EXAMPLES` is enabled and are not installed.

## Coding Style & Naming Conventions

Use four-space indentation and follow the surrounding file's brace style. Use `snake_case` filenames and variables; class names follow patterns such as `Domain1D`, `FvMesh1D`, `Field1D`, and `VolScalarField1D`. Existing public 1D domain, mesh, and field methods mostly use capitalized underscore names, while factories such as `Domain1D::from_start_and_length` and helper free functions retain `snake_case` or their established spelling. Private data members commonly end in `_`. Keep APIs in the appropriate `cfd` namespace and use `#pragma once` in headers. Existing namespaces include `cfd::geometry`, `cfd::mesh`, `cfd::boundary`, and `cfd::field`.

Use `std::size_t` for container-sized counts and indices where negative values are not meaningful, as in the 1D grading API; do not assume which fundamental unsigned type it aliases on a particular platform. Check requested allocation sizes against the relevant container's `max_size()` before allocating. Mesh coordinates and public grading ratios remain `double`; the grading face generator uses `long double` for intermediate exponent and weight calculations, then validates the resulting `double` faces and cell centers. For numeric literals, use suffixes where they prevent overflow in compound integer expressions or resolve template type mismatches; small comparisons and loop bounds do not need decorative suffixes. `NUMERIC_TYPES_CHEATSHEET.md` summarizes the project's C++ numeric type and literal guidance; its platform-specific size tables describe typical GCC x86_64 Linux sizes, not portable C++ guarantees.

When assigning grading exponents by cell position, use `p` for the cell-position index to distinguish that mapping from face traversal indices.

Add new implementation files explicitly to `CMakeLists.txt`. GNU/Clang builds enable `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, and `-Wshadow` privately for the library; address warnings introduced by changes. No repository-wide formatter or linter configuration is provided.

## Testing Guidelines

Checks are standalone executables; no testing framework, CTest registration, or coverage threshold is configured. Follow the `test_<component>` naming pattern and register new executables in CMake. Whenever an executable file is created, deleted, or renamed, update both `CMakeLists.txt` and `.vscode/launch.json` accordingly so the executable build entries and launch configurations stay in sync. For numerical changes, exercise normal values, invalid inputs, boundary cases, and floating-point limits. Prefer checks that return a nonzero status on failure; inspect output from demonstration-only examples.

`test_domain1d` checks `Domain1D` contracts including invalid values, floating-point limits, containment, equality, and rejected updates. The separate 2D and 3D domain APIs are implemented but are not covered by the current domain example. `test_field1d` checks scalar and vector field allocation and arithmetic, uniform and nonuniform mesh binding, mesh identity, copy/move construction and assignment, mutable and const span access, location-specific types, checked indexing, incompatible fields, and zero division. The mesh examples check explicit and generated geometry, represented interval lengths, storage sizes, invalid faces and inputs, floating-point limits, all grading distributions, symmetric patterns, ratio-one limits, exact endpoint anchoring, center representability, and total-ratio conversion. These checks return nonzero on failed contracts.

## Commit & Pull Request Guidelines

History uses short, imperative subjects such as `Update domain1d mesh1d`; no formal prefix convention is established. Name the affected component and keep each commit focused.

Pull requests should describe the behavior change, affected modules, and validation commands with results. Link relevant issues. Keep generated build output out of commits.

## Document Maintenance

Update the root `FILEPATH.md` inventory whenever you (the AI) perform any of the following tasks:

- Add a new module, directory, or test executable
- Modify CMake configurations, build flags, or dependencies
- Introduce new coding standards or architectural patterns
- Add, remove, or rename a project file, or change a file's path.

