# Project State & Architecture Ledger (STATE.md)

This ledger describes the working tree inspected on 2026-11-11, including uncommitted changes. It records implemented behavior separately from intended behavior and placeholders.

## 1. Executive Summary & Design Philosophy

`cfd-toolkit` is early-stage C++ infrastructure for finite-volume research. Its implemented path is one-dimensional: an interval supplies bounds to a mesh; fields and an equation matrix reference that mesh; an implicit diffusion routine and a boundary object add coefficients; a tridiagonal solver solves the resulting algebraic system. Components are composed explicitly by client code rather than through a complete equation or simulation framework.

The mesh has owner/neighbour and incident-face queries, but the only implemented spatial assembly is a partially refactored 1D diffusion operator. In particular, its boundary-cell rows lack internal-face contributions. A runnable demonstration is therefore not evidence of a correct complete diffusion solution. Two- and three-dimensional domain values exist; multidimensional meshes, fields and operators do not.

## 2. Technical Stack & Toolchain

- CMake 3.20 or newer builds project version 0.1.0, with C++20 published as a target requirement. A C++20-capable compiler is required; CMake does not declare an exhaustive supported compiler list. The inspection build used GNU C++ 13.3.0.
- `cfd_toolkit` is a static library, with in-tree alias `cfd::toolkit`. Sources are explicitly listed. GNU and Clang library builds receive private `-Wall -Wextra -Wpedantic -Wconversion -Wshadow` flags.
- No external dependencies are declared. `BUILD_EXAMPLES` defaults to ON and adds six standalone executables under the build tree's `bin/`. There is no CTest registration or test framework in the root configuration.
- Installation exports the library and headers and a `cfd::toolkit` CMake package target, using `GNUInstallDirs` and same-major-version compatibility. Installed placeholder headers do not imply implemented APIs.

## 3. Architecture & Directory Topology

Public interfaces are under `include/cfd/`; corresponding implementations are under `src/`. Namespaces and directories organize the components as follows:

| Layer | Paths | Actual scope |
| --- | --- | --- |
| Geometry | `geometry/domain/` | Validated 1D interval and Cartesian 2D/3D domain values. |
| Mesh | `mesh/mesh1d/`, `mesh/mesh2d/`, `mesh/mesh3d/` | Concrete 1D uniform and graded mesh and face generator; multidimensional mesh files are reserved. |
| Values and fields | `vector/`, `field/field1d/` | One-component vector and cell/face scalar and vector fields; multidimensional field and vector capabilities are reserved. |
| Boundary | `boundary/boundary1d/` | Fixed-value diffusion assembly and stored normal-gradient data; patch files are placeholders. |
| Discretization | `schemes/`, `finite_volume/fvc/`, `finite_volume/fvm/` | Independent two-point gradient and interpolation functions; incomplete implicit 1D Laplacian; remaining operator files are placeholders. |
| Assembly and algebra | `fv_matrix/`, `linear_algebra/` | 1D mesh-bound tridiagonal system and TDMA; other matrix and solver files are placeholders. |
| Workflow and services | `solvers/`, `io/`, `time/`, `transport_model/` | Reserved workflow, output, time and transport modules. |
| Clients | `examples/` | Standalone contract checks and a rod demonstration. No separate `apps/` or `tests/` target layout is configured. |

`cmake/` contains the package configuration template. Domain values own coordinates independently of meshes; meshes own geometry; fields and FV matrices hold non-owning mesh references. Numerical operators write into the FV matrix, while TDMA accepts its underlying algebraic matrix.

## 4. Mathematical & FVM Discretization State

### Equation and face conventions

The intended assembled term is the cell-integrated negative diffusion operator

\[
\int_{V_P}-\nabla\!\cdot(\Gamma\nabla\phi)\,dV
= \sum_{f\in\partial V_P}(-\Gamma\nabla\phi\cdot n_{P,f}) A_f,
\qquad A\phi=b.
\]

Only its described subset is implemented. The mesh is a line with unit cross-sectional face area, so cell volume is cell length. Faces are numbered from 0 to `N_Cells()`, with the west and east boundary at the endpoints. An internal face's owner is its west cell and neighbour its east cell. The canonical owner-outward orientation is negative at the west boundary and positive at other faces. `s_f(p,f)` and `e_PN(p,f)` each return -1 for an incident west face and +1 for an incident east face; these are scalar 1D signs, not multidimensional normal vectors.

For an internal face between west owner P and east neighbour N, with positive center-to-face distances \(d_P,d_N\), the code uses

\[
\Gamma_f=\frac{d_P+d_N}{d_P/\Gamma_P+d_N/\Gamma_N},\qquad
D_f=\frac{\Gamma_f A_f}{d_P+d_N},\qquad
q_{P,f}=D_f(\phi_P-\phi_N).
\]

If either cell diffusivity is zero, it sets \(\Gamma_f=0\); otherwise the harmonic interpolation requires positive finite values. `fvm::Laplacian` validates finite nonnegative cell diffusivities and mesh identity, then accumulates \(+D_f\) on the visited row's diagonal and \(-D_f\) in the adjacent column. It uses `Add_a_P(p, D_f)` and `Add_OffDiagonal(p, N(p, f), -D_f)` without directional off-diagonal calls. The orientation factor `s_f(p,f) * e_PN(p,f)` is +1 for each incident face. It currently loops over cells `1 <= p < N_Cells()-1` and their two faces: both boundary rows are omitted, including all rows on a two-cell mesh. Thus the actual matrix is generally nonsymmetric and does not represent the complete internal-face balance. It adds no boundary-face term. A non-finite computed conductance throws; accumulated internal matrix entries are not separately checked for overflow.

For a fixed value \(\phi_b\), `DirichletBoundary1D::Apply` separately adds \(D_b=\Gamma_P/d_{Pb}\) to the boundary diagonal and \(D_b\phi_b\) to the RHS, for unit area. It validates mesh identity, size, side, value, diffusivity, distance and finite resulting entries. `NeumannBoundary1D` stores \(g_n=\partial\phi/\partial n\), with coordinate derivative \(-g_n\) at Start and \(+g_n\) at End; the associated outward diffusive flux would be \(-\Gamma g_n\). No Neumann matrix contribution is implemented. `BoundaryType::Robin` exists as an enum value, without a mixed-boundary implementation.

There is no implemented convection/divergence, transient derivative, source assembly or full governing-equation solver. The rod example attempts steady source-free conduction, but the present assembled system is incomplete.

### Independent numerical schemes

For finite field values and finite positive distances, `schemes::gradient::central_difference(phi_p, phi_n, distance_p, distance_n)` returns \((\phi_N-\phi_P)/(d_P+d_N)\). It is a center-to-center secant; on a nonuniform mesh the shared face need not be the center midpoint. `schemes::interpolation::arithmetic_mean` returns \((d_N\phi_P+d_P\phi_N)/(d_P+d_N)\), and `harmonic_mean` returns the resistance-weighted coefficient above. The latter requires strictly positive diffusivities. These routines validate inputs, use `long double` intermediates and report unrepresentable results via exceptions. The Laplacian calls harmonic interpolation but computes conductance directly; it does not call the gradient function. Upwind is a placeholder.

## 5. Data Structures & Assembly Seam

The following signatures use their enclosing `cfd` namespace and capture the principal public entry points; the referenced headers are authoritative for overloads and exception details.

### Domains and mesh

- `geometry::Domain1D(double length)`, `Domain1D(double start_coord, double end_coord)` and `static Domain1D from_start_and_length(double start_coord, double domain_length)` store two authoritative `double` endpoints. `Start_Coord()`, `End_Coord()`, `Length()` and `IsContain(double)` are const queries; `Set_Bounds(double,double)`, `Set_Start_Coord(double)`, `set_End_Coord(double)` and `Set_Length(double)` validate updates. Bounds and represented positive extent must be finite. Containment is closed and equality exact. `Domain2D` and `Domain3D` store independent axis intervals, expose `X_Domain()` / `Y_Domain()` / `Z_Domain()` snapshots, `IsContain(...)`, axis setters and `Set_Domains(...)`.
- `mesh::FvMesh1D(const geometry::Domain1D&, std::size_t)` constructs uniform geometry; the overload with `(const geometry::Domain1D&, std::size_t, GradingType, double)` constructs generated geometry. At least one cell is required. It snapshots endpoints and owns three `std::vector<double>` arrays: faces, centers and cell volumes. Faces are strictly increasing with representable interior centers; endpoints are preserved. Copy/move construction is supported; assignment is deleted. A moved-from mesh is empty and extent queries throw.
- Counts and extent are `N_Cells()`, `N_Faces()`, `Start_Coord()`, `End_Coord()`, `Length()`. Read-only spans are `Face_Coords()`, `Cell_Center_Coords()`, `Cell_Volumes()`; checked scalar access is `Face_Coord(FaceId)`, `Cell_Center_Coord(CellId)`, `Cell_Volume(CellId)`. `CellId` and `FaceId` alias `std::size_t`.
- Connectivity is `Owner(FaceId) -> CellId`, `Neighbour(FaceId) -> std::optional<CellId>`, `N(CellId, FaceId) -> CellId`, and `s_f(CellId, FaceId)` / `e_PN(CellId, FaceId) -> double`. Boundary faces have no neighbour. `Face_Area(FaceId)` returns 1. Geometry helpers include `wFace_Coord`, `eFace_Coord`, `wFaceDx`, `eFaceDx`, `wCellDx`, `eCellDx`, `Dx_Owner`, `Dx_Neighbour` and `d_PN`, each taking the relevant cell or internal-face index. Indexed and incidence checks reject invalid requests. `Print_Details(std::ostream&) const` prints geometry.
- `mesh::grading::Make_Faces(const geometry::Domain1D&, std::size_t, GradingType, double) -> std::vector<double>` supports `Uniform`, `Forward`, `Backward`, `BothEnds`, `Center`. `Convert_R_To_r(std::size_t, GradingType, double) -> double` converts total width ratio to exponent-step ratio. Both validate inputs and representability.

### Fields and boundary objects

`field::Field1D<ValueType, LocationTag>(std::string, const mesh::FvMesh1D&, const ValueType& initial_val = ValueType{})` owns a name and contiguous `std::vector<ValueType>` and references the mesh. The supported value types are `double` and `vector::Vector1D` (one `double x`); tags are `CellLocation` and `FaceLocation`. The aliases are `VolScalarField1D`, `SurfaceScalarField1D`, `VolVectorField1D` and `SurfaceVectorField1D`. The mesh must outlive the field and remain unmoved; non-const mesh rvalue construction is deleted.

`Name()`, `Mesh()`, `size()`, face-only `N_Faces()`, `operator[](std::size_t)`, checked `at(std::size_t)`, mutable/const `values()`, `span()` and legacy `Values()`, and `Fill(const ValueType&)` expose storage. Copy/move construction is available; assignment and field arithmetic require the same mesh object and matching size. Both value types support field addition/subtraction and scalar scaling; scalar fields also support elementwise field multiplication/division. Field division rejects zero. Binary operators are in `field1d_operators.hpp`.

`boundary::DirichletBoundary1D(std::string, BoundarySide1D, double)` exposes `Name()`, `side()`, `Value()` and `Apply(const field::VolScalarField1D&, fv_matrix::FvMatrix1D&) const`. `boundary::NeumannBoundary1D(std::string, BoundarySide1D, double)` exposes `name()`, `side()`, `type()`, `normal_gradient()`, `outward_normal()`, `coordinate_gradient()` and `set_normal_gradient(double)`. They are separate concrete types, not a shared polymorphic patch interface.

### Matrix, operator and solver

`fv_matrix::FvMatrix1D(const mesh::FvMesh1D&)` stores a mesh reference and a `linear_algebra::TridiagonalMatrix` sized to its cells. The mesh must outlive it and remain unmoved. Copy/move construction is available; assignment is deleted. `Mesh() const`, `size() const` and mutable/const `Matrix()` provide access. `a_W(std::size_t)`, `a_P(std::size_t)`, `a_E(std::size_t)`, `b(std::size_t)` read row coefficients; matching `Set_a_W`, `Set_a_P`, `Set_a_E`, `Set_b` and `Add_a_W`, `Add_a_P`, `Add_a_E`, `Add_b` accept `(std::size_t, double)`. `Add_OffDiagonal(std::size_t row, std::size_t col, double value)` provides a row/column assembly seam over tridiagonal storage: an adjacent column below the row maps to lower storage and one above maps to upper storage. Equal indices throw `std::invalid_argument` before bounds checks; out-of-range and nonadjacent indices throw `std::out_of_range`. `Reset() noexcept` zeros storage.

`linear_algebra::TridiagonalMatrix(std::size_t)` owns four row-indexed `std::vector<double>` arrays of length N: diagonal, lower, upper and RHS. `Diag`, `Lower`, `Upper`, `Source` each have indexed read and mutable/const vector overloads, with indexed `Set_*` and `Add_*`; `size()` and `Reset()` complete the storage API. Boundary lower/upper slots exist even though they do not correspond to neighboring unknowns. Mutable vector access can change sizes, which TDMA checks.

`void fvm::Laplacian(const field::VolScalarField1D& gamma, fv_matrix::FvMatrix1D& matrix)` and Dirichlet `Apply` accumulate on the same mesh-bound matrix; neither resets it. The caller controls assembly order and passes `matrix.Matrix()` to either `static void linear_algebra::TDMASolver::solve(TridiagonalMatrix&, std::vector<double>&)` or `static std::vector<double> solve(TridiagonalMatrix)`. The first eliminates in place; the second solves a copy when passed an lvalue. TDMA rejects empty or inconsistent storage and pivots with absolute value below `1.0e-14`; it has no pivoting or preconditioner.

## 6. Verification Status & Verified Baselines

The execution log in `TASK_PLAN.md` records successful library builds for the matrix API, an all-target `make` and six executable exits of 0 at Step 1.3, and all-target builds after Steps 2.1 and 2.2. It also records a successful rod-target rebuild and a comparison against a freshly built pre-refactor `HEAD`: stdout was byte-identical at 1,148 bytes and both stderr streams were empty. This is a parity observation for those runs, not evidence that the current diffusion operator is complete. The planned `applications/` driver Makefile was not present; the CMake target was used.

On 2026-11-11, `cmake --build build --parallel` completed all configured targets in the current working tree. The updated uniform-mesh example uses `s_f` and `e_PN` in place of the removed normal accessors, so the earlier compilation failure is resolved. However, `./build/bin/test_uniform_mesh1d < /dev/null` exited 1 with `Internal diffusion stencil is incorrect`. The log's Step 2.1 description of symmetric contributions to both rows does not match the current interior-row loop, and its earlier passing check cannot establish the present numerical result.

| Executable | Observed result and scope |
| --- | --- |
| `test_domain1d` | Earlier inspection: exit 0; interval construction, updates, invalid inputs, containment and floating-point contracts. |
| `test_non_uniform_mesh1d` | Earlier inspection: exit 0; 1D mesh and generated-geometry contracts. |
| `test_grading1d` | Earlier inspection: exit 0; distribution, invalid-input, symmetry and ratio checks. Reported maximum symmetry distance error `8.882e-16` (tolerance `8.000e-12`) and total-ratio error `2.132e-14` (tolerance `1.600e-10`). |
| `test_field1d` | Earlier inspection: exit 0; scalar/vector allocation, access, mesh identity, assignment and arithmetic contracts. |
| `test_uniform_mesh1d` | Current run: exit 1 on the internal diffusion stencil assertion; later checks in that run are not established. |
| `steady_heat_conduction_rod1d` | Current run: exit 0; demonstration without a numerical assertion. Five cells on [0,1], \(\Gamma=1\), boundary values 400 and 200. It printed `[400, 350, 300, 250, 200]`; the complete constant-coefficient linear solution at the five cell centers is `[380, 340, 300, 260, 220]`. The omitted boundary-row internal couplings explain the difference. |

No dedicated executable benchmark for 2D/3D domains or independently verified convergence study is configured. Passing the above standalone checks does not validate the complete diffusion operator.

## 7. Known Technical Debt & Immediate Roadmap

**Implemented foundations:** validated interval/domain values; generated 1D geometry and connectivity; mesh-bound scalar/vector fields; interpolation and two-point gradient functions; tridiagonal storage and TDMA; fixed-value boundary contribution. Their contracts and limits are described above.

**Unfinished or absent:** the Laplacian omits boundary-cell internal couplings and its header's internal-face description is broader than the implementation; the uniform-mesh check now builds but fails its internal diffusion stencil assertion. Neumann assembly, mixed boundaries and patch infrastructure are absent. Multidimensional meshes/fields/operators, explicit FVC operators, convection, transient and source terms, upwind, equation workflows, sparse and iterative solvers, transport, simulation time and output writers are reserved or placeholders. The rod demonstration has no solution assertion and currently prints a nonphysical discrete baseline for its stated boundary problem.

**Candidate next milestones:**

1. Complete internal-face assembly for both incident rows, including two-cell meshes, using the row/column matrix seam.
2. Verify coefficients, conservation, symmetry, single/two-cell cases, nonuniform and discontinuous/zero diffusivity, boundary contributions and the linear rod solution; then run the full default build and checks.
3. Implement and verify Neumann and, if needed, mixed boundary assembly before relying on those boundary types in equation workflows.
4. Develop remaining operators and higher-dimensional geometry, storage and solver workflows only alongside explicit discretization and verification contracts.
