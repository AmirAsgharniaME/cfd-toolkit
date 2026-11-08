# Target: Systematic Direction-Agnostic Laplacian Assembly

## Goal
Decouple face flux calculations from hardcoded directional grid slots (a_W, a_E) by introducing a storage-neutral matrix seam and refactoring Laplacian1D assembly.

## Roadmap & Checklist
- [ ] Phase 1: Storage-neutral Matrix API (`FvMatrix1D`)
  - [ ] Step 1.1: Declare `Add_OffDiagonal(std::size_t row, std::size_t col, double value)` in `include/cfd/fv_matrix/fv_matrix1d.hpp`.
  - [ ] Step 1.2: Implement row/col routing logic in `src/fv_matrix/fv_matrix1d.cpp`.
  - [ ] Step 1.3: Compile and verify existing driver regression.

- [ ] Phase 2: Systematic Laplacian Operator Refactoring
  - [ ] Step 2.1: Rewrite internal face loop in `src/finite_volume/fvm/fvm1d/laplacian1d.cpp` using face normals and owner/neighbour outward signs.
  - [ ] Step 2.2: Route face fluxes via `Add_OffDiagonal` and remove direct East/West coefficient calls.

- [ ] Phase 3: Benchmark & Verification
  - [ ] Step 3.1: Recompile 1D steady heat conduction rod driver.
  - [ ] Step 3.2: Verify temperature profile and matrix values against reference output.

---

## Execution Log (Appended after each step)
<!-- The agent appends a brief entry here after completing each step -->
