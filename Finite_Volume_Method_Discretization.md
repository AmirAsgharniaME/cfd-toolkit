# Dimension-Independent Finite Volume Method (FVM) Discretization for Diffusion

This document establishes the mathematical derivation and systematic, dimension-independent algorithmic framework for discretizing the steady-state diffusion equation using the Finite Volume Method (FVM).

---

## 1. Governing Equation & Control Volume Integration

We consider the steady-state diffusion equation with a volumetric source term:
$$-\nabla \cdot (\Gamma \nabla \phi) = S$$

where:
- $\phi$: scalar transported variable (e.g., temperature $T$)
- $\Gamma$: diffusion coefficient (e.g., thermal conductivity $k$)
- $S$: volumetric source term

Integrating this equation over an arbitrary control volume $V_P$ centered at cell $P$:
$$\int_{V_P} -\nabla \cdot (\Gamma \nabla \phi) \, dV = \int_{V_P} S \, dV$$

---

## 2. Gauss Divergence Theorem Application

Defining the diffusive flux vector as:
$$\mathbf{F} = \Gamma \nabla \phi$$

We apply Gauss's Divergence Theorem:
$$\iiint_{V_P} (\nabla \cdot \mathbf{F}) \, dV = \oiint_{\partial V_P} (\mathbf{F} \cdot \hat{\mathbf{n}}) \, dA$$

Substituting back into the governing integral:
$$-\oint_{\partial V_P} (\Gamma \nabla \phi) \cdot \hat{\mathbf{n}} \, dA = \bar{S}_P V_P$$

where:
- $\partial V_P$: closed boundary enclosing control volume $V_P$
- $\hat{\mathbf{n}}$: outward-pointing unit normal vector on $\partial V_P$
- $\bar{S}_P$: volume-averaged source term, approximated as $\int_{V_P} S \, dV \approx \bar{S}_P V_P$

---

## 3. Discrete Face Representation & Face Normal Convention

Approximating the surface integral as a discrete summation over all bounding faces $f \in \mathcal{F}(P)$:
$$-\sum_{f \in \mathcal{F}(P)} \int_{A_f} (\Gamma \nabla \phi \cdot \hat{\mathbf{n}}_f) \, dA \approx -\sum_{f \in \mathcal{F}(P)} (\Gamma_f \nabla \phi|_f \cdot \hat{\mathbf{n}}_f) A_f = \bar{S}_P V_P$$

### Sign and Orientation Convention ($s_f$):
Let:
- $P$: owner cell index.
- $N(f)$: neighboring cell index sharing face $f$.
- $\mathbf{d}_{P,N(f)}$: displacement vector from cell center $P$ to cell center $N(f)$.
- $d_{P,N(f)} = \|\mathbf{d}_{P,N(f)}\|$: distance between cell centers.
- $s_f$: face orientation factor relative to the owner-to-neighbor vector:
  $$s_f = \begin{cases} +1 & \text{if outward normal } \hat{\mathbf{n}}_f \text{ points from } P \to N(f) \\ -1 & \text{if outward normal } \hat{\mathbf{n}}_f \text{ points from } N(f) \to P \end{cases}$$

### Two-Point Gradient Approximation:
Using a central difference approximation along the face normal:
$$\nabla \phi|_f \cdot \hat{\mathbf{n}}_f \approx s_f \left( \frac{\phi_{N(f)} - \phi_P}{d_{P,N(f)}} \right)$$

> **Note on Orthogonality:** This two-point stencil is exact for orthogonal grids where the line connecting cell centers $P$ and $N(f)$ is perpendicular to face $f$. On non-orthogonal unstructured meshes, a non-orthogonal correction term $(\mathbf{T} \cdot \nabla \phi)$ is added.

Substituting this approximation yields:
$$-\sum_{f \in \mathcal{F}(P)} \Gamma_f A_f s_f \left( \frac{\phi_{N(f)} - \phi_P}{d_{P,N(f)}} \right) = \bar{S}_P V_P$$

---

## 4. Dimension-Independent Matrix Assembly

Define the face diffusion conductance coefficient $D_f$:
$$D_f = \frac{\Gamma_f A_f}{d_{P,N(f)}}$$

Expanding the algebraic contribution for face $f$:
$$-D_f s_f (\phi_{N(f)} - \phi_P) = \underbrace{(D_f s_f) \phi_P}_{\text{Diagonal Contribution}} - \underbrace{(D_f s_f) \phi_{N(f)}}_{\text{Off-Diagonal Contribution}}$$

Summing over all faces yields the linear algebraic equation:
$$\left( \sum_{f \in \mathcal{F}(P)} D_f s_f \right) \phi_P - \sum_{f \in \mathcal{F}(P)} (D_f s_f) \phi_{N(f)} = \bar{S}_P V_P$$

### Assembly Rules:
For every internal face $f$ bounding cell $P$:
1. **Diagonal Entry ($A_{P,P}$):**
   $$A_{P,P} \mathrel{+}= D_f s_f$$
2. **Off-Diagonal Entry ($A_{P,N(f)}$):**
   $$A_{P,N(f)} \mathrel{+}= -D_f s_f$$
3. **Right-Hand Side Vector ($b_P$):**
   $$b_P = \bar{S}_P V_P$$

---

## 5. Verification with 1D East/West Formulation

To verify consistency with Patankar's classical 1D notation:
- **East Face ($e$):**
  - Neighbor: $E$ ($N(e) = E$)
  - Outward unit normal: $\hat{\mathbf{n}}_e = +\hat{\mathbf{i}}$ (points $P \to E$) $\implies s_e = +1$
  - Conductance: $D_e = \frac{\Gamma_e A_e}{d_{PE}}$
  - Contribution:
    $$A_{P,P} \mathrel{+}= D_e, \quad A_{P,E} \mathrel{+}= -D_e$$

- **West Face ($w$):**
  - Neighbor: $W$ ($N(w) = W$)
  - Outward unit normal: $\hat{\mathbf{n}}_w = -\hat{\mathbf{i}}$ (points opposite to $P \to W$ coordinate increase) $\implies s_w = -1$
  - Gradient approximation:
    $$\nabla \phi|_w \cdot \hat{\mathbf{n}}_w \approx (-1) \left( \frac{\phi_W - \phi_P}{d_{PW}} \right) = \frac{\phi_P - \phi_W}{d_{PW}}$$
  - Conductance: $D_w = \frac{\Gamma_w A_w}{d_{PW}}$
  - Contribution:
    $$A_{P,P} \mathrel{+}= D_w, \quad A_{P,W} \mathrel{+}= -D_w$$

Combining both faces:
$$(D_w + D_e) \phi_P - D_w \phi_W - D_e \phi_E = \bar{S}_P V_P$$
Matching Patankar's standard formula:
$$a_P \phi_P = a_W \phi_W + a_E \phi_E + b$$
where $a_W = D_w$, $a_E = D_e$, and $a_P = a_W + a_E$.

---

## 6. Solver Architectures: 1D vs Multi-Dimensional

- **1D Problems:** The global matrix $A$ forms a strictly tridiagonal matrix, solved directly with $\mathcal{O}(N)$ complexity using the **Tri-Diagonal Matrix Algorithm (TDMA / Thomas Algorithm)**.
- **2D/3D Unstructured Problems:** The face-loop approach populates a general sparse matrix (e.g., CSR or LDU format). Solved using iterative solvers (Gauss-Seidel, Preconditioned Conjugate Gradient (PCG), or BiCGStab).

---

## 7. C++ Implementation Blueprint
```cpp
#include <vector>
#include <cmath>

struct Face {
int neighbor_id;       // Index of neighbor cell N(f)
double area;           // Face surface area A_f
double gamma;          // Harmonic mean of diffusion coefficient Gamma_f
double distance;       // Distance between cell centers d_{P, N(f)}
double normal_sign;    // Orientation factor s_f (+1.0 or -1.0)
};

struct Cell {
int id;
double volume;
double source;
std::vector<Face> faces;
};

// Global Matrix Assembly Loop
void assemble_diffusion_matrix(
const std::vector<Cell>& cells,
std::vector<double>& diag,
std::vector<std::vector<std::pair<int, double>>>& off_diag,
std::vector<double>& b) 
{
const size_t n_cells = cells.size();
diag.assign(n_cells, 0.0);
off_diag.resize(n_cells);
b.assign(n_cells, 0.0);

for (size_t P = 0; P < n_cells; ++P) {
b[P] = cells[P].source * cells[P].volume;

for (const auto& face : cells[P].faces) {
int N = face.neighbor_id;
double D_f = (face.gamma * face.area) / face.distance;
double coeff = D_f * face.normal_sign;

// Diagonal entry update
diag[P] += coeff;

// Off-diagonal entry update (Sparse representation)
off_diag[P].emplace_back(N, -coeff);
}
}
}

