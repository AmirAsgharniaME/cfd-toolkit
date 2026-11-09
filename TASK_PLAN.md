# Target: Systematic Direction-Agnostic Laplacian Assembly

## Goal
Decouple face flux calculations from hardcoded directional grid slots (a_W, a_E) by introducing a storage-neutral matrix seam and refactoring Laplacian1D assembly.

## Roadmap & Checklist

### Phase 1: Storage-neutral Matrix API (`FvMatrix1D`)
- [ ] **Step 1.1: Declare Add_OffDiagonal API**
  - Target: `include/cfd/fv_matrix/fv_matrix1d.hpp`
  - Spec: Declare public method `void Add_OffDiagonal(std::size_t row, std::size_t col, double value);`
  - Details: Add Doxygen comments explaining that this routes general off-diagonal entries into the underlying 1D tridiagonal storage.

- [ ] **Step 1.2: Implement Add_OffDiagonal Routing Logic**
  - Target: `src/fv_matrix/fv_matrix1d.cpp`
  - Spec: In `FvMatrix1D::Add_OffDiagonal`, check column index:
    - If `col == row - 1`: add `value` to lower off-diagonal (West slot).
    - If `col == row + 1`: add `value` to upper off-diagonal (East slot).
    - If `col == row`: throw `std::invalid_argument` (diagonal must go to Add_Diagonal / a_P).
    - Otherwise: throw `std::out_of_range` (unsupported stencil index for 1D tridiagonal).

- [ ] **Step 1.3: Compile & Regression Sanity Check**
  - Target: Project build system
  - Spec: Build existing libraries and drivers using `make`. Verify zero compilation errors and zero regression.

### Phase 2: Systematic Laplacian Operator Refactoring
- [ ] **Step 2.1: Refactor Internal Face Assembly to Use Add_OffDiagonal**
  - Target: `src/finite_volume/fvm/fvm1d/laplacian1d.cpp`
  - Spec: Loop over internal faces. For each face with owner cell $P$ and neighbour cell $N$:
    - Compute face diffusion conductance $D_f = \Gamma_f \frac{A_f}{d_{PN}}$.
    - Use `Add_Diagonal(P, D_f)` and `Add_Diagonal(N, D_f)`.
    - Use `Add_OffDiagonal(P, N, -D_f)` and `Add_OffDiagonal(N, P, -D_f)`.
    - Completely remove direct calls to directional methods like `Add_a_E` and `Add_a_W`.

- [ ] **Step 2.2: Refactor Boundary Face Contributions**
  - Target: `src/finite_volume/fvm/fvm1d/laplacian1d.cpp`
  - Spec: Ensure boundary condition applications (Dirichlet/Neumann) do not rely on directional member functions, routing boundary diagonal/source additions systematically.

### Phase 3: Benchmark & Verification
- [ ] **Step 3.1: Recompile 1D Steady Heat Conduction Benchmark**
  - Target: `applications/` driver Makefile
  - Spec: Clean build of the 1D heat conduction rod executable.

- [ ] **Step 3.2: Verify Numerical Precision**
  - Target: Terminal output comparison
  - Spec: Run the benchmark driver and ensure the tridiagonal matrix values and final converged temperature profile match the pre-refactor reference output bit-for-bit.

---

## Execution Log (Appended after each step)
<!-- The agent appends a brief entry here after completing each step -->
