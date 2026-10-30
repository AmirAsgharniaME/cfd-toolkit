# Repository Guidelines

## Project Structure & Module Organization

This repository provides a C++20 library for computational fluid dynamics (CFD) research, focusing on robust Finite Volume Method (FVM) implementations. Public headers live under `include/cfd/`; implementations live under `src/` in corresponding module directories. Keep headers (`.hpp`) and implementations (`.cpp`) together by module, and add implementation files to the explicit source list in `CMakeLists.txt`. File and directory presence does not imply a usable API: much of the architecture is scaffolding.

Current implementation status:

| Module paths under `include/cfd/` and `src/` | Implemented behavior or reserved scope |
| --- | --- |
| `geometry/domain/` | `Domain1D`, `Domain2D`, and `Domain3D` represent finite coordinate-agnostic intervals, Cartesian rectangles, and cuboids, respectively. |
| `mesh/` and `mesh/mesh1d/` | `cfd::mesh::GradingType` (`include/cfd/mesh/grading_type.hpp`) is a dimension-agnostic strategy enum; concrete face generation is kept as the free function `cfd::mesh::grading::make_1d_faces` (`mesh1d/grading1d.hpp` / `src/mesh/mesh1d/grading1d.cpp`), separate from mesh ownership. It takes a `std::size_t` cell count of at least two and implements `Uniform`, `Forward`, `Backward`, `BothEnds`, and `Center` distributions. For graded modes, the ratio is the cell-width growth factor moving away from the refined region and must be finite and at least 1; `Uniform` accepts any positive finite ratio and ignores its value. Before allocation, the generator checks that `N + 1` faces and `N` long-double weights fit their respective vector sizes; excessive counts throw `std::length_error`. Generated faces preserve exact domain endpoints and reject unrepresentable faces or cells without a representable interior center. `FvMesh1D` owns common mesh geometry; `UniformFvMesh1D` constructs uniform meshes and `NonUniformFvMesh1D` accepts explicit faces. `UniformMesh1D` and `Mesh1D` are compatibility aliases for `UniformFvMesh1D`. |
| `field/field1d/` | `VolScalarField1D` stores cell- or face-sampled scalar values; `ScalarField1D` is a compatibility alias. Volume vector and surface field files are placeholders. |
| `boundary/boundary1d/` | Dirichlet and Neumann boundary objects are implemented; patch boundaries are placeholders. Shared boundary enums live in `boundary/types.hpp`; field locations live in `field/types.hpp`. |
| `mesh/mesh2d/`, `mesh/mesh3d/` | The uniform 2D header/source are empty; the uniform 3D files are placeholders. There is no implemented multidimensional mesh API. |
| `finite_volume/fvc/`, `finite_volume/fvm/` | Explicit gradient/divergence and implicit time-derivative/divergence/Laplacian/source files are 1D placeholders. 2D/3D directories are reserved with `.gitkeep`. |
| `fv_matrix/`, `linear_algebra/`, `solvers/` | Matrix containers named for 1D/2D/3D, algebraic matrices/vectors/solvers, and 1D equation workflows are scaffolding. 2D/3D equation-solver directories are reserved. |
| `io/`, `schemes/`, `time/`, `transport_model/`, `vector/` | Reserved writer, interpolation, simulation-time, transport, and geometric-vector files; these do not yet provide functional APIs. Geometric-vector headers have no corresponding source files. |

Field directories for 2D/3D are also reserved with `.gitkeep`. Use the current `finite_volume/fvc` and `finite_volume/fvm` layout and the `vol_scalar_field1d` / `uniform_fv_mesh1d` filenames when adding code or includes. Mesh grading strategy names are dimension-agnostic and belong in `mesh/`; concrete 1D face-generation helpers belong in `mesh/mesh1d/`.

The 1D mesh input flow begins in caller code; there is no central mesh factory or input reader. Callers construct `UniformFvMesh1D` from a `Domain1D` and cell count (or bounds and count), or construct `NonUniformFvMesh1D` from a domain and explicit face vector. `grading::make_1d_faces` is a separate face-vector producer: callers may pass its result to `NonUniformFvMesh1D`, but generation itself does not create a mesh. Both mesh classes converge on the protected `FvMesh1D` constructor, which validates and owns the faces, then derives widths and centers exposed through read-only geometry queries and spans. The data flow and dependency graph are documented in `include/cfd/mesh/mesh1d/README.md`.

`NonUniformFvMesh1D` requires at least two finite, strictly increasing face coordinates whose endpoints exactly match the supplied `Domain1D`. It snapshots the bounds and face array; `FvMesh1D` derives widths and centers and rejects unrepresentable interior centers.

Preserve the separation between domain values, mesh geometry, and field storage. `Domain1D` stores authoritative endpoints with a finite, positive represented extent. Its coordinate-agnostic API uses `start()`, `end()`, `length()`, `set_bounds(start, end)`, `set_start(start)`, `set_end(end)`, and `set_length(length)`; Cartesian axis names belong to `Domain2D`/`Domain3D` compositions or coordinate mappings. `Domain2D` and `Domain3D` compose validated `Domain1D` values and return independent axis snapshots; they do not guarantee representable area or volume. Domain containment uses closed bounds and equality compares endpoints exactly. Rejected domain updates preserve the existing value. Mesh construction snapshots domain bounds, so later domain mutations do not alter meshes. `FvMesh1D` validates its complete interval using `Domain1D` and exposes `start()`, `end()`, `length()` (with `x_start()`/`x_end()` compatibility aliases) and read-only geometry spans; actual cell widths can differ from nominal uniform spacing, and cell volumes assume unit cross-sectional area. Scalar fields retain a reference to their uniform mesh, which must outlive them; do not move from or move-assign a mesh while fields or views depend on it. Field arithmetic requires the same mesh object, sampling location, and storage size.

`UniformFvMesh1D` accepts either a `Domain1D` and cell count or explicit `start`, `end`, and cell count, with both construction paths using the same domain validation. `dx()`, `spacing()`, and `nominal_spacing()` return `domain.length() / N`, using the represented endpoint difference even when a requested domain length rounded. Face generation snapshots `domain.start()` and `domain.end()`, allocates `N + 1` faces, and preserves the boundary coordinates exactly. The base mesh derives `N` actual widths and interior centers from those faces. Use actual widths for finite-volume metrics. Invalid geometry or zero cells throw `std::invalid_argument`; excessive face counts throw `std::length_error`. A valid domain can still lack enough representable coordinates for the requested mesh.

`FvMesh1D` has a protected constructor for derived meshes that provide explicit face arrays. It validates the complete interval, strictly increasing finite faces, and distinct interior cell centers; `NonUniformFvMesh1D` uses this constructor after also checking that its first and last faces exactly match the supplied domain. Meshes support copy construction and move construction/assignment; copy assignment is deleted. A moved-from mesh must be destroyed or assigned a new value before geometry queries. Checked indexed geometry queries throw `std::out_of_range`; `operator[]` is unchecked.

`CMakeLists.txt` keeps the library source list explicit and grouped by subsystem;
new implementation files must be added there deliberately. Standalone example
executables are registered under `BUILD_EXAMPLES` and their files are nested
by component and dimension:

- `test_domain1d`: `examples/test_geometry/test_domain/test_domain1d/test_domain1d.cpp` (also checks 2D/3D domain contracts).
- `test_uniform_mesh1d`: `examples/test_mesh/test_mesh1d/test_uniform_mesh1d/test_uniform_mesh1d.cpp`.
- `test_non_uniform_mesh1d`: `examples/test_mesh/test_mesh1d/test_non_uniform_mesh1d/test_non_uniform_mesh1d.cpp`.
- `test_grading1d`: `examples/test_mesh/test_mesh1d/test_grading1d/test_grading1d.cpp`.
- `test_scalar_field1d`: `examples/test_field/test_field1d/test_scalar_field1d/test_scalar_field1d.cpp`.

Component documentation lives beside public headers at `include/cfd/geometry/domain/README.md`, `include/cfd/mesh/mesh1d/README.md`, and `include/cfd/field/field1d/README.md`. The root `FILEPATH.md` is the project path inventory, grouped into documentation, headers, sources, class pairs, free-function header/source pairs, and subsystem sections. Record implemented free-function APIs such as `grading::make_1d_faces` under `Functions (header and source)`, separately from class pairs. Build directories such as `build/` and `build_release/` contain generated artifacts and are ignored by Git.

## Build, Test, and Development Commands

Use CMake 3.20 or newer and a compiler supporting C++20. Run commands from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/bin/test_domain1d < /dev/null
./build/bin/test_uniform_mesh1d < /dev/null
./build/bin/test_non_uniform_mesh1d
./build/bin/test_grading1d
./build/bin/test_scalar_field1d < /dev/null
```

Configuration enables examples by default through `BUILD_EXAMPLES`; pass `-DBUILD_EXAMPLES=OFF` for a library-only build. Executables go to `${CMAKE_BINARY_DIR}/bin`. Build a single check with `cmake --build build --target test_scalar_field1d`. The domain, uniform-mesh, and scalar-field examples call `std::cin.get()`, so redirect stdin for unattended runs; the non-uniform mesh contract check exits directly.

The root CMake configuration builds the static `cfd_toolkit` library with the in-tree alias `cfd::toolkit`. C++20 is a public target requirement, and include paths use separate build/install interfaces. No external dependencies are currently declared. Installation uses `GNUInstallDirs` and exports `cfd::toolkit`; consumers call `find_package(cfd-toolkit CONFIG REQUIRED)` and link that target. The package config template is `cmake/cfd-toolkitConfig.cmake.in`; generated config, version, and target files are installed under `${CMAKE_INSTALL_LIBDIR}/cmake/cfd-toolkit`, with `SameMajorVersion` compatibility. To check installation, use `cmake --install build --prefix /tmp/cfd-toolkit-install` after building.

Sources are listed explicitly rather than discovered by globbing. Adding a `.cpp` under `src/` does not include it in the build. Installation copies the entire `include/` directory, including component documentation and placeholder headers; installed presence does not establish API completeness. Example executables are built only when `BUILD_EXAMPLES` is enabled and are not installed.

## Coding Style & Naming Conventions

Use four-space indentation and follow the surrounding file's brace style. Use `snake_case` filenames, functions, and variables; class names follow patterns such as `Domain1D`, `UniformFvMesh1D`, and `VolScalarField1D`. Private data members commonly end in `_`. Keep APIs in the appropriate `cfd` namespace and use `#pragma once` in headers. Existing namespaces include `cfd::geometry`, `cfd::mesh`, `cfd::boundary`, and `cfd::field::field1d`.

Use `std::size_t` for container-sized counts and indices where negative values are not meaningful, as in the 1D grading API; do not assume which fundamental unsigned type it aliases on a particular platform. Check requested allocation sizes against the relevant container's `max_size()` before allocating. Mesh coordinates and public grading ratios remain `double`; the grading face generator uses `long double` for intermediate exponent and weight calculations, then validates the resulting `double` faces and cell centers. For numeric literals, use suffixes where they prevent overflow in compound integer expressions or resolve template type mismatches; small comparisons and loop bounds do not need decorative suffixes. `NUMERIC_TYPES_CHEATSHEET.md` summarizes the project's C++ numeric type and literal guidance; its platform-specific size tables describe typical GCC x86_64 Linux sizes, not portable C++ guarantees.

Add new implementation files explicitly to `CMakeLists.txt`. GNU/Clang builds enable `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, and `-Wshadow` privately for the library; address warnings introduced by changes. No repository-wide formatter or linter configuration is provided.

## Testing Guidelines

Checks are standalone executables; no testing framework, CTest registration, or coverage threshold is configured. Follow the `test_<component>` naming pattern and register new executables in CMake. For numerical changes, exercise normal values, invalid inputs, boundary cases, and floating-point limits. Prefer checks that return a nonzero status on failure; inspect output from demonstration-only examples.

`test_domain1d` checks 1D/2D/3D domain contracts, including invalid values, floating-point limits, containment, equality, rejected updates, and snapshot independence. `test_scalar_field1d` checks allocation, arithmetic, checked indexing, incompatible fields, and zero division. `test_uniform_mesh1d` checks constructor equivalence, represented domain lengths, geometry storage sizes, snapshot independence after each domain setter, cell geometry, explicit-face base mesh validation, invalid inputs, and floating-point limits before printing mesh details. `test_non_uniform_mesh1d` checks explicit nonuniform geometry, snapshot independence, exact bounds, and invalid face sequences. `test_grading1d` checks all grading types for even and odd cell counts, symmetric patterns, ratio-one limits, exact endpoint anchoring, input validation, center representability, and compatibility with `NonUniformFvMesh1D`. These checks return nonzero on failed contracts.

## Commit & Pull Request Guidelines

History uses short, imperative subjects such as `Update domain1d mesh1d`; no formal prefix convention is established. Name the affected component and keep each commit focused.

Pull requests should describe the behavior change, affected modules, and validation commands with results. Link relevant issues and update component documentation when public contracts change. Keep generated build output out of commits.

## Document Maintenance

Always update this `AGENTS.md` in the same turn/commit whenever a significant or fundamental project change affects its architecture, supported behavior, public API, module status, build workflow, or development conventions. Do not wait for an explicit reminder. Keep the description accurate and concise.

Also update the root `FILEPATH.md` inventory whenever you (the AI) perform any of the following tasks:

- Add a new module, directory, or test executable
- Modify CMake configurations, build flags, or dependencies
- Introduce new coding standards or architectural patterns
- Add, remove, or rename a project file, or change a file's path.

When a change meets both conditions, update both documents in the same turn/commit.
