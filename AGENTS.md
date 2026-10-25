# Repository Guidelines

## Project Structure & Module Organization

This repository provides a C++20 library for computational fluid dynamics research. Public headers live under `include/cfd/`; implementations live under `src/` and are grouped by module. Current dimensional coverage varies by module: geometry domains and finite-volume matrix containers cover 1D, 2D, and 3D; the mesh layer has a shared 1D finite-volume base plus uniform meshes in 1D, 2D, and 3D; explicit (`finite_volume/fvc`) and implicit (`finite_volume/fvm`) operators and equation-solver workflows are currently 1D. Other modules provide boundary conditions, volume and surface fields, output (`io`), linear algebra, numerical schemes, simulation time, transport properties, and geometric vectors. Keep headers (`.hpp`) and implementations (`.cpp`) in the corresponding module directories, and add implementation files to the explicit source list in `CMakeLists.txt`.

Standalone sanity executables are organized by component under `examples/test_geometry/`, `examples/test_mesh/`, and `examples/test_field/`. CMake builds the domain, uniform 1D mesh, and scalar 1D field executables when `BUILD_EXAMPLES` is enabled and places them in `build/bin/`. Component documentation may live beside its public headers, including the geometry domain, 1D mesh, and 1D field READMEs. The root `CMakeLists.txt` defines the static `cfd_toolkit` library and its `cfd::toolkit` alias, installs the public headers, and exports a CMake package; its config template lives in `cmake/`. Build directories such as `build/` and `build_release/` contain generated artifacts and are ignored by Git.

## Build, Test, and Development Commands

Use CMake 3.20 or newer and a compiler supporting C++20. Run commands from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/bin/test_domain1d < /dev/null
./build/bin/test_uniform_mesh1d
./build/bin/test_scalar_field1d
```

Configuration enables examples by default; pass `-DBUILD_EXAMPLES=OFF` for a library-only build. Build a single check with `cmake --build build --target test_scalar_field1d`. The domain example waits for input, so redirect stdin for unattended runs. The project builds a static library and exposes the `cfd::toolkit` target both to in-tree consumers and through its installed CMake package.

## Coding Style & Naming Conventions

Use four-space indentation and follow the surrounding file's brace style. Use `snake_case` filenames, functions, and variables; class names follow patterns such as `Domain1D` and `UniformMesh1D`. Private data members commonly end in `_`. Keep APIs in the appropriate `cfd` namespace and use `#pragma once` in headers.

Add new implementation files explicitly to `CMakeLists.txt`. GNU/Clang builds enable `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, and `-Wshadow`; address warnings introduced by changes. No repository-wide formatter or linter configuration is provided.

## Testing Guidelines

Checks are standalone executables; no testing framework, CTest registration, or coverage threshold is configured. Follow the `test_<component>` naming pattern and register new executables in CMake. For numerical changes, exercise normal values, invalid inputs, boundary cases, and floating-point limits. Prefer checks that return a nonzero status on failure; inspect output from demonstration-only examples.

## Commit & Pull Request Guidelines

History uses short, imperative subjects such as `Update domain1d mesh1d`; no formal prefix convention is established. Name the affected component and keep each commit focused.

Pull requests should describe the behavior change, affected modules, and validation commands with results. Link relevant issues and update component documentation when public contracts change. Keep generated build output out of commits.

## Document Maintenance
Whenever you (the AI) perform any of the following tasks:
- Add a new module, directory, or test executable
- Modify CMake configurations, build flags, or dependencies
- Introduce new coding standards or architectural patterns
- If the path of any file changes, the `file_path` file in the root of the project must be updated.
You MUST update this `AGENTS.md` file in the same turn/commit to reflect the changes. Keep it synchronized without waiting for explicit user reminders.
