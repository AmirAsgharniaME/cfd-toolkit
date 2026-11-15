# Architectural Report: Shared 1D Boundary Assembly

## 1. Problem Statement / User Query (State my query clearly so future agents understand the exact goal)

The user wants to simplify boundary-condition implementation in `include/cfd/boundary/boundary1d` by introducing a common base class and three derived boundary classes, using composition to reduce repeated code. The three existing conditions are Dirichlet, Neumann, and Robin. This is an architecture proposal only: do not modify or write C++ source code files in this task.

## 2. Codebase Impact Analysis (Exact file paths, affected classes/methods)

Current public declarations:

- `include/cfd/boundary/boundary1d/dirichlet_boundary1d.hpp`: `cfd::boundary::DirichletBoundary1D`, constructor, `Name()`, `Side()`, `Value()`, and `Apply(...)`.
- `include/cfd/boundary/boundary1d/neumann_boundary1d.hpp`: `cfd::boundary::NeumannBoundary1D`, both constructors, lowercase metadata/gradient accessors, `set_normal_gradient(...)`, and `Apply(...)`.
- `include/cfd/boundary/boundary1d/robin_boundary1d.hpp`: `cfd::boundary::RobinBoundary1D`, constructor, lowercase metadata/coefficient accessors, setters, and `Apply(...)`.
- `include/cfd/boundary/types.hpp`: `BoundaryType`, `BoundarySide1D`, and `GradientType` enums.

Current implementations and repeated assembly validation:

- `src/boundary/boundary1d/dirichlet_boundary1d.cpp`: validates mesh identity, nonempty matching sizes, boundary side, owner diffusivity, distance, area, contributions, and accumulated diagonal/source before adding the Dirichlet diagonal and source.
- `src/boundary/boundary1d/neumann_boundary1d.cpp`: repeats common mesh/size/geometry/diffusivity validation and preflights its source-only Neumann contribution; it also converts coordinate gradients to outward-normal gradients.
- `src/boundary/boundary1d/robin_boundary1d.cpp`: repeats common validation and preflights its diagonal/source Robin contribution; its first-order closure is `phi_b = phi_P`.
- `CMakeLists.txt`: explicitly lists the three implementation paths above.

Existing behavior is consumed in `tests/test_mesh/test_mesh1d/test_uniform_mesh1d/test_uniform_mesh1d.cpp` and `examples/steady_heat_conduction_rod1d/steady_heat_conduction_rod1d.cpp`. The test driver exercises boundary signs, constructor/accessor contracts, invalid values, overflow safety, and assembly. The rod example constructs Dirichlet objects directly and applies them separately.

## 3. Proposed Architectural Solution (Concrete C++20 specs, exact method signatures, logic flow, error handling)

Add a non-owning shared assembly context in the boundary implementation layer and a small polymorphic metadata base for the three public condition classes. Keep the boundary equations and their condition-specific stored values in the three derived classes. Composition then places the reusable geometry/validation object inside the common base; inheritance expresses that each concrete condition is a 1D boundary condition. Avoid putting condition-specific formulas in the base.

Proposed internal helper value:

```cpp
struct BoundaryAssemblyContext1D
{
    const mesh::FvMesh1D& mesh;
    std::size_t face_id;
    std::size_t owner;
    double gamma_owner;
    double face_area;
    double owner_face_distance;
};
```

Provide one implementation-layer preparation function, preferably translation-unit-local in a shared boundary source if source organization allows, or declared in a private internal header if needed by all three `.cpp` files:

```cpp
[[nodiscard]] BoundaryAssemblyContext1D Prepare_Boundary_Assembly(
    BoundarySide1D side,
    const field::VolScalarField1D& gamma,
    const fv_matrix::FvMatrix1D& matrix,
    std::string_view condition_name);
```

The preparation function checks that `&gamma.Mesh() == &matrix.Mesh()`, mesh cell count is nonzero, field and matrix sizes equal the cell count, side is `Start` or `End`, owner index is valid, owner diffusivity is finite and nonnegative, owner-to-face distance is finite and positive, and face area is finite and positive. It resolves Start to face 0 and End to `N_Faces() - 1`. It throws `std::invalid_argument` for bad identity, sizes, side, diffusivity, owner, or geometry, retaining the existing input-validation exception category. Condition-specific contribution calculations and accumulated-row checks remain in the derived `Apply` methods.

Proposed public base declaration (new `include/cfd/boundary/boundary1d/boundary_condition1d.hpp`):

```cpp
class BoundaryCondition1D
{
public:
    virtual ~BoundaryCondition1D() = default;
    [[nodiscard]] const std::string& Name() const noexcept;
    [[nodiscard]] BoundarySide1D Side() const noexcept;
    virtual void Apply(const field::VolScalarField1D& gamma,
                       fv_matrix::FvMatrix1D& matrix) const = 0;

protected:
    BoundaryCondition1D(std::string name, BoundarySide1D side);
    [[nodiscard]] BoundaryAssemblyContext1D Prepare_Assembly(
        const field::VolScalarField1D& gamma,
        const fv_matrix::FvMatrix1D& matrix,
        std::string_view condition_name) const;

private:
    std::string name_;
    BoundarySide1D side_;
};
```

Make `DirichletBoundary1D`, `NeumannBoundary1D`, and `RobinBoundary1D` publicly derive from `BoundaryCondition1D`; mark each existing `Apply(...) const` with `override`. Preserve the existing concrete constructors and condition-specific accessors/setters, including `DirichletBoundary1D::Name()` / `Side()` compatibility through inherited methods. Keep Neumann gradient conversion and Robin/Dirichlet constructor validation behavior as currently implemented; in particular, do not silently make Dirichlet construction reject invalid side/value if its existing contract defers validation to `Apply`.

Each derived `Apply` follows this order: validate its own stored data; call `Prepare_Assembly`; compute its existing contribution; validate finite contribution and resulting matrix entries; only then mutate the matrix. For Dirichlet, add `gamma_owner * face_area / owner_face_distance` to `a_P(owner)` and that value times the prescribed boundary value to `b(owner)`. For Neumann, add `gamma_owner * normal_gradient * face_area` to `b(owner)` only. For Robin, add `transfer_coefficient * face_area` to `a_P(owner)` and that diagonal contribution times `ambient_value` to `b(owner)`. Preserve current outward-normal sign convention and Robin closure. Invalid inputs continue to throw `std::invalid_argument`; nonfinite contributions or accumulated entries continue to throw `std::overflow_error`. Preserve the all-validation-before-mutation guarantee.

If runtime polymorphic collections are not needed, keep the base abstract and stack-constructible concrete classes; do not introduce ownership containers, factories, templates, or a general boundary registry. Keep `BoundaryType` enum/accessors compatible; adding a base virtual `type()` is unnecessary for this refactor and would change the Dirichlet public API without helping the shared assembly path.

## 4. Risks & Backward Compatibility (Regression prevention)

- Adding a virtual function table changes object layout and ABI for these public classes; downstream binaries must be rebuilt. Source compatibility should be preserved by retaining the current constructors and condition-specific accessors/setters.
- Inherited `Name()` and `Side()` should remain callable on `DirichletBoundary1D`; consider thin forwarding methods only if compiler/API checks show an ambiguity or source compatibility issue. Preserve Neumann/Robin lowercase `name()` and `side()` spellings.
- Do not erase physical differences: Neumann only updates source; Dirichlet and Robin update diagonal and source. Preserve their sign conventions, exact geometry use, and current handling of zero diffusivity.
- Preserve each class's present constructor validation behavior, setter behavior, exception categories, and preflight-before-write semantics. A shared helper must not update the matrix or validate condition-specific parameters.
- `gamma` and matrix must continue to reference the identical mesh object. Equal geometry is insufficient.
- A base reference must have a virtual destructor because `Apply` is virtual. No owning raw pointers should be introduced.
- If adding a new implementation or public header, update the explicit CMake source list only when a new `.cpp` is introduced, and update root `FILEPATH.md` inventory as repository policy requires. Update `AGENTS.md` only if a future task expressly asks and repository constraints allow it; no such edit is part of this proposal.
- Existing boundary contract checks and the rod example are regression gates; retain their current direct construction and `Apply` usage.

## 5. Verification Criteria (Exact build command, driver execution, expected output)

After a future implementation task authorizes C++ changes, build the existing Debug configuration and all targets from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

Run the boundary contract driver (boundary checks are part of this executable):

```sh
./build/bin/test_uniform_mesh1d < /dev/null
```

Expected successful output is exactly:

```text
FvMesh1D contract checks passed
```

Run the existing integration example:

```sh
./build/bin/steady_heat_conduction_rod1d < /dev/null
```

For the unchanged five-cell rod case, verify the reported steady-state cell temperatures are approximately `380.000`, `340.000`, `300.000`, `260.000`, and `220.000` K, and that the assembled rows retain the expected tridiagonal boundary contributions: diagonal `15, 10, 10, 10, 15`, internal off-diagonals `-5`, and right-hand side `4000, 0, 0, 0, 2000`. Also retain/add checks for base-reference dispatch, all three concrete classes, invalid mesh identity/size/side/data, overflow rejection, and unchanged matrices after rejected applications. These are proposed verification criteria; no build or driver was run for this architecture-only task.
